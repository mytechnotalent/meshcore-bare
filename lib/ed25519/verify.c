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
 * FILE: verify.c
 *
 * DESCRIPTION:
 * Core implementation for verify.c.
 *
 * BRIEF:
 * Provides definitions and logic for verify.c.
 *
 * AUTHOR: Kevin Thomas
 * DATE: August 2026
 */

#include "ed_25519.h"
#include "sha512.h"
#include "ge.h"
#include "sc.h"

/**
 * @brief Core implementation for verify.c.
 * 
 * @param signature The signature parameter.
 * @param message The message parameter.
 * @param message_len The message_len parameter.
 * @param public_key The public_key parameter.
 * @return int The resulting int value.
 */
static int consttime_equal(const unsigned char *x, const unsigned char *y) {
    unsigned char r = 0;
    r = x[0] ^ y[0];
    #define F(i) r |= x[i] ^ y[i]
    F(1);
    F(2);
    F(3);
    F(4);
    F(5);
    F(6);
    F(7);
    F(8);
    F(9);
    F(10);
    F(11);
    F(12);
    F(13);
    F(14);
    F(15);
    F(16);
    F(17);
    F(18);
    F(19);
    F(20);
    F(21);
    F(22);
    F(23);
    F(24);
    F(25);
    F(26);
    F(27);
    F(28);
    F(29);
    F(30);
    F(31);
    #undef F
    return !r;
}

/**
 * @brief Core implementation for verify.c.
 * 
 * @param signature The signature parameter.
 * @param message The message parameter.
 * @param message_len The message_len parameter.
 * @param public_key The public_key parameter.
 * @return int The resulting int value.
 */
int ed25519_verify(const unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key) {
    unsigned char h[64];
    unsigned char checker[32];
    sha512_context hash;
    ge_p3 A;
    ge_p2 R;
    if (signature[63] & 224) {
        return 0;
    }
    if (ge_frombytes_negate_vartime(&A, public_key) != 0) {
        return 0;
    }
    sha512_init(&hash);
    sha512_update(&hash, signature, 32);
    sha512_update(&hash, public_key, 32);
    sha512_update(&hash, message, message_len);
    sha512_final(&hash, h);
    sc_reduce(h);
    ge_double_scalarmult_vartime(&R, h, &A, signature + 32);
    ge_tobytes(checker, &R);
    if (!consttime_equal(checker, signature)) {
        return 0;
    }
    return 1;
}

