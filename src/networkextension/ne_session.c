#include <ne_session.h>
#include <stdio.h>

bool ne_session_set_socket_attributes(int socket, char *hostname, void *unknown) {
    printf("DARLING SYSTEM_NETWORKEXTENSION STUB: %s\n", __PRETTY_FUNCTION__);
    return false;
}

int ne_session_service_get_dns_service_id(uuid_t uuid, ne_session_type_t session_type, const char *hostname) {
    printf("DARLING SYSTEM_NETWORKEXTENSION STUB: %s\n", __PRETTY_FUNCTION__);
    return -1;
}