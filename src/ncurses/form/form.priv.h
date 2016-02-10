/****************************************************************************
 * Copyright (c) 1998-2006,2008 Free Software Foundation, Inc.              *
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

/* $Id: form.priv.h,v 0.27 2008/09/08 20:29:05 tom Exp $ */

#ifndef FORM_PRIV_H
#define FORM_PRIV_H 1

#include "curses.priv.h"
#include "mf_common.h"

#if USE_WIDEC_SUPPORT
#if HAVE_WCTYPE_H
#include <wctype.h>
#endif

#ifndef MB_LEN_MAX
#define MB_LEN_MAX 8 /* should be >= MB_CUR_MAX, but that may be a function */
#endif

#define FIELD_CELL NCURSES_CH_T

#define NCURSES_FIELD_INTERNALS char** expanded; WINDOW *working;
#define NCURSES_FIELD_EXTENSION , (char **)0, (WINDOW *)0

#else

#define FIELD_CELL char

#define NCURSES_FIELD_EXTENSION /* nothing */

#endif

#include "form.h"

/* form  status values */
#define _OVLMODE         (0x04U) /* Form is in overlay mode                */
#define _WINDOW_MODIFIED (0x10U) /* Current field window has been modified */
#define _FCHECK_REQUIRED (0x20U) /* Current field needs validation         */

/* field status values */
#define _CHANGED         (0x01U) /* Field has been changed                 */
#define _NEWTOP          (0x02U) /* Vertical scrolling occurred            */
#define _NEWPAGE         (0x04U) /* field begins new page of form          */
#define _MAY_GROW        (0x08U) /* dynamic field may still grow           */

/* fieldtype status values */
#define _LINKED_TYPE     (0x01U) /* Type is a linked type                  */
#define _HAS_ARGS        (0x02U) /* Type has arguments                     */
#define _HAS_CHOICE      (0x04U) /* Type has choice methods                */
#define _RESIDENT        (0x08U) /* Type is built-in                       */

/* This are the field options required to be a selectable field in field
   navigation requests */
#define O_SELECTABLE (O_ACTIVE | O_VISIBLE)

/* If form is NULL replace form argument by default-form */
#define Normalize_Form(form) \
  ((form) = (form != 0) ? (form) : _nc_Default_Form)

/* If field is NULL replace field argument by default-field */
#define Normalize_Field(field) \
  ((field) = (field != 0) ? (field) : _nc_Default_Field)

/* Retrieve forms window */
#define Get_Form_Window(form) \
  ((form)->sub?(form)->sub:((form)->win?(form)->win:stdscr))

/* Calculate the size for a single buffer for this field */
#define Buffer_Length(field) ((field)->drows * (field)->dcols)

/* Calculate the total size of all buffers for this field */
#define Total_Buffer_Size(field) \
   ( (Buffer_Length(field) + 1) * (1+(field)->nbuf) * sizeof(FIELD_CELL) )

/* Logic to determine whether or not a field is single lined */
#define Single_Line_Field(field) \
   (((field)->rows + (field)->nrow) == 1)

/* Logic to determine whether or not a field is selectable */
#define Field_Is_Selectable(f)     (((unsigned)((f)->opts) & O_SELECTABLE)==O_SELECTABLE)
#define Field_Is_Not_Selectable(f) (((unsigned)((f)->opts) & O_SELECTABLE)!=O_SELECTABLE)

typedef struct typearg
  {
    struct typearg *left;
    struct typearg *right;
  }
TypeArgument;

/* This is a dummy request code (normally invalid) to be used internally
   with the form_driver() routine to position to the first active field
   on the form
*/
#define FIRST_ACTIVE_MAGIC (-291056)

#define ALL_FORM_OPTS  (                \
			O_NL_OVERLOAD  |\
			O_BS_OVERLOAD   )

#define ALL_FIELD_OPTS (Field_Options)( \
			O_VISIBLE |\
			O_ACTIVE  |\
			O_PUBLIC  |\
			O_EDIT    |\
			O_WRAP    |\
			O_BLANK   |\
			O_AUTOSKIP|\
			O_NULLOK  |\
			O_PASSOK  |\
			O_STATIC   )

#define C_BLANK ' '
#define is_blank(c) ((c)==C_BLANK)

#define C_ZEROS '\0'

extern NCURSES_EXPORT_VAR(const FIELDTYPE *) _nc_Default_FieldType;

extern NCURSES_EXPORT(TypeArgument *) _nc_Make_Argument (const FIELDTYPE*, va_list*, int*);
extern NCURSES_EXPORT(TypeArgument *) _nc_Copy_Argument (const FIELDTYPE*, const TypeArgument*, int*);
extern NCURSES_EXPORT(void) _nc_Free_Argument (const FIELDTYPE*, TypeArgument*);
extern NCURSES_EXPORT(bool) _nc_Copy_Type (FIELD*, FIELD const *);
extern NCURSES_EXPORT(void) _nc_Free_Type (FIELD *);

extern NCURSES_EXPORT(int) _nc_Synchronize_Attributes (FIELD*);
extern NCURSES_EXPORT(int) _nc_Synchronize_Options (FIELD*, Field_Options);
extern NCURSES_EXPORT(int) _nc_Set_Form_Page (FORM*, int, FIELD*);
extern NCURSES_EXPORT(int) _nc_Refresh_Current_Field (FORM*);
extern NCURSES_EXPORT(FIELD *) _nc_First_Active_Field (FORM*);
extern NCURSES_EXPORT(bool) _nc_Internal_Validation (FORM*);
extern NCURSES_EXPORT(int) _nc_Set_Current_Field (FORM*, FIELD*);
extern NCURSES_EXPORT(int) _nc_Position_Form_Cursor (FORM*);

#if USE_WIDEC_SUPPORT
extern NCURSES_EXPORT(wchar_t *) _nc_Widen_String(char *, int *);
#endif

#ifdef TRACE

#define returnField(code)	TRACE_RETURN(code,field)
#define returnFieldPtr(code)	TRACE_RETURN(code,field_ptr)
#define returnForm(code)	TRACE_RETURN(code,form)
#define returnFieldType(code)	TRACE_RETURN(code,field_type)
#define returnFormHook(code)	TRACE_RETURN(code,form_hook)

extern NCURSES_EXPORT(FIELD **)	    _nc_retrace_field_ptr (FIELD **);
extern NCURSES_EXPORT(FIELD *)	    _nc_retrace_field (FIELD *);
extern NCURSES_EXPORT(FIELDTYPE *)  _nc_retrace_field_type (FIELDTYPE *);
extern NCURSES_EXPORT(FORM *)  _nc_retrace_form (FORM *);
extern NCURSES_EXPORT(Form_Hook)  _nc_retrace_form_hook (Form_Hook);

#else /* !TRACE */

#define returnFieldPtr(code)	return code
#define returnFieldType(code)	return code
#define returnField(code)	return code
#define returnForm(code)	return code
#define returnFormHook(code)	return code

#endif /* TRACE/!TRACE */

/*
 * Use Check_CTYPE_Field() to simplify FIELDTYPE's that use only the ccheck()
 * function.
 */
#if USE_WIDEC_SUPPORT
#define Check_CTYPE_Field(result, buffer, width, ccheck) \
  while (*buffer && *buffer == ' ') \
    buffer++; \
  if (*buffer) \
    { \
      bool blank = FALSE; \
      int len; \
      int n; \
      wchar_t *list = _nc_Widen_String((char *)buffer, &len); \
      if (list != 0) \
	{ \
	  result = TRUE; \
	  for (n = 0; n < len; ++n) \
	    { \
	      if (blank) \
		{ \
		  if (list[n] != ' ') \
		    { \
		      result = FALSE; \
		      break; \
		    } \
		} \
	      else if (list[n] == ' ') \
		{ \
		  blank = TRUE; \
		  result = (n + 1 >= width); \
		} \
	      else if (!ccheck(list[n], NULL)) \
		{ \
		  result = FALSE; \
		  break; \
		} \
	    } \
	  free(list); \
	} \
    }
#else
#define Check_CTYPE_Field(result, buffer, width, ccheck) \
  while (*buffer && *buffer == ' ') \
    buffer++; \
  if (*buffer) \
    { \
      unsigned char *s = buffer; \
      int l = -1; \
      while (*buffer && ccheck(*buffer, NULL)) \
	buffer++; \
      l = (int)(buffer - s); \
      while (*buffer && *buffer == ' ') \
	buffer++; \
      result = ((*buffer || (l < width)) ? FALSE : TRUE); \
    }
#endif

#endif /* FORM_PRIV_H */
