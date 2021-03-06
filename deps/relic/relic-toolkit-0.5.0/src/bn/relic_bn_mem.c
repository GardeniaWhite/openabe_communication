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
 * Implementation of the multiple precision integer memory management routines.
 *
 * @ingroup bn
 */

#include <errno.h>

#if ALLOC != AUTO
#include <malloc.h>
#endif

#include "relic_core.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void bn_init(bn_t a, int digits) {
#if ALLOC == DYNAMIC
	if (digits % BN_SIZE != 0) {
		/* Pad the number of digits to a multiple of the block. */
		digits += (BN_SIZE - digits % BN_SIZE);
	}

	if (a != NULL) {
		a->dp = NULL;
#if RLC_ALIGN == 1
		a->dp = (dig_t *)malloc(digits * sizeof(dig_t));
#elif OPSYS == WINDOWS
		a->dp = _aligned_malloc(digits * sizeof(dig_t), RLC_ALIGN);
#else
		int r = posix_memalign((void **)&a->dp, RLC_ALIGN, digits * sizeof(dig_t));
		if (r == ENOMEM) {
			THROW(ERR_NO_MEMORY);
		}
		if (r == EINVAL) {
			THROW(ERR_NO_VALID);
		}
#endif
	}

	if (a->dp == NULL) {
		free(a);
		THROW(ERR_NO_MEMORY);
	}
#else
	/* Verify if the number of digits is sane. */
	if (digits > BN_SIZE) {
		THROW(ERR_NO_PRECI);
	} else {
		digits = BN_SIZE;
	}
#endif
	if (a != NULL) {
		a->used = 0;
		a->alloc = digits;
		a->sign = BN_POS;
	}
}

void bn_clean(bn_t a) {
#if ALLOC == DYNAMIC
	if (a != NULL) {
		if (a->dp != NULL) {
#if OPSYS == WINDOWS && RLC_ALIGN > 1
			_aligned_free(a->dp);
#else
			free(a->dp);
#endif
			a->dp = NULL;
		}
		a->alloc = 0;
	}
#endif
	if (a != NULL) {
		a->used = 0;
		a->sign = BN_POS;
	}
}

void bn_grow(bn_t a, int digits) {
#if ALLOC == DYNAMIC
	dig_t *t;

	if (a->alloc < digits) {
		/* At least add BN_SIZE more digits. */
		digits += (BN_SIZE * 2) - (digits % BN_SIZE);
		t = (dig_t *)realloc(a->dp, (DIGIT / 8) * digits);
		if (t == NULL) {
			THROW(ERR_NO_MEMORY);
		}
		a->dp = t;
		/* Set the newly allocated digits to zero. */
		a->alloc = digits;
	}
#else /* ALLOC == AUTO || ALLOC == STACK */
	if (digits > BN_SIZE) {
		THROW(ERR_NO_PRECI)
	}
	(void)a;
#endif
}

void bn_trim(bn_t a) {
	while (a->used > 0 && a->dp[a->used - 1] == 0) {
		--(a->used);
	}
	/* Zero can't be negative. */
	if (a->used <= 0) {
		a->used = 1;
		a->dp[0] = 0;
		a->sign = BN_POS;
	}
}
