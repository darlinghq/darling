/*
	Copyright (c) 2018 Apple Inc. All rights reserved.
	
	This pseduo-header file contains declarations for private symbols that are exported in libsystem_info.dylib but don't
	have a proper header file. This file allows InstallAPI, which parses header files to infer the symbols exported by a
	library, to know about these symbols.
*/

#if (defined(DS_AVAILABLE))
int				_ds_running(void);
#endif
mach_port_t		_getaddrinfo_interface_async_call(const char *nodename, const char *servname, const struct addrinfo *hints, const char *interface, getaddrinfo_async_callback callback, void *context);
mach_port_t		_getnameinfo_interface_async_call(const struct sockaddr *sa, size_t len, int flags, const char *interface, getnameinfo_async_callback callback, void *context);
#if (defined(DS_AVAILABLE))
void			_si_disable_opendirectory(void);
#endif
extern uint32_t	gL1CacheEnabled;
int32_t			getgroupcount(const char *name, gid_t basegid);
int32_t			getgrouplist_2(const char *name, gid_t basegid, gid_t **groups);
