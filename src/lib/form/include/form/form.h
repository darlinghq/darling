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


#ifndef _form_H_
#define _form_H_

void* _nc_Copy_Argument(void);
void* _nc_Copy_Type(void);
void* _nc_First_Active_Field(void);
void* _nc_Free_Argument(void);
void* _nc_Free_Type(void);
void* _nc_Internal_Validation(void);
void* _nc_Make_Argument(void);
void* _nc_Position_Form_Cursor(void);
void* _nc_Refresh_Current_Field(void);
void* _nc_Set_Current_Field(void);
void* _nc_Set_Form_Page(void);
void* _nc_Synchronize_Attributes(void);
void* _nc_Synchronize_Options(void);
void* _nc_Widen_String(void);
void* current_field(void);
void* data_ahead(void);
void* data_behind(void);
void* dup_field(void);
void* dynamic_field_info(void);
void* field_arg(void);
void* field_back(void);
void* field_buffer(void);
void* field_count(void);
void* field_fore(void);
void* field_index(void);
void* field_info(void);
void* field_init(void);
void* field_just(void);
void* field_opts(void);
void* field_opts_off(void);
void* field_opts_on(void);
void* field_pad(void);
void* field_status(void);
void* field_term(void);
void* field_type(void);
void* field_userptr(void);
void* form_driver(void);
void* form_fields(void);
void* form_init(void);
void* form_opts(void);
void* form_opts_off(void);
void* form_opts_on(void);
void* form_page(void);
void* form_request_by_name(void);
void* form_request_name(void);
void* form_sub(void);
void* form_term(void);
void* form_userptr(void);
void* form_win(void);
void* free_field(void);
void* free_fieldtype(void);
void* free_form(void);
void* link_field(void);
void* link_fieldtype(void);
void* move_field(void);
void* new_field(void);
void* new_fieldtype(void);
void* new_form(void);
void* new_page(void);
void* pos_form_cursor(void);
void* post_form(void);
void* scale_form(void);
void* set_current_field(void);
void* set_field_back(void);
void* set_field_buffer(void);
void* set_field_fore(void);
void* set_field_init(void);
void* set_field_just(void);
void* set_field_opts(void);
void* set_field_pad(void);
void* set_field_status(void);
void* set_field_term(void);
void* set_field_type(void);
void* set_field_userptr(void);
void* set_fieldtype_arg(void);
void* set_fieldtype_choice(void);
void* set_form_fields(void);
void* set_form_init(void);
void* set_form_opts(void);
void* set_form_page(void);
void* set_form_sub(void);
void* set_form_term(void);
void* set_form_userptr(void);
void* set_form_win(void);
void* set_max_field(void);
void* set_new_page(void);
void* unpost_form(void);

#endif
