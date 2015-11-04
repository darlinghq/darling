/****************************************************************************
 * Copyright (c) 1998-2004,2006 Free Software Foundation, Inc.              *
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
* Module m_new                                                             *
* Creation and destruction of new menus                                    *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_new.c,v 1.18 2006/11/04 19:04:06 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  MENU *new_menu(ITEM **items)
|   
|   Description   :  Creates a new menu connected to the item pointer
|                    array items and returns a pointer to the new menu.
|                    The new menu is initialized with the values from the
|                    default menu.
|
|   Return Values :  NULL on error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(MENU *)
new_menu(ITEM ** items)
{
  int err = E_SYSTEM_ERROR;
  MENU *menu = (MENU *) calloc(1, sizeof(MENU));

  T((T_CALLED("new_menu(%p)"), items));
  if (menu)
    {
      *menu = _nc_Default_Menu;
      menu->status = 0;
      menu->rows = menu->frows;
      menu->cols = menu->fcols;
      if (items && *items)
	{
	  if (!_nc_Connect_Items(menu, items))
	    {
	      err = E_NOT_CONNECTED;
	      free(menu);
	      menu = (MENU *) 0;
	    }
	}
    }

  if (!menu)
    SET_ERROR(err);

  returnMenu(menu);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int free_menu(MENU *menu)  
|   
|   Description   :  Disconnects menu from its associated item pointer 
|                    array and frees the storage allocated for the menu.
|
|   Return Values :  E_OK               - success
|                    E_BAD_ARGUMENT     - Invalid menu pointer passed
|                    E_POSTED           - Menu is already posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
free_menu(MENU * menu)
{
  T((T_CALLED("free_menu(%p)"), menu));
  if (!menu)
    RETURN(E_BAD_ARGUMENT);

  if (menu->status & _POSTED)
    RETURN(E_POSTED);

  if (menu->items)
    _nc_Disconnect_Items(menu);

  if ((menu->status & _MARK_ALLOCATED) && menu->mark)
    free(menu->mark);

  free(menu);
  RETURN(E_OK);
}

/* m_new.c ends here */
