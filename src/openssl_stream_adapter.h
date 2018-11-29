/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file openssl_stream_adapter.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_OPENSSL_STREAM_ADAPTER_H_
#define  __RTCBASE_OPENSSL_STREAM_ADAPTER_H_

#include <string>
#include <memory>
#include <vector>

#include "event_loop.h"
#include "buffer.h"
#include "ssl_stream_adapter.h"
#include "openssl_identity.h"
#include "ssl_identity.h"

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_cipher_st SSL_CIPHER;
typedef struct x509_store_ctx_st X509_STORE_CTX;

namespace rtcbase {

class OpenSSLStreamAdapter : public SSLStreamAdapter {
public:
    explicit OpenSSLStreamAdapter(StreamInterface* stream, EventLoop* el);
    ~OpenSSLStreamAdapter() override;
    
    void set_identity(SSLIdentity* identity) override;

    // Default argument is for compatibility
    void set_server_role(SSLRole role = SSL_SERVER) override;
    bool set_peer_certificate_digest(const std::string& digest_alg,
            const unsigned char* digest_val,
            size_t digest_len,
            SSLPeerCertificateDigestError* error = nullptr) override;

    //std::unique_ptr<SSLCertificate> GetPeerCertificate() const override;

    // Goes from state SSL_NONE to either SSL_CONNECTING or SSL_WAIT, depending
    // on whether the underlying stream is already open or not.
    int start_SSL() override;
    void set_mode(SSLMode mode) override;
    void set_max_protocol_version(SSLProtocolVersion version) override;
    void set_initial_retransmission_timeout(int timeout_ms) override;
     
    bool get_ssl_cipher_suite(int* cipher) override;

    // Key Extractor interface
    bool export_keying_material(const std::string& label,
            const uint8_t* context,
            size_t context_len,
            bool use_context,
            uint8_t* result,
            size_t result_len) override; 

    // DTLS-SRTP interface
    bool set_dtls_srtp_crypto_suites(const std::vector<int>& crypto_suites) override;
    bool get_dtls_srtp_crypto_suite(int* crypto_suite) override;

protected:
    void on_event(StreamInterface* stream, int events, int err) override;
    
private:
    enum SSLState {
        // Before calling one of the StartSSL methods, data flows
        // in clear text.
        SSL_NONE,
        SSL_WAIT,  // waiting for the stream to open to start SSL negotiation
        SSL_CONNECTING,  // SSL negotiation in progress
        SSL_CONNECTED,  // SSL stream successfully established
        SSL_ERROR,  // some SSL error occurred, stream is closed
        SSL_CLOSED  // Clean close
    };

private:
    // The following three methods return 0 on success and a negative
    // error code on failure. The error code may be from OpenSSL or -1
    // on some other error cases, so it can't really be interpreted
    // unfortunately.

    // Prepare SSL library, state is SSL_CONNECTING.
    int begin_SSL();
    // Perform SSL negotiation steps.
    int continue_SSL();

    // Error handler helper. signal is given as true for errors in
    // asynchronous contexts (when an error method was not returned
    // through some other method), and in that case an SE_CLOSE event is
    // raised on the stream with the specified error.
    // A 0 error means a graceful close, otherwise there is not really enough
    // context to interpret the error code.
    void error_info(const char* context, int err, bool signal);
    void cleanup();
    
    // SSL library configuration
    SSL_CTX* setup_SSL_context();
    
    // Verify the peer certificate matches the signaled digest.
    bool verify_peer_certificate();

    // SSL verification check
    bool SSL_post_connection_check(SSL* ssl,
            const X509* peer_cert,
            const std::string& peer_digest);
 
    static int SSL_verify_callback(int ok, X509_STORE_CTX* store); 
    
    bool waiting_to_verify_peer_certificate() const {
        return client_auth_enabled() && !_peer_certificate_verified;
    }

    bool has_peer_certificate_digest() const {
        return !_peer_certificate_digest_algorithm.empty() &&
            !_peer_certificate_digest_value.empty();
    }
    
    friend void dtls_handshake_timeout_callback(EventLoop* el, 
            TimerWatcher* w, void* data);

private:
    SSLState _state;
    SSLRole _role;
    int _ssl_error_code;  // valid when state_ == SSL_ERROR or SSL_CLOSED
    // Whether the SSL negotiation is blocked on needing to read or
    // write to the wrapped stream.
    bool _ssl_read_needs_write;
    bool _ssl_write_needs_read;

    SSL* _ssl;
    SSL_CTX* _ssl_ctx;

    // Our key and certificate.
    std::unique_ptr<OpenSSLIdentity> _identity;
    // The certificate that the peer presented. Initially null, until the
    // connection is established.
    std::unique_ptr<OpenSSLCertificate> _peer_certificate;
    bool _peer_certificate_verified = false;
    // The digest of the certificate that the peer must present.
    Buffer _peer_certificate_digest_value;
    std::string _peer_certificate_digest_algorithm;

    // The DtlsSrtp ciphers
    std::string _srtp_ciphers;

    // Do DTLS or not
    SSLMode _ssl_mode;

    // Max. allowed protocol version
    SSLProtocolVersion _ssl_max_version;
    
    // A 50-ms initial timeout ensures rapid setup on fast connections, but may
    // be too aggressive for low bandwidth links.
    int _dtls_handshake_timeout_ms = 50;

    EventLoop* _el;
    TimerWatcher* _handshake_timer_watcher;
};

} // namespace rtcbase

#endif  //__RTCBASE_OPENSSL_STREAM_ADAPTER_H_


