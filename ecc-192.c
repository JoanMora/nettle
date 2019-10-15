/* ecc-192.c

   Compile time constant (but machine dependent) tables.

   Copyright (C) 2013, 2014, 2019 Niels Möller
   Copyright (C) 2019 Wim Lewis

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

/* Development of Nettle's ECC support was funded by the .SE Internet Fund. */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>

/* FIXME: Remove ecc.h include, once prototypes of more internal
   functions are moved to ecc-internal.h */
#include "ecc.h"
#include "ecc-internal.h"

#define USE_REDC 0

#include "ecc-192.h"

#if HAVE_NATIVE_ecc_192_modp

#define ecc_192_modp nettle_ecc_192_modp
void
ecc_192_modp (const struct ecc_modulo *m, mp_limb_t *rp);

/* Use that p = 2^{192} - 2^64 - 1, to eliminate 128 bits at a time. */

#elif GMP_NUMB_BITS == 32
/* p is 6 limbs, p = B^6 - B^2 - 1 */
static void
ecc_192_modp (const struct ecc_modulo *m UNUSED, mp_limb_t *rp)
{
  mp_limb_t cy;

  /* Reduce from 12 to 9 limbs (top limb small)*/
  cy = mpn_add_n (rp + 2, rp + 2, rp + 8, 4);
  cy = sec_add_1 (rp + 6, rp + 6, 2, cy);
  cy += mpn_add_n (rp + 4, rp + 4, rp + 8, 4);
  assert (cy <= 2);

  rp[8] = cy;

  /* Reduce from 9 to 6 limbs */
  cy = mpn_add_n (rp, rp, rp + 6, 3);
  cy = sec_add_1 (rp + 3, rp + 3, 2, cy);
  cy += mpn_add_n (rp + 2, rp + 2, rp + 6, 3);
  cy = sec_add_1 (rp + 5, rp + 5, 1, cy);
  
  assert (cy <= 1);
  cy = cnd_add_n (cy, rp, ecc_Bmodp, 6);
  assert (cy == 0);  
}
#elif GMP_NUMB_BITS == 64
/* p is 3 limbs, p = B^3 - B - 1 */
static void
ecc_192_modp (const struct ecc_modulo *m UNUSED, mp_limb_t *rp)
{
  mp_limb_t cy;

  /* Reduce from 6 to 5 limbs (top limb small)*/
  cy = mpn_add_n (rp + 1, rp + 1, rp + 4, 2);
  cy = sec_add_1 (rp + 3, rp + 3, 1, cy);
  cy += mpn_add_n (rp + 2, rp + 2, rp + 4, 2);
  assert (cy <= 2);

  rp[4] = cy;

  /* Reduce from 5 to 4 limbs (high limb small) */
  cy = mpn_add_n (rp, rp, rp + 3, 2);
  cy = sec_add_1 (rp + 2, rp + 2, 1, cy);
  cy += mpn_add_n (rp + 1, rp + 1, rp + 3, 2);

  assert (cy <= 1);
  cy = cnd_add_n (cy, rp, ecc_Bmodp, 3);
  assert (cy == 0);  
}
  
#else
#define ecc_192_modp ecc_mod
#endif

/* If there are extra bits, ecc_192_zero_p would need an initial
   reduction. */
#if ECC_LIMB_SIZE * GMP_NUMB_BITS != 192
#error Unsupported limb size
#endif

static int
ecc_192_zero_p (const struct ecc_modulo *p, const volatile mp_limb_t *rp)
{
  volatile mp_limb_t is_non_zero, is_not_p;
  mp_size_t i;

  for (i = 0, is_non_zero = is_not_p = 0; i < ECC_LIMB_SIZE; i++)
    {
      is_non_zero |= rp[i];
      is_not_p |= (rp[i] ^ p->m[i]);
    }

  return (is_non_zero == 0) | (is_not_p == 0);
}


#define ECC_192_SQRT_ITCH (6*ECC_LIMB_SIZE)

static int
ecc_192_sqrt (const struct ecc_modulo *p,
	      mp_limb_t *rp,
	      const mp_limb_t *cp,
	      mp_limb_t *scratch)
{
  /* This computes the square root modulo p192 using the identity:

     sqrt(c) = c^(2^190 - 2^62)  (mod P-192)

     which can be seen as a special case of Tonelli-Shanks with e=1.
  */

  /* We use our scratch space for three temporaries (TA, TB, TC) all of
     which are 2*size long to allow for multiplication/squaring */

#define TA scratch
#define TB (scratch + 2*ECC_LIMB_SIZE)
#define TC (scratch + 4*ECC_LIMB_SIZE)

  ecc_mod_pow_2k_mul(p, TA, cp, 1, cp,  TB);   /* [1] TA <-- c^3              */
  ecc_mod_pow_2k_mul(p, TB, TA, 2, TA,  TC);   /* [2] TB <-- c^(2^4 - 1)      */
  ecc_mod_pow_2k_mul(p, TA, TB, 4, TB,  TC);   /* [3] TA <-- c^(2^8 - 1)      */
  ecc_mod_pow_2k_mul(p, TB, TA, 8, TA,  TC);   /* [4] TB <-- c^(2^16 - 1)     */
  ecc_mod_pow_2k_mul(p, TA, TB, 16, TB, TC);   /* [5] TA <-- c^(2^32 - 1)     */
  ecc_mod_pow_2k_mul(p, TB, TA, 32, TA, TC);   /* [6] TB <-- c^(2^64 - 1)     */
  ecc_mod_pow_2k_mul(p, TC, TB, 64, TB, TA);   /* [7] TC <-- c^(2^128 - 1)    */

  ecc_mod_pow_2k    (p, rp, TC,     62, TA);   /* [8] r  <-- c^(2^190 - 2^62) */

  /* Check that input was a square, R^2 = C, for non-squares we'd get
     R^2 = -C. */
  ecc_mod_sqr(p, TA, rp);
  ecc_mod_sub(p, TA, TA, cp);

  return ecc_192_zero_p (p, TA);

#undef TA
#undef TB
#undef TC
}


const struct ecc_curve _nettle_secp_192r1 =
{
  {
    192,
    ECC_LIMB_SIZE,
    ECC_BMODP_SIZE,
    ECC_REDC_SIZE,
    ECC_MOD_INV_ITCH (ECC_LIMB_SIZE),
    ECC_192_SQRT_ITCH,
    0,

    ecc_p,
    ecc_Bmodp,
    ecc_Bmodp_shifted,    
    ecc_redc_ppm1,
    ecc_pp1h,

    ecc_192_modp,
    ecc_192_modp,
    ecc_mod_inv,
    ecc_192_sqrt,
    NULL,
  },
  {
    192,
    ECC_LIMB_SIZE,
    ECC_BMODQ_SIZE,
    0,
    ECC_MOD_INV_ITCH (ECC_LIMB_SIZE),
    0,
    0,

    ecc_q,
    ecc_Bmodq,
    ecc_Bmodq_shifted,
    NULL,
    ecc_qp1h,

    ecc_mod,
    ecc_mod,
    ecc_mod_inv,
    NULL,
    NULL
  },
  
  USE_REDC,
  ECC_PIPPENGER_K,
  ECC_PIPPENGER_C,

  ECC_ADD_JJJ_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_A_ITCH (ECC_LIMB_SIZE),
  ECC_MUL_G_ITCH (ECC_LIMB_SIZE),
  ECC_J_TO_A_ITCH (ECC_LIMB_SIZE),

  ecc_add_jjj,
  ecc_mul_a,
  ecc_mul_g,
  ecc_j_to_a,

  ecc_b,
  ecc_g,
  NULL,
  ecc_unit,
  ecc_table
};

const struct ecc_curve *nettle_get_secp_192r1(void)
{
  return &_nettle_secp_192r1;
}
