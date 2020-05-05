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


#include <LDAP/LDAP.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ber_alloc(void)
{
    if (verbose) puts("STUB: ber_alloc called");
    return NULL;
}

void* ber_alloc_t(void)
{
    if (verbose) puts("STUB: ber_alloc_t called");
    return NULL;
}

void* ber_bprint(void)
{
    if (verbose) puts("STUB: ber_bprint called");
    return NULL;
}

void* ber_bvarray_add(void)
{
    if (verbose) puts("STUB: ber_bvarray_add called");
    return NULL;
}

void* ber_bvarray_free(void)
{
    if (verbose) puts("STUB: ber_bvarray_free called");
    return NULL;
}

void* ber_bvdup(void)
{
    if (verbose) puts("STUB: ber_bvdup called");
    return NULL;
}

void* ber_bvecadd(void)
{
    if (verbose) puts("STUB: ber_bvecadd called");
    return NULL;
}

void* ber_bvecfree(void)
{
    if (verbose) puts("STUB: ber_bvecfree called");
    return NULL;
}

void* ber_bvfree(void)
{
    if (verbose) puts("STUB: ber_bvfree called");
    return NULL;
}

void* ber_dump(void)
{
    if (verbose) puts("STUB: ber_dump called");
    return NULL;
}

void* ber_dup(void)
{
    if (verbose) puts("STUB: ber_dup called");
    return NULL;
}

void* ber_dupbv(void)
{
    if (verbose) puts("STUB: ber_dupbv called");
    return NULL;
}

void* ber_errno_addr(void)
{
    if (verbose) puts("STUB: ber_errno_addr called");
    return NULL;
}

void* ber_error_print(void)
{
    if (verbose) puts("STUB: ber_error_print called");
    return NULL;
}

void* ber_first_element(void)
{
    if (verbose) puts("STUB: ber_first_element called");
    return NULL;
}

void* ber_flatten(void)
{
    if (verbose) puts("STUB: ber_flatten called");
    return NULL;
}

void* ber_flatten2(void)
{
    if (verbose) puts("STUB: ber_flatten2 called");
    return NULL;
}

void* ber_flush(void)
{
    if (verbose) puts("STUB: ber_flush called");
    return NULL;
}

void* ber_free(void)
{
    if (verbose) puts("STUB: ber_free called");
    return NULL;
}

void* ber_free_buf(void)
{
    if (verbose) puts("STUB: ber_free_buf called");
    return NULL;
}

void* ber_get_bitstringa(void)
{
    if (verbose) puts("STUB: ber_get_bitstringa called");
    return NULL;
}

void* ber_get_boolean(void)
{
    if (verbose) puts("STUB: ber_get_boolean called");
    return NULL;
}

void* ber_get_enum(void)
{
    if (verbose) puts("STUB: ber_get_enum called");
    return NULL;
}

void* ber_get_int(void)
{
    if (verbose) puts("STUB: ber_get_int called");
    return NULL;
}

void* ber_get_next(void)
{
    if (verbose) puts("STUB: ber_get_next called");
    return NULL;
}

void* ber_get_null(void)
{
    if (verbose) puts("STUB: ber_get_null called");
    return NULL;
}

void* ber_get_option(void)
{
    if (verbose) puts("STUB: ber_get_option called");
    return NULL;
}

void* ber_get_stringa(void)
{
    if (verbose) puts("STUB: ber_get_stringa called");
    return NULL;
}

void* ber_get_stringal(void)
{
    if (verbose) puts("STUB: ber_get_stringal called");
    return NULL;
}

void* ber_get_stringb(void)
{
    if (verbose) puts("STUB: ber_get_stringb called");
    return NULL;
}

void* ber_get_stringbv(void)
{
    if (verbose) puts("STUB: ber_get_stringbv called");
    return NULL;
}

void* ber_get_tag(void)
{
    if (verbose) puts("STUB: ber_get_tag called");
    return NULL;
}

void* ber_init(void)
{
    if (verbose) puts("STUB: ber_init called");
    return NULL;
}

void* ber_init2(void)
{
    if (verbose) puts("STUB: ber_init2 called");
    return NULL;
}

void* ber_init_w_nullc(void)
{
    if (verbose) puts("STUB: ber_init_w_nullc called");
    return NULL;
}

void* ber_mem2bv(void)
{
    if (verbose) puts("STUB: ber_mem2bv called");
    return NULL;
}

void* ber_memalloc(void)
{
    if (verbose) puts("STUB: ber_memalloc called");
    return NULL;
}

void* ber_memcalloc(void)
{
    if (verbose) puts("STUB: ber_memcalloc called");
    return NULL;
}

void* ber_memfree(void)
{
    if (verbose) puts("STUB: ber_memfree called");
    return NULL;
}

void* ber_memrealloc(void)
{
    if (verbose) puts("STUB: ber_memrealloc called");
    return NULL;
}

void* ber_memvfree(void)
{
    if (verbose) puts("STUB: ber_memvfree called");
    return NULL;
}

void* ber_next_element(void)
{
    if (verbose) puts("STUB: ber_next_element called");
    return NULL;
}

void* ber_peek_tag(void)
{
    if (verbose) puts("STUB: ber_peek_tag called");
    return NULL;
}

void* ber_printf(void)
{
    if (verbose) puts("STUB: ber_printf called");
    return NULL;
}

void* ber_put_berval(void)
{
    if (verbose) puts("STUB: ber_put_berval called");
    return NULL;
}

void* ber_put_bitstring(void)
{
    if (verbose) puts("STUB: ber_put_bitstring called");
    return NULL;
}

void* ber_put_boolean(void)
{
    if (verbose) puts("STUB: ber_put_boolean called");
    return NULL;
}

void* ber_put_enum(void)
{
    if (verbose) puts("STUB: ber_put_enum called");
    return NULL;
}

void* ber_put_int(void)
{
    if (verbose) puts("STUB: ber_put_int called");
    return NULL;
}

void* ber_put_null(void)
{
    if (verbose) puts("STUB: ber_put_null called");
    return NULL;
}

void* ber_put_ostring(void)
{
    if (verbose) puts("STUB: ber_put_ostring called");
    return NULL;
}

void* ber_put_seq(void)
{
    if (verbose) puts("STUB: ber_put_seq called");
    return NULL;
}

void* ber_put_set(void)
{
    if (verbose) puts("STUB: ber_put_set called");
    return NULL;
}

void* ber_put_string(void)
{
    if (verbose) puts("STUB: ber_put_string called");
    return NULL;
}

void* ber_read(void)
{
    if (verbose) puts("STUB: ber_read called");
    return NULL;
}

void* ber_remaining(void)
{
    if (verbose) puts("STUB: ber_remaining called");
    return NULL;
}

void* ber_reset(void)
{
    if (verbose) puts("STUB: ber_reset called");
    return NULL;
}

void* ber_scanf(void)
{
    if (verbose) puts("STUB: ber_scanf called");
    return NULL;
}

void* ber_set_option(void)
{
    if (verbose) puts("STUB: ber_set_option called");
    return NULL;
}

void* ber_skip_tag(void)
{
    if (verbose) puts("STUB: ber_skip_tag called");
    return NULL;
}

void* ber_sockbuf_add_io(void)
{
    if (verbose) puts("STUB: ber_sockbuf_add_io called");
    return NULL;
}

void* ber_sockbuf_alloc(void)
{
    if (verbose) puts("STUB: ber_sockbuf_alloc called");
    return NULL;
}

void* ber_sockbuf_ctrl(void)
{
    if (verbose) puts("STUB: ber_sockbuf_ctrl called");
    return NULL;
}

void* ber_sockbuf_free(void)
{
    if (verbose) puts("STUB: ber_sockbuf_free called");
    return NULL;
}

void* ber_sockbuf_remove_io(void)
{
    if (verbose) puts("STUB: ber_sockbuf_remove_io called");
    return NULL;
}

void* ber_sos_dump(void)
{
    if (verbose) puts("STUB: ber_sos_dump called");
    return NULL;
}

void* ber_start_seq(void)
{
    if (verbose) puts("STUB: ber_start_seq called");
    return NULL;
}

void* ber_start_set(void)
{
    if (verbose) puts("STUB: ber_start_set called");
    return NULL;
}

void* ber_str2bv(void)
{
    if (verbose) puts("STUB: ber_str2bv called");
    return NULL;
}

void* ber_strdup(void)
{
    if (verbose) puts("STUB: ber_strdup called");
    return NULL;
}

void* ber_write(void)
{
    if (verbose) puts("STUB: ber_write called");
    return NULL;
}

void* der_alloc(void)
{
    if (verbose) puts("STUB: der_alloc called");
    return NULL;
}

void* ldap_X509dn2bv(void)
{
    if (verbose) puts("STUB: ldap_X509dn2bv called");
    return NULL;
}

void* ldap_abandon(void)
{
    if (verbose) puts("STUB: ldap_abandon called");
    return NULL;
}

void* ldap_abandon_ext(void)
{
    if (verbose) puts("STUB: ldap_abandon_ext called");
    return NULL;
}

void* ldap_add(void)
{
    if (verbose) puts("STUB: ldap_add called");
    return NULL;
}

void* ldap_add_ext(void)
{
    if (verbose) puts("STUB: ldap_add_ext called");
    return NULL;
}

void* ldap_add_ext_s(void)
{
    if (verbose) puts("STUB: ldap_add_ext_s called");
    return NULL;
}

void* ldap_add_result_entry(void)
{
    if (verbose) puts("STUB: ldap_add_result_entry called");
    return NULL;
}

void* ldap_add_s(void)
{
    if (verbose) puts("STUB: ldap_add_s called");
    return NULL;
}

void* ldap_attributetype2bv(void)
{
    if (verbose) puts("STUB: ldap_attributetype2bv called");
    return NULL;
}

void* ldap_attributetype2name(void)
{
    if (verbose) puts("STUB: ldap_attributetype2name called");
    return NULL;
}

void* ldap_attributetype2str(void)
{
    if (verbose) puts("STUB: ldap_attributetype2str called");
    return NULL;
}

void* ldap_attributetype_free(void)
{
    if (verbose) puts("STUB: ldap_attributetype_free called");
    return NULL;
}

void* ldap_bind(void)
{
    if (verbose) puts("STUB: ldap_bind called");
    return NULL;
}

void* ldap_bind_s(void)
{
    if (verbose) puts("STUB: ldap_bind_s called");
    return NULL;
}

void* ldap_bv2dn(void)
{
    if (verbose) puts("STUB: ldap_bv2dn called");
    return NULL;
}

void* ldap_bv2rdn(void)
{
    if (verbose) puts("STUB: ldap_bv2rdn called");
    return NULL;
}

void* ldap_cancel(void)
{
    if (verbose) puts("STUB: ldap_cancel called");
    return NULL;
}

void* ldap_cancel_s(void)
{
    if (verbose) puts("STUB: ldap_cancel_s called");
    return NULL;
}

void* ldap_compare(void)
{
    if (verbose) puts("STUB: ldap_compare called");
    return NULL;
}

void* ldap_compare_ext(void)
{
    if (verbose) puts("STUB: ldap_compare_ext called");
    return NULL;
}

void* ldap_compare_ext_s(void)
{
    if (verbose) puts("STUB: ldap_compare_ext_s called");
    return NULL;
}

void* ldap_compare_s(void)
{
    if (verbose) puts("STUB: ldap_compare_s called");
    return NULL;
}

void* ldap_contentrule2bv(void)
{
    if (verbose) puts("STUB: ldap_contentrule2bv called");
    return NULL;
}

void* ldap_contentrule2name(void)
{
    if (verbose) puts("STUB: ldap_contentrule2name called");
    return NULL;
}

void* ldap_contentrule2str(void)
{
    if (verbose) puts("STUB: ldap_contentrule2str called");
    return NULL;
}

void* ldap_contentrule_free(void)
{
    if (verbose) puts("STUB: ldap_contentrule_free called");
    return NULL;
}

void* ldap_control_create(void)
{
    if (verbose) puts("STUB: ldap_control_create called");
    return NULL;
}

void* ldap_control_find(void)
{
    if (verbose) puts("STUB: ldap_control_find called");
    return NULL;
}

void* ldap_control_free(void)
{
    if (verbose) puts("STUB: ldap_control_free called");
    return NULL;
}

void* ldap_controls_free(void)
{
    if (verbose) puts("STUB: ldap_controls_free called");
    return NULL;
}

void* ldap_count_entries(void)
{
    if (verbose) puts("STUB: ldap_count_entries called");
    return NULL;
}

void* ldap_count_messages(void)
{
    if (verbose) puts("STUB: ldap_count_messages called");
    return NULL;
}

void* ldap_count_references(void)
{
    if (verbose) puts("STUB: ldap_count_references called");
    return NULL;
}

void* ldap_count_values(void)
{
    if (verbose) puts("STUB: ldap_count_values called");
    return NULL;
}

void* ldap_count_values_len(void)
{
    if (verbose) puts("STUB: ldap_count_values_len called");
    return NULL;
}

void* ldap_create(void)
{
    if (verbose) puts("STUB: ldap_create called");
    return NULL;
}

void* ldap_create_assertion_control_value(void)
{
    if (verbose) puts("STUB: ldap_create_assertion_control_value called");
    return NULL;
}

void* ldap_create_control(void)
{
    if (verbose) puts("STUB: ldap_create_control called");
    return NULL;
}

void* ldap_create_page_control(void)
{
    if (verbose) puts("STUB: ldap_create_page_control called");
    return NULL;
}

void* ldap_create_page_control_value(void)
{
    if (verbose) puts("STUB: ldap_create_page_control_value called");
    return NULL;
}

void* ldap_create_sort_control(void)
{
    if (verbose) puts("STUB: ldap_create_sort_control called");
    return NULL;
}

void* ldap_create_sort_keylist(void)
{
    if (verbose) puts("STUB: ldap_create_sort_keylist called");
    return NULL;
}

void* ldap_create_vlv_control(void)
{
    if (verbose) puts("STUB: ldap_create_vlv_control called");
    return NULL;
}

void* ldap_dcedn2dn(void)
{
    if (verbose) puts("STUB: ldap_dcedn2dn called");
    return NULL;
}

void* ldap_delete(void)
{
    if (verbose) puts("STUB: ldap_delete called");
    return NULL;
}

void* ldap_delete_ext(void)
{
    if (verbose) puts("STUB: ldap_delete_ext called");
    return NULL;
}

void* ldap_delete_ext_s(void)
{
    if (verbose) puts("STUB: ldap_delete_ext_s called");
    return NULL;
}

void* ldap_delete_result_entry(void)
{
    if (verbose) puts("STUB: ldap_delete_result_entry called");
    return NULL;
}

void* ldap_delete_s(void)
{
    if (verbose) puts("STUB: ldap_delete_s called");
    return NULL;
}

void* ldap_dn2ad_canonical(void)
{
    if (verbose) puts("STUB: ldap_dn2ad_canonical called");
    return NULL;
}

void* ldap_dn2bv(void)
{
    if (verbose) puts("STUB: ldap_dn2bv called");
    return NULL;
}

void* ldap_dn2dcedn(void)
{
    if (verbose) puts("STUB: ldap_dn2dcedn called");
    return NULL;
}

void* ldap_dn2domain(void)
{
    if (verbose) puts("STUB: ldap_dn2domain called");
    return NULL;
}

void* ldap_dn2str(void)
{
    if (verbose) puts("STUB: ldap_dn2str called");
    return NULL;
}

void* ldap_dn2ufn(void)
{
    if (verbose) puts("STUB: ldap_dn2ufn called");
    return NULL;
}

void* ldap_dn_normalize(void)
{
    if (verbose) puts("STUB: ldap_dn_normalize called");
    return NULL;
}

void* ldap_dnfree(void)
{
    if (verbose) puts("STUB: ldap_dnfree called");
    return NULL;
}

void* ldap_domain2dn(void)
{
    if (verbose) puts("STUB: ldap_domain2dn called");
    return NULL;
}

void* ldap_domain2hostlist(void)
{
    if (verbose) puts("STUB: ldap_domain2hostlist called");
    return NULL;
}

void* ldap_err2string(void)
{
    if (verbose) puts("STUB: ldap_err2string called");
    return NULL;
}

void* ldap_explode_dn(void)
{
    if (verbose) puts("STUB: ldap_explode_dn called");
    return NULL;
}

void* ldap_explode_rdn(void)
{
    if (verbose) puts("STUB: ldap_explode_rdn called");
    return NULL;
}

void* ldap_extended_operation(void)
{
    if (verbose) puts("STUB: ldap_extended_operation called");
    return NULL;
}

void* ldap_extended_operation_s(void)
{
    if (verbose) puts("STUB: ldap_extended_operation_s called");
    return NULL;
}

void* ldap_find_control(void)
{
    if (verbose) puts("STUB: ldap_find_control called");
    return NULL;
}

void* ldap_first_attribute(void)
{
    if (verbose) puts("STUB: ldap_first_attribute called");
    return NULL;
}

void* ldap_first_entry(void)
{
    if (verbose) puts("STUB: ldap_first_entry called");
    return NULL;
}

void* ldap_first_message(void)
{
    if (verbose) puts("STUB: ldap_first_message called");
    return NULL;
}

void* ldap_first_reference(void)
{
    if (verbose) puts("STUB: ldap_first_reference called");
    return NULL;
}

void* ldap_free_sort_keylist(void)
{
    if (verbose) puts("STUB: ldap_free_sort_keylist called");
    return NULL;
}

void* ldap_free_urldesc(void)
{
    if (verbose) puts("STUB: ldap_free_urldesc called");
    return NULL;
}

void* ldap_get_attribute_ber(void)
{
    if (verbose) puts("STUB: ldap_get_attribute_ber called");
    return NULL;
}

void* ldap_get_dn(void)
{
    if (verbose) puts("STUB: ldap_get_dn called");
    return NULL;
}

void* ldap_get_dn_ber(void)
{
    if (verbose) puts("STUB: ldap_get_dn_ber called");
    return NULL;
}

void* ldap_get_entry_controls(void)
{
    if (verbose) puts("STUB: ldap_get_entry_controls called");
    return NULL;
}

void* ldap_get_option(void)
{
    if (verbose) puts("STUB: ldap_get_option called");
    return NULL;
}

void* ldap_get_values(void)
{
    if (verbose) puts("STUB: ldap_get_values called");
    return NULL;
}

void* ldap_get_values_len(void)
{
    if (verbose) puts("STUB: ldap_get_values_len called");
    return NULL;
}

void* ldap_init(void)
{
    if (verbose) puts("STUB: ldap_init called");
    return NULL;
}

void* ldap_initialize(void)
{
    if (verbose) puts("STUB: ldap_initialize called");
    return NULL;
}

void* ldap_is_ldap_url(void)
{
    if (verbose) puts("STUB: ldap_is_ldap_url called");
    return NULL;
}

void* ldap_is_ldapi_url(void)
{
    if (verbose) puts("STUB: ldap_is_ldapi_url called");
    return NULL;
}

void* ldap_is_ldaps_url(void)
{
    if (verbose) puts("STUB: ldap_is_ldaps_url called");
    return NULL;
}

void* ldap_matchingrule2bv(void)
{
    if (verbose) puts("STUB: ldap_matchingrule2bv called");
    return NULL;
}

void* ldap_matchingrule2name(void)
{
    if (verbose) puts("STUB: ldap_matchingrule2name called");
    return NULL;
}

void* ldap_matchingrule2str(void)
{
    if (verbose) puts("STUB: ldap_matchingrule2str called");
    return NULL;
}

void* ldap_matchingrule_free(void)
{
    if (verbose) puts("STUB: ldap_matchingrule_free called");
    return NULL;
}

void* ldap_matchingruleuse2bv(void)
{
    if (verbose) puts("STUB: ldap_matchingruleuse2bv called");
    return NULL;
}

void* ldap_matchingruleuse2name(void)
{
    if (verbose) puts("STUB: ldap_matchingruleuse2name called");
    return NULL;
}

void* ldap_matchingruleuse2str(void)
{
    if (verbose) puts("STUB: ldap_matchingruleuse2str called");
    return NULL;
}

void* ldap_matchingruleuse_free(void)
{
    if (verbose) puts("STUB: ldap_matchingruleuse_free called");
    return NULL;
}

void* ldap_memalloc(void)
{
    if (verbose) puts("STUB: ldap_memalloc called");
    return NULL;
}

void* ldap_memcalloc(void)
{
    if (verbose) puts("STUB: ldap_memcalloc called");
    return NULL;
}

void* ldap_memfree(void)
{
    if (verbose) puts("STUB: ldap_memfree called");
    return NULL;
}

void* ldap_memrealloc(void)
{
    if (verbose) puts("STUB: ldap_memrealloc called");
    return NULL;
}

void* ldap_memvfree(void)
{
    if (verbose) puts("STUB: ldap_memvfree called");
    return NULL;
}

void* ldap_modify(void)
{
    if (verbose) puts("STUB: ldap_modify called");
    return NULL;
}

void* ldap_modify_ext(void)
{
    if (verbose) puts("STUB: ldap_modify_ext called");
    return NULL;
}

void* ldap_modify_ext_s(void)
{
    if (verbose) puts("STUB: ldap_modify_ext_s called");
    return NULL;
}

void* ldap_modify_s(void)
{
    if (verbose) puts("STUB: ldap_modify_s called");
    return NULL;
}

void* ldap_modrdn(void)
{
    if (verbose) puts("STUB: ldap_modrdn called");
    return NULL;
}

void* ldap_modrdn2(void)
{
    if (verbose) puts("STUB: ldap_modrdn2 called");
    return NULL;
}

void* ldap_modrdn2_s(void)
{
    if (verbose) puts("STUB: ldap_modrdn2_s called");
    return NULL;
}

void* ldap_modrdn_s(void)
{
    if (verbose) puts("STUB: ldap_modrdn_s called");
    return NULL;
}

void* ldap_mods_free(void)
{
    if (verbose) puts("STUB: ldap_mods_free called");
    return NULL;
}

void* ldap_msgdelete(void)
{
    if (verbose) puts("STUB: ldap_msgdelete called");
    return NULL;
}

void* ldap_msgfree(void)
{
    if (verbose) puts("STUB: ldap_msgfree called");
    return NULL;
}

void* ldap_msgid(void)
{
    if (verbose) puts("STUB: ldap_msgid called");
    return NULL;
}

void* ldap_msgtype(void)
{
    if (verbose) puts("STUB: ldap_msgtype called");
    return NULL;
}

void* ldap_nameform2bv(void)
{
    if (verbose) puts("STUB: ldap_nameform2bv called");
    return NULL;
}

void* ldap_nameform2name(void)
{
    if (verbose) puts("STUB: ldap_nameform2name called");
    return NULL;
}

void* ldap_nameform2str(void)
{
    if (verbose) puts("STUB: ldap_nameform2str called");
    return NULL;
}

void* ldap_nameform_free(void)
{
    if (verbose) puts("STUB: ldap_nameform_free called");
    return NULL;
}

void* ldap_next_attribute(void)
{
    if (verbose) puts("STUB: ldap_next_attribute called");
    return NULL;
}

void* ldap_next_entry(void)
{
    if (verbose) puts("STUB: ldap_next_entry called");
    return NULL;
}

void* ldap_next_message(void)
{
    if (verbose) puts("STUB: ldap_next_message called");
    return NULL;
}

void* ldap_next_reference(void)
{
    if (verbose) puts("STUB: ldap_next_reference called");
    return NULL;
}

void* ldap_objectclass2bv(void)
{
    if (verbose) puts("STUB: ldap_objectclass2bv called");
    return NULL;
}

void* ldap_objectclass2name(void)
{
    if (verbose) puts("STUB: ldap_objectclass2name called");
    return NULL;
}

void* ldap_objectclass2str(void)
{
    if (verbose) puts("STUB: ldap_objectclass2str called");
    return NULL;
}

void* ldap_objectclass_free(void)
{
    if (verbose) puts("STUB: ldap_objectclass_free called");
    return NULL;
}

void* ldap_open(void)
{
    if (verbose) puts("STUB: ldap_open called");
    return NULL;
}

void* ldap_parse_extended_result(void)
{
    if (verbose) puts("STUB: ldap_parse_extended_result called");
    return NULL;
}

void* ldap_parse_intermediate(void)
{
    if (verbose) puts("STUB: ldap_parse_intermediate called");
    return NULL;
}

void* ldap_parse_page_control(void)
{
    if (verbose) puts("STUB: ldap_parse_page_control called");
    return NULL;
}

void* ldap_parse_pageresponse_control(void)
{
    if (verbose) puts("STUB: ldap_parse_pageresponse_control called");
    return NULL;
}

void* ldap_parse_passwd(void)
{
    if (verbose) puts("STUB: ldap_parse_passwd called");
    return NULL;
}

void* ldap_parse_reference(void)
{
    if (verbose) puts("STUB: ldap_parse_reference called");
    return NULL;
}

void* ldap_parse_result(void)
{
    if (verbose) puts("STUB: ldap_parse_result called");
    return NULL;
}

void* ldap_parse_sasl_bind_result(void)
{
    if (verbose) puts("STUB: ldap_parse_sasl_bind_result called");
    return NULL;
}

void* ldap_parse_sort_control(void)
{
    if (verbose) puts("STUB: ldap_parse_sort_control called");
    return NULL;
}

void* ldap_parse_sortresponse_control(void)
{
    if (verbose) puts("STUB: ldap_parse_sortresponse_control called");
    return NULL;
}

void* ldap_parse_vlv_control(void)
{
    if (verbose) puts("STUB: ldap_parse_vlv_control called");
    return NULL;
}

void* ldap_parse_vlvresponse_control(void)
{
    if (verbose) puts("STUB: ldap_parse_vlvresponse_control called");
    return NULL;
}

void* ldap_parse_whoami(void)
{
    if (verbose) puts("STUB: ldap_parse_whoami called");
    return NULL;
}

void* ldap_passwd(void)
{
    if (verbose) puts("STUB: ldap_passwd called");
    return NULL;
}

void* ldap_passwd_s(void)
{
    if (verbose) puts("STUB: ldap_passwd_s called");
    return NULL;
}

void* ldap_perror(void)
{
    if (verbose) puts("STUB: ldap_perror called");
    return NULL;
}

void* ldap_put_vrFilter(void)
{
    if (verbose) puts("STUB: ldap_put_vrFilter called");
    return NULL;
}

void* ldap_rdn2bv(void)
{
    if (verbose) puts("STUB: ldap_rdn2bv called");
    return NULL;
}

void* ldap_rdn2str(void)
{
    if (verbose) puts("STUB: ldap_rdn2str called");
    return NULL;
}

void* ldap_rdnfree(void)
{
    if (verbose) puts("STUB: ldap_rdnfree called");
    return NULL;
}

void* ldap_rename(void)
{
    if (verbose) puts("STUB: ldap_rename called");
    return NULL;
}

void* ldap_rename2(void)
{
    if (verbose) puts("STUB: ldap_rename2 called");
    return NULL;
}

void* ldap_rename2_s(void)
{
    if (verbose) puts("STUB: ldap_rename2_s called");
    return NULL;
}

void* ldap_rename_s(void)
{
    if (verbose) puts("STUB: ldap_rename_s called");
    return NULL;
}

void* ldap_result(void)
{
    if (verbose) puts("STUB: ldap_result called");
    return NULL;
}

void* ldap_result2error(void)
{
    if (verbose) puts("STUB: ldap_result2error called");
    return NULL;
}

void* ldap_sasl_bind(void)
{
    if (verbose) puts("STUB: ldap_sasl_bind called");
    return NULL;
}

void* ldap_sasl_bind_s(void)
{
    if (verbose) puts("STUB: ldap_sasl_bind_s called");
    return NULL;
}

void* ldap_sasl_interactive_bind_s(void)
{
    if (verbose) puts("STUB: ldap_sasl_interactive_bind_s called");
    return NULL;
}

void* ldap_scherr2str(void)
{
    if (verbose) puts("STUB: ldap_scherr2str called");
    return NULL;
}

void* ldap_search(void)
{
    if (verbose) puts("STUB: ldap_search called");
    return NULL;
}

void* ldap_search_ext(void)
{
    if (verbose) puts("STUB: ldap_search_ext called");
    return NULL;
}

void* ldap_search_ext_s(void)
{
    if (verbose) puts("STUB: ldap_search_ext_s called");
    return NULL;
}

void* ldap_search_s(void)
{
    if (verbose) puts("STUB: ldap_search_s called");
    return NULL;
}

void* ldap_search_st(void)
{
    if (verbose) puts("STUB: ldap_search_st called");
    return NULL;
}

void* ldap_set_option(void)
{
    if (verbose) puts("STUB: ldap_set_option called");
    return NULL;
}

void* ldap_set_rebind_proc(void)
{
    if (verbose) puts("STUB: ldap_set_rebind_proc called");
    return NULL;
}

void* ldap_set_search_results_callback(void)
{
    if (verbose) puts("STUB: ldap_set_search_results_callback called");
    return NULL;
}

void* ldap_simple_bind(void)
{
    if (verbose) puts("STUB: ldap_simple_bind called");
    return NULL;
}

void* ldap_simple_bind_s(void)
{
    if (verbose) puts("STUB: ldap_simple_bind_s called");
    return NULL;
}

void* ldap_sort_entries(void)
{
    if (verbose) puts("STUB: ldap_sort_entries called");
    return NULL;
}

void* ldap_sort_strcasecmp(void)
{
    if (verbose) puts("STUB: ldap_sort_strcasecmp called");
    return NULL;
}

void* ldap_sort_values(void)
{
    if (verbose) puts("STUB: ldap_sort_values called");
    return NULL;
}

void* ldap_start_tls_s(void)
{
    if (verbose) puts("STUB: ldap_start_tls_s called");
    return NULL;
}

void* ldap_str2attributetype(void)
{
    if (verbose) puts("STUB: ldap_str2attributetype called");
    return NULL;
}

void* ldap_str2contentrule(void)
{
    if (verbose) puts("STUB: ldap_str2contentrule called");
    return NULL;
}

void* ldap_str2dn(void)
{
    if (verbose) puts("STUB: ldap_str2dn called");
    return NULL;
}

void* ldap_str2matchingrule(void)
{
    if (verbose) puts("STUB: ldap_str2matchingrule called");
    return NULL;
}

void* ldap_str2matchingruleuse(void)
{
    if (verbose) puts("STUB: ldap_str2matchingruleuse called");
    return NULL;
}

void* ldap_str2nameform(void)
{
    if (verbose) puts("STUB: ldap_str2nameform called");
    return NULL;
}

void* ldap_str2objectclass(void)
{
    if (verbose) puts("STUB: ldap_str2objectclass called");
    return NULL;
}

void* ldap_str2rdn(void)
{
    if (verbose) puts("STUB: ldap_str2rdn called");
    return NULL;
}

void* ldap_str2structurerule(void)
{
    if (verbose) puts("STUB: ldap_str2structurerule called");
    return NULL;
}

void* ldap_str2syntax(void)
{
    if (verbose) puts("STUB: ldap_str2syntax called");
    return NULL;
}

void* ldap_strdup(void)
{
    if (verbose) puts("STUB: ldap_strdup called");
    return NULL;
}

void* ldap_structurerule2bv(void)
{
    if (verbose) puts("STUB: ldap_structurerule2bv called");
    return NULL;
}

void* ldap_structurerule2name(void)
{
    if (verbose) puts("STUB: ldap_structurerule2name called");
    return NULL;
}

void* ldap_structurerule2str(void)
{
    if (verbose) puts("STUB: ldap_structurerule2str called");
    return NULL;
}

void* ldap_structurerule_free(void)
{
    if (verbose) puts("STUB: ldap_structurerule_free called");
    return NULL;
}

void* ldap_syntax2bv(void)
{
    if (verbose) puts("STUB: ldap_syntax2bv called");
    return NULL;
}

void* ldap_syntax2name(void)
{
    if (verbose) puts("STUB: ldap_syntax2name called");
    return NULL;
}

void* ldap_syntax2str(void)
{
    if (verbose) puts("STUB: ldap_syntax2str called");
    return NULL;
}

void* ldap_syntax_free(void)
{
    if (verbose) puts("STUB: ldap_syntax_free called");
    return NULL;
}

void* ldap_unbind(void)
{
    if (verbose) puts("STUB: ldap_unbind called");
    return NULL;
}

void* ldap_unbind_ext(void)
{
    if (verbose) puts("STUB: ldap_unbind_ext called");
    return NULL;
}

void* ldap_unbind_ext_s(void)
{
    if (verbose) puts("STUB: ldap_unbind_ext_s called");
    return NULL;
}

void* ldap_unbind_s(void)
{
    if (verbose) puts("STUB: ldap_unbind_s called");
    return NULL;
}

void* ldap_url_desc2str(void)
{
    if (verbose) puts("STUB: ldap_url_desc2str called");
    return NULL;
}

void* ldap_url_parse(void)
{
    if (verbose) puts("STUB: ldap_url_parse called");
    return NULL;
}

void* ldap_value_free(void)
{
    if (verbose) puts("STUB: ldap_value_free called");
    return NULL;
}

void* ldap_value_free_len(void)
{
    if (verbose) puts("STUB: ldap_value_free_len called");
    return NULL;
}

void* ldap_whoami(void)
{
    if (verbose) puts("STUB: ldap_whoami called");
    return NULL;
}

void* ldap_whoami_s(void)
{
    if (verbose) puts("STUB: ldap_whoami_s called");
    return NULL;
}

void* ldap_x_mb_to_utf8(void)
{
    if (verbose) puts("STUB: ldap_x_mb_to_utf8 called");
    return NULL;
}

void* ldap_x_mbs_to_utf8s(void)
{
    if (verbose) puts("STUB: ldap_x_mbs_to_utf8s called");
    return NULL;
}

void* ldap_x_ucs4_to_utf8(void)
{
    if (verbose) puts("STUB: ldap_x_ucs4_to_utf8 called");
    return NULL;
}

void* ldap_x_utf8_to_mb(void)
{
    if (verbose) puts("STUB: ldap_x_utf8_to_mb called");
    return NULL;
}

void* ldap_x_utf8_to_ucs4(void)
{
    if (verbose) puts("STUB: ldap_x_utf8_to_ucs4 called");
    return NULL;
}

void* ldap_x_utf8_to_wc(void)
{
    if (verbose) puts("STUB: ldap_x_utf8_to_wc called");
    return NULL;
}

void* ldap_x_utf8s_to_mbs(void)
{
    if (verbose) puts("STUB: ldap_x_utf8s_to_mbs called");
    return NULL;
}

void* ldap_x_utf8s_to_wcs(void)
{
    if (verbose) puts("STUB: ldap_x_utf8s_to_wcs called");
    return NULL;
}

void* ldap_x_wc_to_utf8(void)
{
    if (verbose) puts("STUB: ldap_x_wc_to_utf8 called");
    return NULL;
}

void* ldap_x_wcs_to_utf8s(void)
{
    if (verbose) puts("STUB: ldap_x_wcs_to_utf8s called");
    return NULL;
}

void* ldif_close(void)
{
    if (verbose) puts("STUB: ldif_close called");
    return NULL;
}

void* ldif_countlines(void)
{
    if (verbose) puts("STUB: ldif_countlines called");
    return NULL;
}

void* ldif_fetch_url(void)
{
    if (verbose) puts("STUB: ldif_fetch_url called");
    return NULL;
}

void* ldif_getline(void)
{
    if (verbose) puts("STUB: ldif_getline called");
    return NULL;
}

void* ldif_is_not_printable(void)
{
    if (verbose) puts("STUB: ldif_is_not_printable called");
    return NULL;
}

void* ldif_must_b64_encode_register(void)
{
    if (verbose) puts("STUB: ldif_must_b64_encode_register called");
    return NULL;
}

void* ldif_must_b64_encode_release(void)
{
    if (verbose) puts("STUB: ldif_must_b64_encode_release called");
    return NULL;
}

void* ldif_open(void)
{
    if (verbose) puts("STUB: ldif_open called");
    return NULL;
}

void* ldif_open_url(void)
{
    if (verbose) puts("STUB: ldif_open_url called");
    return NULL;
}

void* ldif_parse_line(void)
{
    if (verbose) puts("STUB: ldif_parse_line called");
    return NULL;
}

void* ldif_parse_line2(void)
{
    if (verbose) puts("STUB: ldif_parse_line2 called");
    return NULL;
}

void* ldif_put(void)
{
    if (verbose) puts("STUB: ldif_put called");
    return NULL;
}

void* ldif_put_wrap(void)
{
    if (verbose) puts("STUB: ldif_put_wrap called");
    return NULL;
}

void* ldif_read_record(void)
{
    if (verbose) puts("STUB: ldif_read_record called");
    return NULL;
}

void* ldif_sput(void)
{
    if (verbose) puts("STUB: ldif_sput called");
    return NULL;
}

void* ldif_sput_wrap(void)
{
    if (verbose) puts("STUB: ldif_sput_wrap called");
    return NULL;
}
