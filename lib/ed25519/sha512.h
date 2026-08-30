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
 * FILE: sha512.h
 *
 * DESCRIPTION:
 * Core implementation for sha512.h.
 *
 * BRIEF:
 * Provides definitions and logic for sha512.h.
 *
 * AUTHOR: Kevin Thomas
 * DATE: August 2026
 */

#ifndef SHA512_H
#define SHA512_H

#include <stddef.h>

#include "fixedint.h"

/* state */

/**
 * @brief Core implementation for sha512.h.
 */
typedef struct sha512_context_ {
    uint64_t  length, state[8];
    size_t curlen;
    unsigned char buf[128];
} sha512_context;

/**
 * @brief Core implementation for sha512.h.
 * 
 * @param md The md parameter.
 * @return int The resulting int value.
 */
int sha512_init(sha512_context * md);

/**
 * @brief Core implementation for sha512.h.
 * 
 * @param md The md parameter.
 * @param out The out parameter.
 * @return int The resulting int value.
 */
int sha512_final(sha512_context * md, unsigned char *out);

/**
 * @brief Core implementation for sha512.h.
 * 
 * @param md The md parameter.
 * @param in The in parameter.
 * @param inlen The inlen parameter.
 * @return int The resulting int value.
 */
int sha512_update(sha512_context * md, const unsigned char *in, size_t inlen);

/**
 * @brief Core implementation for sha512.h.
 * 
 * @param message The message parameter.
 * @param message_len The message_len parameter.
 * @param out The out parameter.
 * @return int The resulting int value.
 */
int sha512(const unsigned char *message, size_t message_len, unsigned char *out);

#endif

