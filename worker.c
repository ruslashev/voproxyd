#define _GNU_SOURCE

#include "address_manager.h"
#include "buffer.h"
#include "config.h"
#include "epoll.h"
#include "errors.h"
#include "log.h"
#include "socket.h"
#include "visca.h"
#include "sony_visca.h"
#include "worker.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define VOPROXYD_STRING_BUFFERS_EXTEND_LENGTH 4096
#define VOPROXYD_MAX_EPOLL_EVENTS 128
#define VOPROXYD_MAX_RX_MESSAGE_LENGTH 4096
#define VOPROXYD_SHELL_PATH "/bin/sh"
#define VOPROXYD_PIPE_READ_BUFFER_LENGTH 8192
#define VOPROXYD_STRING_BUFFERS_INITIAL_LENGTH 1024

int g_current_event_fd;

static struct ap_state state;
static int signal_fd, inotify_fd, timer_fd;
static int can_do_discovery = 1;

static void string_ensure_fits_substring(char **hay, size_t *hay_buf_length, size_t needle_length)
{
    size_t hay_length = strlen(*hay);
    char *hay_resized;

    while (hay_length + needle_length + 1 > *hay_buf_length) {
        *hay_buf_length += VOPROXYD_STRING_BUFFERS_EXTEND_LENGTH;

        hay_resized = realloc(*hay, *hay_buf_length);
        if (hay_resized == NULL) {
            free(*hay);
            die(ERR_NOMEM, "realloc of size %zd failed", *hay_buf_length);
        }

        *hay = hay_resized;
    }
}

static void string_concat(char **string, size_t *length, const char *buffer)
{
    size_t old_string_length = strlen(*string), buffer_length = strlen(buffer);

    string_ensure_fits_substring(string, length, buffer_length);

    memcpy(*string + old_string_length, buffer, buffer_length);

    (*string)[old_string_length + buffer_length] = '\0';
}

static void print_mem_usage()
{
    FILE* file;
    char buffer[1024] = "";
    int curr_real, curr_virt;

    file = fopen("/proc/self/status", "r");
    if (!file)
        die(ERR_OPEN, "failed to open /proc/self/status");

    while (fscanf(file, " %1023s", buffer) == 1) {
        if (strcmp(buffer, "VmRSS:") == 0)
            fscanf(file, " %d", &curr_real);
        if (strcmp(buffer, "VmSize:") == 0)
            fscanf(file, " %d", &curr_virt);
    }

    fclose(file);

    log("current real: %d", curr_real);
    log("current virt: %d", curr_virt);
}

static int add_signal_handler(struct ap_state *state)
{
    sigset_t mask;
    int signal_fd;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        die(ERR_SIGNALS, "sigprocmask() failed: %s", strerror(errno));

    signal_fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (signal_fd == -1)
        die(ERR_SIGNALS, "signalfd() failed: %s", strerror(errno));

    epoll_add_fd(state, signal_fd, FDT_SIGNAL, 1);

    return signal_fd;
}

static void watch_config_file(int inotify_fd)
{
    int watch_descriptor;
    char *config_file = config_get_config_filename();

    watch_descriptor = inotify_add_watch(inotify_fd, config_file, IN_ALL_EVENTS);
    if (watch_descriptor < 0)
        die(ERR_INOTIFY, "failed to watch file \"%s\": %s", config_file, strerror(errno));

    free(config_file);
}

static int add_inotify(struct ap_state *state)
{
    int inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (inotify_fd < 0)
        die(ERR_INOTIFY, "failed to init inotify instance: %s", strerror(errno));

    epoll_add_fd(state, inotify_fd, FDT_INOTIFY, 1);

    watch_config_file(inotify_fd);

    return inotify_fd;
}

static int add_timer(struct ap_state *state)
{
    int timer_fd;
    struct itimerspec new, old;
    struct timespec ts;

    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd < 0)
        die(ERR_TIMER, "failed to create timer fd: %s", strerror(errno));

    memset(&new, 0, sizeof(struct itimerspec));
    memset(&old, 0, sizeof(struct itimerspec));

    ts.tv_sec = 30;
    ts.tv_nsec = 0;

    new.it_value = ts;
    new.it_interval = ts;

    if (timerfd_settime(timer_fd, 0, &new, &old) < 0)
        die(ERR_TIMER, "failed to set timer interval");

    epoll_add_fd(state, timer_fd, FDT_TIMER, 1);

    return timer_fd;
}

static int handle_tcp_message(struct ap_state *state, const uint8_t *message, ssize_t length,
        int *close)
{
    (void)state;
    (void)message;
    (void)close;

    log("handle tcp msg of len %zu\n", length);

    return 0;
}

static int handle_udp_message(const struct ap_state *state, uint8_t *message, ssize_t length)
{
    buffer_t *message_buf = cons_buffer(length);

    free(message_buf->data);

    message_buf->data = message;

    visca_handle_message(message_buf, state->current_event);
    /* sony_visca_handle_message(message_buf, state->current_event); */

    free(message_buf);

    return 0;
}

static int epoll_handle_read_queue_udp(struct ap_state *state)
{
    ssize_t message_length;
    uint8_t rx_message[VOPROXYD_MAX_RX_MESSAGE_LENGTH];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int close = 0;

    message_length = recvfrom(state->current, rx_message, sizeof(rx_message), MSG_DONTWAIT,
            (struct sockaddr*)&addr, &addr_len);

    state->current_event->addr = (struct sockaddr*)&addr;
    state->current_event->addr_len = addr_len;

    if (!(message_length == -1 && errno == EAGAIN))
        log("recvfrom fd = %d addr = %s:%d message_length = %zd", state->current,
                inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), message_length);

    if (message_length == 0) {
        log("close connection on socket fd = %d", state->current);
        epoll_close_fd(state, state->current);
        return 0;
    }

    if (message_length == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            errno = 0;
            return 0;
        }

        epoll_close_fd(state, state->current);
        die(ERR_READ, "error reading on socket fd = %d: %s", state->current, strerror(errno));
    }

    handle_udp_message(state, rx_message, message_length);

    if (close || state->close_after_read) {
        state->close_after_read = 0;
        epoll_close_fd(state, state->current);
        return 0;
    }

    return 1;
}

static void epoll_handle_signal(int signal_fd, int *running)
{
    struct signalfd_siginfo signal_info;
    int signal;

    if (read(signal_fd, &signal_info, sizeof(signal_info)) != sizeof(signal_info))
        die(ERR_READ, "failed to read signal fd: %s", strerror(errno));

    signal = signal_info.ssi_signo;

    log("received signal %d (%s)", signal, strsignal(signal));

    *running = 0;
}

static void display_event_info(const struct inotify_event *i)
{
    log("wd = %d", i->wd);

    log("mask = ");
    if (i->mask & IN_ACCESS)        log("IN_ACCESS ");
    if (i->mask & IN_ATTRIB)        log("IN_ATTRIB ");
    if (i->mask & IN_CLOSE_NOWRITE) log("IN_CLOSE_NOWRITE ");
    if (i->mask & IN_CLOSE_WRITE)   log("IN_CLOSE_WRITE ");
    if (i->mask & IN_CREATE)        log("IN_CREATE ");
    if (i->mask & IN_DELETE)        log("IN_DELETE ");
    if (i->mask & IN_DELETE_SELF)   log("IN_DELETE_SELF ");
    if (i->mask & IN_IGNORED)       log("IN_IGNORED ");
    if (i->mask & IN_ISDIR)         log("IN_ISDIR ");
    if (i->mask & IN_MODIFY)        log("IN_MODIFY ");
    if (i->mask & IN_MOVE_SELF)     log("IN_MOVE_SELF ");
    if (i->mask & IN_MOVED_FROM)    log("IN_MOVED_FROM ");
    if (i->mask & IN_MOVED_TO)      log("IN_MOVED_TO ");
    if (i->mask & IN_OPEN)          log("IN_OPEN ");
    if (i->mask & IN_Q_OVERFLOW)    log("IN_Q_OVERFLOW ");
    if (i->mask & IN_UNMOUNT)       log("IN_UNMOUNT ");
}

static void epoll_handle_inotify(int inotify_fd)
{
    char read_buffer[10 * (sizeof(struct inotify_event) + NAME_MAX + 1)]
        __attribute__ ((aligned(__alignof__(struct inotify_event))));
    ssize_t len;
    const struct inotify_event *event;

    len = read(inotify_fd, read_buffer, sizeof(read_buffer));
    if (len == -1 && errno != EAGAIN)
        die(ERR_READ, "failed to read from inotify fd: %s", strerror(errno));

    if (len <= 0)
        return;

    for (char *ptr = read_buffer; ptr < read_buffer + len;
            ptr += sizeof(struct inotify_event) + event->len) {
        event = (const struct inotify_event*)ptr;

        display_event_info(event);

        watch_config_file(inotify_fd);
    }
}

static void command_output_ready(const char *output, int exit_code)
{
    char *output_copy, *it;

    if (exit_code != 0 || strstr(output, "not found") != NULL
            || strstr(output, "ERROR") != NULL) {
        log("error: failed to do discovery using external tools.\n"
                "to use it make sure to install python package WSDiscovery:\n"
                "\n"
                "    pip3 install --user WSDiscovery\n"
                "\n"
                "command output: '%s'", output);
        can_do_discovery = 0;
        return;
    }

    log("command output: '%s' exit code = %d", output, exit_code);
    log(" ");

    if (strlen(output) > 4096)
        die(ERR_PIPE, "worker: error: command output is too long");

    output_copy = malloc(strlen(output) + 1);
    if (!output_copy)
        die(ERR_NOMEM, "failed to malloc(%zd)", strlen(output));

    strncpy(output_copy, output, strlen(output));

    output_copy[strlen(output)] = '\0';

    it = strtok(output_copy, "\n");
    while (it != NULL) {
        log("extracted address: '%s'", it);

        address_mngr_add_address(it);

        it = strtok(NULL, "\n");
        log(" ");
    }

    free(output_copy);
}

static int get_waitpid_exit_code(int waitpid_status)
{
    if (WIFEXITED(waitpid_status))
        return WEXITSTATUS(waitpid_status);

    if (WIFSIGNALED(waitpid_status))
        return WTERMSIG(waitpid_status);

    if (WIFSTOPPED(waitpid_status))
        return WSTOPSIG(waitpid_status);

    return -1;
}

static void run_command(struct ap_state *state, const char *command)
{
    int pipe_fds[2];
    pid_t pid;

    if (pipe2(pipe_fds, O_CLOEXEC | O_NONBLOCK) == -1)
        die(ERR_PIPE, "pipe2() failed: %s", strerror(errno));

    pid = fork();

    if (pid == -1)
        die(ERR_FORK, "Failed to fork process: %s", strerror(errno));

    if (pid == 0) {
        if (dup2(pipe_fds[1], STDOUT_FILENO) == -1)
            die(ERR_DUP, "failed to dup2 stdout: %s", strerror(errno));

        if (dup2(pipe_fds[1], STDERR_FILENO) == -1)
            die(ERR_DUP, "failed to dup2 stderr: %s", strerror(errno));

        execl(VOPROXYD_SHELL_PATH, "sh", "-c", command, (char*)NULL);

        die(ERR_EXEC, "execl() failed: %s", strerror(errno));
    }

    close(pipe_fds[1]);

    epoll_add_fd(state, pipe_fds[0], FDT_PIPE, 1);

    /* dumb */
    for (struct tracking_ll_t *it = state->tracked_events; it != NULL; it = it->next)
        if (it->event->type == FDT_PIPE && it->event->fd == pipe_fds[0]) {
            it->event->child_pid = pid;

            it->event->command_output_len = VOPROXYD_STRING_BUFFERS_INITIAL_LENGTH;

            it->event->command_output = malloc(it->event->command_output_len * sizeof(char));
            if (!it->event->command_output)
                die(ERR_NOMEM, "failed to malloc(%zd)", it->event->command_output_len);

            it->event->command_output[0] = '\0';
        }
}

static void epoll_handle_pipe(struct ap_state *state, int *continue_reading)
{
    char read_buffer[VOPROXYD_PIPE_READ_BUFFER_LENGTH];
    ssize_t bytes_read;

    bytes_read = read(state->current, read_buffer, VOPROXYD_PIPE_READ_BUFFER_LENGTH);

    *continue_reading = 0;

    if (bytes_read == 0)
        return;

    if (bytes_read > 0) {
        read_buffer[bytes_read] = '\0';
        string_concat(&state->current_event->command_output,
                &state->current_event->command_output_len, read_buffer);
        *continue_reading = 1;
        return;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK)
        return;

    die(ERR_READ, "error reading: %s", strerror(errno));
}

static void free_command(struct ap_state *state)
{
    free(state->current_event->command_output);

    epoll_close_fd(state, state->current);

    for (struct tracking_ll_t *it = state->tracked_events; it != NULL; it = it->next)
        if (it->event->type == FDT_PIPE && it->event->fd == state->current) {
            ll_delete_node(&state->tracked_events, it);
            break;
        }
}

static void epoll_handle_pipe_queue(struct ap_state *state)
{
    int continue_reading = 1;
    int proc_status;

    while (continue_reading)
        epoll_handle_pipe(state, &continue_reading);

    if (state->close_after_read) {
        state->close_after_read = 0;

        if (waitpid(state->current_event->child_pid, &proc_status, WUNTRACED) == -1)
            die(ERR_WAITPID, "waitpid() of pid = %d failed: %s", state->current_event->child_pid,
                    strerror(errno));

        command_output_ready(state->current_event->command_output, get_waitpid_exit_code(proc_status));

        free_command(state);
    }

    log("read pipe buffer");
}

static void epoll_handle_timer(struct ap_state *state)
{
    uint64_t value;

    log("timer tick");
    worker_do_external_discovery();
    print_mem_usage();

    read(state->current, &value, 8);
}

static void epoll_handle_hangup(struct ap_state *state)
{
    int proc_status;
    pid_t pid;

    log("hangup on fd = %d", state->current);

    if (state->current_event->type != FDT_PIPE) {
        log("closing on hangup non-pipe fd = %d of type %d", state->current,
                state->current_event->type);
        epoll_close_fd(state, state->current);
        return;
    }

    pid = waitpid(state->current_event->child_pid, &proc_status, WUNTRACED);
    if (pid == -1)
        die(ERR_WAITPID, "waitpid() of pid = %d failed: %s", state->current_event->child_pid,
                strerror(errno));

    command_output_ready(state->current_event->command_output, get_waitpid_exit_code(proc_status));

    free_command(state);
}

static void epoll_handle_event(struct ap_state *state, const struct epoll_event *event, int *running)
{
    int continue_reading = 1, client_fd;

    if (state->current_event->type == FDT_INOTIFY)
        return;

    log(" ");
    log("new event on fd = %d", state->current);

    g_current_event_fd = state->current;

    epoll_handle_event_errors(state, event);

    if ((event->events & (unsigned)EPOLLHUP) && !(event->events & (unsigned)EPOLLIN)) {
        epoll_handle_hangup(state);
        return;
    }

    state->close_after_read = !!((event->events & (unsigned)EPOLLHUP) |
            (event->events & (unsigned)EPOLLRDHUP));

    switch (state->current_event->type) {
        case FDT_TCP_LISTEN:
            client_fd = socket_accept(state->current);
            epoll_add_fd(state, client_fd, FDT_TCP, 1);
            break;
        case FDT_UDP:
            while (continue_reading) {
                continue_reading = epoll_handle_read_queue_udp(state);
            }
            break;
        case FDT_SIGNAL:
            epoll_handle_signal(state->current, running);
            break;
        case FDT_INOTIFY:
            epoll_handle_inotify(state->current);
            break;
        case FDT_PIPE:
            epoll_handle_pipe_queue(state);
            break;
        case FDT_TIMER:
            epoll_handle_timer(state);
            break;
        default:
            die(ERR_EPOLL_EVENT, "epoll_handle_event: unknown event type %d",
                    state->current_event->type);
    }
}

static void main_loop(struct ap_state *state)
{
    struct epoll_event events[VOPROXYD_MAX_EPOLL_EVENTS];
    int num_events, ev_idx, running = 1;

    while (running) {
        num_events = epoll_wait(state->epoll_fd, events, VOPROXYD_MAX_EPOLL_EVENTS, -1);

        if (num_events == -1 && errno != EINTR) {
            die(ERR_EPOLL_WAIT, "epoll_wait() failed: %s", strerror(errno));
        }

        for (ev_idx = 0; ev_idx < num_events; ++ev_idx) {
            state->close_after_read = 0;
            state->current_event = events[ev_idx].data.ptr;
            state->current = state->current_event->fd;
            epoll_handle_event(state, &events[ev_idx], &running);
        }
    }
}

void worker_init()
{
    memset(&state, 0, sizeof(struct ap_state));

    g_current_event_fd = 0;

    state.epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (state.epoll_fd == -1)
        die(ERR_EPOLL_CREATE, "epoll_create1() failed: %s", strerror(errno));

    signal_fd = add_signal_handler(&state);

    inotify_fd = add_inotify(&state);

    timer_fd = add_timer(&state);

    log("epoll fd = %d sig fd = %d infy fd = %d", state.epoll_fd, signal_fd, inotify_fd);
    log(" ");
}

void worker_start()
{
    log("start main loop");
    main_loop(&state);

    ll_free_list(&state.tracked_events);

    close(timer_fd);
    close(inotify_fd);
    close(signal_fd);
    close(state.epoll_fd);
}

void worker_add_udp_fd(int fd)
{
    epoll_add_fd(&state, fd, FDT_UDP, 1);
}

void worker_do_external_discovery()
{
    if (!can_do_discovery)
        return;

    log("starting external discovery...");
    run_command(&state, "discover | grep 192 | cut -d' ' -f 3");
}

