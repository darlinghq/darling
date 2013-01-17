#ifndef STRVIS_H
#define STRVIS_H
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	VIS_OCTAL	0x01	/* use octal \ddd format */
#define	VIS_CSTYLE	0x02	/* use \[nrft0..] where appropriate */

/*
 * to alter set of characters encoded (default is to encode all
 * non-graphic except space, tab, and newline).
 */
#define	VIS_SP		0x04	/* also encode space */
#define	VIS_TAB		0x08	/* also encode tab */
#define	VIS_NL		0x10	/* also encode newline */
#define	VIS_WHITE	(VIS_SP | VIS_TAB | VIS_NL)
#define	VIS_SAFE	0x20	/* only encode "unsafe" characters */

/*
 * other
 */
#define	VIS_NOSLASH	0x40	/* inhibit printing '\' */
#define	VIS_HTTPSTYLE	0x80	/* http-style escape % HEX HEX */
#define	VIS_GLOB	0x100	/* encode glob(3) magics and '#' */

/*
 * unvis return codes
 */
#define	UNVIS_VALID	 1	/* character valid */
#define	UNVIS_VALIDPUSH	 2	/* character valid, push back passed char */
#define	UNVIS_NOCHAR	 3	/* valid sequence, no character produced */
#define	UNVIS_SYNBAD	-1	/* unrecognized escape sequence */
#define	UNVIS_ERROR	-2	/* decoder in unknown state (unrecoverable) */

/*
 * unvis flags
 */
#define	UNVIS_END	1	/* no more characters */


char* vis(char *dst, int c, int flag, int nextc);
int strvis(char *dst, const char *src, int flag);
int strnvis(char *dst, const char *src, size_t siz, int flag);
int strvisx(char *dst, const char *src, size_t len, int flag);
int	strunvis(char *, const char *);
int	strunvisx(char *, const char *, int);
int	unvis(char *, int, int *, int);

#ifdef __cplusplus
}
#endif

#endif

