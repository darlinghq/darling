#ifndef LINUX_ABORT_WITH_PAYLOAD_H
#define LINUX_ABORT_WITH_PAYLOAD_H

long sys_abort_with_payload(unsigned int reason_namespace, unsigned long long reason_code, void *payload, unsigned int payload_size, const char *reason_string, unsigned long long reason_flags);

#endif // LINUX_ABORT_WITH_PAYLOAD_H
