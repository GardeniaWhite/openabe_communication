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
 * Implementation of the memory-management routines for temporary double
 * precision digit vectors.
 *
 * @ingroup dv
 */

#include <stdlib.h>
#include <errno.h>

#if ALLOC != AUTO
#include <malloc.h>
#endif

#include "relic_core.h"
#include "relic_conf.h"
#include "relic_dv.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

#if ALLOC == DYNAMIC

void dv_new_dynam(dv_t *a, int digits) {
	if (digits > DV_DIGS) {
		THROW(ERR_NO_PRECI);
	}
#if RLC_ALIGN == 1
	*a = malloc(digits * (DIGIT / 8));
#elif OPSYS == WINDOWS
	*a = _aligned_malloc(digits * (DIGIT / 8), RLC_ALIGN);
#else
	int r = posix_memalign((void **)a, RLC_ALIGN, digits * (DIGIT / 8));
	if (r == ENOMEM) {
		THROW(ERR_NO_MEMORY);
	}
	if (r == EINVAL) {
		THROW(ERR_NO_CONFIG);
	}
#endif

	if (*a == NULL) {
		THROW(ERR_NO_MEMORY);
	}
}

void dv_free_dynam(dv_t *a) {
	if ((*a) != NULL) {
#if OPSYS == WINDOWS && RLC_ALIGN > 1
		_aligned_free(*a);
#else
		free(*a);
#endif
	}
	(*a) = NULL;
}

#endif
