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
 * FILE: seed.c
 *
 * DESCRIPTION:
 * Core implementation for seed.c.
 *
 * BRIEF:
 * Provides definitions and logic for seed.c.
 *
 * AUTHOR: Kevin Thomas
 * DATE: August 2026
 */

#include "ed_25519.h"

#ifndef ED25519_NO_SEED

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#else
#include <stdio.h>
#endif

/**
 * @brief Core implementation for seed.c.
 * 
 * @param arg The argument.
 * @return Value The resulting value.
 */
int ed25519_create_seed(unsigned char *seed) {
#ifdef _WIN32
    HCRYPTPROV prov;
    if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))  {
        return 1;
    }
    if (!CryptGenRandom(prov, 32, seed))  {
        CryptReleaseContext(prov, 0);
        return 1;
    }
    CryptReleaseContext(prov, 0);
#else
    FILE *f = fopen("/dev/urandom", "rb");
    if (f == NULL) {
        return 1;
    }
    fread(seed, 1, 32, f);
    fclose(f);
#endif
    return 0;
}

#endif

