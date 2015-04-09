project(libc-net)

cmake_minimum_required(VERSION 2.4.0)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../../platform-include/net/if_var.h")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")

add_definitions(-Dinet_aton=__inet_aton -Dinet_addr=__inet_addr)

set(net_sources
	FreeBSD/addr2ascii.c
	FreeBSD/ascii2addr.c
	FreeBSD/inet_addr.c
	FreeBSD/inet_lnaof.c
	FreeBSD/inet_makeaddr.c
	FreeBSD/inet_neta.c
	FreeBSD/inet_net_ntop.c
	FreeBSD/inet_netof.c
	FreeBSD/inet_net_pton.c
	FreeBSD/inet_network.c
	FreeBSD/inet_ntoa.c
	FreeBSD/linkaddr.c
	FreeBSD/nsap_addr.c
	FreeBSD/recv.c
	FreeBSD/send.c
	FreeBSD/sockatmark.c
	FreeBSD/sourcefilter.c
)

add_library(libc-net OBJECT ${net_sources})
