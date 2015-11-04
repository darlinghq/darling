/****************************************************************************
 * Copyright (c) 1998-2005,2008 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *   Author:  Juergen Pfeifer, 1995,1997                                    *
 ****************************************************************************/

/***************************************************************************
* Module m_request_name                                                    *
* Routines to handle external names of menu requests                       *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_req_name.c,v 1.20 2008/09/13 18:59:17 tom Exp $")

static const char *request_names[MAX_MENU_COMMAND - MIN_MENU_COMMAND + 1] =
{
  "LEFT_ITEM",
  "RIGHT_ITEM",
  "UP_ITEM",
  "DOWN_ITEM",
  "SCR_ULINE",
  "SCR_DLINE",
  "SCR_DPAGE",
  "SCR_UPAGE",
  "FIRST_ITEM",
  "LAST_ITEM",
  "NEXT_ITEM",
  "PREV_ITEM",
  "TOGGLE_ITEM",
  "CLEAR_PATTERN",
  "BACK_PATTERN",
  "NEXT_MATCH",
  "PREV_MATCH"
};

#define A_SIZE (sizeof(request_names)/sizeof(request_names[0]))

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  const char * menu_request_name (int request);
|   
|   Description   :  Get the external name of a menu request.
|
|   Return Values :  Pointer to name      - on success
|                    NULL                 - on invalid request code
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(const char *)
menu_request_name(int request)
{
  T((T_CALLED("menu_request_name(%d)"), request));
  if ((request < MIN_MENU_COMMAND) || (request > MAX_MENU_COMMAND))
    {
      SET_ERROR(E_BAD_ARGUMENT);
      returnCPtr((const char *)0);
    }
  else
    returnCPtr(request_names[request - MIN_MENU_COMMAND]);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int menu_request_by_name (const char *str);
|   
|   Description   :  Search for a request with this name.
|
|   Return Values :  Request Id       - on success
|                    E_NO_MATCH       - request not found
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
menu_request_by_name(const char *str)
{
  /* because the table is so small, it doesn't really hurt
     to run sequentially through it.
   */
  unsigned int i = 0;
  char buf[16];

  T((T_CALLED("menu_request_by_name(%s)"), _nc_visbuf(str)));

  if (str)
    {
      strncpy(buf, str, sizeof(buf));
      while ((i < sizeof(buf)) && (buf[i] != '\0'))
	{
	  buf[i] = toupper(UChar(buf[i]));
	  i++;
	}

      for (i = 0; i < A_SIZE; i++)
	{
	  if (strncmp(request_names[i], buf, sizeof(buf)) == 0)
	    returnCode(MIN_MENU_COMMAND + (int)i);
	}
    }
  RETURN(E_NO_MATCH);
}

/* m_req_name.c ends here */
