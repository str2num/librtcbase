/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ssl_identity.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SSL_IDENTITY_H_
#define  __RTCBASE_SSL_IDENTITY_H_

#include <vector>
#include <memory>
#include <string>

#include "constructor_magic.h"

namespace rtcbase {

class SSLCertificate {
public:
    virtual ~SSLCertificate() { }

    virtual SSLCertificate* get_reference() const = 0;
    
    std::unique_ptr<SSLCertificate> get_unique_reference() const;

    // Gets the name of the digest algorithm that was used to compute this
    // certificate's signature.
    virtual bool get_signature_digest_algorithm(std::string* algorithm) const = 0;

    // Compute the digest of the certificate given algorithm
    virtual bool compute_digest(const std::string& algorithm,
            unsigned char* digest,
            size_t size,
            size_t* length) const = 0;

    // Returns the time in seconds relative to epoch, 1970-01-01T00:00:00Z (UTC),
    // or -1 if an expiration time could not be retrieved.
    virtual int64_t certificate_expiration_time() const = 0;
};

// SSLCertChain is a simple wrapper for a vector of SSLCertificates. It serves
// primarily to ensure proper memory management (especially deletion) of the
// SSLCertificate pointers.
class SSLCertChain {
public:
    explicit SSLCertChain(std::vector<std::unique_ptr<SSLCertificate>> certs);
    // These constructors copy the provided SSLCertificate(s), so the caller
    // retains ownership.
    explicit SSLCertChain(const std::vector<SSLCertificate*>& certs);
    explicit SSLCertChain(const SSLCertificate* cert);
    ~SSLCertChain();

    // Vector access methods.
    size_t get_size() const { return _certs.size(); }

    // Returns a temporary reference, only valid until the chain is destroyed.
    const SSLCertificate& get(size_t pos) const { return *(_certs[pos]); }

    // Returns a new SSLCertChain object instance wrapping the same underlying
    // certificate chain.  Caller is responsible for freeing the returned object.
    SSLCertChain* copy() const;

private:
    std::vector<std::unique_ptr<SSLCertificate>> _certs;

    RTC_DISALLOW_COPY_AND_ASSIGN(SSLCertChain);
};

enum KeyType {
    KT_RSA,
    KT_ECDSA,
    KT_LAST,
    KT_DEFAULT = KT_ECDSA
};

static const int k_rsa_default_mod_size = 1024;
static const int k_rsa_default_exponent = 0x10001;  // = 2^16+1 = 65537
static const int k_rsa_min_mod_size = 1024;
static const int k_rsa_max_mod_size = 8192;

static const int k_default_certificate_lifetime_in_seconds = 60 * 60 * 24 * 30;  // 30 days
// Certificate validity window.
// This is to compensate for slightly incorrect system clocks.
static const int k_certificate_window_in_seconds = -60 * 60 * 24;

struct RSAParams {
    unsigned int mod_size;
    unsigned int pub_exp;
};

enum ECCurve {
    EC_NIST_P256,
    /* EC_FANCY*/
    EC_LAST
};

class KeyParams {
public: 
    explicit KeyParams(KeyType key_type = KT_DEFAULT);
    
    // Generate a a KeyParams for ECDSA specifying the curve.
    //static KeyParams ECDSA(ECCurve curve = EC_NIST_P256);

    bool is_valid() const;
    
    RSAParams rsa_params() const;
    
    ECCurve ec_curve() const;

    KeyType type() const { return _type; }

private:
    KeyType _type;
    union {
        RSAParams rsa;
        ECCurve curve;
    } _params;
};

struct SSLIdentityParams {
    std::string common_name;
    time_t not_before;
    time_t not_after;
    KeyParams key_params;
};

class SSLIdentity {
public:
    static SSLIdentity* generate_with_expiration(const std::string& common_name,
            const KeyParams& key_params,
            time_t certificate_lifetime);

    static SSLIdentity* generate(const std::string& common_name,
            const KeyParams& key_params);
    
    virtual ~SSLIdentity() {}
    
    // Returns a new SSLIdentity object instance wrapping the same
    // identity information.
    // Caller is responsible for freeing the returned object.
    // TODO(hbos,torbjorng): Rename to a less confusing name.
    virtual SSLIdentity* get_reference() const = 0; 

    // Returns a temporary reference to the certificate.
    virtual const SSLCertificate& certificate() const = 0;
};

// Convert from ASN1 time as restricted by RFC 5280 to seconds from 1970-01-01
// 00.00 ("epoch").  If the ASN1 time cannot be read, return -1.  The data at
// |s| is not 0-terminated; its char count is defined by |length|.
int64_t ASN1_time_to_sec(const unsigned char* s, size_t length, bool long_format);

} // namespace rtcbase

#endif  //__RTCBASE_SSL_IDENTITY_H_


