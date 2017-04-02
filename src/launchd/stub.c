#include <unistd.h>
void audit_token_to_au32(void* atoken, uid_t *auidp, uid_t *euidp, gid_t *egidp, ...)
{
	*euidp = getuid();
	*egidp = getgid();
}

