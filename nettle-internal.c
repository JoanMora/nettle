/* nettle-internal.c
 *
 * Things that are used only by the testsuite and benchmark, and
 * subject to change.
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2002, 2014 Niels Möller
 *  
 * The nettle library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 * 
 * The nettle library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with the nettle library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02111-1301, USA.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>

#include "nettle-internal.h"
#include "blowfish.h"
#include "des.h"
#include "chacha.h"
#include "salsa20.h"

/* NOTE: A bit ugly. Ignores weak keys, and pretends the set_key
   functions have no return value. */
const struct nettle_cipher
nettle_des = {
  "des", sizeof(struct des_ctx),
  DES_BLOCK_SIZE, DES_KEY_SIZE,
  (nettle_set_key_func *) des_set_key,
  (nettle_set_key_func *) des_set_key,
  (nettle_crypt_func *) des_encrypt,
  (nettle_crypt_func *) des_decrypt
};

const struct nettle_cipher
nettle_des3 = {
 "des3", sizeof(struct des3_ctx),
 DES3_BLOCK_SIZE, DES3_KEY_SIZE,
 (nettle_set_key_func *) des3_set_key,
 (nettle_set_key_func *) des3_set_key,
 (nettle_crypt_func *) des3_encrypt,
 (nettle_crypt_func *) des3_decrypt
};

/* NOTE: This is not as nice as one might think, as we pretend
   blowfish_set_key has no return value. */
const struct nettle_cipher
nettle_blowfish128 =
  { "blowfish128", sizeof(struct blowfish_ctx),
    BLOWFISH_BLOCK_SIZE, BLOWFISH128_KEY_SIZE,
    (nettle_set_key_func *) blowfish128_set_key,
    (nettle_set_key_func *) blowfish128_set_key,
    (nettle_crypt_func *) blowfish_encrypt,
    (nettle_crypt_func *) blowfish_decrypt
  };

/* Sets a fix zero iv. For benchmarking only. */
static void
chacha_set_key_hack(void *ctx, const uint8_t *key)
{
  static const uint8_t nonce[CHACHA_NONCE_SIZE];
  chacha256_set_key (ctx, key);
  chacha_set_nonce (ctx, nonce);
}

/* Claim zero block size, to classify as a stream cipher. */
const struct nettle_cipher
nettle_chacha = {
  "chacha", sizeof(struct chacha_ctx),
  0, CHACHA256_KEY_SIZE,
  chacha_set_key_hack, chacha_set_key_hack,
  (nettle_crypt_func *) chacha_crypt,
  (nettle_crypt_func *) chacha_crypt
};

/* Sets a fix zero iv. For benchmarking only. */
static void
salsa20_set_key_hack(void *ctx, const uint8_t *key)
{
  static const uint8_t iv[SALSA20_IV_SIZE];
  salsa20_256_set_key (ctx, key);
  salsa20_set_iv (ctx, iv);
}

/* Claim zero block size, to classify as a stream cipher. */
const struct nettle_cipher
nettle_salsa20 = {
  "salsa20", sizeof(struct salsa20_ctx),
  0, SALSA20_256_KEY_SIZE,
  salsa20_set_key_hack, salsa20_set_key_hack,
  (nettle_crypt_func *) salsa20_crypt,
  (nettle_crypt_func *) salsa20_crypt
};

const struct nettle_cipher
nettle_salsa20r12 = {
  "salsa20r12", sizeof(struct salsa20_ctx),
  0, SALSA20_256_KEY_SIZE,
  salsa20_set_key_hack, salsa20_set_key_hack,
  (nettle_crypt_func *) salsa20r12_crypt,
  (nettle_crypt_func *) salsa20r12_crypt
};

