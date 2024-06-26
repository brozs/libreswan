/* crypto interfaces
 *
 * Copyright (C) 1998-2001,2013 D. Hugh Redelmeier <hugh@mimosa.com>
 * Copyright (C) 2003-2008 Michael C. Richardson <mcr@xelerance.com>
 * Copyright (C) 2003-2010 Paul Wouters <paul@xelerance.com>
 * Copyright (C) 2009-2012 Avesh Agarwal <avagarwa@redhat.com>
 * Copyright (C) 2012-2013 Paul Wouters <paul@libreswan.org>
 * Copyright (C) 2013 Florian Weimer <fweimer@redhat.com>
 * Copyright (C) 2016 Andrew Cagney <cagney@gnu.org>
 * Copyright (C) 2018 Sahana Prasad <sahana.prasad07@gmail.com>
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

#include "constants.h"		/* for BYTES_FOR_BITS() */
#include "ietf_constants.h"
#include "ike_alg.h"
#include "ike_alg_hash.h"
#include "ike_alg_hash_ops.h"

static const uint8_t asn1_pkcs1_1_5_rsa_sha1_blob[1+ASN1_PKCS1_1_5_RSA_SIZE] = {
	ASN1_PKCS1_1_5_RSA_SIZE,
	ASN1_PKCS1_1_5_RSA_SHA1_BLOB
};
static const uint8_t asn1_ecdsa_sha1_blob[1+ASN1_ECDSA_SHA1_SIZE] = {
	ASN1_ECDSA_SHA1_SIZE,
	ASN1_ECDSA_SHA1_BLOB
};

const struct hash_desc ike_alg_hash_sha1 = {
	.common = {
		.fqn = "SHA1",
		.names = "sha,sha1",
		.algo_type = IKE_ALG_HASH,
		.id = {
			[IKEv1_OAKLEY_ID] = OAKLEY_SHA1,
			[IKEv1_IPSEC_ID] = -1,
			[IKEv2_ALG_ID] = IKEv2_HASH_ALGORITHM_SHA1,
		},
		.fips = { .approved = true, },
	},
	.nss = {
		.oid_tag = SEC_OID_SHA1,
		.derivation_mechanism = CKM_SHA1_KEY_DERIVATION,
		.pkcs1_1_5_rsa_oid_tag = SEC_OID_PKCS1_SHA1_WITH_RSA_ENCRYPTION,
	},
	.hash_digest_size = SHA1_DIGEST_SIZE,
	.hash_block_size = 64,	/* B from RFC 2104 */
	.hash_ops = &ike_alg_hash_nss_ops,

	.digital_signature_blob = {
		[DIGITAL_SIGNATURE_PKCS1_1_5_RSA_BLOB] = THING_AS_HUNK(asn1_pkcs1_1_5_rsa_sha1_blob),
		[DIGITAL_SIGNATURE_ECDSA_BLOB] = THING_AS_HUNK(asn1_ecdsa_sha1_blob),
	},
};
