#include "address_manager.h"
#include "avltree.h"
#include "socket.h"
#include "worker.h"
#include "log.h"

struct avl_tree_t *address_map;

void address_mngr_init()
{
    avl_tree_construct(address_map);
}

static int create_unique_port_from_ip(const char *ip)
{
    return 0; /* idk lol */
}

void address_mngr_add_address()
{
    const char *fix_compilation_ip, *fix_compilation_port;
    int port, fd;
    struct soap_instance *instance;

    port = create_unique_port_from_ip(fix_compilation_ip);

    fd = socket_create_udp(port);

    instance = soap_instance_allocate(fix_compilation_ip, fix_compilation_port);

    worker_add_udp_fd(fd);

    log("add address map fd %d -> port %d -> ip %s", fd, port, fix_compilation_ip);
    avl_tree_insert(address_map, fd, instance);
}

struct soap_instance* address_mngr_get_soap_instance_from_fd(int fd)
{
    return avl_tree_find(address_map, fd);
}

static void node_destruction_cb(struct avl_node_t *node)
{
    close(node->key);
    soap_instance_deallocate(node->data);
}

void address_mngr_destruct()
{
    avl_tree_destruct(address_map, node_destruction_cb);
}

