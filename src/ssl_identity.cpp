/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file src/ssl_identity.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <ctime>
#include <string>
#include <string.h>
#include <utility>
#include <algorithm>

#include "time_utils.h"
#include "openssl_identity.h"
#include "ssl_identity.h"

namespace rtcbase {

SSLCertChain::SSLCertChain(std::vector<std::unique_ptr<SSLCertificate>> certs)
    : _certs(std::move(certs)) {}

SSLCertChain::SSLCertChain(const std::vector<SSLCertificate*>& certs) {
    _certs.resize(certs.size());
    std::transform(
            certs.begin(), certs.end(), _certs.begin(),
            [](const SSLCertificate* cert) -> std::unique_ptr<SSLCertificate> {
            return cert->get_unique_reference();
            });
}

SSLCertChain::SSLCertChain(const SSLCertificate* cert) {
    _certs.push_back(cert->get_unique_reference());
}

SSLCertChain::~SSLCertChain() {}

SSLCertChain* SSLCertChain::copy() const {
    std::vector<std::unique_ptr<SSLCertificate>> new_certs(_certs.size());
    std::transform(_certs.begin(), _certs.end(), new_certs.begin(),
            [](const std::unique_ptr<SSLCertificate>& cert)
            -> std::unique_ptr<SSLCertificate> {
            return cert->get_unique_reference();
            });
    return new SSLCertChain(std::move(new_certs));
}

std::unique_ptr<SSLCertificate> SSLCertificate::get_unique_reference() const {
    return std::unique_ptr<SSLCertificate>(get_reference());
}

KeyParams::KeyParams(KeyType key_type) {
    if (KT_ECDSA == key_type) {
        _type = KT_ECDSA;
        _params.curve = EC_NIST_P256;
    } else if (KT_RSA == key_type) {
        _type = KT_RSA;
        _params.rsa.mod_size = k_rsa_default_mod_size;
        _params.rsa.pub_exp = k_rsa_default_exponent;
    }
}

bool KeyParams::is_valid() const {
    if (KT_RSA == _type) {
        return (_params.rsa.mod_size >= k_rsa_min_mod_size &&
               _params.rsa.mod_size <= k_rsa_max_mod_size &&
               _params.rsa.pub_exp > _params.rsa.mod_size);
    } else if (KT_ECDSA == _type) {
        return (_params.curve == EC_NIST_P256); 
    }
    return false;
}

RSAParams KeyParams::rsa_params() const {
    return _params.rsa;
}

ECCurve KeyParams::ec_curve() const {
    return _params.curve;
}

SSLIdentity* SSLIdentity::generate_with_expiration(const std::string& common_name,
        const KeyParams& key_params,
        time_t certificate_lifetime)
{
    return OpenSSLIdentity::generate_with_expiration(common_name, key_params, 
            certificate_lifetime);
}

SSLIdentity* SSLIdentity::generate(const std::string& common_name,
        const KeyParams& key_params)
{
    return OpenSSLIdentity::generate_with_expiration(
            common_name, key_params, k_default_certificate_lifetime_in_seconds);
}

// Read |n| bytes from ASN1 number string at *|pp| and return the numeric value.
// Update *|pp| and *|np| to reflect number of read bytes.
static inline int ASN1_read_int(const unsigned char** pp, size_t* np, size_t n) {
    const unsigned char* p = *pp;
    int x = 0;
    for (size_t i = 0; i < n; i++) {
        x = 10 * x + p[i] - '0';
    }
    *pp = p + n;
    *np = *np - n;
    return x;
}

int64_t ASN1_time_to_sec(const unsigned char* s, size_t length, bool long_format) {
    size_t bytes_left = length;

    // Make sure the string ends with Z.  Doing it here protects the strspn call
    // from running off the end of the string in Z's absense.
    if (length == 0 || s[length - 1] != 'Z') {
        return -1;
    }

    // Make sure we only have ASCII digits so that we don't need to clutter the
    // code below and ASN1ReadInt with error checking.
    size_t n = strspn(reinterpret_cast<const char*>(s), "0123456789");
    if (n + 1 != length) {
        return -1;
    }

    int year;

    // Read out ASN1 year, in either 2-char "UTCTIME" or 4-char "GENERALIZEDTIME"
    // format.  Both format use UTC in this context.
    if (long_format) {
        // ASN1 format: yyyymmddhh[mm[ss[.fff]]]Z where the Z is literal, but
        // RFC 5280 requires us to only support exactly yyyymmddhhmmssZ.

        if (bytes_left < 11) {
            return -1;
        }

        year = ASN1_read_int(&s, &bytes_left, 4);
        year -= 1900;
    } else {
        // ASN1 format: yymmddhhmm[ss]Z where the Z is literal, but RFC 5280
        // requires us to only support exactly yymmddhhmmssZ.

        if (bytes_left < 9) {
            return -1;
        }

        year = ASN1_read_int(&s, &bytes_left, 2);
        if (year < 50)  {// Per RFC 5280 4.1.2.5.1
            year += 100;
        }
    }

    std::tm tm;
    tm.tm_year = year;

    // Read out remaining ASN1 time data and store it in |tm| in documented
    // std::tm format.
    tm.tm_mon = ASN1_read_int(&s, &bytes_left, 2) - 1;
    tm.tm_mday = ASN1_read_int(&s, &bytes_left, 2);
    tm.tm_hour = ASN1_read_int(&s, &bytes_left, 2);
    tm.tm_min = ASN1_read_int(&s, &bytes_left, 2);
    tm.tm_sec = ASN1_read_int(&s, &bytes_left, 2);

    if (bytes_left != 1) {
        // Now just Z should remain.  Its existence was asserted above.
        return -1;
    }

    return tm_to_seconds(tm);
}

} // namespace rtcbase


