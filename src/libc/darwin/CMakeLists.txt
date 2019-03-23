project(libc-darwin)

cmake_minimum_required(VERSION 2.4.0)

set(darwin_sources
	atexit_receipt.c
	compatibility_hacks.c
	forceLibcToBuild.c
	init_cpu_capabilities.c
	kvm.c
	variant.c
	darling_hacks.c
	)

add_library(libc-darwin OBJECT ${darwin_sources})
