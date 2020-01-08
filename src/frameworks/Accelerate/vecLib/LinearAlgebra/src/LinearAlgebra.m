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


#include <LinearAlgebra/LinearAlgebra.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* la_add_attributes(void)
{
    if (verbose) puts("STUB: la_add_attributes called");
    return NULL;
}

void* la_diagonal_matrix_from_vector(void)
{
    if (verbose) puts("STUB: la_diagonal_matrix_from_vector called");
    return NULL;
}

void* la_difference(void)
{
    if (verbose) puts("STUB: la_difference called");
    return NULL;
}

void* la_elementwise_product(void)
{
    if (verbose) puts("STUB: la_elementwise_product called");
    return NULL;
}

void* la_identity_matrix(void)
{
    if (verbose) puts("STUB: la_identity_matrix called");
    return NULL;
}

void* la_inner_product(void)
{
    if (verbose) puts("STUB: la_inner_product called");
    return NULL;
}

void* la_matrix_cols(void)
{
    if (verbose) puts("STUB: la_matrix_cols called");
    return NULL;
}

void* la_matrix_from_double_buffer(void)
{
    if (verbose) puts("STUB: la_matrix_from_double_buffer called");
    return NULL;
}

void* la_matrix_from_double_buffer_nocopy(void)
{
    if (verbose) puts("STUB: la_matrix_from_double_buffer_nocopy called");
    return NULL;
}

void* la_matrix_from_float_buffer(void)
{
    if (verbose) puts("STUB: la_matrix_from_float_buffer called");
    return NULL;
}

void* la_matrix_from_float_buffer_nocopy(void)
{
    if (verbose) puts("STUB: la_matrix_from_float_buffer_nocopy called");
    return NULL;
}

void* la_matrix_from_splat(void)
{
    if (verbose) puts("STUB: la_matrix_from_splat called");
    return NULL;
}

void* la_matrix_product(void)
{
    if (verbose) puts("STUB: la_matrix_product called");
    return NULL;
}

void* la_matrix_rows(void)
{
    if (verbose) puts("STUB: la_matrix_rows called");
    return NULL;
}

void* la_matrix_slice(void)
{
    if (verbose) puts("STUB: la_matrix_slice called");
    return NULL;
}

void* la_matrix_to_double_buffer(void)
{
    if (verbose) puts("STUB: la_matrix_to_double_buffer called");
    return NULL;
}

void* la_matrix_to_float_buffer(void)
{
    if (verbose) puts("STUB: la_matrix_to_float_buffer called");
    return NULL;
}

void* la_norm_as_double(void)
{
    if (verbose) puts("STUB: la_norm_as_double called");
    return NULL;
}

void* la_norm_as_float(void)
{
    if (verbose) puts("STUB: la_norm_as_float called");
    return NULL;
}

void* la_normalized_vector(void)
{
    if (verbose) puts("STUB: la_normalized_vector called");
    return NULL;
}

void* la_outer_product(void)
{
    if (verbose) puts("STUB: la_outer_product called");
    return NULL;
}

void* la_release(void)
{
    if (verbose) puts("STUB: la_release called");
    return NULL;
}

void* la_remove_attributes(void)
{
    if (verbose) puts("STUB: la_remove_attributes called");
    return NULL;
}

void* la_retain(void)
{
    if (verbose) puts("STUB: la_retain called");
    return NULL;
}

void* la_scale_with_double(void)
{
    if (verbose) puts("STUB: la_scale_with_double called");
    return NULL;
}

void* la_scale_with_float(void)
{
    if (verbose) puts("STUB: la_scale_with_float called");
    return NULL;
}

void* la_solve(void)
{
    if (verbose) puts("STUB: la_solve called");
    return NULL;
}

void* la_splat_from_double(void)
{
    if (verbose) puts("STUB: la_splat_from_double called");
    return NULL;
}

void* la_splat_from_float(void)
{
    if (verbose) puts("STUB: la_splat_from_float called");
    return NULL;
}

void* la_splat_from_matrix_element(void)
{
    if (verbose) puts("STUB: la_splat_from_matrix_element called");
    return NULL;
}

void* la_splat_from_vector_element(void)
{
    if (verbose) puts("STUB: la_splat_from_vector_element called");
    return NULL;
}

void* la_status(void)
{
    if (verbose) puts("STUB: la_status called");
    return NULL;
}

void* la_sum(void)
{
    if (verbose) puts("STUB: la_sum called");
    return NULL;
}

void* la_transpose(void)
{
    if (verbose) puts("STUB: la_transpose called");
    return NULL;
}

void* la_vector_from_matrix_col(void)
{
    if (verbose) puts("STUB: la_vector_from_matrix_col called");
    return NULL;
}

void* la_vector_from_matrix_diagonal(void)
{
    if (verbose) puts("STUB: la_vector_from_matrix_diagonal called");
    return NULL;
}

void* la_vector_from_matrix_row(void)
{
    if (verbose) puts("STUB: la_vector_from_matrix_row called");
    return NULL;
}

void* la_vector_from_splat(void)
{
    if (verbose) puts("STUB: la_vector_from_splat called");
    return NULL;
}

void* la_vector_length(void)
{
    if (verbose) puts("STUB: la_vector_length called");
    return NULL;
}

void* la_vector_slice(void)
{
    if (verbose) puts("STUB: la_vector_slice called");
    return NULL;
}

void* la_vector_to_double_buffer(void)
{
    if (verbose) puts("STUB: la_vector_to_double_buffer called");
    return NULL;
}

void* la_vector_to_float_buffer(void)
{
    if (verbose) puts("STUB: la_vector_to_float_buffer called");
    return NULL;
}
