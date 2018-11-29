/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file openssl_identity.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_OPENSSL_IDENTITY_H_
#define  __RTCBASE_OPENSSL_IDENTITY_H_

#include <memory>
#include <string>

#include <openssl/evp.h>
#include <openssl/x509.h>

#include "constructor_magic.h"
#include "ssl_identity.h"

namespace rtcbase {

class OpenSSLKeyPair {
public:
    explicit OpenSSLKeyPair(EVP_PKEY* pkey) : _pkey(pkey) { }
   
    virtual ~OpenSSLKeyPair();

    static OpenSSLKeyPair* generate(const KeyParams& key_params);
    
    virtual OpenSSLKeyPair* get_reference();

    EVP_PKEY* pkey() const { return _pkey; }

private:
    void add_reference();

private:   
    EVP_PKEY* _pkey;

    RTC_DISALLOW_COPY_AND_ASSIGN(OpenSSLKeyPair);
};

class OpenSSLCertificate : public SSLCertificate {
public:
    explicit OpenSSLCertificate(X509* x509) : _x509(x509) {
        add_reference();
    }

    ~OpenSSLCertificate() override;

    OpenSSLCertificate* get_reference() const override;

    X509* x509() const { return _x509; }

    static OpenSSLCertificate* generate(OpenSSLKeyPair* key_pair,
            const SSLIdentityParams& params);
    
    bool get_signature_digest_algorithm(std::string* algorithm) const override;
    
    // Compute the digest of the certificate given algorithm
    bool compute_digest(const std::string& algorithm,
            unsigned char* digest,
            size_t size,
            size_t* length) const override; 

    // Compute the digest of a certificate as an X509 *
    static bool compute_digest(const X509* x509,
            const std::string& algorithm,
            unsigned char* digest,
            size_t size,
            size_t* length); 
    
    int64_t certificate_expiration_time() const override;

private:
    void add_reference() const;

private:
    X509* _x509;
    
    RTC_DISALLOW_COPY_AND_ASSIGN(OpenSSLCertificate);
};

class OpenSSLIdentity : public SSLIdentity {
public:
    static OpenSSLIdentity* generate_with_expiration(const std::string& common_name,
            const KeyParams& key_params,
            time_t certificate_lifetime);
    
    ~OpenSSLIdentity() override;
    const OpenSSLCertificate& certificate() const override;
    OpenSSLIdentity* get_reference() const override;
   
    // Configure an SSL context object to use our key and certificate.
    bool configure_identity(SSL_CTX* ctx);

private:
    OpenSSLIdentity(std::unique_ptr<OpenSSLKeyPair> key_pair, 
            std::unique_ptr<OpenSSLCertificate> certificate);
    OpenSSLIdentity(std::unique_ptr<OpenSSLKeyPair> key_pair,
                              std::unique_ptr<SSLCertChain> cert_chain);

    static OpenSSLIdentity* generate_internal(const SSLIdentityParams& params);
        
private:
    std::unique_ptr<OpenSSLKeyPair> _key_pair;
    std::unique_ptr<SSLCertChain> _cert_chain;

    RTC_DISALLOW_COPY_AND_ASSIGN(OpenSSLIdentity);
};

} // namespace rtcbase

#endif  //__RTCBASE_OPENSSL_IDENTITY_H_


