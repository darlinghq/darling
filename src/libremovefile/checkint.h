/* 
 * Copyright (c) 2006 Apple Computer, Inc.  All rights reserved.
 */

#ifndef __CHECKINT_H__
#define __CHECKINT_H__

/* ObjC++ Guard */
#if defined(__OBJC__) && defined(__cplusplus)
#error "checkint.h does not support Objective C++"
#elif defined(__cplusplus)
#error "checkint.h does not support C++"
#endif

#include <stdint.h>
#include <limits.h>

__BEGIN_DECLS

enum {
	CHECKINT_NO_ERROR = 0,
	CHECKINT_OVERFLOW_ERROR = (1 << 0),
	CHECKINT_TYPE_ERROR = (1 << 1)
};

#define check_int32_add(x, y, err)		__checkint_int32_add(x,y,err)
#define check_uint32_add(x, y, err)		__checkint_uint32_add(x,y,err)
#define check_int64_add(x, y, err)		__checkint_int64_add(x,y,err)
#define check_uint64_add(x, y, err)		__checkint_uint64_add(x,y,err)

#define check_int32_sub(x, y, err)		__checkint_int32_sub(x,y,err)
#define check_uint32_sub(x, y, err)		__checkint_uint32_sub(x,y,err)
#define check_int64_sub(x, y, err)		__checkint_int64_sub(x,y,err)
#define check_uint64_sub(x, y, err)		__checkint_uint64_sub(x,y,err)

#define check_int32_mul(x, y, err)		__checkint_int32_mul(x,y,err)
#define check_uint32_mul(x, y, err)		__checkint_uint32_mul(x,y,err)
#define check_int64_mul(x, y, err)		__checkint_int64_mul(x,y,err)
#define check_uint64_mul(x, y, err)		__checkint_uint64_mul(x,y,err)

#define check_int32_div(x, y, err)		__checkint_int32_div(x,y,err)
#define check_uint32_div(x, y, err)		__checkint_uint32_div(x,y,err)
#define check_int64_div(x, y, err)		__checkint_int64_div(x,y,err)
#define check_uint64_div(x, y, err)		__checkint_uint64_div(x,y,err)

/***
 * Private Interfaces
 *
 * Please do not directly use any interfaces below this point.  They are
 * considered an implementation detail of the above, supported, interfaces
 * and are subject to change at any time without warning.
 ***/

#define __CHECKINT_INLINE static inline __attribute__((always_inline))
#define __CHECKINT_UNLIKELY(X)  __builtin_expect((X),0)

__CHECKINT_INLINE int32_t
__checkint_is_mixed_sign32(int32_t x, int32_t y) {return ((x ^ y) < 0);}

__CHECKINT_INLINE int32_t
__checkint_is_mixed_sign64(int64_t x, int64_t y) {return ((x ^ y) < 0);}

__CHECKINT_INLINE int32_t
__checkint_int32_type_error(int32_t* err)  {*err |= CHECKINT_TYPE_ERROR; return -1;}

__CHECKINT_INLINE int32_t
__checkint_uint32_type_error(int32_t* err) {*err |= CHECKINT_TYPE_ERROR; return -1;}

__CHECKINT_INLINE int32_t
__checkint_int64_type_error(int32_t* err)  {*err |= CHECKINT_TYPE_ERROR; return -1;}

__CHECKINT_INLINE int32_t
__checkint_uint64_type_error(int32_t* err) {*err |= CHECKINT_TYPE_ERROR; return -1;}

__CHECKINT_INLINE int32_t
__checkint_int32_add(int64_t x, int64_t y, int32_t* err) {
	int64_t z = x + y;
	if (__CHECKINT_UNLIKELY(x < INT32_MIN || x > INT32_MAX || y < INT32_MIN || y > INT32_MAX)) {
		*err |= CHECKINT_OVERFLOW_ERROR;
	}
	if (z > INT32_MAX || z < INT32_MIN) *err |= CHECKINT_OVERFLOW_ERROR;
	return (int32_t)z;
}

__CHECKINT_INLINE uint32_t
__checkint_uint32_add(int64_t x, int64_t y, int32_t* err) {
	int64_t z = x + y;
	if (__CHECKINT_UNLIKELY((x & 0xffffffff00000000ull) || (y & 0xffffffff00000000ull))) *err |= CHECKINT_OVERFLOW_ERROR;
	if (__CHECKINT_UNLIKELY(z > UINT_MAX || z < 0)) *err |= CHECKINT_OVERFLOW_ERROR;
	return (uint32_t)z;
}

__CHECKINT_INLINE int64_t
__checkint_int64_add_signed_signed(int64_t x, int64_t y, int32_t* err) {
	/* Mixed-sign additions cannot overflow */
	if (__checkint_is_mixed_sign64(x,y)) {
	/* else, both arguments negative */
	} else if (y < 0) {
		if (__CHECKINT_UNLIKELY(x < LLONG_MIN - y)) *err |= CHECKINT_OVERFLOW_ERROR;
	/* else, both arguments positive */
	} else {
		if (__CHECKINT_UNLIKELY(LLONG_MAX - x < y)) *err |= CHECKINT_OVERFLOW_ERROR;
        }
	return x + y;
}

__CHECKINT_INLINE int64_t
__checkint_int64_add_signed_unsigned(int64_t x, uint64_t y, int32_t* err) {
  if(__CHECKINT_UNLIKELY(((int64_t)(LLONG_MAX - y)) < x))
           *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x + y;
}

__CHECKINT_INLINE int64_t
__checkint_int64_add_unsigned_signed(uint64_t x, int64_t y, int32_t* err) {
   return __checkint_int64_add_signed_unsigned(y, x, err);
}

__CHECKINT_INLINE int64_t
__checkint_int64_add_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
 int64_t diff = LLONG_MAX - y;
   if(__CHECKINT_UNLIKELY(diff < 0 || ((uint64_t) diff) < x))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   return x + y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_add_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY((ULLONG_MAX - y) < x))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   return x + y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_add_signed_signed(int64_t x, int64_t y, int32_t* err) {
  if(__CHECKINT_UNLIKELY(((x < 0 && y >= 0) || (x >= 0 && y < 0)) && (x + y) < 0))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
  else if(__CHECKINT_UNLIKELY(x < 0 && y < 0))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x + y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_add_signed_unsigned(int64_t x, uint64_t y, int32_t* err) {
  if(x > 0)
        return __checkint_uint64_add_unsigned_unsigned(x, y, err);
  if(__CHECKINT_UNLIKELY((y < ((uint64_t)LLONG_MAX + 1)) && (((int64_t) (x + y)) < 0)))
           *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x + y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_add_unsigned_signed(uint64_t x, int64_t y, int32_t* err) {
   return __checkint_uint64_add_signed_unsigned(y, x, err);
}

__CHECKINT_INLINE int32_t
__checkint_int32_sub(int64_t x, int64_t y, int32_t* err) {
	if (__CHECKINT_UNLIKELY(x < INT32_MIN || x > INT32_MAX || y < INT32_MIN || y > INT32_MAX)) {
		*err |= CHECKINT_OVERFLOW_ERROR;
	}
	int64_t z = x - y;
	if (__CHECKINT_UNLIKELY(z > INT_MAX || z < INT_MIN)) *err |= CHECKINT_OVERFLOW_ERROR;
	return (int32_t)z;
}

__CHECKINT_INLINE uint32_t
__checkint_uint32_sub(int64_t x, int64_t y, int32_t* err) {
	int64_t z = x - y;
	if (__CHECKINT_UNLIKELY((x & 0xffffffff00000000ull) || (y & 0xffffffff00000000ull))) *err |= CHECKINT_OVERFLOW_ERROR;
	if (__CHECKINT_UNLIKELY(z > UINT_MAX || z < 0)) *err |= CHECKINT_OVERFLOW_ERROR;
	return (uint32_t)z;
}

__CHECKINT_INLINE int64_t
__checkint_int64_sub_signed_signed(int64_t x, int64_t y, int32_t* err) {
  if(__checkint_is_mixed_sign64(x, y))
  {
     /* Positive x subtract a negative y */
     if(x >= 0)
     {
            if(__CHECKINT_UNLIKELY(x > LLONG_MAX + y))
                *err = *err | CHECKINT_OVERFLOW_ERROR;
     }
     /* Negative x subtract a positive y */
     else
     {
            if(__CHECKINT_UNLIKELY(x < LLONG_MIN + y))
                *err = *err | CHECKINT_OVERFLOW_ERROR;
     }
  }
  /* Both negative, or both positive, no possible overflow */
  return x - y;
}

__CHECKINT_INLINE int64_t
__checkint_int64_sub_signed_unsigned(int64_t x, uint64_t y, int32_t* err) {
  if(__CHECKINT_UNLIKELY(x < ((int64_t)(LLONG_MIN + y))))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x - y;
}

__CHECKINT_INLINE int64_t
__checkint_int64_sub_unsigned_signed(uint64_t x, int64_t y, int32_t* err) {
  if(__CHECKINT_UNLIKELY(x > ((uint64_t)(LLONG_MAX + y)) || y == LLONG_MIN))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x - y;
}

__CHECKINT_INLINE int64_t
__checkint_int64_sub_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY(x > y && ((x - y) > LLONG_MAX)))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   else if(__CHECKINT_UNLIKELY(x < y && ((y - x - 1) > LLONG_MAX)))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   return x - y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_sub_signed_signed(int64_t x, int64_t y, int32_t* err) {
  if(__CHECKINT_UNLIKELY(((x < 0 && y <= 0) || (x >= 0 && y > 0)) && (x - y) < 0))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
  else if(__CHECKINT_UNLIKELY(x < 0 && y > 0))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x - y;
} 

__CHECKINT_INLINE uint64_t
__checkint_uint64_sub_signed_unsigned(int64_t x, uint64_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY(y > ((uint64_t) LLONG_MAX + 1) || ((int64_t) y) > x))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   return x - y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_sub_unsigned_signed(uint64_t x, int64_t y, int32_t* err) {
  if(x <= LLONG_MAX)
        return __checkint_uint64_sub_signed_signed(x, y, err);
  else if (__CHECKINT_UNLIKELY(y == LLONG_MIN || -y > ULLONG_MAX - x))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x - y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_sub_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY(x < y))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   return x - y;
}

__CHECKINT_INLINE int32_t
__checkint_int32_mul(int64_t x, int64_t y, int32_t* err) {
	int64_t z = x * y;
	if (__CHECKINT_UNLIKELY(x < INT32_MIN || x > INT32_MAX || y < INT32_MIN || y > INT32_MAX)) {
		*err |= CHECKINT_OVERFLOW_ERROR;
	}
	if (__CHECKINT_UNLIKELY(z > INT_MAX || z < INT_MIN)) *err |= CHECKINT_OVERFLOW_ERROR;
	return (int32_t)z;
}

__CHECKINT_INLINE uint32_t
__checkint_uint32_mul(int64_t x, int64_t y, int32_t* err) {
	int64_t z = x * y;
	if (__CHECKINT_UNLIKELY((x & 0xffffffff00000000ull) || (y & 0xffffffff00000000ull))) *err |= CHECKINT_OVERFLOW_ERROR;
	if (__CHECKINT_UNLIKELY(z > UINT_MAX || z < 0)) *err |= CHECKINT_OVERFLOW_ERROR;
	return (uint32_t)z;
}

__CHECKINT_INLINE int64_t
__checkint_int64_mul_signed_signed(int64_t x, int64_t y, int32_t* err) {
  if(x == 0 || y == 0) return 0;

  if(!__checkint_is_mixed_sign64(x, y))
  {
    if(x > 0)
    {
       if(__CHECKINT_UNLIKELY(LLONG_MAX/x < y))
            *err = *err | CHECKINT_OVERFLOW_ERROR;
    }
    else
    {
       if(__CHECKINT_UNLIKELY(x == LLONG_MIN || y == LLONG_MIN))
            *err = *err | CHECKINT_OVERFLOW_ERROR;
       if(__CHECKINT_UNLIKELY(LLONG_MAX/(-x) < (-y)))
            *err = *err | CHECKINT_OVERFLOW_ERROR;
    }
  }
  else
  {
    if(x < 0)
    {
       if(__CHECKINT_UNLIKELY(x < LLONG_MIN/y))
            *err = *err | CHECKINT_OVERFLOW_ERROR;
    }
    else if(__CHECKINT_UNLIKELY(y < LLONG_MIN/x))
            *err = *err | CHECKINT_OVERFLOW_ERROR;
  }
  return x * y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_mul_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
  if(x == 0) return 0;
       
  if(__CHECKINT_UNLIKELY(ULLONG_MAX/x < y))
     *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x * y;
}


__CHECKINT_INLINE int64_t
__checkint_int64_mul_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
  if(x == 0) return 0;
       
  if(__CHECKINT_UNLIKELY(LLONG_MAX/x < y))
     *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x * y;
}

__CHECKINT_INLINE int64_t
__checkint_int64_mul_signed_unsigned(int64_t x, uint64_t y, int32_t* err) {
  if(y == 0) return 0;
     
  if(x >= 0)
       return __checkint_int64_mul_unsigned_unsigned(x, y, err);
  else
       if(__CHECKINT_UNLIKELY(x < LLONG_MIN/y || x > LLONG_MAX/y))
            *err = *err | CHECKINT_OVERFLOW_ERROR;
  return x * y;
}

__CHECKINT_INLINE int64_t
__checkint_int64_mul_unsigned_signed(uint64_t x, int64_t y, int32_t* err) {
   return __checkint_int64_mul_signed_unsigned(y, x, err);
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_mul_signed_signed(int64_t x, int64_t y, int32_t* err) {
  if(__CHECKINT_UNLIKELY((x < 0 && y > 0) || (x > 0 && y < 0)))
     *err = *err | CHECKINT_OVERFLOW_ERROR;
  else if(x > 0 && y > 0)
     return __checkint_uint64_mul_unsigned_unsigned(x, y, err);
  else
     return __checkint_uint64_mul_unsigned_unsigned(-x, -y, err);
  return x * y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_mul_signed_unsigned(int64_t x, uint64_t y, int32_t* err) {
  if(x >= 0)
      return __checkint_uint64_mul_unsigned_unsigned(x, y, err);
  *err = *err | CHECKINT_OVERFLOW_ERROR;
  return (uint64_t) (x * y);
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_mul_unsigned_signed(uint64_t x, int64_t y, int32_t* err) {
   return __checkint_uint64_mul_signed_unsigned(y, x, err);
}

__CHECKINT_INLINE int32_t
__checkint_int32_div_signed_signed(int32_t x, int32_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY((x == INT_MIN) && y == -1)) {
       *err = *err | CHECKINT_OVERFLOW_ERROR;
       return 0;
   }
   return x / y;
}

__CHECKINT_INLINE int32_t
__checkint_int32_div_signed_unsigned(int32_t x, uint32_t y, int32_t* err) {
  if(y <= INT_MAX)
        return x / (int32_t) y;
  return 0;  
}

__CHECKINT_INLINE int32_t
__checkint_int32_div_unsigned_signed(uint32_t x, int32_t y, int32_t* err) {
  if(x == ((uint32_t) INT_MAX + 1) && y == -1)
	return INT_MIN;
  if(__CHECKINT_UNLIKELY(x > ((uint32_t) INT_MAX + 1) && y == -1))
                *err = *err | CHECKINT_OVERFLOW_ERROR;
  else if(__CHECKINT_UNLIKELY(x > INT_MAX && y == 1))
                *err = *err | CHECKINT_OVERFLOW_ERROR;
  if(x <= INT_MAX)
    return ((int32_t) x) / y;
  if(y > 0)
    return x / y;
  return -(x / (uint32_t) -y);
}

__CHECKINT_INLINE int32_t
__checkint_int32_div_unsigned_unsigned(uint32_t x, uint32_t y, int32_t* err) {
   uint32_t result = x / y;
   if(__CHECKINT_UNLIKELY(result > INT_MAX))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   return x / y;
}

__CHECKINT_INLINE uint32_t
__checkint_uint32_div_signed_signed(int32_t x, int32_t y, int32_t* err) {
 int32_t result = x / y;
   if(x == INT_MIN && y == -1)
	return ((uint32_t) -x);
   if(__CHECKINT_UNLIKELY(result < 0))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   if(x >= 0 && y > 0)
     return x / y;
   else if(x < 0 && y > 0)
     return -((uint32_t) -x / y);
   else if(x > 0 && y < 0)
     return -(x / (uint32_t) -y);
   else
     return ((uint32_t) -x / (uint32_t) -y);
}

__CHECKINT_INLINE uint32_t
__checkint_uint32_div_signed_unsigned(int32_t x, uint32_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY(x < 0 && ((uint32_t) -x) >= y))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   if(x >= 0)
   	return x / y;
   return -(((uint32_t) -x) / y); 
}

__CHECKINT_INLINE uint32_t
__checkint_uint32_div_unsigned_signed(uint32_t x, int32_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY(y < 0 && ((uint32_t) -y) <= x))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   if(y > 0)
     return x / y;
   return -(x / ((uint32_t) -y));
}

__CHECKINT_INLINE uint32_t
__checkint_uint32_div_unsigned_unsigned(uint32_t x, uint32_t y, int32_t* err) {
   return x / y;
}

__CHECKINT_INLINE int64_t
__checkint_int64_div_signed_signed(int64_t x, int64_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY((x == LLONG_MIN) && y == -1)) {
      *err = *err | CHECKINT_OVERFLOW_ERROR;
      return 0;
   }
   return x / y;
}

__CHECKINT_INLINE int64_t
__checkint_int64_div_signed_unsigned(int64_t x, uint64_t y, int32_t* err) {
  if(y <= LLONG_MAX)
        return x / (int64_t) y;
  return 0;  
}

__CHECKINT_INLINE int64_t
__checkint_int64_div_unsigned_signed(uint64_t x, int64_t y, int32_t* err) {
  if(x == ((uint64_t) LLONG_MAX + 1) && y == -1)
        return LLONG_MIN;
  if(__CHECKINT_UNLIKELY(x > ((uint64_t) LLONG_MAX + 1) && y == -1))
                *err = *err | CHECKINT_OVERFLOW_ERROR;
  else if(__CHECKINT_UNLIKELY(x > LLONG_MAX && y == 1))
                *err = *err | CHECKINT_OVERFLOW_ERROR;
  if(x <= LLONG_MAX)
    return ((int64_t) x) / y;
  if(y > 0)
    return x / y;
  return -(x / (uint64_t) -y);
}

__CHECKINT_INLINE int64_t
__checkint_int64_div_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
   uint64_t result = x / y;
   if(__CHECKINT_UNLIKELY(result > LLONG_MAX))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   return x / y;
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_div_signed_signed(int64_t x, int64_t y, int32_t* err) {
   int64_t result = x / y;
   if(x == LLONG_MIN && y == -1)
	return ((uint64_t)LLONG_MAX) + 1;
   if(__CHECKINT_UNLIKELY(result < 0))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   if(x >= 0 && y > 0)
     return x / y;
   else if(x < 0 && y > 0)
     return -((uint64_t) -x / y);
   else if(x > 0 && y < 0)
     return -(x / (uint64_t) -y);
   else
     return ((uint64_t) -x / (uint64_t) -y);
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_div_signed_unsigned(int64_t x, uint64_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY(x < 0 && ((uint64_t) -x) >= y))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   if(x >= 0)
        return x / y;
   return -(((uint64_t) -x) / y);
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_div_unsigned_signed(uint64_t x, int64_t y, int32_t* err) {
   if(__CHECKINT_UNLIKELY(y < 0 && ((uint64_t) -y) <= x))
        *err = *err | CHECKINT_OVERFLOW_ERROR;
   if(y > 0)
     return x / y;
   return -(x / ((uint64_t) -y));
}

__CHECKINT_INLINE uint64_t
__checkint_uint64_div_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
   return x / y;
}

#undef __CHECKINT_INLINE

/******/

#ifdef __GNUC__
#define	__checkint_same_type(e1, e2)	__builtin_types_compatible_p(__typeof__(e1), __typeof__(e2))
#define	__checkint_cond_expr(c, e1, e2)	__builtin_choose_expr(c, e1, e2)
#else
#error "need compiler support for __checkint_same_type() and __checkint_cond_expr()"
#endif

/******/

#define __checkint_is_signed(x)		(__checkint_same_type(x, int8_t) || __checkint_same_type(x, int16_t) || __checkint_same_type(x, int32_t) || __checkint_same_type(x, int64_t) || __checkint_same_type(x, signed long))
#define __checkint_is_unsigned(x)	(__checkint_same_type(x, uint8_t) || __checkint_same_type(x, uint16_t) || __checkint_same_type(x, uint32_t) || __checkint_same_type(x, uint64_t) || __checkint_same_type(x, uintptr_t) || __checkint_same_type(x, unsigned long))

#define __checkint_is_signed_signed(x, y) (__checkint_is_signed(x) && __checkint_is_signed(y))
#define __checkint_is_signed_unsigned(x, y) (__checkint_is_signed(x) && __checkint_is_unsigned(y))
#define __checkint_is_unsigned_signed(x, y) (__checkint_is_unsigned(x) && __checkint_is_signed(y))
#define __checkint_is_unsigned_unsigned(x, y) (__checkint_is_unsigned(x) && __checkint_is_unsigned(y))

/******/

#define __CHECKINT_SIGN_DEMUX(type,oper,x,y,err) \
	(__checkint_cond_expr(__checkint_is_signed_signed(x, y), __checkint_ ## type ## _ ## oper ## _signed_signed(x, y, err), \
	 __checkint_cond_expr(__checkint_is_signed_unsigned(x, y), __checkint_ ## type ## _ ## oper ## _signed_unsigned(x, y, err), \
	 __checkint_cond_expr(__checkint_is_unsigned_signed(x, y), __checkint_ ## type ## _ ## oper ## _unsigned_signed(x, y, err), \
	 __checkint_cond_expr(__checkint_is_unsigned_unsigned(x, y), __checkint_ ## type ## _ ## oper ## _unsigned_unsigned(x, y, err), \
		__checkint_ ## type ## _type_error(err))))))

#define __checkint_int64_add(x,y,err) __CHECKINT_SIGN_DEMUX(int64,add,x,y,err)
#define __checkint_uint64_add(x,y,err) __CHECKINT_SIGN_DEMUX(uint64,add,x,y,err)

#define __checkint_int64_sub(x,y,err) __CHECKINT_SIGN_DEMUX(int64,sub,x,y,err)
#define __checkint_uint64_sub(x,y,err) __CHECKINT_SIGN_DEMUX(uint64,sub,x,y,err)

#define __checkint_int64_mul(x,y,err) __CHECKINT_SIGN_DEMUX(int64,mul,x,y,err)
#define __checkint_uint64_mul(x,y,err) __CHECKINT_SIGN_DEMUX(uint64,mul,x,y,err)

#define __checkint_int32_div(x,y,err) __CHECKINT_SIGN_DEMUX(int32,div,x,y,err)
#define __checkint_int64_div(x,y,err) __CHECKINT_SIGN_DEMUX(int64,div,x,y,err)
#define __checkint_uint32_div(x,y,err) __CHECKINT_SIGN_DEMUX(uint32,div,x,y,err)
#define __checkint_uint64_div(x,y,err) __CHECKINT_SIGN_DEMUX(uint64,div,x,y,err)

__END_DECLS

#endif /* __CHECKINT_H__ */
