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
 * FILE: keypair.c
 *
 * DESCRIPTION:
 * Core implementation for keypair.c.
 *
 * BRIEF:
 * Provides definitions and logic for keypair.c.
 *
 * AUTHOR: Kevin Thomas
 * DATE: August 2026
 */

#include "ed_25519.h"
#include "sha512.h"
#include "ge.h"

/**
 * @brief Core implementation for keypair.c.
 * 
 * @param public_key The public_key parameter.
 * @param private_key The private_key parameter.
 * @return None
 */
void ed25519_create_keypair(unsigned char *public_key, unsigned char *private_key, const unsigned char *seed) {
    ge_p3 A;
    sha512(seed, 32, private_key);
    private_key[0] &= 248;
    private_key[31] &= 63;
    private_key[31] |= 64;
    ge_scalarmult_base(&A, private_key);
    ge_p3_tobytes(public_key, &A);
}

/**
 * @brief Core implementation for keypair.c.
 * 
 * @param public_key The public_key parameter.
 * @param private_key The private_key parameter.
 * @return None
 */
void ed25519_derive_pub(unsigned char *public_key, const unsigned char *private_key) {
    ge_p3 A;
    ge_scalarmult_base(&A, private_key);
    ge_p3_tobytes(public_key, &A);
}

