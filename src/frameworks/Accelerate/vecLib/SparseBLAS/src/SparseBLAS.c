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


#include <SparseBLAS/SparseBLAS.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* sparse_commit(void)
{
    if (verbose) puts("STUB: sparse_commit called");
    return NULL;
}

void* sparse_elementwise_norm_double(void)
{
    if (verbose) puts("STUB: sparse_elementwise_norm_double called");
    return NULL;
}

void* sparse_elementwise_norm_float(void)
{
    if (verbose) puts("STUB: sparse_elementwise_norm_float called");
    return NULL;
}

void* sparse_extract_block_double(void)
{
    if (verbose) puts("STUB: sparse_extract_block_double called");
    return NULL;
}

void* sparse_extract_block_float(void)
{
    if (verbose) puts("STUB: sparse_extract_block_float called");
    return NULL;
}

void* sparse_extract_sparse_column_double(void)
{
    if (verbose) puts("STUB: sparse_extract_sparse_column_double called");
    return NULL;
}

void* sparse_extract_sparse_column_float(void)
{
    if (verbose) puts("STUB: sparse_extract_sparse_column_float called");
    return NULL;
}

void* sparse_extract_sparse_row_double(void)
{
    if (verbose) puts("STUB: sparse_extract_sparse_row_double called");
    return NULL;
}

void* sparse_extract_sparse_row_float(void)
{
    if (verbose) puts("STUB: sparse_extract_sparse_row_float called");
    return NULL;
}

void* sparse_get_block_dimension_for_col(void)
{
    if (verbose) puts("STUB: sparse_get_block_dimension_for_col called");
    return NULL;
}

void* sparse_get_block_dimension_for_row(void)
{
    if (verbose) puts("STUB: sparse_get_block_dimension_for_row called");
    return NULL;
}

void* sparse_get_matrix_nonzero_count(void)
{
    if (verbose) puts("STUB: sparse_get_matrix_nonzero_count called");
    return NULL;
}

void* sparse_get_matrix_nonzero_count_for_column(void)
{
    if (verbose) puts("STUB: sparse_get_matrix_nonzero_count_for_column called");
    return NULL;
}

void* sparse_get_matrix_nonzero_count_for_row(void)
{
    if (verbose) puts("STUB: sparse_get_matrix_nonzero_count_for_row called");
    return NULL;
}

void* sparse_get_matrix_number_of_columns(void)
{
    if (verbose) puts("STUB: sparse_get_matrix_number_of_columns called");
    return NULL;
}

void* sparse_get_matrix_number_of_rows(void)
{
    if (verbose) puts("STUB: sparse_get_matrix_number_of_rows called");
    return NULL;
}

void* sparse_get_matrix_property(void)
{
    if (verbose) puts("STUB: sparse_get_matrix_property called");
    return NULL;
}

void* sparse_get_vector_nonzero_count_double(void)
{
    if (verbose) puts("STUB: sparse_get_vector_nonzero_count_double called");
    return NULL;
}

void* sparse_get_vector_nonzero_count_float(void)
{
    if (verbose) puts("STUB: sparse_get_vector_nonzero_count_float called");
    return NULL;
}

void* sparse_inner_product_dense_double(void)
{
    if (verbose) puts("STUB: sparse_inner_product_dense_double called");
    return NULL;
}

void* sparse_inner_product_dense_float(void)
{
    if (verbose) puts("STUB: sparse_inner_product_dense_float called");
    return NULL;
}

void* sparse_inner_product_sparse_double(void)
{
    if (verbose) puts("STUB: sparse_inner_product_sparse_double called");
    return NULL;
}

void* sparse_inner_product_sparse_float(void)
{
    if (verbose) puts("STUB: sparse_inner_product_sparse_float called");
    return NULL;
}

void* sparse_insert_block_double(void)
{
    if (verbose) puts("STUB: sparse_insert_block_double called");
    return NULL;
}

void* sparse_insert_block_float(void)
{
    if (verbose) puts("STUB: sparse_insert_block_float called");
    return NULL;
}

void* sparse_insert_col_double(void)
{
    if (verbose) puts("STUB: sparse_insert_col_double called");
    return NULL;
}

void* sparse_insert_col_float(void)
{
    if (verbose) puts("STUB: sparse_insert_col_float called");
    return NULL;
}

void* sparse_insert_entries_double(void)
{
    if (verbose) puts("STUB: sparse_insert_entries_double called");
    return NULL;
}

void* sparse_insert_entries_float(void)
{
    if (verbose) puts("STUB: sparse_insert_entries_float called");
    return NULL;
}

void* sparse_insert_entry_double(void)
{
    if (verbose) puts("STUB: sparse_insert_entry_double called");
    return NULL;
}

void* sparse_insert_entry_float(void)
{
    if (verbose) puts("STUB: sparse_insert_entry_float called");
    return NULL;
}

void* sparse_insert_row_double(void)
{
    if (verbose) puts("STUB: sparse_insert_row_double called");
    return NULL;
}

void* sparse_insert_row_float(void)
{
    if (verbose) puts("STUB: sparse_insert_row_float called");
    return NULL;
}

void* sparse_matrix_block_create_double(void)
{
    if (verbose) puts("STUB: sparse_matrix_block_create_double called");
    return NULL;
}

void* sparse_matrix_block_create_float(void)
{
    if (verbose) puts("STUB: sparse_matrix_block_create_float called");
    return NULL;
}

void* sparse_matrix_create_double(void)
{
    if (verbose) puts("STUB: sparse_matrix_create_double called");
    return NULL;
}

void* sparse_matrix_create_float(void)
{
    if (verbose) puts("STUB: sparse_matrix_create_float called");
    return NULL;
}

void* sparse_matrix_destroy(void)
{
    if (verbose) puts("STUB: sparse_matrix_destroy called");
    return NULL;
}

void* sparse_matrix_product_dense_double(void)
{
    if (verbose) puts("STUB: sparse_matrix_product_dense_double called");
    return NULL;
}

void* sparse_matrix_product_dense_float(void)
{
    if (verbose) puts("STUB: sparse_matrix_product_dense_float called");
    return NULL;
}

void* sparse_matrix_product_sparse_double(void)
{
    if (verbose) puts("STUB: sparse_matrix_product_sparse_double called");
    return NULL;
}

void* sparse_matrix_product_sparse_float(void)
{
    if (verbose) puts("STUB: sparse_matrix_product_sparse_float called");
    return NULL;
}

void* sparse_matrix_trace_double(void)
{
    if (verbose) puts("STUB: sparse_matrix_trace_double called");
    return NULL;
}

void* sparse_matrix_trace_float(void)
{
    if (verbose) puts("STUB: sparse_matrix_trace_float called");
    return NULL;
}

void* sparse_matrix_triangular_solve_dense_double(void)
{
    if (verbose) puts("STUB: sparse_matrix_triangular_solve_dense_double called");
    return NULL;
}

void* sparse_matrix_triangular_solve_dense_float(void)
{
    if (verbose) puts("STUB: sparse_matrix_triangular_solve_dense_float called");
    return NULL;
}

void* sparse_matrix_variable_block_create_double(void)
{
    if (verbose) puts("STUB: sparse_matrix_variable_block_create_double called");
    return NULL;
}

void* sparse_matrix_variable_block_create_float(void)
{
    if (verbose) puts("STUB: sparse_matrix_variable_block_create_float called");
    return NULL;
}

void* sparse_matrix_vector_product_dense_double(void)
{
    if (verbose) puts("STUB: sparse_matrix_vector_product_dense_double called");
    return NULL;
}

void* sparse_matrix_vector_product_dense_float(void)
{
    if (verbose) puts("STUB: sparse_matrix_vector_product_dense_float called");
    return NULL;
}

void* sparse_operator_norm_double(void)
{
    if (verbose) puts("STUB: sparse_operator_norm_double called");
    return NULL;
}

void* sparse_operator_norm_float(void)
{
    if (verbose) puts("STUB: sparse_operator_norm_float called");
    return NULL;
}

void* sparse_outer_product_dense_double(void)
{
    if (verbose) puts("STUB: sparse_outer_product_dense_double called");
    return NULL;
}

void* sparse_outer_product_dense_float(void)
{
    if (verbose) puts("STUB: sparse_outer_product_dense_float called");
    return NULL;
}

void* sparse_pack_vector_double(void)
{
    if (verbose) puts("STUB: sparse_pack_vector_double called");
    return NULL;
}

void* sparse_pack_vector_float(void)
{
    if (verbose) puts("STUB: sparse_pack_vector_float called");
    return NULL;
}

void* sparse_permute_cols_double(void)
{
    if (verbose) puts("STUB: sparse_permute_cols_double called");
    return NULL;
}

void* sparse_permute_cols_float(void)
{
    if (verbose) puts("STUB: sparse_permute_cols_float called");
    return NULL;
}

void* sparse_permute_rows_double(void)
{
    if (verbose) puts("STUB: sparse_permute_rows_double called");
    return NULL;
}

void* sparse_permute_rows_float(void)
{
    if (verbose) puts("STUB: sparse_permute_rows_float called");
    return NULL;
}

void* sparse_set_matrix_property(void)
{
    if (verbose) puts("STUB: sparse_set_matrix_property called");
    return NULL;
}

void* sparse_unpack_vector_double(void)
{
    if (verbose) puts("STUB: sparse_unpack_vector_double called");
    return NULL;
}

void* sparse_unpack_vector_float(void)
{
    if (verbose) puts("STUB: sparse_unpack_vector_float called");
    return NULL;
}

void* sparse_vector_add_with_scale_dense_double(void)
{
    if (verbose) puts("STUB: sparse_vector_add_with_scale_dense_double called");
    return NULL;
}

void* sparse_vector_add_with_scale_dense_float(void)
{
    if (verbose) puts("STUB: sparse_vector_add_with_scale_dense_float called");
    return NULL;
}

void* sparse_vector_norm_double(void)
{
    if (verbose) puts("STUB: sparse_vector_norm_double called");
    return NULL;
}

void* sparse_vector_norm_float(void)
{
    if (verbose) puts("STUB: sparse_vector_norm_float called");
    return NULL;
}

void* sparse_vector_triangular_solve_dense_double(void)
{
    if (verbose) puts("STUB: sparse_vector_triangular_solve_dense_double called");
    return NULL;
}

void* sparse_vector_triangular_solve_dense_float(void)
{
    if (verbose) puts("STUB: sparse_vector_triangular_solve_dense_float called");
    return NULL;
}
