/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ssl_stream_adapter.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SSL_STREAM_ADAPTER_H_
#define  __RTCBASE_SSL_STREAM_ADAPTER_H_

#include <vector>
#include <string>

#include "event_loop.h"
#include "stream.h"
#include "ssl_identity.h"

namespace rtcbase {

// Constants for SRTP profiles.
const int SRTP_INVALID_CRYPTO_SUITE = 0;
#ifndef SRTP_AES128_CM_SHA1_80
const int SRTP_AES128_CM_SHA1_80 = 0x0001;
#endif
#ifndef SRTP_AES128_CM_SHA1_32
const int SRTP_AES128_CM_SHA1_32 = 0x0002;
#endif
#ifndef SRTP_AEAD_AES_128_GCM
const int SRTP_AEAD_AES_128_GCM = 0x0007;
#endif
#ifndef SRTP_AEAD_AES_256_GCM
const int SRTP_AEAD_AES_256_GCM = 0x0008;
#endif

// TODO(guoweis): Move this to SDP layer and use int form internally.
// webrtc:5043.
const char CS_AES_CM_128_HMAC_SHA1_80[] = "AES_CM_128_HMAC_SHA1_80";
const char CS_AES_CM_128_HMAC_SHA1_32[] = "AES_CM_128_HMAC_SHA1_32";
const char CS_AEAD_AES_128_GCM[] = "AEAD_AES_128_GCM";
const char CS_AEAD_AES_256_GCM[] = "AEAD_AES_256_GCM";

// Given the DTLS-SRTP protection profile ID, as defined in
// https://tools.ietf.org/html/rfc4568#section-6.2 , return the SRTP profile
// name, as defined in https://tools.ietf.org/html/rfc5764#section-4.1.2.
std::string srtp_crypto_suite_to_name(int crypto_suite);

// The reverse of above conversion.
int srtp_crypto_suite_from_name(const std::string& crypto_suite);

// Get key length and salt length for given crypto suite. Returns true for
// valid suites, otherwise false.
bool get_srtp_key_and_salt_lengths(int crypto_suite, int* key_length,
    int* salt_length);

// Returns true if the given crypto suite name uses a GCM cipher.
bool is_gcm_crypto_suite_name(const std::string& crypto_suite);

struct CryptoOptions {
    CryptoOptions() {}

    // Helper method to return an instance of the CryptoOptions with GCM crypto
    // suites disabled. This method should be used instead of depending on current
    // default values set by the constructor.
    static CryptoOptions no_gcm();

    // Enable GCM crypto suites from RFC 7714 for SRTP. GCM will only be used
    // if both sides enable it.
    bool enable_gcm_crypto_suites = false;

    // If set to true, encrypted RTP header extensions as defined in RFC 6904
    // will be negotiated. They will only be used if both peers support them.
    bool enable_encrypted_rtp_header_extensions = false;
};

// Returns supported crypto suites, given |crypto_options|.
// CS_AES_CM_128_HMAC_SHA1_32 will be preferred by default.
std::vector<int> get_supported_dtls_srtp_crypto_suites(
        const rtcbase::CryptoOptions& crypto_options);

enum SSLRole { SSL_CLIENT, SSL_SERVER };
enum SSLMode { SSL_MODE_TLS, SSL_MODE_DTLS };
enum SSLProtocolVersion {
    SSL_PROTOCOL_TLS_10,
    SSL_PROTOCOL_TLS_11,
    SSL_PROTOCOL_TLS_12,
    SSL_PROTOCOL_DTLS_10 = SSL_PROTOCOL_TLS_11,
    SSL_PROTOCOL_DTLS_12 = SSL_PROTOCOL_TLS_12,
};

enum class SSLPeerCertificateDigestError {
    NONE,
    UNKNOWN_ALGORITHM,
    INVALID_LENGTH,
    VERIFICATION_FAILED,
};

// Used to send back UMA histogram value. Logged when Dtls handshake fails.
enum class SSLHandshakeError { UNKNOWN, INCOMPATIBLE_CIPHERSUITE, MAX_VALUE };

class SSLStreamAdapter : public StreamAdapterInterface {
public:
    // Instantiate an SSLStreamAdapter wrapping the given stream,
    // (using the selected implementation for the platform).
    // Caller is responsible for freeing the returned object.
    static SSLStreamAdapter* create(StreamInterface* stream, EventLoop* el);
    
    explicit SSLStreamAdapter(StreamInterface* stream)
        : StreamAdapterInterface(stream), _ignore_bad_cert(false),
        _client_auth_enabled(true) { }
    ~SSLStreamAdapter() override {}

    void set_ignore_bad_cert(bool ignore) { _ignore_bad_cert = ignore; }
    bool ignore_bad_cert() const { return _ignore_bad_cert; }

    void set_client_auth_enabled(bool enabled) { _client_auth_enabled = enabled; }
    bool client_auth_enabled() const { return _client_auth_enabled; }

    // Specify our SSL identity: key and certificate. SSLStream takes ownership
    // of the SSLIdentity object and will free it when appropriate. Should be
    // called no more than once on a given SSLStream instance.
    virtual void set_identity(SSLIdentity* identity) = 0;

    // Call this to indicate that we are to play the server role (or client role,
    // if the default argument is replaced by SSL_CLIENT).
    // The default argument is for backward compatibility.
    // TODO(ekr@rtfm.com): rename this SetRole to reflect its new function
    virtual void set_server_role(SSLRole role = SSL_SERVER) = 0;

    // Do DTLS or TLS.
    virtual void set_mode(SSLMode mode) = 0;     

    // Set maximum supported protocol version. The highest version supported by
    // both ends will be used for the connection, i.e. if one party supports
    // DTLS 1.0 and the other DTLS 1.2, DTLS 1.0 will be used.
    // If requested version is not supported by underlying crypto library, the
    // next lower will be used.
    virtual void set_max_protocol_version(SSLProtocolVersion version) = 0;     
    
    // Set the initial retransmission timeout for DTLS messages. When the timeout
    // expires, the message gets retransmitted and the timeout is exponentially
    // increased.
    // This should only be called before StartSSL().
    virtual void set_initial_retransmission_timeout(int timeout_ms) = 0;

    // StartSSL starts negotiation with a peer, whose certificate is verified
    // using the certificate digest. Generally, SetIdentity() and possibly
    // SetServerRole() should have been called before this.
    // SetPeerCertificateDigest() must also be called. It may be called after
    // StartSSLWithPeer() but must be called before the underlying stream opens.
    //
    // Use of the stream prior to calling StartSSL will pass data in clear text.
    // Calling StartSSL causes SSL negotiation to begin as soon as possible: right
    // away if the underlying wrapped stream is already opened, or else as soon as
    // it opens.
    //
    // StartSSL returns a negative error code on failure. Returning 0 means
    // success so far, but negotiation is probably not complete and will continue
    // asynchronously. In that case, the exposed stream will open after
    // successful negotiation and verification, or an SE_CLOSE event will be
    // raised if negotiation fails.
    virtual int start_SSL() = 0; 

    // Specify the digest of the certificate that our peer is expected to use.
    // Only this certificate will be accepted during SSL verification. The
    // certificate is assumed to have been obtained through some other secure
    // channel (such as the signaling channel). This must specify the terminal
    // certificate, not just a CA. SSLStream makes a copy of the digest value.
    virtual bool set_peer_certificate_digest(const std::string& digest_alg,
            const unsigned char* digest_val,
            size_t digest_len,
            SSLPeerCertificateDigestError* error = nullptr) = 0;
    
    // Retrieves the IANA registration id of the cipher suite used for the
    // connection (e.g. 0x2F for "TLS_RSA_WITH_AES_128_CBC_SHA").
    virtual bool get_ssl_cipher_suite(int* cipher_suite);

    // Key Exporter interface from RFC 5705
    // Arguments are:
    // label               -- the exporter label.
    //                        part of the RFC defining each exporter
    //                        usage (IN)
    // context/context_len -- a context to bind to for this connection;
    //                        optional, can be NULL, 0 (IN)
    // use_context         -- whether to use the context value
    //                        (needed to distinguish no context from
    //                        zero-length ones).
    // result              -- where to put the computed value
    // result_len          -- the length of the computed value
    virtual bool export_keying_material(const std::string& label,
            const uint8_t* context,
            size_t context_len,
            bool use_context,
            uint8_t* result,
            size_t result_len);

    // DTLS-SRTP interface
    virtual bool set_dtls_srtp_crypto_suites(const std::vector<int>& crypto_suites);
    virtual bool get_dtls_srtp_crypto_suite(int* crypto_suite);
    
    rtcbase::Signal1<SSLHandshakeError> signal_ssl_handshake_error;

private:
    // If true, the server certificate need not match the configured
    // server_name, and in fact missing certificate authority and other
    // verification errors are ignored.
    bool _ignore_bad_cert;

    // If true (default), the client is required to provide a certificate during
    // handshake. If no certificate is given, handshake fails. This applies to
    // server mode only.
    bool _client_auth_enabled;
};

} // namespace rtcbase

#endif  //__RTCBASE_SSL_STREAM_ADAPTER_H_


