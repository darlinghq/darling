#ifndef CGEVENTREF_H_
#define CGEVENTREF_H_

#ifdef DARLING_BUILD
#	include <QEvent>
typedef QEvent *CGEventRef;
#else
struct __CGEvent;
typedef struct __CGEvent *CGEventRef;
#endif

#endif

