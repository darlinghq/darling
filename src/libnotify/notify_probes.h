// fake DTrace probes for libnotify

#define NOTIFY_REGISTER_MACH_PORT(...) (0)
#define NOTIFY_REGISTER_MACH_PORT_ENABLED(...) (0)

#define NOTIFY_POST(...) (0)
#define NOTIFY_POST_ENABLED(...) (0)

#define NOTIFY_CHECK(...) (0)
#define NOTIFY_CHECK_ENABLED(...) (0)

#define NOTIFY_DELIVER_START(...) (0)
#define NOTIFY_DELIVER_START_ENABLED(...) (0)

#define NOTIFY_DELIVER_END(...) (0)
#define NOTIFY_DELIVER_END_ENABLED(...) (0)
