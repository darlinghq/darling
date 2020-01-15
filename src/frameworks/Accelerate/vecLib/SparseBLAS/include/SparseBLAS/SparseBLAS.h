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


#ifndef _SparseBLAS_H_
#define _SparseBLAS_H_

void* sparse_commit(void);
void* sparse_elementwise_norm_double(void);
void* sparse_elementwise_norm_float(void);
void* sparse_extract_block_double(void);
void* sparse_extract_block_float(void);
void* sparse_extract_sparse_column_double(void);
void* sparse_extract_sparse_column_float(void);
void* sparse_extract_sparse_row_double(void);
void* sparse_extract_sparse_row_float(void);
void* sparse_get_block_dimension_for_col(void);
void* sparse_get_block_dimension_for_row(void);
void* sparse_get_matrix_nonzero_count(void);
void* sparse_get_matrix_nonzero_count_for_column(void);
void* sparse_get_matrix_nonzero_count_for_row(void);
void* sparse_get_matrix_number_of_columns(void);
void* sparse_get_matrix_number_of_rows(void);
void* sparse_get_matrix_property(void);
void* sparse_get_vector_nonzero_count_double(void);
void* sparse_get_vector_nonzero_count_float(void);
void* sparse_inner_product_dense_double(void);
void* sparse_inner_product_dense_float(void);
void* sparse_inner_product_sparse_double(void);
void* sparse_inner_product_sparse_float(void);
void* sparse_insert_block_double(void);
void* sparse_insert_block_float(void);
void* sparse_insert_col_double(void);
void* sparse_insert_col_float(void);
void* sparse_insert_entries_double(void);
void* sparse_insert_entries_float(void);
void* sparse_insert_entry_double(void);
void* sparse_insert_entry_float(void);
void* sparse_insert_row_double(void);
void* sparse_insert_row_float(void);
void* sparse_matrix_block_create_double(void);
void* sparse_matrix_block_create_float(void);
void* sparse_matrix_create_double(void);
void* sparse_matrix_create_float(void);
void* sparse_matrix_destroy(void);
void* sparse_matrix_product_dense_double(void);
void* sparse_matrix_product_dense_float(void);
void* sparse_matrix_product_sparse_double(void);
void* sparse_matrix_product_sparse_float(void);
void* sparse_matrix_trace_double(void);
void* sparse_matrix_trace_float(void);
void* sparse_matrix_triangular_solve_dense_double(void);
void* sparse_matrix_triangular_solve_dense_float(void);
void* sparse_matrix_variable_block_create_double(void);
void* sparse_matrix_variable_block_create_float(void);
void* sparse_matrix_vector_product_dense_double(void);
void* sparse_matrix_vector_product_dense_float(void);
void* sparse_operator_norm_double(void);
void* sparse_operator_norm_float(void);
void* sparse_outer_product_dense_double(void);
void* sparse_outer_product_dense_float(void);
void* sparse_pack_vector_double(void);
void* sparse_pack_vector_float(void);
void* sparse_permute_cols_double(void);
void* sparse_permute_cols_float(void);
void* sparse_permute_rows_double(void);
void* sparse_permute_rows_float(void);
void* sparse_set_matrix_property(void);
void* sparse_unpack_vector_double(void);
void* sparse_unpack_vector_float(void);
void* sparse_vector_add_with_scale_dense_double(void);
void* sparse_vector_add_with_scale_dense_float(void);
void* sparse_vector_norm_double(void);
void* sparse_vector_norm_float(void);
void* sparse_vector_triangular_solve_dense_double(void);
void* sparse_vector_triangular_solve_dense_float(void);

#endif
