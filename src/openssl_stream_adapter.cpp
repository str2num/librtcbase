/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file openssl_stream_adapter.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/tls1.h>
#include <openssl/x509v3.h>

#include "logging.h"
#include "openssl.h"
#include "openssl_stream_adapter.h"
#include "openssl_digest.h"
#include "openssl_adapter.h"

namespace rtcbase {

#if (OPENSSL_VERSION_NUMBER >= 0x10001000L)
#define HAVE_DTLS_SRTP
#endif

#ifdef HAVE_DTLS_SRTP
// SRTP cipher suite table. |internal_name| is used to construct a
// colon-separated profile strings which is needed by
// SSL_CTX_set_tlsext_use_srtp().
struct SrtpCipherMapEntry {
  const char* internal_name;
  const int id;
};

// This isn't elegant, but it's better than an external reference
static SrtpCipherMapEntry srtp_cipher_map[] = {
    {"SRTP_AES128_CM_SHA1_80", SRTP_AES128_CM_SHA1_80},
    {"SRTP_AES128_CM_SHA1_32", SRTP_AES128_CM_SHA1_32},
    {"SRTP_AEAD_AES_128_GCM", SRTP_AEAD_AES_128_GCM},
    {"SRTP_AEAD_AES_256_GCM", SRTP_AEAD_AES_256_GCM},
    {nullptr, 0}};
#endif

//////////////// StreamBIO ///////////////////////

static int stream_write(BIO* h, const char* buf, int num);
static int stream_read(BIO* h, char* buf, int size);
static int stream_puts(BIO* h, const char* str);
static long stream_ctrl(BIO* h, int cmd, long arg1, void* arg2);
static int stream_new(BIO* h);
static int stream_free(BIO* data);

// TODO(davidben): This should be const once BoringSSL is assumed.
static BIO_METHOD methods_stream = {
    BIO_TYPE_BIO,
    "stream",
    stream_write,
    stream_read,
    stream_puts,
    0,
    stream_ctrl,
    stream_new,
    stream_free,
    NULL,
};

static BIO_METHOD* BIO_s_stream() { return(&methods_stream); }

static BIO* BIO_new_stream(StreamInterface* stream) {
    BIO* ret = BIO_new(BIO_s_stream());
    if (ret == NULL) {
        return NULL;
    }
    ret->ptr = stream;
    return ret;
}

// bio methods return 1 (or at least non-zero) on success and 0 on failure.

static int stream_new(BIO* b) {
    b->shutdown = 0;
    b->init = 1;
    b->num = 0;  // 1 means end-of-stream
    b->ptr = 0;
    return 1;
}

static int stream_free(BIO* b) {
  if (b == NULL) {
    return 0;
  }
  return 1;
}

static int stream_read(BIO* b, char* out, int outl) {
    if (!out) {
        return -1;
    }
    StreamInterface* stream = static_cast<StreamInterface*>(b->ptr);
    BIO_clear_retry_flags(b);
    size_t read;
    int error;
    StreamResult result = stream->read(out, outl, &read, &error);
    if (result == SR_SUCCESS) {
        return static_cast<int>(read);
    } else if (result == SR_EOS) {
        b->num = 1;
    } else if (result == SR_BLOCK) {
        BIO_set_retry_read(b);
    }
    return -1;
}

static int stream_write(BIO* b, const char* in, int inl) {
    if (!in)
        return -1;
    StreamInterface* stream = static_cast<StreamInterface*>(b->ptr);
    BIO_clear_retry_flags(b);
    size_t written;
    int error;
    StreamResult result = stream->write(in, inl, &written, &error);
    if (result == SR_SUCCESS) {
        return static_cast<int>(written);
    } else if (result == SR_BLOCK) {
        BIO_set_retry_write(b);
    }
    return -1;
}

static int stream_puts(BIO* b, const char* str) {
    return stream_write(b, str, static_cast<int>(strlen(str)));
}

static long stream_ctrl(BIO* b, int cmd, long num, void* ptr) {
    (void)num;
    (void)ptr;

    switch (cmd) {
        case BIO_CTRL_RESET:
            return 0;
        case BIO_CTRL_EOF:
            return b->num;
        case BIO_CTRL_WPENDING:
        case BIO_CTRL_PENDING:
            return 0;
        case BIO_CTRL_FLUSH:
            return 1;
        case BIO_CTRL_DGRAM_QUERY_MTU:
            // openssl defaults to mtu=256 unless we return something here.
            // The handshake doesn't actually need to send packets above 1k,
            // so this seems like a sensible value that should work in most cases.
            // Webrtc uses the same value for video packets.
            return 1200;
        default:
            return 0;
    }
}

void dtls_handshake_timeout_callback(EventLoop* el, TimerWatcher* w, void* data) {
    LOG(LS_TRACE) << "DTLS timeout expired";

    if (!data) {
        el->stop_timer(w);
        return;
    }

    OpenSSLStreamAdapter* ssl_stream = (OpenSSLStreamAdapter*)data;
    DTLSv1_handle_timeout(ssl_stream->_ssl);
    ssl_stream->continue_SSL();
}

////////////////////// OpenSSLStreamAdapter //////////////////////

OpenSSLStreamAdapter::OpenSSLStreamAdapter(StreamInterface* stream, 
        EventLoop* el)
    : SSLStreamAdapter(stream),
    _state(SSL_NONE),
    _role(SSL_CLIENT),
    _ssl_read_needs_write(false),
    _ssl_write_needs_read(false),
    _ssl(NULL),
    _ssl_ctx(NULL),
    _ssl_mode(SSL_MODE_TLS),
    _ssl_max_version(SSL_PROTOCOL_TLS_12),
    _el(el),
    _handshake_timer_watcher(NULL)
{
    if (_el) {
        _handshake_timer_watcher = 
            _el->create_timer(dtls_handshake_timeout_callback, this, false);
    }
}

OpenSSLStreamAdapter::~OpenSSLStreamAdapter() {
    cleanup();
}

void OpenSSLStreamAdapter::on_event(StreamInterface* stream, int events,
        int err) 
{
    int events_to_signal = 0;
    int signal_error = 0;
    if ((events & SE_OPEN)) {
        LOG(LS_TRACE) << "OpenSSLStreamAdapter::OnEvent SE_OPEN";
        if (_state != SSL_WAIT) {
            events_to_signal |= SE_OPEN;
        } else {
            _state = SSL_CONNECTING;
            if (int err = begin_SSL()) {
                error_info("BeginSSL", err, true);
                return;
            }
        }
    }
    
    if ((events & (SE_READ|SE_WRITE))) {
        LOG(LS_TRACE) << "OpenSSLStreamAdapter::OnEvent"
            << ((events & SE_READ) ? " SE_READ" : "")
            << ((events & SE_WRITE) ? " SE_WRITE" : "");
        if (_state == SSL_NONE) {
            events_to_signal |= events & (SE_READ|SE_WRITE);
        } else if (_state == SSL_CONNECTING) {
            if (int err = continue_SSL()) {
                error_info("ContinueSSL", err, true);
                return;
            }
        } else if (_state == SSL_CONNECTED) {
            if (((events & SE_READ) && _ssl_write_needs_read) ||
                    (events & SE_WRITE)) 
            {
                LOG(LS_TRACE) << " -- onStreamWriteable";
                events_to_signal |= SE_WRITE;
            }
            if (((events & SE_WRITE) && _ssl_read_needs_write) ||
                    (events & SE_READ)) 
            {
                LOG(LS_TRACE) << " -- onStreamReadable";
                events_to_signal |= SE_READ;
            }
        }
    }
    
    if ((events & SE_CLOSE)) {
        LOG(LS_TRACE) << "OpenSSLStreamAdapter::OnEvent(SE_CLOSE, " << err << ")";
        cleanup();
        events_to_signal |= SE_CLOSE;
        // SE_CLOSE is the only event that uses the final parameter to OnEvent().
        signal_error = err;
    }
    if (events_to_signal) {
        StreamAdapterInterface::on_event(stream, events_to_signal, signal_error);
    }
}

int OpenSSLStreamAdapter::begin_SSL() {
    if (_state != SSL_CONNECTING) {
        LOG(LS_WARNING) << "State is not SSL_CONNECTING before begin_SSL";
        return -1;
    }
    
    // The underlying stream has opened.
    LOG(LS_TRACE) << "BeginSSL with peer.";

    BIO* bio = nullptr;

    // First set up the context.
    _ssl_ctx = setup_SSL_context();
    if (!_ssl_ctx) {
        return -1;
    }
     
    bio = BIO_new_stream(static_cast<StreamInterface*>(stream()));
    if (!bio) {
        return -1;
    }

    _ssl = SSL_new(_ssl_ctx);
    if (!_ssl) {
        BIO_free(bio);
        return -1;
    }

    SSL_set_app_data(_ssl, this);

    SSL_set_bio(_ssl, bio, bio);  // the SSL object owns the bio now.
    if (_ssl_mode == SSL_MODE_DTLS) {
        // Enable read-ahead for DTLS so whole packets are read from internal BIO
        // before parsing. This is done internally by BoringSSL for DTLS.
        SSL_set_read_ahead(_ssl, 1);
    }

    SSL_set_mode(_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE |
            SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

    // Specify an ECDH group for ECDHE ciphers, otherwise OpenSSL cannot
    // negotiate them when acting as the server. Use NIST's P-256 which is
    // commonly supported. BoringSSL doesn't need explicit configuration and has
    // a reasonable default set.
    EC_KEY* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (ecdh == nullptr) {
        return -1;
    }
    SSL_set_options(_ssl, SSL_OP_SINGLE_ECDH_USE);
    SSL_set_tmp_ecdh(_ssl, ecdh);
    EC_KEY_free(ecdh);

    // Do the connect
    return continue_SSL();    
}

int OpenSSLStreamAdapter::continue_SSL() {
    LOG(LS_TRACE) << "ContinueSSL";

    _el->stop_timer(_handshake_timer_watcher);

    int code = (_role == SSL_CLIENT) ? SSL_connect(_ssl) : SSL_accept(_ssl);
    int ssl_error;
    
    switch (ssl_error = SSL_get_error(_ssl, code)) {
        case SSL_ERROR_NONE:
            LOG(LS_TRACE) << " -- success";
            _state = SSL_CONNECTED;
            
            if (!waiting_to_verify_peer_certificate()) {
                // We have everything we need to start the connection, so signal
                // SE_OPEN. If we need a client certificate fingerprint and don't have
                // it yet, we'll instead signal SE_OPEN in SetPeerCertificateDigest.
                //
                // TODO(deadbeef): Post this event asynchronously to unwind the stack.
                // The caller of ContinueSSL may be the same object listening for these
                // events and may not be prepared for reentrancy.
                // PostEvent(SE_OPEN | SE_READ | SE_WRITE, 0);
                StreamAdapterInterface::on_event(stream(), SE_OPEN | SE_READ | SE_WRITE,
                        0);
            } 
            break;
        case SSL_ERROR_WANT_READ: 
            {
                LOG(LS_TRACE) << " -- error want read";
                struct timeval timeout;
                if (DTLSv1_get_timeout(_ssl, &timeout)) {
                    int delay = timeout.tv_sec * 1000000 + timeout.tv_usec;
                    if (delay > 0) {
                        _el->start_timer(_handshake_timer_watcher, delay);
                    }
                }
            }
            break;
        case SSL_ERROR_WANT_WRITE:
            LOG(LS_TRACE) << " -- error want write";
            break;
        case SSL_ERROR_ZERO_RETURN:
        default:
            LOG(LS_TRACE) << " -- error " << code;
            SSLHandshakeError ssl_handshake_err = SSLHandshakeError::UNKNOWN;
            int err_code = ERR_peek_last_error();
            if (err_code != 0 && ERR_GET_REASON(err_code) == SSL_R_NO_SHARED_CIPHER) {
                ssl_handshake_err = SSLHandshakeError::INCOMPATIBLE_CIPHERSUITE;
            }
            signal_ssl_handshake_error(ssl_handshake_err); 
            return (ssl_error != 0) ? ssl_error : -1;
    }

    return 0;
}

void OpenSSLStreamAdapter::error_info(const char* context, 
        int err, 
        bool signal) 
{
    LOG(LS_WARNING) << "OpenSSLStreamAdapter::Error("
        << context << ", " << err << ")";
    _state = SSL_ERROR;
    _ssl_error_code = err;
    cleanup();
    if (signal) {
        StreamAdapterInterface::on_event(stream(), SE_CLOSE, err);
    }
}

void OpenSSLStreamAdapter::cleanup() {
    LOG(LS_TRACE) << "Cleanup";

    if (_state != SSL_ERROR) {
        _state = SSL_CLOSED;
        _ssl_error_code = 0;
    }

    if (_ssl) {
        int ret = SSL_shutdown(_ssl);
        if (ret < 0) {
            LOG(LS_WARNING) << "SSL_shutdown failed, error = "
                << SSL_get_error(_ssl, ret);
        }

        SSL_free(_ssl);
        _ssl = NULL;
    }
    if (_ssl_ctx) {
        SSL_CTX_free(_ssl_ctx);
        _ssl_ctx = NULL;
    }
    _identity.reset();
    _peer_certificate.reset();
    
    if (_el && _handshake_timer_watcher) {
        _el->delete_timer(_handshake_timer_watcher);
        _handshake_timer_watcher = NULL;
    }
}

SSL_CTX* OpenSSLStreamAdapter::setup_SSL_context() { 
    SSL_CTX* ctx = nullptr;

    const SSL_METHOD* method;
    switch (_ssl_max_version) {
        case SSL_PROTOCOL_TLS_10:
        case SSL_PROTOCOL_TLS_11:
            // OpenSSL doesn't support setting min/max versions, so we always use
            // (D)TLS 1.0 if a max. version below the max. available is requested.
            if (_ssl_mode == SSL_MODE_DTLS) {
                if (_role == SSL_CLIENT) {
                    method = DTLSv1_client_method();
                } else {
                    method = DTLSv1_server_method();
                }
            } else {
                if (_role == SSL_CLIENT) {
                    method = TLSv1_client_method();
                } else {
                    method = TLSv1_server_method();
                }
            }
            break;
        case SSL_PROTOCOL_TLS_12:
        default:
            if (_ssl_mode == SSL_MODE_DTLS) {
#if (OPENSSL_VERSION_NUMBER >= 0x10002000L)
                // DTLS 1.2 only available starting from OpenSSL 1.0.2
                if (_role == SSL_CLIENT) {
                    method = DTLS_client_method();
                } else {
                    method = DTLS_server_method();
                }
#else
                if (_role == SSL_CLIENT) {
                    method = DTLSv1_client_method();
                } else {
                    method = DTLSv1_server_method();
                }
#endif
            } else {
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
                // New API only available starting from OpenSSL 1.1.0
                if (_role == SSL_CLIENT) {
                    method = TLS_client_method();
                } else {
                    method = TLS_server_method();
                }
#else
                if (_role == SSL_CLIENT) {
                    method = SSLv23_client_method();
                } else {
                    method = SSLv23_server_method();
                }
#endif
            }
            break;
    }
    ctx = SSL_CTX_new(method);

    if (ctx == nullptr) {
        return nullptr;
    }

    if (_identity && !_identity->configure_identity(ctx)) {
        SSL_CTX_free(ctx);
        return nullptr;
    }
     
#if !defined(NDEBUG)
    SSL_CTX_set_info_callback(ctx, OpenSSLAdapter::SSL_info_callback);
#endif

    int mode = SSL_VERIFY_PEER;
    if (client_auth_enabled()) {
        // Require a certificate from the client.
        // Note: Normally this is always true in production, but it may be disabled
        // for testing purposes (e.g. SSLAdapter unit tests).
        mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
    }

    SSL_CTX_set_verify(ctx, mode, SSL_verify_callback);
    SSL_CTX_set_verify_depth(ctx, 4);

    // Select list of available ciphers. Note that !SHA256 and !SHA384 only
    // remove HMAC-SHA256 and HMAC-SHA384 cipher suites, not GCM cipher suites
    // with SHA256 or SHA384 as the handshake hash.
    // This matches the list of SSLClientSocketOpenSSL in Chromium.
    SSL_CTX_set_cipher_list(ctx,
            "DEFAULT:!NULL:!aNULL:!SHA256:!SHA384:!aECDH:!AESGCM+AES256:!aPSK");
    
    if (!_srtp_ciphers.empty()) {
        if (SSL_CTX_set_tlsext_use_srtp(ctx, _srtp_ciphers.c_str())) {
            SSL_CTX_free(ctx);
            return nullptr;
        }
    }

    return ctx;
}

bool OpenSSLStreamAdapter::SSL_post_connection_check(SSL* ssl,
        const X509* peer_cert,
        const std::string &peer_digest) 
{
    (void)ssl;

    if (!peer_cert && peer_digest.empty()) {
        return false;
    }
    return true;
}

void OpenSSLStreamAdapter::set_identity(SSLIdentity* identity) {
    _identity.reset(static_cast<OpenSSLIdentity*>(identity));
}

void OpenSSLStreamAdapter::set_server_role(SSLRole role) {
    _role = role;
}

void OpenSSLStreamAdapter::set_mode(SSLMode mode) {
    _ssl_mode = mode;
}

void OpenSSLStreamAdapter::set_max_protocol_version(SSLProtocolVersion version) {
    _ssl_max_version = version;
}

void OpenSSLStreamAdapter::set_initial_retransmission_timeout(
        int timeout_ms) 
{
    _dtls_handshake_timeout_ms = timeout_ms;
}

int OpenSSLStreamAdapter::start_SSL() {
    if (_state != SSL_NONE) {
        // Don't allow StartSSL to be called twice.
        return -1;
    } 
    
    if (StreamAdapterInterface::get_state() != SS_OPEN) {
        _state = SSL_WAIT;
        return 0;
    }

    _state = SSL_CONNECTING;
    if (int err = begin_SSL()) {
        error_info("BeginSSL", err, false);
        return err;
    }

    return 0;
}

bool OpenSSLStreamAdapter::verify_peer_certificate() {
    if (!has_peer_certificate_digest() || !_peer_certificate) {
        LOG(LS_WARNING) << "Missing digest or peer certificate.";
        return false;
    }

    unsigned char digest[EVP_MAX_MD_SIZE];
    size_t digest_length;
    if (!OpenSSLCertificate::compute_digest(
                _peer_certificate->x509(), _peer_certificate_digest_algorithm, digest,
                sizeof(digest), &digest_length)) 
    {
        LOG(LS_WARNING) << "Failed to compute peer cert digest.";
        return false;
    }

    Buffer computed_digest(digest, digest_length);
    if (computed_digest != _peer_certificate_digest_value) {
        LOG(LS_WARNING)
            << "Rejected peer certificate due to mismatched digest.";
        return false;
    }
    // Ignore any verification error if the digest matches, since there is no
    // value in checking the validity of a self-signed cert issued by untrusted
    // sources.
    LOG(LS_TRACE) << "Accepted peer certificate.";
    _peer_certificate_verified = true;
    return true;
}

bool OpenSSLStreamAdapter::set_peer_certificate_digest(const std::string
        &digest_alg,
        const unsigned char*
        digest_val,
        size_t digest_len,
        SSLPeerCertificateDigestError* error) 
{
    size_t expected_len;
    
    if (error) {
        *error = SSLPeerCertificateDigestError::NONE;
    }

    if (!OpenSSLDigest::get_digest_size(digest_alg, &expected_len)) {
        LOG(LS_WARNING) << "Unknown digest algorithm: " << digest_alg;
        if (error) {
            *error = SSLPeerCertificateDigestError::UNKNOWN_ALGORITHM;
        }
        return false;
    }
    if (expected_len != digest_len) {
        if (error) {
            *error = SSLPeerCertificateDigestError::INVALID_LENGTH;
        }
        return false;
    }

    _peer_certificate_digest_value.set_data(digest_val, digest_len);
    _peer_certificate_digest_algorithm = digest_alg;
    
    if (!_peer_certificate) {
        // Normal case, where the digest is set before we obtain the certificate
        // from the handshake.
        return true;
    }

    if (!verify_peer_certificate()) {
        error_info("SetPeerCertificateDigest", -1, false);
        if (error) {
            *error = SSLPeerCertificateDigestError::VERIFICATION_FAILED;
        }
        return false;
    } 

    return true;
}

bool OpenSSLStreamAdapter::get_ssl_cipher_suite(int* cipher_suite) {
    if (_state != SSL_CONNECTED) {
        return false;
    }

    const SSL_CIPHER* current_cipher = SSL_get_current_cipher(_ssl);
    if (current_cipher == nullptr) {
        return false;
    }

    *cipher_suite = static_cast<uint16_t>(SSL_CIPHER_get_id(current_cipher));
    return true;
}

// Key Extractor interface
bool OpenSSLStreamAdapter::export_keying_material(const std::string& label,
        const uint8_t* context,
        size_t context_len,
        bool use_context,
        uint8_t* result,
        size_t result_len) 
{
    int i;

    i = SSL_export_keying_material(_ssl, result, result_len, label.c_str(),
            label.length(), const_cast<uint8_t*>(context),
            context_len, use_context);

    if (i != 1) {
        return false;
    }

    return true;
}

bool OpenSSLStreamAdapter::set_dtls_srtp_crypto_suites(
        const std::vector<int>& ciphers) 
{
#ifdef HAVE_DTLS_SRTP
    std::string internal_ciphers;

    if (_state != SSL_NONE) {
        return false;
    }

    for (std::vector<int>::const_iterator cipher = ciphers.begin();
            cipher != ciphers.end(); ++cipher) 
    {
        bool found = false;
        for (SrtpCipherMapEntry* entry = srtp_cipher_map; entry->internal_name;
                ++entry) 
        {
            if (*cipher == entry->id) {
                found = true;
                if (!internal_ciphers.empty()) {
                    internal_ciphers += ":";
                }
                internal_ciphers += entry->internal_name;
                break;
            }
        }

        if (!found) {
            LOG(LS_FATAL) << "Could not find cipher: " << *cipher;
            return false;
        }
    }

    if (internal_ciphers.empty()) {
        return false;
    }

    _srtp_ciphers = internal_ciphers;
    return true;
#else
    return false;
#endif
}

bool OpenSSLStreamAdapter::get_dtls_srtp_crypto_suite(int* crypto_suite) {
#ifdef HAVE_DTLS_SRTP
    if (_state != SSL_CONNECTED) {
        return false;
    }

    const SRTP_PROTECTION_PROFILE* srtp_profile =
        SSL_get_selected_srtp_profile(_ssl);

    if (!srtp_profile) {
        return false;
    }

    *crypto_suite = srtp_profile->id;
    //ASSERT(!SrtpCryptoSuiteToName(*crypto_suite).empty());
    return true;
#else
    return false;
#endif
}

int OpenSSLStreamAdapter::SSL_verify_callback(int ok, X509_STORE_CTX* store) {
    (void)ok;
    // Get our SSL structure from the store
    SSL* ssl = reinterpret_cast<SSL*>(X509_STORE_CTX_get_ex_data(
                store,
                SSL_get_ex_data_X509_STORE_CTX_idx()));
    OpenSSLStreamAdapter* stream =
        reinterpret_cast<OpenSSLStreamAdapter*>(SSL_get_app_data(ssl));

    X509* cert = X509_STORE_CTX_get_current_cert(store);
    int depth = X509_STORE_CTX_get_error_depth(store);
    
    // For now We ignore the parent certificates and verify the leaf against
    // the digest.
    //
    // TODO(jiayl): Verify the chain is a proper chain and report the chain to
    // |stream->peer_certificate_|.
    if (depth > 0) {
        LOG(LS_TRACE) << "Ignored chained certificate at depth " << depth;
        return 1;
    }   
    
    // If the peer certificate digest isn't known yet, we'll wait to verify
    // until it's known, and for now just return a success status.
    if (stream->_peer_certificate_digest_algorithm.empty()) {
        LOG(LS_TRACE) << "Waiting to verify certificate until digest is known.";
        return 1;
    }

    if (!stream->_peer_certificate_verified) {
        // Record the peer's certificate.
        stream->_peer_certificate.reset(new OpenSSLCertificate(cert));

        if (!stream->verify_peer_certificate()) {
            X509_STORE_CTX_set_error(store, X509_V_ERR_CERT_REJECTED);
            return 0;
        }
    }
    return 1; 
} 

} // namespace rtcbase


