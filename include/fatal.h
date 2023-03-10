/* logging declarations
 *
 * Copyright (C) 1998-2001,2013 D. Hugh Redelmeier <hugh@mimosa.com>
 * Copyright (C) 2004 Michael Richardson <mcr@xelerance.com>
 * Copyright (C) 2012-2013 Paul Wouters <paul@libreswan.org>
 * Copyright (C) 2017-2019 Andrew Cagney <cagney@gnu.org>
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

#ifndef FATAL_H
#define FATAL_H

#include "lswcdefs.h"

enum pluto_exit_code;
struct logger;
struct jambuf;

/*
 * XXX: The message format is:
 *   FATAL ERROR: <log-prefix><message...>
 * and not:
 *   <log-prefix>FATAL ERROR: <message...>
 */

void fatal(enum pluto_exit_code pec, const struct logger *logger,
	   const char *message, ...) PRINTF_LIKE(3) NEVER_RETURNS;
void fatal_errno(enum pluto_exit_code pec, const struct logger *logger,
		 int error, const char *message, ...) PRINTF_LIKE(4) NEVER_RETURNS;

void fatal_jambuf_to_logger(struct jambuf *buf, enum pluto_exit_code pec,
			    const struct logger *logger) NEVER_RETURNS;

#define LLOG_FATAL_JAMBUF(PEC, LOGGER, BUF)				\
	JAMBUF(BUF)							\
	for (jam_logger_rc_prefix(BUF, LOGGER, FATAL_FLAGS);		\
	     BUF != NULL;						\
	     fatal_jambuf_to_logger(BUF, PEC, LOGGER), BUF = NULL)

#endif
