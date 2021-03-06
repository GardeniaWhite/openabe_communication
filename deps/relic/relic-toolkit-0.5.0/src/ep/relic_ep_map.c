/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (C) 2007-2019 RELIC Authors
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file
 * for contact information.
 *
 * RELIC is free software; you can redistribute it and/or modify it under the
 * terms of the version 2.1 (or later) of the GNU Lesser General Public License
 * as published by the Free Software Foundation; or version 2.0 of the Apache
 * License as published by the Apache Software Foundation. See the LICENSE files
 * for more details.
 *
 * RELIC is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the LICENSE files for more details.
 *
 * You should have received a copy of the GNU Lesser General Public or the
 * Apache License along with RELIC. If not, see <https://www.gnu.org/licenses/>
 * or <https://www.apache.org/licenses/>.
 */

/**
 * @file
 *
 * Implementation of hashing to a prime elliptic curve.
 *
 * @ingroup ep
 */

#include "relic_core.h"
#include "relic_md.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void ep_map(ep_t p, const uint8_t *msg, int len) {
	bn_t k;
	fp_t t;
	uint8_t digest[MD_LEN];

	bn_null(k);
	fp_null(t);

	TRY {
		bn_new(k);
		fp_new(t);

		md_map(digest, msg, len);
		bn_read_bin(k, digest, RLC_MIN(FP_BYTES, MD_LEN));

		fp_prime_conv(p->x, k);
		fp_zero(p->y);
		fp_set_dig(p->z, 1);

		while (1) {
			ep_rhs(t, p);

			if (fp_srt(p->y, t)) {
				p->norm = 1;
				break;
			}
			fp_add_dig(p->x, p->x, 1);
		}

		/* Now, multiply by cofactor to get the correct group. */
		ep_curve_get_cof(k);
		if (bn_bits(k) < DIGIT) {
			ep_mul_dig(p, p, k->dp[0]);
		} else {
			ep_mul(p, p, k);
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(k);
		fp_free(t);
	}
}
