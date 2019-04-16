#include "address_manager.h"
#include "avltree.h"
#include "soap_instance.h"

struct avl_tree_t *address_map;

void address_mngr_init()
{
    avl_tree_construct(address_map);
}

static void create_unique_port_from_ip(const char *ip)
{
    return 0; /* idk lol */
}

void address_mngr_add_address()
{
    const char *fix_compilation_ip, *fix_compilation_port;
    int port = create_unique_port_from_ip(fix_compilation_ip);
    struct soap_instance *instance = soap_instance_allocate(fix_compilation_ip, fix_compilation_port);

    log("add address map %d -> %s", port, fix_compilation_ip);
    avl_tree_insert(address_map, port, instance);
}

static void destruct_soap_instance(void *addr)
{
    soap_instance_deallocate(addr);
}

void address_mngr_destruct()
{
    avl_tree_destruct(address_map, destruct_soap_instance);
}

