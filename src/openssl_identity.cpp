/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file openssl_identity.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include "logging.h"
#include "random.h"
#include "openssl.h"
#include "openssl_digest.h"
#include "openssl_identity.h"

namespace rtcbase {

static void log_ssl_errors(const std::string& prefix) {
    char error_buf[200];
    unsigned long err = 0;
    
    while ((err == ERR_get_error()) != 0) {
        ERR_error_string_n(err, error_buf, sizeof(error_buf));
        LOG(LS_FATAL) << prefix << ": " << error_buf << "\n";
    }
}

//////////////////// OpenSSLKeyPair start //////////////////

static EVP_PKEY* make_key(const KeyParams& key_params) {
    LOG(LS_TRACE) << "Making key pair";

    EVP_PKEY* pkey = EVP_PKEY_new();
    if (key_params.type() == KT_RSA) {
        int key_length = key_params.rsa_params().mod_size;
        BIGNUM* exponent = BN_new();
        RSA* rsa = RSA_new();
        if (!pkey || !exponent || !rsa ||
                !BN_set_word(exponent, key_params.rsa_params().pub_exp) ||
                !RSA_generate_key_ex(rsa, key_length, exponent, NULL) ||
                !EVP_PKEY_assign_RSA(pkey, rsa)) 
        {
            EVP_PKEY_free(pkey);
            BN_free(exponent);
            RSA_free(rsa);
            LOG(LS_FATAL) << "Failed to make RSA key pair";
            return NULL;
        }
        // ownership of rsa struct was assigned, don't free it.
        BN_free(exponent);
    } else if (key_params.type() == KT_ECDSA) {
        if (key_params.ec_curve() == EC_NIST_P256) {
            EC_KEY* ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
            
            // Ensure curve name is included when EC key is serialized.
            // Without this call, OpenSSL versions before 1.1.0 will create
            // certificates that don't work for TLS.
            // This is a no-op for BoringSSL and OpenSSL 1.1.0+
            EC_KEY_set_asn1_flag(ec_key, OPENSSL_EC_NAMED_CURVE);

            if (!pkey || !ec_key || !EC_KEY_generate_key(ec_key) ||
                    !EVP_PKEY_assign_EC_KEY(pkey, ec_key))
            {
                EVP_PKEY_free(pkey);
                EC_KEY_free(ec_key);
                LOG(LS_FATAL) << "Failed to make EC key pair";
                return NULL;
            }
            // ownership of ec_key struct was assigned, don't free it
        } else {
            EVP_PKEY_free(pkey);
            LOG(LS_FATAL) << "ECDSA key requested for unknown curve";
            return NULL;
        }
    } else {
        EVP_PKEY_free(pkey);
        LOG(LS_FATAL) << "Key type requested not understood";
        return NULL;
    }

    LOG(LS_TRACE) << "Returning key pair";
    return pkey;
}

OpenSSLKeyPair::~OpenSSLKeyPair() {
    EVP_PKEY_free(_pkey);
}

OpenSSLKeyPair* OpenSSLKeyPair::generate(const KeyParams& key_params) {
    EVP_PKEY* pkey = make_key(key_params);
    if (!pkey) {
        log_ssl_errors("Generating key pair");
        return NULL;
    }
    return new OpenSSLKeyPair(pkey);
}

OpenSSLKeyPair* OpenSSLKeyPair::get_reference() {
    add_reference();
    return new OpenSSLKeyPair(_pkey);
}

void OpenSSLKeyPair::add_reference() {
    CRYPTO_add(&_pkey->references, 1, CRYPTO_LOCK_EVP_PKEY);
}

/////////////////// OpenSSLKeyPair end /////////////////

////////////////// OpenSSLCertificate start ////////////

// Random bits for certificate serial number
static const int SERIAL_RAND_BITS = 64;

// Generate a self-signed certificate, with the public key from the
// given key pair. Caller is responsible for freeing the returned object.
static X509* make_certificate(EVP_PKEY* pkey, const SSLIdentityParams& params) {
    LOG(LS_TRACE) << "Making certificate for " << params.common_name;
    X509* x509 = NULL;
    BIGNUM* serial_number = NULL;
    X509_NAME* name = NULL;
    time_t epoch_off = 0;  // Time offset since epoch.

    if ((x509 = X509_new()) == NULL) {
        goto error;
    }

    if (!X509_set_pubkey(x509, pkey)) {
        goto error;
    }

    // serial number
    // temporary reference to serial number inside x509 struct
    ASN1_INTEGER* asn1_serial_number;
    if ((serial_number = BN_new()) == NULL ||
            !BN_pseudo_rand(serial_number, SERIAL_RAND_BITS, 0, 0) ||
            (asn1_serial_number = X509_get_serialNumber(x509)) == NULL ||
            !BN_to_ASN1_INTEGER(serial_number, asn1_serial_number))
    {
        goto error;
    }

    if (!X509_set_version(x509, 2L))  {// version 3 
        goto error;
    }

    // There are a lot of possible components for the name entries. In
    // our P2P SSL mode however, the certificates are pre-exchanged
    // (through the secure XMPP channel), and so the certificate
    // identification is arbitrary. It can't be empty, so we set some
    // arbitrary common_name. Note that this certificate goes out in
    // clear during SSL negotiation, so there may be a privacy issue in
    // putting anything recognizable here.
    if ((name = X509_NAME_new()) == NULL ||
            !X509_NAME_add_entry_by_NID(
                name, NID_commonName, MBSTRING_UTF8,
                (unsigned char*)params.common_name.c_str(), -1, -1, 0) ||
            !X509_set_subject_name(x509, name) ||
            !X509_set_issuer_name(x509, name))
    {
        goto error;
    }

    if (!X509_time_adj(X509_get_notBefore(x509), params.not_before, &epoch_off) ||
            !X509_time_adj(X509_get_notAfter(x509), params.not_after, &epoch_off))
    {
        goto error;
    }

    if (!X509_sign(x509, pkey, EVP_sha256())) {
        goto error;
    }

    BN_free(serial_number);
    X509_NAME_free(name);
    LOG(LS_TRACE) << "Returning certificate";
    return x509;

error:
    BN_free(serial_number);
    X509_NAME_free(name);
    X509_free(x509);
    return NULL;
}

static void print_cert(X509* x509) {
    BIO* temp_memory_bio = BIO_new(BIO_s_mem());
    if (!temp_memory_bio) {
        LOG(LS_FATAL) << "Failed to allocate temporary memory bio";
        return;
    }
    X509_print_ex(temp_memory_bio, x509, XN_FLAG_SEP_CPLUS_SPC, 0);
    BIO_write(temp_memory_bio, "\0", 1);
    char* buffer;
    BIO_get_mem_data(temp_memory_bio, &buffer);
    LOG(LS_TRACE) << buffer;
    BIO_free(temp_memory_bio);
}

OpenSSLCertificate* OpenSSLCertificate::generate(
        OpenSSLKeyPair* key_pair,
        const SSLIdentityParams& params)
{
    SSLIdentityParams actual_params(params);
    if (actual_params.common_name.empty()) {
        // Using a random string, arbitrarily 8chars long.
        actual_params.common_name = create_random_string(8);
    }

    X509* x509 = make_certificate(key_pair->pkey(), actual_params);
    if (!x509) {
        log_ssl_errors("Generating certificate");
        return NULL;
    }
    
    print_cert(x509);

    OpenSSLCertificate* ret = new OpenSSLCertificate(x509);
    X509_free(x509);
    return ret;
}

OpenSSLCertificate::~OpenSSLCertificate() {
    X509_free(_x509);
}

OpenSSLCertificate* OpenSSLCertificate::get_reference() const {
    return new OpenSSLCertificate(_x509);
}

void OpenSSLCertificate::add_reference() const {
    if (!_x509) {
        return;
    }
    CRYPTO_add(&_x509->references, 1, CRYPTO_LOCK_X509);
}

// NOTE: This implementation only functions correctly after InitializeSSL
// and before CleanupSSL.
bool OpenSSLCertificate::get_signature_digest_algorithm(
        std::string* algorithm) const 
{
    int nid = OBJ_obj2nid(_x509->sig_alg->algorithm);
    switch (nid) {
        case NID_md5WithRSA:
        case NID_md5WithRSAEncryption:
            *algorithm = DIGEST_MD5;
            break;
        case NID_ecdsa_with_SHA1:
        case NID_dsaWithSHA1:
        case NID_dsaWithSHA1_2:
        case NID_sha1WithRSA:
        case NID_sha1WithRSAEncryption:
            *algorithm = DIGEST_SHA_1;
            break;
        case NID_ecdsa_with_SHA224:
        case NID_sha224WithRSAEncryption:
        case NID_dsa_with_SHA224:
            *algorithm = DIGEST_SHA_224;
            break;
        case NID_ecdsa_with_SHA256:
        case NID_sha256WithRSAEncryption:
        case NID_dsa_with_SHA256:
            *algorithm = DIGEST_SHA_256;
            break;
        case NID_ecdsa_with_SHA384:
        case NID_sha384WithRSAEncryption:
            *algorithm = DIGEST_SHA_384;
            break;
        case NID_ecdsa_with_SHA512:
        case NID_sha512WithRSAEncryption:
            *algorithm = DIGEST_SHA_512;
            break;
        default:
            // Unknown algorithm.  There are several unhandled options that are less
            // common and more complex.
            LOG(LS_FATAL) << "Unknown signature algorithm NID: " << nid;
            algorithm->clear();
            return false;
    }
    return true;
}

bool OpenSSLCertificate::compute_digest(const std::string& algorithm,
        unsigned char* digest,
        size_t size,
        size_t* length) const 
{
    return compute_digest(_x509, algorithm, digest, size, length);
}

bool OpenSSLCertificate::compute_digest(const X509* x509,
        const std::string& algorithm,
        unsigned char* digest,
        size_t size,
        size_t* length) 
{
    const EVP_MD* md;
    unsigned int n;

    if (!OpenSSLDigest::get_digest_EVP(algorithm, &md)) {
        return false;
    }

    if (size < static_cast<size_t>(EVP_MD_size(md))) {
        return false;
    }

    X509_digest(x509, md, digest, &n);

    *length = n;

    return true;
}

// Documented in sslidentity.h.
int64_t OpenSSLCertificate::certificate_expiration_time() const {
    ASN1_TIME* expire_time = X509_get_notAfter(_x509);
    bool long_format;

    if (expire_time->type == V_ASN1_UTCTIME) {
        long_format = false;
    } else if (expire_time->type == V_ASN1_GENERALIZEDTIME) {
        long_format = true;
    } else {
        return -1;
    }

    return ASN1_time_to_sec(expire_time->data, expire_time->length, long_format);
}

////////////////////// OpenSSLCertificate end /////////////////

/////////////////////// OpenSSLIdentity start/////////////////

OpenSSLIdentity::OpenSSLIdentity(std::unique_ptr<OpenSSLKeyPair> key_pair,
        std::unique_ptr<OpenSSLCertificate> certificate) :
    _key_pair(std::move(key_pair))
{
    std::vector<std::unique_ptr<SSLCertificate>> certs;
    certs.push_back(std::move(certificate));
    _cert_chain.reset(new SSLCertChain(std::move(certs)));
}

OpenSSLIdentity::OpenSSLIdentity(std::unique_ptr<OpenSSLKeyPair> key_pair,
        std::unique_ptr<SSLCertChain> cert_chain)
    : _key_pair(std::move(key_pair)), _cert_chain(std::move(cert_chain)) 
{
}

OpenSSLIdentity::~OpenSSLIdentity() = default;

OpenSSLIdentity* OpenSSLIdentity::generate_with_expiration(
        const std::string& common_name,
        const KeyParams& key_params,
        time_t certificate_lifetime)
{
    SSLIdentityParams params;
    params.key_params = key_params;
    params.common_name = common_name;
    time_t now = time(NULL);
    params.not_before = now + k_certificate_window_in_seconds;
    params.not_after = now + certificate_lifetime;
    
    if (params.not_before > params.not_after) {
        return NULL;
    }
    return generate_internal(params);
}

OpenSSLIdentity* OpenSSLIdentity::generate_internal(
        const SSLIdentityParams& params) 
{
    std::unique_ptr<OpenSSLKeyPair> key_pair(
            OpenSSLKeyPair::generate(params.key_params));
    if (key_pair) {
        std::unique_ptr<OpenSSLCertificate> certificate(
                OpenSSLCertificate::generate(key_pair.get(), params));
        if (certificate) {
            return new OpenSSLIdentity(std::move(key_pair), std::move(certificate));
        }
    }
    LOG(LS_TRACE) << "Identity generation failed";
    return NULL;
}

const OpenSSLCertificate& OpenSSLIdentity::certificate() const {
    return *static_cast<const OpenSSLCertificate*>(&_cert_chain->get(0));
}

OpenSSLIdentity* OpenSSLIdentity::get_reference() const {
    return new OpenSSLIdentity(std::unique_ptr<OpenSSLKeyPair>(_key_pair->get_reference()),
            std::unique_ptr<SSLCertChain>(_cert_chain->copy()));
}

bool OpenSSLIdentity::configure_identity(SSL_CTX* ctx) {
    // 1 is the documented success return code.
    const OpenSSLCertificate* cert = &certificate();
    if (SSL_CTX_use_certificate(ctx, cert->x509()) != 1 ||
            SSL_CTX_use_PrivateKey(ctx, _key_pair->pkey()) != 1) {
        log_ssl_errors("Configuring key and certificate");
        return false;
    }
    
    /*
    // If a chain is available, use it.
    for (size_t i = 1; i < _cert_chain->get_size(); ++i) {
        cert = static_cast<const OpenSSLCertificate*>(&_cert_chain->get(i));
        if (SSL_CTX_add1_chain_cert(ctx, cert->x509()) != 1) {
            log_ssl_errors("Configuring intermediate certificate");
            return false;
        }
    }
    */
    return true;
}

} // namespace rtcbase


