#ifndef _SESSION_AGENT_STATUS_COM_H_
#define _SESSION_AGENT_STATUS_COM_H_

typedef int SessionAgentState;

#define kSA_state_desktopshowing 1

SessionAgentState SASSessionStateForUser(int uid);

#endif
