#include <stdbool.h>

bool ne_session_set_socket_attributes(int socket, char *hostname, void *unknown);

// not sure what this type is (it's probably an enum or enum-like type)
typedef int ne_session_status_t;
