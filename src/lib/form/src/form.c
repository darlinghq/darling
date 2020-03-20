/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <form/form.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* _nc_Copy_Argument(void)
{
    if (verbose) puts("STUB: _nc_Copy_Argument called");
    return NULL;
}

void* _nc_Copy_Type(void)
{
    if (verbose) puts("STUB: _nc_Copy_Type called");
    return NULL;
}

void* _nc_First_Active_Field(void)
{
    if (verbose) puts("STUB: _nc_First_Active_Field called");
    return NULL;
}

void* _nc_Free_Argument(void)
{
    if (verbose) puts("STUB: _nc_Free_Argument called");
    return NULL;
}

void* _nc_Free_Type(void)
{
    if (verbose) puts("STUB: _nc_Free_Type called");
    return NULL;
}

void* _nc_Internal_Validation(void)
{
    if (verbose) puts("STUB: _nc_Internal_Validation called");
    return NULL;
}

void* _nc_Make_Argument(void)
{
    if (verbose) puts("STUB: _nc_Make_Argument called");
    return NULL;
}

void* _nc_Position_Form_Cursor(void)
{
    if (verbose) puts("STUB: _nc_Position_Form_Cursor called");
    return NULL;
}

void* _nc_Refresh_Current_Field(void)
{
    if (verbose) puts("STUB: _nc_Refresh_Current_Field called");
    return NULL;
}

void* _nc_Set_Current_Field(void)
{
    if (verbose) puts("STUB: _nc_Set_Current_Field called");
    return NULL;
}

void* _nc_Set_Form_Page(void)
{
    if (verbose) puts("STUB: _nc_Set_Form_Page called");
    return NULL;
}

void* _nc_Synchronize_Attributes(void)
{
    if (verbose) puts("STUB: _nc_Synchronize_Attributes called");
    return NULL;
}

void* _nc_Synchronize_Options(void)
{
    if (verbose) puts("STUB: _nc_Synchronize_Options called");
    return NULL;
}

void* _nc_Widen_String(void)
{
    if (verbose) puts("STUB: _nc_Widen_String called");
    return NULL;
}

void* current_field(void)
{
    if (verbose) puts("STUB: current_field called");
    return NULL;
}

void* data_ahead(void)
{
    if (verbose) puts("STUB: data_ahead called");
    return NULL;
}

void* data_behind(void)
{
    if (verbose) puts("STUB: data_behind called");
    return NULL;
}

void* dup_field(void)
{
    if (verbose) puts("STUB: dup_field called");
    return NULL;
}

void* dynamic_field_info(void)
{
    if (verbose) puts("STUB: dynamic_field_info called");
    return NULL;
}

void* field_arg(void)
{
    if (verbose) puts("STUB: field_arg called");
    return NULL;
}

void* field_back(void)
{
    if (verbose) puts("STUB: field_back called");
    return NULL;
}

void* field_buffer(void)
{
    if (verbose) puts("STUB: field_buffer called");
    return NULL;
}

void* field_count(void)
{
    if (verbose) puts("STUB: field_count called");
    return NULL;
}

void* field_fore(void)
{
    if (verbose) puts("STUB: field_fore called");
    return NULL;
}

void* field_index(void)
{
    if (verbose) puts("STUB: field_index called");
    return NULL;
}

void* field_info(void)
{
    if (verbose) puts("STUB: field_info called");
    return NULL;
}

void* field_init(void)
{
    if (verbose) puts("STUB: field_init called");
    return NULL;
}

void* field_just(void)
{
    if (verbose) puts("STUB: field_just called");
    return NULL;
}

void* field_opts(void)
{
    if (verbose) puts("STUB: field_opts called");
    return NULL;
}

void* field_opts_off(void)
{
    if (verbose) puts("STUB: field_opts_off called");
    return NULL;
}

void* field_opts_on(void)
{
    if (verbose) puts("STUB: field_opts_on called");
    return NULL;
}

void* field_pad(void)
{
    if (verbose) puts("STUB: field_pad called");
    return NULL;
}

void* field_status(void)
{
    if (verbose) puts("STUB: field_status called");
    return NULL;
}

void* field_term(void)
{
    if (verbose) puts("STUB: field_term called");
    return NULL;
}

void* field_type(void)
{
    if (verbose) puts("STUB: field_type called");
    return NULL;
}

void* field_userptr(void)
{
    if (verbose) puts("STUB: field_userptr called");
    return NULL;
}

void* form_driver(void)
{
    if (verbose) puts("STUB: form_driver called");
    return NULL;
}

void* form_fields(void)
{
    if (verbose) puts("STUB: form_fields called");
    return NULL;
}

void* form_init(void)
{
    if (verbose) puts("STUB: form_init called");
    return NULL;
}

void* form_opts(void)
{
    if (verbose) puts("STUB: form_opts called");
    return NULL;
}

void* form_opts_off(void)
{
    if (verbose) puts("STUB: form_opts_off called");
    return NULL;
}

void* form_opts_on(void)
{
    if (verbose) puts("STUB: form_opts_on called");
    return NULL;
}

void* form_page(void)
{
    if (verbose) puts("STUB: form_page called");
    return NULL;
}

void* form_request_by_name(void)
{
    if (verbose) puts("STUB: form_request_by_name called");
    return NULL;
}

void* form_request_name(void)
{
    if (verbose) puts("STUB: form_request_name called");
    return NULL;
}

void* form_sub(void)
{
    if (verbose) puts("STUB: form_sub called");
    return NULL;
}

void* form_term(void)
{
    if (verbose) puts("STUB: form_term called");
    return NULL;
}

void* form_userptr(void)
{
    if (verbose) puts("STUB: form_userptr called");
    return NULL;
}

void* form_win(void)
{
    if (verbose) puts("STUB: form_win called");
    return NULL;
}

void* free_field(void)
{
    if (verbose) puts("STUB: free_field called");
    return NULL;
}

void* free_fieldtype(void)
{
    if (verbose) puts("STUB: free_fieldtype called");
    return NULL;
}

void* free_form(void)
{
    if (verbose) puts("STUB: free_form called");
    return NULL;
}

void* link_field(void)
{
    if (verbose) puts("STUB: link_field called");
    return NULL;
}

void* link_fieldtype(void)
{
    if (verbose) puts("STUB: link_fieldtype called");
    return NULL;
}

void* move_field(void)
{
    if (verbose) puts("STUB: move_field called");
    return NULL;
}

void* new_field(void)
{
    if (verbose) puts("STUB: new_field called");
    return NULL;
}

void* new_fieldtype(void)
{
    if (verbose) puts("STUB: new_fieldtype called");
    return NULL;
}

void* new_form(void)
{
    if (verbose) puts("STUB: new_form called");
    return NULL;
}

void* new_page(void)
{
    if (verbose) puts("STUB: new_page called");
    return NULL;
}

void* pos_form_cursor(void)
{
    if (verbose) puts("STUB: pos_form_cursor called");
    return NULL;
}

void* post_form(void)
{
    if (verbose) puts("STUB: post_form called");
    return NULL;
}

void* scale_form(void)
{
    if (verbose) puts("STUB: scale_form called");
    return NULL;
}

void* set_current_field(void)
{
    if (verbose) puts("STUB: set_current_field called");
    return NULL;
}

void* set_field_back(void)
{
    if (verbose) puts("STUB: set_field_back called");
    return NULL;
}

void* set_field_buffer(void)
{
    if (verbose) puts("STUB: set_field_buffer called");
    return NULL;
}

void* set_field_fore(void)
{
    if (verbose) puts("STUB: set_field_fore called");
    return NULL;
}

void* set_field_init(void)
{
    if (verbose) puts("STUB: set_field_init called");
    return NULL;
}

void* set_field_just(void)
{
    if (verbose) puts("STUB: set_field_just called");
    return NULL;
}

void* set_field_opts(void)
{
    if (verbose) puts("STUB: set_field_opts called");
    return NULL;
}

void* set_field_pad(void)
{
    if (verbose) puts("STUB: set_field_pad called");
    return NULL;
}

void* set_field_status(void)
{
    if (verbose) puts("STUB: set_field_status called");
    return NULL;
}

void* set_field_term(void)
{
    if (verbose) puts("STUB: set_field_term called");
    return NULL;
}

void* set_field_type(void)
{
    if (verbose) puts("STUB: set_field_type called");
    return NULL;
}

void* set_field_userptr(void)
{
    if (verbose) puts("STUB: set_field_userptr called");
    return NULL;
}

void* set_fieldtype_arg(void)
{
    if (verbose) puts("STUB: set_fieldtype_arg called");
    return NULL;
}

void* set_fieldtype_choice(void)
{
    if (verbose) puts("STUB: set_fieldtype_choice called");
    return NULL;
}

void* set_form_fields(void)
{
    if (verbose) puts("STUB: set_form_fields called");
    return NULL;
}

void* set_form_init(void)
{
    if (verbose) puts("STUB: set_form_init called");
    return NULL;
}

void* set_form_opts(void)
{
    if (verbose) puts("STUB: set_form_opts called");
    return NULL;
}

void* set_form_page(void)
{
    if (verbose) puts("STUB: set_form_page called");
    return NULL;
}

void* set_form_sub(void)
{
    if (verbose) puts("STUB: set_form_sub called");
    return NULL;
}

void* set_form_term(void)
{
    if (verbose) puts("STUB: set_form_term called");
    return NULL;
}

void* set_form_userptr(void)
{
    if (verbose) puts("STUB: set_form_userptr called");
    return NULL;
}

void* set_form_win(void)
{
    if (verbose) puts("STUB: set_form_win called");
    return NULL;
}

void* set_max_field(void)
{
    if (verbose) puts("STUB: set_max_field called");
    return NULL;
}

void* set_new_page(void)
{
    if (verbose) puts("STUB: set_new_page called");
    return NULL;
}

void* unpost_form(void)
{
    if (verbose) puts("STUB: unpost_form called");
    return NULL;
}
