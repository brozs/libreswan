/* default route lookup, for libreswan
 *
 * Copyright (C) 2018,2022 Andrew Cagney
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <https://www.gnu.org/licenses/gpl2.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifndef ADDR_LOOKUP_H
#define ADDR_LOOKUP_H

#include <stdbool.h>

#include "lset.h"

struct starter_end;
struct logger;

enum resolve_status {
	RESOLVE_FAILURE = -1,
	RESOLVE_SUCCESS = 0,
	RESOLVE_PLEASE_CALL_AGAIN = 1,
};

enum resolve_status resolve_defaultroute_one(struct starter_end *host,
					     struct starter_end *peer,
					     lset_t verbose_rc_flags,
					     struct logger *logger);

bool resolve_default_route(struct starter_end *host,
			   struct starter_end *peer,
			   lset_t verbose_rc_flags,
			   struct logger *logger);

#endif
