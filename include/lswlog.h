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

#ifndef _LSWLOG_H_
#define _LSWLOG_H_

#include <stdarg.h>
#include <stdio.h>		/* for FILE */
#include <stddef.h>		/* for size_t */

#include "lset.h"
#include "lswcdefs.h"
#include "jambuf.h"
#include "passert.h"
#include "constants.h"		/* for DBG_... */
#include "where.h"		/* used by macros */
#include "fd.h"			/* for null_fd */
#include "impair.h"
#include "pexpect.h"
#include "fatal.h"

#define LOG_WIDTH	((size_t)1024)	/* roof of number of chars in log line */

extern bool log_to_stderr;          /* should log go to stderr? */


/*
 * Codes for status messages returned to whack.
 *
 * These are 3 digit decimal numerals.  The structure is inspired by
 * section 4.2 of RFC959 (FTP).  Since these will end up as the exit
 * status of whack, they must be less than 256.
 *
 * NOTE: ipsec_auto(8) knows about some of these numbers -- change
 * carefully.
 */

enum rc_type {
	RC_COMMENT,		/* non-commital utterance with 000 prefix(does not affect exit status) */
	RC_RAW,			/* ditto, but also suppresses the 000 prefix */
	RC_LOG,			/* message aimed at log (does not affect exit status) */
	RC_LOG_SERIOUS,		/* serious message aimed at log (does not affect exit status) */
	RC_SUCCESS,		/* success (exit status 0) */
	RC_INFORMATIONAL,	/* should get relayed to user - if there is one */
	RC_INFORMATIONAL_TRAFFIC, /* status of an established IPSEC (aka Phase 2) state */

	/* failure, but not definitive */
	RC_RETRANSMISSION = 10,

	/* improper request */
	RC_EXIT_FLOOR = 20,
	RC_DUPNAME = RC_EXIT_FLOOR,	/* attempt to reuse a connection name */
	RC_UNKNOWN_NAME,	/* connection name unknown or state number */
	RC_ORIENT,		/* cannot orient connection: neither end is us */
	RC_CLASH,		/* clash between two Road Warrior connections OVERLOADED */
	RC_DEAF,		/* need --listen before --initiate */
	RC_ROUTE,		/* cannot route */
	RC_RTBUSY,		/* cannot unroute: route busy */
	RC_BADID,		/* malformed --id */
	RC_NOKEY,		/* no key found through DNS */
	RC_NOPEERIP,		/* cannot initiate when peer IP is unknown */
	RC_INITSHUNT,		/* cannot initiate a shunt-oly connection */
	RC_WILDCARD,		/* cannot initiate when ID has wildcards */
	RC_CRLERROR,		/* CRL fetching disabled or obsolete reread cmd */
	RC_WHACK_PROBLEM,	/* whack-detected problem */

	/* permanent failure */
	RC_BADWHACKMESSAGE = 30,
	RC_NORETRANSMISSION,
	RC_INTERNALERR,
	RC_OPPOFAILURE,		/* Opportunism failed */
	RC_CRYPTOFAILED,	/* system too busy to perform required
				* cryptographic operations */
	RC_AGGRALGO,		/* multiple algorithms requested in phase 1 aggressive */
	RC_FATAL,		/* fatal error encountered, and negotiation aborted */

	/* entry of secrets */
	RC_ENTERSECRET = 40,
	RC_USERPROMPT = 41,

	RC_EXIT_ROOF = 100,

	/*
	 * progress: start of range for successful state transition.
	 * Actual value is RC_NEW_V[12]_STATE plus the new state code.
	 */
	RC_NEW_V1_STATE = RC_EXIT_ROOF,
	RC_NEW_V2_STATE = 150,

	/*
	 * Start of range for notification.
	 *
	 * Actual value is RC_NOTIFICATION plus code for notification
	 * that should be generated by this Pluto.  RC_NOTIFICATION.
	 * Since notifications are two octets, that's 64435+200 which
	 * overflows the 3-digit prefix, oops.
	 */
	RC_NOTIFICATION = 200,	/* as per IKE notification messages */
};


/*
 * A generic buffer for accumulating unbounded output.
 *
 * The buffer's contents can be directed to various logging streams.
 */

struct jambuf;

/*
 * By default messages are broadcast (to both log files and whack),
 * mix-in one of these options to limit this.
 *
 * This means that a simple RC_* code will go to both whack and and
 * the log files.
 */

#define RC_MASK              0x00fffff	/* rc_type max is 64435+200 */
#define STREAM_MASK          0x0f00000
#define LOG_PREFIX_MASK	     0xf000000

enum log_prefix {
	AUTO_PREFIX =        0x0000000,
	NO_PREFIX =          0x1000000,
        ADD_PREFIX =         0x2000000,
};

enum stream {
	/*                                 syslog()                      */
	/*                                Severity  Whack  Tools  Prefix */
	ALL_STREAMS        = 0x0000000, /* WARNING   yes    err?   <o>   */
	LOG_STREAM         = 0x0100000, /* WARNING    no    err?   <o>   */
	WHACK_STREAM       = 0x0200000, /*   N/A     yes    err    <o>   */
	DEBUG_STREAM       = 0x0300000, /*  DEBUG     no    err    | <o> */
	ERROR_STREAM       = 0x0400000, /*   ERR     yes    err    <o>   */
	PEXPECT_STREAM     = 0x0500000, /*   ERR     yes    err    EXPECTATION FAILED: <o> */
	PASSERT_STREAM     = 0x0600000, /*   ERR     yes    err    ABORT: ASSERTION_FAILED: <o> */
	FATAL_STREAM       = 0x0700000, /*   ERR     yes    err    FATAL ERROR: <o> */
	NO_STREAM          = 0x0f00000, /*   N/A     N/A                 */
	/*
	 * <o>: add prefix when object is available
	 *
	 * | <o>: add both "| " and prefex when object is available and
         * feature is enabled
	 *
	 * err?: write to stderr when enabled (tests log_to_stderr,
	 * typically via -v).  Used by tools such as whack.
	 */
};

#define DEBUG_PREFIX		"| "
#define ERROR_PREFIX		"ERROR: "
#define PEXPECT_PREFIX		"EXPECTATION FAILED: "
#define PASSERT_PREFIX		"FATAL: ASSERTION FAILED: "
#define FATAL_PREFIX		"FATAL ERROR: "

#define DEBUG_FLAGS		(DEBUG_STREAM)
#define PEXPECT_FLAGS		(PEXPECT_STREAM|RC_LOG_SERIOUS)
#define PASSERT_FLAGS		(PASSERT_STREAM|RC_LOG_SERIOUS)
#define ERROR_FLAGS		(ERROR_STREAM|RC_LOG_SERIOUS)
#define FATAL_FLAGS		(FATAL_STREAM|RC_LOG_SERIOUS)
#define PRINTF_FLAGS		(NO_PREFIX|WHACK_STREAM)

/*
 * Broadcast a log message.
 *
 * By default send it to the log file and any attached whacks (both
 * globally and the object).
 *
 * If any *_STREAM flag is specified then only send the message to
 * that stream.
 *
 * llog() is a catch-all for code that may or may not have ST.
 * For instance a responder decoding a message may not yet have
 * created the state.  It will will use ST, MD, or nothing as the
 * prefix, and logs to ST's whackfd when possible.
 */

struct logger_object_vec {
	const char *name;
	bool free_object;
	size_t (*jam_object_prefix)(struct jambuf *buf, const void *object);
#define jam_logger_prefix(BUF, LOGGER) (LOGGER)->object_vec->jam_object_prefix(BUF, (LOGGER)->object)
	/*
	 * When opportunistic encryption or the initial responder, for
	 * instance, some logging is suppressed.
	 */
	bool (*suppress_object_log)(const void *object);
#define suppress_log(LOGGER) (LOGGER)->object_vec->suppress_object_log((LOGGER)->object)
};

void jam_logger_rc_prefix(struct jambuf *buf, const struct logger *logger, lset_t rc_flags);

bool suppress_object_log_false(const void *object);
bool suppress_object_log_true(const void *object);
size_t jam_object_prefix_none(struct jambuf *buf, const void *object);

#ifndef GLOBAL_LOGGER
extern const struct logger global_logger;
#define GLOBAL_LOGGER
#endif

struct logger {
	struct fd *global_whackfd;
	struct fd *object_whackfd;
	const void *object;
	const struct logger_object_vec *object_vec;
	where_t where;
	/* used by timing to nest its logging output */
	int timing_level;
	lset_t debugging;
};

#define PRI_LOGGER "logger@%p/"PRI_FD"/"PRI_FD
#define pri_logger(LOGGER) (LOGGER), (LOGGER) == NULL ? NULL : (LOGGER)->global_whackfd, (LOGGER) == NULL ? NULL : (LOGGER)->object_whackfd

void llog(lset_t rc_flags,
	  const struct logger *log,
	  const char *format, ...) PRINTF_LIKE(3);

void llog_va_list(lset_t rc_flags, const struct logger *logger,
		  const char *message, va_list ap) VPRINTF_LIKE(3);

void jambuf_to_logger(struct jambuf *buf, const struct logger *logger, lset_t rc_flags);

#define LLOG_JAMBUF(RC_FLAGS, LOGGER, BUF)				\
	JAMBUF(BUF)							\
		for (jam_logger_rc_prefix(BUF, LOGGER, RC_FLAGS);	\
		     BUF != NULL;					\
		     jambuf_to_logger(BUF, (LOGGER), RC_FLAGS), BUF = NULL)

void llog_dump(lset_t rc_flags,
	       const struct logger *log,
	       const void *p, size_t len);
#define llog_dump_hunk(RC_FLAGS, LOGGER, HUNK)				\
	{								\
		const typeof(HUNK) *hunk_ = &(HUNK); /* evaluate once */ \
		llog_dump(RC_FLAGS, LOGGER, hunk_->ptr, hunk_->len);	\
	}

void llog_base64_bytes(lset_t rc_flags,
		       const struct logger *log,
		       const void *p, size_t len);
#define llog_base64_hunk(RC_FLAGS, LOGGER, HUNK)			\
	{								\
		const typeof(HUNK) *hunk_ = &(HUNK); /* evaluate once */ \
		llog_base64_bytes(RC_FLAGS, LOGGER, hunk_->ptr, hunk_->len); \
	}

void llog_pem_bytes(lset_t rc_flags,
		    const struct logger *log,
		    const char *name,
		    const void *p, size_t len);
#define llog_pem_hunk(RC_FLAGS, LOGGER, NAME, HUNK)			\
	{								\
		const typeof(HUNK) *hunk_ = &(HUNK); /* evaluate once */ \
		llog_pem_bytes(RC_FLAGS, LOGGER, NAME, hunk_->ptr, hunk_->len); \
	}

/*
 * Wrap <message> in a prefix and suffix where the suffix contains
 * errno and message.
 *
 * Notes:
 *
 * Because __VA_ARGS__ may contain function calls that modify ERRNO,
 * errno's value is first saved.
 *
 * While these common-case macros are implemented as wrapper functions
 * so that backtrace will include the below function call and that
 * _includes_ the MESSAGE parameter - makes debugging much easier.
 */

void libreswan_exit(enum pluto_exit_code rc) NEVER_RETURNS;

/*
 * XXX: The message format is:
 *   ERROR: <log-prefix><message...>[: <strerr> (errno)]
 * and not:
 *   <log-prefix>ERROR: <message...>...
 */

void log_error(const struct logger *logger, int error,
	       const char *message, ...) PRINTF_LIKE(3);

#define llog_error(LOGGER, ERRNO, FMT, ...)				\
	{								\
		int e_ = ERRNO; /* save value across va args */		\
		log_error(LOGGER, e_, FMT, ##__VA_ARGS__); \
	}

/* like log_error() but no ERROR: prefix and/or ": " separator */
void llog_errno(lset_t rc_flags, const struct logger *logger, int error,
		const char *message, ...) PRINTF_LIKE(4);

/*
 * Log debug messages to the main log stream, but not the WHACK log
 * stream.
 *
 * NOTE: DBG's action can be a { } block, but that block must not
 * contain commas that are outside quotes or parenthesis.
 * If it does, they will be interpreted by the C preprocesser
 * as macro argument separators.  This happens accidentally if
 * multiple variables are declared in one declaration.
 *
 * Naming: All DBG_*() prefixed functions send stuff to the debug
 * stream unconditionally.  Hence they should be wrapped in DBGP().
 */

extern lset_t cur_debugging;	/* current debugging level */

#define DBGP(cond)	(cur_debugging & (cond))

#define DBGF(COND, MESSAGE, ...)				\
	{							\
		if (DBGP(COND)) {				\
			DBG_log(MESSAGE, ##__VA_ARGS__);	\
		}						\
	}

#define dbg(MESSAGE, ...)					\
	{							\
		if (DBGP(DBG_BASE)) {				\
			DBG_log(MESSAGE, ##__VA_ARGS__);	\
		}						\
	}

void ldbg(const struct logger *logger, const char *message, ...) PRINTF_LIKE(2);

/* LDBG_JAMBUF() is ambigious - LDBG_op() or ldbg() ucase? */
#define LDBGP_JAMBUF(COND, LOGGER, BUF)			\
	for (bool cond_ = COND; cond_; cond_ = false)	\
		LLOG_JAMBUF(DEBUG_STREAM, LOGGER, BUF)


/* DBG_*() are unconditional */
void DBG_log(const char *message, ...) PRINTF_LIKE(1);
void DBG_dump(const char *label, const void *p, size_t len);
#define DBG_dump_hunk(LABEL, HUNK)					\
	{								\
		const typeof(HUNK) *hunk_ = &(HUNK); /* evaluate once */ \
		DBG_dump(LABEL, hunk_->ptr, hunk_->len);		\
	}
#define DBG_dump_thing(LABEL, THING) DBG_dump(LABEL, &(THING), sizeof(THING))

/* LDBG_*(logger, ...) are unconditional wrappers */
#define LDBG_log(LOGGER, ...) llog(DEBUG_STREAM, LOGGER, __VA_ARGS__)
#define LDBG_va_list(LOGGER, ...) llog_va_list(DEBUG_STREAM, LOGGER, __VA_ARGS__)
#define LLOG_dump(LOGGER, ...) llog_dump(DEBUG_STREAM, LOGGER, __VA_ARGS__)

/*
 * Code wrappers that cover up the details of allocating,
 * initializing, de-allocating (and possibly logging) a 'struct
 * lswlog' buffer.
 *
 * BUF (a C variable name) is declared locally as a pointer to a
 * per-thread 'struct jambuf' buffer.
 *
 * Implementation notes:
 *
 * This implementation stores the output in an array on the thread's
 * stack.  It could just as easily use the heap (but that would
 * involve memory overheads) or even a per-thread static variable.
 * Since the BUF variable is a pointer the specifics of the
 * implementation are hidden.
 *
 * This implementation, unlike DBG(), does not have a code block
 * parameter.  Instead it uses a sequence of for-loops to set things
 * up for a code block.  This avoids problems with "," within macro
 * parameters confusing the parser.  It also permits a simple
 * consistent indentation style.
 *
 * The stack array is left largely uninitialized (just a few strategic
 * entries are set).  This avoids the need to zero LOG_WITH bytes.
 *
 * Apparently chaining void function calls using a comma is valid C?
 */

/*
 * Scratch buffer for accumulating extra output.
 *
 * XXX: case should be expanded to illustrate how to stuff a truncated
 * version of the output into the LOG buffer.
 *
 * For instance:
 */

#if 0
void lswbuf(struct jambuf *log)
{
	LSWBUF(buf) {
		jam(buf, "written to buf");
		lswlogl(log, buf); /* add to calling array */
	}
}
#endif

/* for a switch statement */

void libreswan_bad_case(const char *expression, long value, where_t where) NEVER_RETURNS;

#define bad_case(N)	libreswan_bad_case(#N, (N), HERE)

#define impaired_passert(BEHAVIOUR, LOGGER, ASSERTION)			\
	{								\
		if (impair.BEHAVIOUR) {					\
			bool assertion_ = ASSERTION;			\
			if (!assertion_) {				\
				llog(RC_LOG, LOGGER,		\
					    "IMPAIR: assertion '%s' failed", \
					    #ASSERTION);		\
			}						\
		} else {						\
			passert(ASSERTION);				\
		}							\
	}

#endif /* _LSWLOG_H_ */
