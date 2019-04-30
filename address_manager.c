#include "address_manager.h"
#include "avltree.h"
#include "socket.h"
#include "worker.h"
#include "log.h"

struct avl_tree_t address_map;

void address_mngr_init()
{
    avl_tree_construct(&address_map);
}

static int create_unique_port_from_ip(const char *address)
{
    return 0; /* idk lol */
}

void address_mngr_add_address_by_port(int port, const char *address)
{
    int fd;
    struct soap_instance *instance;

    fd = socket_create_udp(port);

    instance = soap_instance_allocate(address);
    if (instance == NULL)
        return;

    worker_add_udp_fd(fd);

    log("add address map fd %d -> port %d -> address %s", fd, port, address);
    avl_tree_insert(&address_map, fd, instance);

    soap_instance_print_info(instance);

    log(" ");
}

void address_mngr_add_address(const char *address)
{
    int port = create_unique_port_from_ip(address);

    address_mngr_add_address_by_port(port, address);
}

struct soap_instance* address_mngr_get_soap_instance_from_fd(int fd)
{
    struct soap_instance* instance = avl_tree_find(&address_map, fd);
    if (instance == NULL)
        die(ERR_SOCKET, "address manager: failed to find fd = %d", fd);

    return instance;
}

static struct soap_instance* find_soap_instance_matching_ip(struct avl_node_t *node, const char *ip)
{
    if (node == NULL)
        return NULL;

    struct soap_instance *res, *data;

    if ((res = find_soap_instance_matching_ip(node->left, ip)) != NULL)
        return res;

    if ((res = find_soap_instance_matching_ip(node->right, ip)) != NULL)
        return res;

    data = node->data;

    if (strstr(data->service_endpoint, ip) != NULL)
        return data;

    return NULL;
}

struct soap_instance* address_mngr_find_soap_instance_matching_ip(const char *ip)
{
    if (address_map.root == NULL)
        return NULL;

    return find_soap_instance_matching_ip(address_map.root, ip);
}

static void node_destruction_cb(struct avl_node_t *node)
{
    close(node->key);
    soap_instance_deallocate(node->data);
}

void address_mngr_destruct()
{
    avl_tree_destruct(&address_map, node_destruction_cb);
}

