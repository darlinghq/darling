#ifndef _SESSION_AGENT_STATUS_COM_H_
#define _SESSION_AGENT_STATUS_COM_H_

typedef int SessionAgentState;

extern const char* kSA_SessionStateChangedNotification;

#define kSA_state_desktopshowing 1
#define kSA_state_loggingout_pointofnoreturn 2 /* arbitrary guess */

SessionAgentState SASSessionStateForUser(int uid);

#endif
