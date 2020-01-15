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


#ifndef _LinearAlgebra_H_
#define _LinearAlgebra_H_

#import <Foundation/Foundation.h>

#import <LinearAlgebra/OS_la_object.h>
#import <LinearAlgebra/OS_la_object.h>

void* la_add_attributes(void);
void* la_diagonal_matrix_from_vector(void);
void* la_difference(void);
void* la_elementwise_product(void);
void* la_identity_matrix(void);
void* la_inner_product(void);
void* la_matrix_cols(void);
void* la_matrix_from_double_buffer(void);
void* la_matrix_from_double_buffer_nocopy(void);
void* la_matrix_from_float_buffer(void);
void* la_matrix_from_float_buffer_nocopy(void);
void* la_matrix_from_splat(void);
void* la_matrix_product(void);
void* la_matrix_rows(void);
void* la_matrix_slice(void);
void* la_matrix_to_double_buffer(void);
void* la_matrix_to_float_buffer(void);
void* la_norm_as_double(void);
void* la_norm_as_float(void);
void* la_normalized_vector(void);
void* la_outer_product(void);
void* la_release(void);
void* la_remove_attributes(void);
void* la_retain(void);
void* la_scale_with_double(void);
void* la_scale_with_float(void);
void* la_solve(void);
void* la_splat_from_double(void);
void* la_splat_from_float(void);
void* la_splat_from_matrix_element(void);
void* la_splat_from_vector_element(void);
void* la_status(void);
void* la_sum(void);
void* la_transpose(void);
void* la_vector_from_matrix_col(void);
void* la_vector_from_matrix_diagonal(void);
void* la_vector_from_matrix_row(void);
void* la_vector_from_splat(void);
void* la_vector_length(void);
void* la_vector_slice(void);
void* la_vector_to_double_buffer(void);
void* la_vector_to_float_buffer(void);

#endif
