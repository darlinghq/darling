/*
 * We need to prefix the elements of DIR with double-underscore to use
 * the implementation namespace.  We could patch all the *dir.c code,
 * but it is easier to use #defines to rename the elements.  This file
 * then gets included by all the *dir.c.
 */
#define	dd_fd		__dd_fd
#define	dd_loc		__dd_loc
#define	dd_size		__dd_size
#define	dd_buf		__dd_buf
#define	dd_len		__dd_len
#define	dd_seek		__dd_seek
#define	dd_flags	__dd_flags
#define	dd_lock		__dd_lock
#define	dd_td		__dd_td
