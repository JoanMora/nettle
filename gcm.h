/* gcm.h
 *
 * Galois counter mode, specified by NIST,
 * http://csrc.nist.gov/publications/nistpubs/800-38D/SP-800-38D.pdf
 *
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2011 Katholieke Universiteit Leuven
 * Copyright (C) 2011, 2014 Niels Möller
 * 
 * Contributed by Nikos Mavrogiannopoulos
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

#ifndef NETTLE_GCM_H_INCLUDED
#define NETTLE_GCM_H_INCLUDED

#include "aes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define gcm_set_key nettle_gcm_set_key
#define gcm_set_iv nettle_gcm_set_iv
#define gcm_update nettle_gcm_update
#define gcm_encrypt nettle_gcm_encrypt
#define gcm_decrypt nettle_gcm_decrypt
#define gcm_digest nettle_gcm_digest

#define gcm_aes128_set_key nettle_gcm_aes128_set_key
#define gcm_aes128_set_iv nettle_gcm_aes128_set_iv
#define gcm_aes128_update nettle_gcm_aes128_update
#define gcm_aes128_encrypt nettle_gcm_aes128_encrypt
#define gcm_aes128_decrypt nettle_gcm_aes128_decrypt
#define gcm_aes128_digest nettle_gcm_aes128_digest

#define gcm_aes192_set_key nettle_gcm_aes192_set_key
#define gcm_aes192_set_iv nettle_gcm_aes192_set_iv
#define gcm_aes192_update nettle_gcm_aes192_update
#define gcm_aes192_encrypt nettle_gcm_aes192_encrypt
#define gcm_aes192_decrypt nettle_gcm_aes192_decrypt
#define gcm_aes192_digest nettle_gcm_aes192_digest

#define gcm_aes256_set_key nettle_gcm_aes256_set_key
#define gcm_aes256_set_iv nettle_gcm_aes256_set_iv
#define gcm_aes256_update nettle_gcm_aes256_update
#define gcm_aes256_encrypt nettle_gcm_aes256_encrypt
#define gcm_aes256_decrypt nettle_gcm_aes256_decrypt
#define gcm_aes256_digest nettle_gcm_aes256_digest

#define gcm_aes_set_key nettle_gcm_aes_set_key
#define gcm_aes_set_iv nettle_gcm_aes_set_iv
#define gcm_aes_update nettle_gcm_aes_update
#define gcm_aes_encrypt nettle_gcm_aes_encrypt
#define gcm_aes_decrypt nettle_gcm_aes_decrypt
#define gcm_aes_digest nettle_gcm_aes_digest

#define GCM_BLOCK_SIZE 16
#define GCM_IV_SIZE (GCM_BLOCK_SIZE - 4)

#define GCM_TABLE_BITS 8

/* Hashing subkey */
struct gcm_key
{
  union nettle_block16 h[1 << GCM_TABLE_BITS];
};

/* Per-message state, depending on the iv */
struct gcm_ctx {
  /* Original counter block */
  union nettle_block16 iv;
  /* Updated for each block. */
  union nettle_block16 ctr;
  /* Hashing state */
  union nettle_block16 x;
  uint64_t auth_size;
  uint64_t data_size;
};

/* FIXME: Should use const for the cipher context. Then needs const for
   nettle_crypt_func, which also rules out using that abstraction for
   arcfour. */
void
gcm_set_key(struct gcm_key *key,
	    void *cipher, nettle_crypt_func *f);

void
gcm_set_iv(struct gcm_ctx *ctx, const struct gcm_key *key,
	   size_t length, const uint8_t *iv);

void
gcm_update(struct gcm_ctx *ctx, const struct gcm_key *key,
	   size_t length, const uint8_t *data);

void
gcm_encrypt(struct gcm_ctx *ctx, const struct gcm_key *key,
	    void *cipher, nettle_crypt_func *f,
	    size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_decrypt(struct gcm_ctx *ctx, const struct gcm_key *key,
	    void *cipher, nettle_crypt_func *f,
	    size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_digest(struct gcm_ctx *ctx, const struct gcm_key *key,
	   void *cipher, nettle_crypt_func *f,
	   size_t length, uint8_t *digest);

/* Convenience macrology (not sure how useful it is) */
/* All-in-one context, with hash subkey, message state, and cipher. */
#define GCM_CTX(type) \
  { struct gcm_key key; struct gcm_ctx gcm; type cipher; }

/* NOTE: Avoid using NULL, as we don't include anything defining it. */
#define GCM_SET_KEY(ctx, set_key, encrypt, key)			\
  do {								\
    (set_key)(&(ctx)->cipher, (key));				\
    if (0) (encrypt)(&(ctx)->cipher, 0, (void *)0, (void *)0);	\
    gcm_set_key(&(ctx)->key, &(ctx)->cipher,			\
		(nettle_crypt_func *) (encrypt));		\
  } while (0)

#define GCM_SET_IV(ctx, length, data)				\
  gcm_set_iv(&(ctx)->gcm, &(ctx)->key, (length), (data))

#define GCM_UPDATE(ctx, length, data)			\
  gcm_update(&(ctx)->gcm, &(ctx)->key, (length), (data))

#define GCM_ENCRYPT(ctx, encrypt, length, dst, src)			\
  (0 ? (encrypt)(&(ctx)->cipher, 0, (void *)0, (void *)0)		\
     : gcm_encrypt(&(ctx)->gcm, &(ctx)->key, &(ctx)->cipher,		\
		   (nettle_crypt_func *) (encrypt),			\
		   (length), (dst), (src)))

#define GCM_DECRYPT(ctx, encrypt, length, dst, src)			\
  (0 ? (encrypt)(&(ctx)->cipher, 0, (void *)0, (void *)0)		\
     : gcm_decrypt(&(ctx)->gcm,  &(ctx)->key, &(ctx)->cipher,		\
		   (nettle_crypt_func *) (encrypt),			\
		   (length), (dst), (src)))

#define GCM_DIGEST(ctx, encrypt, length, digest)			\
  (0 ? (encrypt)(&(ctx)->cipher, 0, (void *)0, (void *)0)		\
     : gcm_digest(&(ctx)->gcm, &(ctx)->key, &(ctx)->cipher,		\
		  (nettle_crypt_func *) (encrypt),			\
		  (length), (digest)))

struct gcm_aes128_ctx GCM_CTX(struct aes128_ctx);

void
gcm_aes128_set_key(struct gcm_aes128_ctx *ctx, const uint8_t *key);

/* FIXME: Define _update and _set_iv as some kind of aliaes,
   there's nothing aes-specific. */
void
gcm_aes128_update (struct gcm_aes128_ctx *ctx,
		   size_t length, const uint8_t *data);
void
gcm_aes128_set_iv (struct gcm_aes128_ctx *ctx,
		   size_t length, const uint8_t *iv);

void
gcm_aes128_encrypt(struct gcm_aes128_ctx *ctx,
		   size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_aes128_decrypt(struct gcm_aes128_ctx *ctx,
		   size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_aes128_digest(struct gcm_aes128_ctx *ctx,
		  size_t length, uint8_t *digest);

struct gcm_aes192_ctx GCM_CTX(struct aes192_ctx);

void
gcm_aes192_set_key(struct gcm_aes192_ctx *ctx, const uint8_t *key);

void
gcm_aes192_update (struct gcm_aes192_ctx *ctx,
		   size_t length, const uint8_t *data);
void
gcm_aes192_set_iv (struct gcm_aes192_ctx *ctx,
		   size_t length, const uint8_t *iv);

void
gcm_aes192_encrypt(struct gcm_aes192_ctx *ctx,
		   size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_aes192_decrypt(struct gcm_aes192_ctx *ctx,
		   size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_aes192_digest(struct gcm_aes192_ctx *ctx,
		  size_t length, uint8_t *digest);

struct gcm_aes256_ctx GCM_CTX(struct aes256_ctx);

void
gcm_aes256_set_key(struct gcm_aes256_ctx *ctx, const uint8_t *key);

void
gcm_aes256_update (struct gcm_aes256_ctx *ctx,
		   size_t length, const uint8_t *data);
void
gcm_aes256_set_iv (struct gcm_aes256_ctx *ctx,
		   size_t length, const uint8_t *iv);

void
gcm_aes256_encrypt(struct gcm_aes256_ctx *ctx,
		   size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_aes256_decrypt(struct gcm_aes256_ctx *ctx,
		   size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_aes256_digest(struct gcm_aes256_ctx *ctx,
		  size_t length, uint8_t *digest);

/* Old aes interface, for backwards compatibility */
struct gcm_aes_ctx GCM_CTX(struct aes_ctx);

void
gcm_aes_set_key(struct gcm_aes_ctx *ctx,
		size_t length, const uint8_t *key);

void
gcm_aes_set_iv(struct gcm_aes_ctx *ctx,
	       size_t length, const uint8_t *iv);

void
gcm_aes_update(struct gcm_aes_ctx *ctx,
	       size_t length, const uint8_t *data);

void
gcm_aes_encrypt(struct gcm_aes_ctx *ctx,
		size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_aes_decrypt(struct gcm_aes_ctx *ctx,
		size_t length, uint8_t *dst, const uint8_t *src);

void
gcm_aes_digest(struct gcm_aes_ctx *ctx, size_t length, uint8_t *digest);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_GCM_H_INCLUDED */
