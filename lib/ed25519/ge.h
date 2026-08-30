/*
  Copyright (c) 2015 Orson Peters <orsonpeters@gmail.com>

  This software is provided 'as-is', without any express or implied warranty. In no event will the
  authors be held liable for any damages arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose, including commercial
  applications, and to alter it and redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you wrote the
     original software. If you use this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be misrepresented as
     being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/

/**
 * FILE: ge.h
 *
 * DESCRIPTION:
 * Core implementation for ge.h.
 *
 * BRIEF:
 * Provides definitions and logic for ge.h.
 *
 * AUTHOR: Kevin Thomas
 * DATE: August 2026
 */

#ifndef GE_H
#define GE_H

#include "fe.h"

/*
ge means group element.

Here the group is the set of pairs (x,y) of field elements (see fe.h)
satisfying -x^2 + y^2 = 1 + d x^2y^2
where d = -121665/121666.

Representations:
  ge_p2 (projective): (X:Y:Z) satisfying x=X/Z, y=Y/Z
  ge_p3 (extended): (X:Y:Z:T) satisfying x=X/Z, y=Y/Z, XY=ZT
  ge_p1p1 (completed): ((X:Z),(Y:T)) satisfying x=X/Z, y=Y/T
  ge_precomp (Duif): (y+x,y-x,2dxy)
*/

/**
 * @brief Core implementation for ge.h.
 */
typedef struct {
  fe X;
  fe Y;
  fe Z;
} ge_p2;

/**
 * @brief Core implementation for ge.h.
 */
typedef struct {
  fe X;
  fe Y;
  fe Z;
  fe T;
} ge_p3;

/**
 * @brief Core implementation for ge.h.
 */
typedef struct {
  fe X;
  fe Y;
  fe Z;
  fe T;
} ge_p1p1;

/**
 * @brief Core implementation for ge.h.
 */
typedef struct {
  fe yplusx;
  fe yminusx;
  fe xy2d;
} ge_precomp;

/**
 * @brief Core implementation for ge.h.
 */
typedef struct {
  fe YplusX;
  fe YminusX;
  fe Z;
  fe T2d;
} ge_cached;

/**
 * @brief Core implementation for ge.h.
 * 
 * @param s The s parameter.
 * @param h The h parameter.
 * @return None
 */
void ge_p3_tobytes(unsigned char *s, const ge_p3 *h);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param s The s parameter.
 * @param h The h parameter.
 * @return None
 */
void ge_tobytes(unsigned char *s, const ge_p2 *h);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param h The h parameter.
 * @param s The s parameter.
 * @return int The resulting int value.
 */
int ge_frombytes_negate_vartime(ge_p3 *h, const unsigned char *s);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @param q The q parameter.
 * @return None
 */
void ge_add(ge_p1p1 *r, const ge_p3 *p, const ge_cached *q);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @param q The q parameter.
 * @return None
 */
void ge_sub(ge_p1p1 *r, const ge_p3 *p, const ge_cached *q);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param a The a parameter.
 * @param A The A parameter.
 * @param b The b parameter.
 * @return None
 */
void ge_double_scalarmult_vartime(ge_p2 *r, const unsigned char *a, const ge_p3 *A, const unsigned char *b);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @param q The q parameter.
 * @return None
 */
void ge_madd(ge_p1p1 *r, const ge_p3 *p, const ge_precomp *q);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @param q The q parameter.
 * @return None
 */
void ge_msub(ge_p1p1 *r, const ge_p3 *p, const ge_precomp *q);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param h The h parameter.
 * @param a The a parameter.
 * @return None
 */
void ge_scalarmult_base(ge_p3 *h, const unsigned char *a);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @return None
 */
void ge_p1p1_to_p2(ge_p2 *r, const ge_p1p1 *p);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @return None
 */
void ge_p1p1_to_p3(ge_p3 *r, const ge_p1p1 *p);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param h The h parameter.
 * @return None
 */
void ge_p2_0(ge_p2 *h);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @return None
 */
void ge_p2_dbl(ge_p1p1 *r, const ge_p2 *p);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param h The h parameter.
 * @return None
 */
void ge_p3_0(ge_p3 *h);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @return None
 */
void ge_p3_dbl(ge_p1p1 *r, const ge_p3 *p);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @return None
 */
void ge_p3_to_cached(ge_cached *r, const ge_p3 *p);

/**
 * @brief Core implementation for ge.h.
 * 
 * @param r The r parameter.
 * @param p The p parameter.
 * @return None
 */
void ge_p3_to_p2(ge_p2 *r, const ge_p3 *p);

#endif

