#ifndef _DARLING_EMULATION_AUDIT_AUDIT_ADDR_H
#define _DARLING_EMULATION_AUDIT_AUDIT_ADDR_H

#include <stdint.h>
#include <bsm/audit.h>

long sys_getaudit_addr(struct auditinfo_addr* auditinfo_addr, int length);
long sys_setaudit_addr(struct auditinfo_addr* auditinfo_addr, int length);

#endif // _DARLING_EMULATION_AUDIT_GETAUDIT_ADDR_H
