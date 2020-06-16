//
//	systemx.h
//	copyfile_test
//	Stolen from the test routines from the apfs project.
//

#ifndef systemx_h_
#define systemx_h_

#define SYSTEMX_QUIET				((void *)1)
#define SYSTEMX_QUIET_STDERR		((void *)2)

int __attribute__((sentinel)) systemx(const char *prog, ...);

#endif /* systemx_h_ */
