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
 * FILE: fe.h
 *
 * DESCRIPTION:
 * Core implementation for fe.h.
 *
 * BRIEF:
 * Provides definitions and logic for fe.h.
 *
 * AUTHOR: Kevin Thomas
 * DATE: August 2026
 */

#ifndef FE_H
#define FE_H

#include "fixedint.h"

/*
    fe means field element.
    Here the field is \Z/(2^255-19).
    An element t, entries t[0]...t[9], represents the integer
    t[0]+2^26 t[1]+2^51 t[2]+2^77 t[3]+2^102 t[4]+...+2^230 t[9].
    Bounds on each t[i] vary depending on context.
*/

typedef int32_t fe[10];

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @return None
 */
void fe_0(fe h);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @return None
 */
void fe_1(fe h);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @param s The s parameter.
 * @return None
 */
void fe_frombytes(fe h, const unsigned char *s);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param s The s parameter.
 * @param h The h parameter.
 * @return None
 */
void fe_tobytes(unsigned char *s, const fe h);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @param f The f parameter.
 * @return None
 */
void fe_copy(fe h, const fe f);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param f The f parameter.
 * @return int The resulting int value.
 */
int fe_isnegative(const fe f);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param f The f parameter.
 * @return int The resulting int value.
 */
int fe_isnonzero(const fe f);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param f The f parameter.
 * @param g The g parameter.
 * @param b The b parameter.
 * @return None
 */
void fe_cmov(fe f, const fe g, unsigned int b);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param f The f parameter.
 * @param g The g parameter.
 * @param b The b parameter.
 * @return None
 */
void fe_cswap(fe f, fe g, unsigned int b);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @param f The f parameter.
 * @return None
 */
void fe_neg(fe h, const fe f);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @param f The f parameter.
 * @param g The g parameter.
 * @return None
 */
void fe_add(fe h, const fe f, const fe g);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param out The out parameter.
 * @param z The z parameter.
 * @return None
 */
void fe_invert(fe out, const fe z);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @param f The f parameter.
 * @return None
 */
void fe_sq(fe h, const fe f);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @param f The f parameter.
 * @return None
 */
void fe_sq2(fe h, const fe f);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @param f The f parameter.
 * @param g The g parameter.
 * @return None
 */
void fe_mul(fe h, const fe f, const fe g);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @param f The f parameter.
 * @return None
 */
void fe_mul121666(fe h, fe f);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param out The out parameter.
 * @param z The z parameter.
 * @return None
 */
void fe_pow22523(fe out, const fe z);

/**
 * @brief Core implementation for fe.h.
 * 
 * @param h The h parameter.
 * @param f The f parameter.
 * @param g The g parameter.
 * @return None
 */
void fe_sub(fe h, const fe f, const fe g);

#endif

