provider notify {
	probe register_mach_port(const char *name, mach_port_name_t notify_port, int flags, int token);
	probe post(const char *name);
	probe check(int token, int check);
	probe deliver_start(const char *name);
	probe deliver_end(const char *name);
};
