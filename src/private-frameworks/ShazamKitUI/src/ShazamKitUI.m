/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

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


#include <ShazamKitUI/ShazamKitUI.h>
#include <stdlib.h>
#include <stdio.h>

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


extern const void* ShazamKitUIVersionString = (void*)0;
extern const void* ShazamKitUIVersionNumber = (void*)0;
extern const void* __swift_reflection_version = (void*)0;

void *__swift_memcpy16_8(void) {
    if (verbose) puts("STUB: __swift_memcpy16_8 called");
    return NULL;
)

void *__swift_noop_void_return(void) {
    if (verbose) puts("STUB: __swift_noop_void_return called");
    return NULL;
)

void *__swift_instantiateConcreteTypeFromMangledNameAbstract(void) {
    if (verbose) puts("STUB: __swift_instantiateConcreteTypeFromMangledNameAbstract called");
    return NULL;
)

void *__swift_instantiateConcreteTypeFromMangledName(void) {
    if (verbose) puts("STUB: __swift_instantiateConcreteTypeFromMangledName called");
    return NULL;
)

void *__swift_memcpy17_8(void) {
    if (verbose) puts("STUB: __swift_memcpy17_8 called");
    return NULL;
)

void *__swift_memcpy40_8(void) {
    if (verbose) puts("STUB: __swift_memcpy40_8 called");
    return NULL;
)

void *__swift_memcpy24_8(void) {
    if (verbose) puts("STUB: __swift_memcpy24_8 called");
    return NULL;
)

void *__chkstk_darwin(void) {
    if (verbose) puts("STUB: __chkstk_darwin called");
    return NULL;
)

void *__chkstk_darwin_llvm_probe(void) {
    if (verbose) puts("STUB: __chkstk_darwin_llvm_probe called");
    return NULL;
)

void *__swift_project_boxed_opaque_existential_1(void) {
    if (verbose) puts("STUB: __swift_project_boxed_opaque_existential_1 called");
    return NULL;
)

void *__swift_destroy_boxed_opaque_existential_1(void) {
    if (verbose) puts("STUB: __swift_destroy_boxed_opaque_existential_1 called");
    return NULL;
)

void *__swift_memcpy74_8(void) {
    if (verbose) puts("STUB: __swift_memcpy74_8 called");
    return NULL;
)

