/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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
#ifndef VECTOR_MAKE_H
#define VECTOR_MAKE_H

#include <simd/base.h>
#include <simd/vector_types.h>

/*
 * simd_make_<type>N: build an N-element vector from scalars and smaller
 * vectors whose elements add up to N, or from a single scalar or vector of
 * any size: a larger one is truncated, and a scalar or smaller one is
 * extended with zeros.
 */

#define SIMD_MAKE(T, S)                                                       \
SIMD_FN simd_##T##2 simd_make_##T##2(S x, S y) { return (simd_##T##2){x, y}; }\
SIMD_FN simd_##T##2 simd_make_##T##2(S x) { return (simd_##T##2){x, 0}; }     \
SIMD_FN simd_##T##2 simd_make_##T##2(simd_##T##2 v) { return v; }             \
SIMD_FN simd_##T##2 simd_make_##T##2(simd_##T##3 v) { return v.xy; }          \
SIMD_FN simd_##T##2 simd_make_##T##2(simd_##T##4 v) { return v.xy; }          \
SIMD_FN simd_##T##3 simd_make_##T##3(S x, S y, S z) {                         \
    return (simd_##T##3){x, y, z};                                            \
}                                                                             \
SIMD_FN simd_##T##3 simd_make_##T##3(simd_##T##2 v, S z) {                    \
    return (simd_##T##3){v.x, v.y, z};                                        \
}                                                                             \
SIMD_FN simd_##T##3 simd_make_##T##3(S x, simd_##T##2 v) {                    \
    return (simd_##T##3){x, v.x, v.y};                                        \
}                                                                             \
SIMD_FN simd_##T##3 simd_make_##T##3(S x) { return (simd_##T##3){x, 0, 0}; }  \
SIMD_FN simd_##T##3 simd_make_##T##3(simd_##T##2 v) {                         \
    return (simd_##T##3){v.x, v.y, 0};                                        \
}                                                                             \
SIMD_FN simd_##T##3 simd_make_##T##3(simd_##T##3 v) { return v; }             \
SIMD_FN simd_##T##3 simd_make_##T##3(simd_##T##4 v) { return v.xyz; }         \
SIMD_FN simd_##T##4 simd_make_##T##4(S x, S y, S z, S w) {                    \
    return (simd_##T##4){x, y, z, w};                                         \
}                                                                             \
SIMD_FN simd_##T##4 simd_make_##T##4(simd_##T##2 v, S z, S w) {               \
    return (simd_##T##4){v.x, v.y, z, w};                                     \
}                                                                             \
SIMD_FN simd_##T##4 simd_make_##T##4(S x, simd_##T##2 v, S w) {               \
    return (simd_##T##4){x, v.x, v.y, w};                                     \
}                                                                             \
SIMD_FN simd_##T##4 simd_make_##T##4(S x, S y, simd_##T##2 v) {               \
    return (simd_##T##4){x, y, v.x, v.y};                                     \
}                                                                             \
SIMD_FN simd_##T##4 simd_make_##T##4(simd_##T##2 a, simd_##T##2 b) {          \
    return (simd_##T##4){a.x, a.y, b.x, b.y};                                 \
}                                                                             \
SIMD_FN simd_##T##4 simd_make_##T##4(simd_##T##3 v, S w) {                    \
    return (simd_##T##4){v.x, v.y, v.z, w};                                   \
}                                                                             \
SIMD_FN simd_##T##4 simd_make_##T##4(S x, simd_##T##3 v) {                    \
    return (simd_##T##4){x, v.x, v.y, v.z};                                   \
}                                                                             \
SIMD_FN simd_##T##4 simd_make_##T##4(S x) { return (simd_##T##4){x, 0, 0, 0}; }\
SIMD_FN simd_##T##4 simd_make_##T##4(simd_##T##2 v) {                         \
    return (simd_##T##4){v.x, v.y, 0, 0};                                     \
}                                                                             \
SIMD_FN simd_##T##4 simd_make_##T##4(simd_##T##3 v) {                         \
    return (simd_##T##4){v.x, v.y, v.z, 0};                                   \
}                                                                             \
SIMD_FN simd_##T##4 simd_make_##T##4(simd_##T##4 v) { return v; }

SIMD_MAKE(float, float)
SIMD_MAKE(double, double)
SIMD_MAKE(int, int)
SIMD_MAKE(uint, unsigned int)
#undef SIMD_MAKE

#endif
