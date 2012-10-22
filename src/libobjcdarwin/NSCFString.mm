/** Implementation for GNUStep of NSString concrete subclasses
   Copyright (C) 1997,1998,2000 Free Software Foundation, Inc.

   Base on code written by Stevo Crvenkovski <stevo@btinternet.com>
   Date: February 1997

   Based on NSGCString and NSString
   Written by:  Andrew Kachites McCallum
   <mccallum@gnu.ai.mit.edu>
   Date: March 1995

   Optimised by  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: October 1998

   Redesign/rewrite by  Richard Frith-Macdonald <rfm@gnu.org>
   Date: September 2000

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.
   
   This file derives a NSCFString class from the NXConstantString implementaion. (Luboš Doležel)
*/

#import "NSCFString.h"
#import <Foundation/NSException.h>

/* Determine the length of the UTF-8 string as a unicode (UTF-16) string.
 * sets the ascii flag according to the content found.
 */
static NSUInteger
lengthUTF8(const uint8_t *p, unsigned l, BOOL *ascii, BOOL *latin1)
{
  const uint8_t	*e = p + l;
  BOOL		a = YES;
  BOOL		l1 = YES;

  l = 0;
  while (p < e)
    {
      uint8_t	c = *p;
      uint32_t	u = c;

      if (c > 0x7f)
	{
	  int i, sle = 0;

	  a = NO;
	  /* calculated the expected sequence length */
	  while (c & 0x80)
	    {
	      c = c << 1;
	      sle++;
	    }

	  /* legal ? */
	  if ((sle < 2) || (sle > 6))
	    {
	      [NSException raise: NSInternalInconsistencyException
			  format: @"Bad sequence length in constant string"];
	    }

	  if (p + sle > e)
	    {
	      [NSException raise: NSInternalInconsistencyException
			  format: @"Short data in constant string"];
	    }

	  /* get the codepoint */
	  for (i = 1; i < sle; i++)
	    {
	      if (p[i] < 0x80 || p[i] >= 0xc0)
		break;
	      u = (u << 6) | (p[i] & 0x3f);
	    }

	  if (i < sle)
	    {
	      [NSException raise: NSInternalInconsistencyException
			  format: @"Codepoint out of range in constant string"];
	    }
	  u = u & ~(0xffffffff << ((5 * sle) + 1));
	  p += sle;

	  /*
	   * We check for invalid codepoints here.
	   */
	  if (u > 0x10ffff || u == 0xfffe || u == 0xffff
	    || (u >= 0xfdd0 && u <= 0xfdef))
	    {
	      [NSException raise: NSInternalInconsistencyException
			  format: @"Codepoint invalid in constant string"];
	    }

	  if ((u >= 0xd800) && (u <= 0xdfff))
	    {
	      [NSException raise: NSInternalInconsistencyException
			  format: @"Bad surrogate pair in constant string"];
	    }
	}
      else
	{
	  p++;
	}

      /*
       * Add codepoint as either a single unichar for BMP
       * or as a pair of surrogates for codepoints over 16 bits.
       */
      if (u < 0x10000)
	{
	  l++;
	  if (u > 255)
	    {
	      l1 = NO;
	    }
	}
      else
	{
	  l += 2;
	}
    }
  if (0 != ascii)
    {
      *ascii = a;
    }
  if (0 != latin1)
    {
      *latin1 = l1;
    }
  return l;
}

/* Sequentially extracts characters from UTF-8 string
 * p = pointer to the utf-8 data
 * l = length (bytes) of the utf-8 data
 * o = pointer to current offset within the data
 * n = pointer to either zero or the next pre-read part of a surrogate pair.
 * The condition for having read the entire string is that the offset (*o)
 * is the number of bytes in the string, and the unichar pointed to by *n
 * is zero (meaning there is no second part of a surrogate pair remaining).
 */
static inline unichar
nextUTF8(const uint8_t *p, unsigned l, unsigned *o, unichar *n)
{
  unsigned	i;

  /* If we still have the second part of a surrogate pair, return it.
   */
  if (*n > 0)
    {
      unichar	u = *n;

      *n = 0;
      return u;
    }

  if ((i = *o) < l)
    {
      uint8_t	c = p[i];
      uint32_t	u = c;

      if (c > 0x7f)
	{
	  int j, sle = 0;

	  /* calculated the expected sequence length */
	  while (c & 0x80)
	    {
	      c = c << 1;
	      sle++;
	    }

	  /* legal ? */
	  if ((sle < 2) || (sle > 6))
	    {
	      [NSException raise: NSInvalidArgumentException
			  format: @"bad multibyte character length"];
	    }

	  if (sle + i > l)
	    {
	      [NSException raise: NSInvalidArgumentException
			  format: @"multibyte character extends beyond data"];
	    }

	  /* get the codepoint */
	  for (j = 1; j < sle; j++)
	    {
	      uint8_t	b = p[i + j];

	      if (b < 0x80 || b >= 0xc0)
		break;
	      u = (u << 6) | (b & 0x3f);
	    }

	  if (j < sle)
	    {
	      [NSException raise: NSInvalidArgumentException
			  format: @"bad data in multibyte character"];
	    }
	  u = u & ~(0xffffffff << ((5 * sle) + 1));
	  i += sle;

	  /*
	   * We discard invalid codepoints here.
	   */
	  if (u > 0x10ffff || u == 0xfffe || u == 0xffff
	    || (u >= 0xfdd0 && u <= 0xfdef))
	    {
	      [NSException raise: NSInvalidArgumentException
			  format: @"invalid unicode codepoint"];
	    }

	  if ((u >= 0xd800) && (u <= 0xdfff))
	    {
	      [NSException raise: NSInvalidArgumentException
			  format: @"unmatched half of surrogate pair"];
	    }
	}
      else
	{
	  i++;
	}

      /*
       * Add codepoint as either a single unichar for BMP
       * or as a pair of surrogates for codepoints over 16 bits.
       */
      if (u >= 0x10000)
	{
	  unichar ul, uh;

	  u -= 0x10000;
	  ul = u & 0x3ff;
	  uh = (u >> 10) & 0x3ff;

	  *n = ul + 0xdc00;	// record second part of pair
	  u = uh + 0xd800;	// return first part.
	}
      *o = i;			// Return new index
      return (unichar)u;
    }
  [NSException raise: NSInvalidArgumentException
	      format: @"no more data in UTF-8 string"];
  return 0;
}

@implementation NSCFString

- (const char*) UTF8String
{
  return nxcsptr;
}

- (unichar) characterAtIndex: (NSUInteger)index
{
  NSUInteger	l = 0;
  unichar	u;
  unichar	n = 0;
  unsigned	i = 0;

  while (i < nxcslen || n > 0)
    {
      u = nextUTF8((const uint8_t *)nxcsptr, nxcslen, &i, &n);
      if (l++ == index)
	{
	  return u;
	}
    }

  [NSException raise: NSInvalidArgumentException
	      format: @"-characterAtIndex: index out of range"];
  return 0;
}

- (void) dealloc
{
  GSNOSUPERDEALLOC;
}

/* Must match the implementation in NSString
 */
- (NSUInteger) hash
{
  if (nxcslen > 0)
    {
      unsigned	ret = 0;
      unichar	n = 0;
      unsigned	i = 0;
      unichar	c;

      while (i < nxcslen)
	{
	  c = nextUTF8((const uint8_t *)nxcsptr, nxcslen, &i, &n);
	  ret = (ret << 5) + ret + c;
	}
      if (0 != n)
	{
	  ret = (ret << 5) + ret + n;	// Add final character
	}
      ret &= 0x0fffffff;
      if (ret == 0)
	{
	  ret = 0x0fffffff;
	}
      return ret;
    }
  else
    {
      return 0x0ffffffe;	/* Hash for an empty string.	*/
    }
}

- (id) initWithBytes: (const void*)bytes
	      length: (NSUInteger)length
	    encoding: (NSStringEncoding)encoding
{
  [NSException raise: NSGenericException
	      format: @"Attempt to init a constant string"];
  return nil;
}

- (id) initWithBytesNoCopy: (void*)bytes
		    length: (NSUInteger)length
		  encoding: (NSStringEncoding)encoding
	      freeWhenDone: (BOOL)flag
{
  [NSException raise: NSGenericException
	      format: @"Attempt to init a constant string"];
  return nil;
}

- (BOOL) isEqual: (id)anObject
{
  if ([anObject isKindOfClass: [NSCFString class]])
  {
    NSCFString* str = (NSCFString*) anObject;
    if (nxcslen != str->nxcslen)
      return NO;
    else
      return memcmp(nxcsptr, str->nxcsptr, nxcslen) == 0;
  }
  else if ([anObject isKindOfClass: [NSString class]])
    return [anObject isEqual: self];
  else
    return NO;
}

- (BOOL) isEqualToString: (NSString*)other
{
  if ([other isKindOfClass: [NSCFString class]])
  {
    NSCFString* str = (NSCFString*) other;
    if (nxcslen != str->nxcslen)
      return NO;
    else
      return memcmp(nxcsptr, str->nxcsptr, nxcslen) == 0;
  }
  else
    return [other isEqual: self];
}

- (NSUInteger) length
{
  return lengthUTF8((const uint8_t*)nxcsptr, nxcslen, 0, 0);
}

- (id) retain
{
  return self;
}

- (oneway void) release
{
  return;
}

- (id) autorelease
{
  return self;
}

- (id) copyWithZone: (NSZone*)z
{
  return self;
}

- (NSZone*) zone
{
  return NSDefaultMallocZone();
}

- (NSStringEncoding) fastestEncoding
{
  return NSUTF8StringEncoding;
}

- (NSStringEncoding) smallestEncoding
{
  return NSUTF8StringEncoding;
}

@end

