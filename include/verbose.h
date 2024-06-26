/* verbose wrapper around logger
 *
 * Copyright (C) 2024  Andrew Cagney
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
 *
 */

#ifndef VERBOSE_H
#define VERBOSE_H

/*
 * Pass-by-value wrapper around logger to make it easy to generate
 * indented debug/verbose logs.
 *
 * Standalone tools, such as <<ipsec showroute>>, can enable more
 * verbose logging when --verbose is specified, vis:
 *
 *	struct verbose verbose = {
 *		.logger = logger,
 *		.rc_flags = (verbose ? LOG_STREAM : 0),
 *	};
 *
 * While pluto, internally, enables more verbose debug logging:
 *
 *	struct verbose verbose = {
 *		.logger = logger,
 *		.rc_flags = (DBGP(DBG_BASE) ? DEBUG_STREAM : 0),
 *	};
 *
 * Functions then pass verbose by value, and increment .level as
 * needed.
 */

#include "lset.h"

struct verbose {
	const struct logger *logger;
	lset_t rc_flags;
	int level;
};

/*
 * Log, but only when VERBOSE is enabled.
 */
#define vlog(FMT, ...)							\
	{								\
		if (verbose.rc_flags) {					\
			llog(verbose.rc_flags, verbose.logger,		\
			     "%*s"FMT,					\
			     verbose.level * 2, "", ##__VA_ARGS__);	\
		}							\
	}

/*
 * Debug-log using VERBOSE, but only when debugging is enabled.
 */

#define vdbg(FMT, ...)							\
	{								\
		if (DBGP(DBG_BASE)) {					\
			llog(DEBUG_STREAM, verbose.logger,		\
			     "%*s"FMT,					\
			     verbose.level * 2, "", ##__VA_ARGS__);	\
		}							\
	}

#endif
