#ifndef EXT_SYSLOG_H
#define EXT_SYSLOG_H

#define SYSLOG_ACTION_READ_ALL       3
#define SYSLOG_ACTION_SIZE_BUFFER   10

int __linux_syslog (int type, char* bufp, int len);

#endif

