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
 * FILE: ed_25519.h
 *
 * DESCRIPTION:
 * Core implementation for ed_25519.h.
 *
 * BRIEF:
 * Provides definitions and logic for ed_25519.h.
 *
 * AUTHOR: Kevin Thomas
 * DATE: August 2026
 */

#ifndef ED25519_H
#define ED25519_H

// Nightcracker's Ed25519 -  https://github.com/orlp/ed25519

#include <stddef.h>

#if defined(_WIN32)
    #if defined(ED25519_BUILD_DLL)
        #define ED25519_DECLSPEC __declspec(dllexport)
    #elif defined(ED25519_DLL)
        #define ED25519_DECLSPEC __declspec(dllimport)
    #else
        #define ED25519_DECLSPEC
    #endif
#else
    #define ED25519_DECLSPEC
#endif

#ifdef __cplusplus

/**
 * @brief Core implementation for ed_25519.h.
 */
extern "C" {
#endif
#ifndef ED25519_NO_SEED
/**
 * @brief Core implementation for ed_25519.h.
 * 
 * @param seed The seed parameter.
 * @return int The resulting int value.
 */
int ED25519_DECLSPEC ed25519_create_seed(unsigned char *seed);
#endif
/**
 * @brief Core implementation for ed_25519.h.
 * 
 * @param public_key The public_key parameter.
 * @param private_key The private_key parameter.
 * @param seed The seed parameter.
 * @return None
 */
void ED25519_DECLSPEC ed25519_create_keypair(unsigned char *public_key, unsigned char *private_key, const unsigned char *seed);
/**
 * @brief Core implementation for ed_25519.h.
 * 
 * @param public_key The public_key parameter.
 * @param private_key The private_key parameter.
 * @return None
 */
void ED25519_DECLSPEC ed25519_derive_pub(unsigned char *public_key, const unsigned char *private_key);
/**
 * @brief Core implementation for ed_25519.h.
 * 
 * @param signature The signature parameter.
 * @param message The message parameter.
 * @param message_len The message_len parameter.
 * @param public_key The public_key parameter.
 * @param private_key The private_key parameter.
 * @return None
 */
void ED25519_DECLSPEC ed25519_sign(unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key, const unsigned char *private_key);
/**
 * @brief Core implementation for ed_25519.h.
 * 
 * @param signature The signature parameter.
 * @param message The message parameter.
 * @param message_len The message_len parameter.
 * @param public_key The public_key parameter.
 * @return int The resulting int value.
 */
int ED25519_DECLSPEC ed25519_verify(const unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key);
/**
 * @brief Core implementation for ed_25519.h.
 * 
 * @param public_key The public_key parameter.
 * @param private_key The private_key parameter.
 * @param scalar The scalar parameter.
 * @return None
 */
void ED25519_DECLSPEC ed25519_add_scalar(unsigned char *public_key, unsigned char *private_key, const unsigned char *scalar);
/**
 * @brief Core implementation for ed_25519.h.
 * 
 * @param shared_secret The shared_secret parameter.
 * @param public_key The public_key parameter.
 * @param private_key The private_key parameter.
 * @return None
 */
void ED25519_DECLSPEC ed25519_key_exchange(unsigned char *shared_secret, const unsigned char *public_key, const unsigned char *private_key);
#ifdef __cplusplus
}
#endif

#endif

