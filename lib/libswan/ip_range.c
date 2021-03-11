/* ip_range type, for libreswan
 *
 * Copyright (C) 2007 Michael Richardson <mcr@xelerance.com>
 * Copyright (C) 2000 Henry Spencer.
 * Copyright (C) 2013 Antony Antony <antony@phenome.org>
 * Copyright (C) 2019 Andrew Cagney <cagney@gnu.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <https://www.gnu.org/licenses/lgpl-2.1.txt>.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 */

/*
 * convert from text form of IP address range specification to binary;
 * and more minor utilities for mask length calculations for IKEv2
 */

#include <string.h>
#include <arpa/inet.h>		/* for ntohl() */

#include "jambuf.h"
#include "ip_range.h"
#include "ip_info.h"
#include "passert.h"
#include "lswlog.h"		/* for pexpect() */

const ip_range unset_range; /* all zeros */

ip_range range2(const ip_address *start, const ip_address *end)
{
	/* does the caller know best? */
	const struct ip_info *st = address_type(start);
	const struct ip_info *et = address_type(end);
	passert(st == et);
	bool ss = address_is_unset(start);
	bool es = address_is_unset(end);
	passert(ss == es);
	ip_range r = {
		.start = *start,
		.end = *end,
	};
	return r;
}

/*
 * Calculate the number of significant bits in the size of the range.
 * floor(lg(|high-low| + 1))
 *
 * ??? this really should use ip_range rather than a pair of ip_address values
 */

int range_host_bits(const ip_range range)
{
	const struct ip_info *afi = range_type(&range);
	if (afi == NULL) {
		/* NULL+unset+unknown */
		return -1;
	}

	struct ip_bytes diff = bytes_diff(afi, range.end.bytes, range.start.bytes);
	int fsb = bytes_first_set_bit(afi, diff);
	return (afi->ip_size * 8) - fsb;
}

/*
 * ttorange - convert text v4 "addr1-addr2" to address_start address_end
 *            v6 allows "subnet/mask" to address_start address_end
 */
err_t ttorange(const char *src, const struct ip_info *afi, ip_range *dst)
{
	*dst = unset_range;
	err_t err;

	/* START or START/MASK or START-END */
	shunk_t end = shunk1(src);
	char sep = '\0';
	shunk_t start = shunk_token(&end, &sep, "/-");

	/* convert start address */
	ip_address start_address;
	err = numeric_to_address(start, afi, &start_address);
	if (err != NULL) {
		return err;
	}

	if (address_is_any(&start_address)) {
		/* XXX: being more specific would mean diag_t */
		return "0.0.0.0 or :: not allowed in range";
	}

	/* get real AFI */
	afi = address_type(&start_address);
	if (afi == NULL) {
		/* should never happen */
		return "INTERNAL ERROR: ttorange() encountered an unknown type";
	}

	switch (sep) {
	case '\0':
	{
		/* single address */
		*dst = (ip_range) {
			.start = start_address,
			.end = start_address,
		};
		return NULL;
	}
	case '/':
	{
		/* START/MASK */
		uintmax_t maskbits = afi->mask_cnt;
		err = shunk_to_uintmax(end, NULL, 0, &maskbits, afi->mask_cnt);
		if (err != NULL) {
			return err;
		}
		/* XXX: should this reject bad addresses */
		*dst = (ip_range) {
			.start = address_from_blit(afi, start_address.bytes,
						   /*routing-prefix*/&keep_bits,
						   /*host-identifier*/&clear_bits,
						   maskbits),
			.end = address_from_blit(afi, start_address.bytes,
						 /*routing-prefix*/&keep_bits,
						 /*host-identifier*/&set_bits,
						 maskbits),
		};
		dst->is_subnet = (afi == &ipv6_info);
		return NULL;
	}
	case '-':
	{
		/* START-END */
		ip_address end_address;
		err = numeric_to_address(end, afi, &end_address);
		if (err != NULL) {
			return err;
		}
		if (addrcmp(&start_address, &end_address) > 0) {
			return "start of range must not be greater than end";
		}
		*dst = (ip_range) {
			.start = start_address,
			.end = end_address,
		};
		return NULL;
	}
	}
	/* SEP is invalid, but being more specific means diag_t */
	return "error";
}

size_t jam_range(struct jambuf *buf, const ip_range *range)
{
	if (range_is_unset(range)) {
		return jam_string(buf, "<unset-range>");
	}

	const struct ip_info *afi = range_type(range);
	if (afi == NULL) {
		return jam_string(buf, "<unknown-range>");
	}

	size_t s = 0;
	s += afi->jam_address(buf, afi, &range->start.bytes);
	/* when a subnet, try to calculate the prefix-bits */
	int prefix_bits = (range->is_subnet ? bytes_prefix_bits(afi, range->start.bytes, range->end.bytes) : -1);
	if (prefix_bits >= 0) {
		s += jam(buf, "/%d", prefix_bits);
	} else {
		s += jam(buf, "-");
		s += afi->jam_address(buf, afi, &range->end.bytes);
	}
	return s;
}

const char *str_range(const ip_range *range, range_buf *out)
{
	struct jambuf buf = ARRAY_AS_JAMBUF(out->buf);
	jam_range(&buf, range);
	return out->buf;
}

ip_range range_from_subnet(const ip_subnet subnet)
{
	const struct ip_info *afi = subnet_type(&subnet);
	if (afi == NULL) {
		/* NULL+unset+unknown */
		return unset_range;
	}

	ip_range r = {
		.start = address_from_blit(afi, subnet.bytes,
					   /*routing-prefix*/&keep_bits,
					   /*host-identifier*/&clear_bits,
					   subnet.maskbits),
		.end = address_from_blit(afi, subnet.bytes,
					 /*routing-prefix*/&keep_bits,
					 /*host-identifier*/&set_bits,
					 subnet.maskbits),
	};
	return r;
}

const struct ip_info *range_type(const ip_range *range)
{
	if (range_is_unset(range)) {
		return NULL;
	}

	/* may return NULL */
	const struct ip_info *start = ip_version_info(range->start.version);
	const struct ip_info *end = ip_version_info(range->end.version);
	if (!pexpect(start == end)) {
		return NULL;
	}
	return start;
}

bool range_is_unset(const ip_range *range)
{
	if (range == NULL) {
		return true;
	}
	return thingeq(*range, unset_range);
}

bool range_is_specified(const ip_range range)
{
	const struct ip_info *afi = range_type(&range);
	if (afi == NULL) {
		/* NULL+unset+unknown */
		return false;
	}

	bool start = address_is_specified(&range.start);
	bool end = address_is_specified(&range.end);
	if (!pexpect(start == end)) {
		return false;
	}
	return start;
}

bool range_size(const ip_range range, uint32_t *size)
{
	*size = 0;

	const struct ip_info *afi = range_type(&range);
	if (afi == NULL) {
		return true; /*return what?!?!?*/
	}

	struct ip_bytes diff = bytes_diff(afi, range.start.bytes, range.end.bytes);

	/* more than 32-bits of host-prefix always overflows. */
	unsigned prefix_bits = bytes_first_set_bit(afi, diff);
	unsigned host_bits = afi->mask_cnt - prefix_bits;
	if (host_bits > 32) {
		*size = UINT32_MAX;
		return true;
	}

	/* can't overflow; but could be 0xffffffff */
	uint32_t n = ntoh_bytes(diff.byte, afi->ip_size);

	/* adding 1 to 0xffffffff overflows */
	if (n == UINT32_MAX) {
		*size = UINT32_MAX;
		return true;
	}

	/* ::1-::1 is one address */
	*size = n + 1;
	return false;
}

bool range_eq(const ip_range l, const ip_range r)
{
	if (range_is_unset(&l) && range_is_unset(&r)) {
		/* unset/NULL ranges are equal */
		return true;
	}
	if (range_is_unset(&l) || range_is_unset(&r)) {
		return false;
	}

	return (bytes_cmp(l.start.version, l.start.bytes,
			  r.start.version, r.start.bytes) == 0 &&
		bytes_cmp(l.end.version, l.end.bytes,
			  r.end.version, r.end.bytes) == 0);
}

bool address_in_range(const ip_address address, const ip_range range)
{
	if (address_is_unset(&address) || range_is_unset(&range)) {
		return false;
	}

	return (bytes_cmp(address.version, address.bytes,
			  range.start.version, range.start.bytes) >= 0 &&
		bytes_cmp(address.version, address.bytes,
			  range.end.version, range.end.bytes) <= 0);
}

bool range_in(const ip_range inner, const ip_range outer)
{
	if (range_is_unset(&inner) || range_is_unset(&outer)) {
		return false;
	}

	return (bytes_cmp(inner.start.version, inner.start.bytes,
			  outer.start.version, outer.start.bytes) >= 0 &&
		bytes_cmp(inner.end.version, inner.end.bytes,
			  outer.end.version, outer.end.bytes) <= 0);
}

ip_address range_start(const ip_range range)
{
	const struct ip_info *afi = range_type(&range);
	if (afi == NULL) {
		return unset_address;
	}

	return address_from_raw(afi, range.start.bytes);
}

ip_address range_end(const ip_range range)
{
	const struct ip_info *afi = range_type(&range);
	if (afi == NULL) {
		return unset_address;
	}

	return address_from_raw(afi, range.end.bytes);
}

bool range_overlap(const ip_range l, const ip_range r)
{
	if (range_is_unset(&l) || range_is_unset(&r)) {
		/* presumably overlap is bad */
		return false;
	}

	/* l before r */
	if (bytes_cmp(l.end.version, l.end.bytes,
		      r.start.version, r.start.bytes) < 0) {
		return false;
	}
	/* l after r */
	if (bytes_cmp(l.start.version, l.start.bytes,
		      r.end.version, r.end.bytes) > 0) {
		return false;
	}

	return true;
}

err_t addresses_to_range(const ip_address start, const ip_address end,
			 ip_range *dst)
{
	*dst = unset_range;

	if (address_is_unset(&start)) {
		/* NULL+unset+unknown */
		return "start address invalid";
	}

	if (address_is_unset(&end)) {
		/* NULL+unset+unknown */
		return "end address invalid";
	}

	if (start.version != end.version) {
		return "conflicting address types";
	}

	/* need both 0 */
	if (address_is_any(&start) && address_is_any(&end)) {
		return "empty address range";
	}

	if (addrcmp(&start, &end) > 0) {
		return "out-of-order";
	}

	*dst = range2(&start, &end);
	return NULL;
}

err_t range_to_subnet(const ip_range range, ip_subnet *dst)
{
	*dst = unset_subnet;
	const struct ip_info *afi = range_type(&range);
	if (afi == NULL) {
		return "invalid range";
	}

	/*
	 * Determine the prefix_bits (the CIDR network part) by
	 * matching leading bits of FROM and TO.  Trailing bits
	 * (subnet address) must be either all 0 (from) or 1 (to).
	 */
	int prefix_bits = bytes_prefix_bits(afi, range.start.bytes, range.end.bytes);
	if (prefix_bits < 0) {
		return "address range is not a subnet";
	}

	*dst = subnet_from_raw(afi->ip_version, range.start.bytes, prefix_bits);
	return NULL;
}
