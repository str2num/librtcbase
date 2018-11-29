/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file openssl_adapter.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include "logging.h"
#include "openssl.h"
#include "openssl_adapter.h"

#define MUTEX_TYPE pthread_mutex_t
#define MUTEX_SETUP(x) pthread_mutex_init(&(x), NULL)
#define MUTEX_CLEANUP(x) pthread_mutex_destroy(&(x))
#define MUTEX_LOCK(x) pthread_mutex_lock(&(x))
#define MUTEX_UNLOCK(x) pthread_mutex_unlock(&(x))
#define THREAD_ID pthread_self()

struct CRYPTO_dynlock_value {
    MUTEX_TYPE mutex;
};

namespace rtcbase {

// This array will store all of the mutexes available to OpenSSL.
static MUTEX_TYPE* mutex_buf = NULL;

static void locking_function(int mode, int n, const char * file, int line) {
    (void)mode;
    (void)file;
    (void)line;

    if (mode & CRYPTO_LOCK) {
        MUTEX_LOCK(mutex_buf[n]);
    } else {
        MUTEX_UNLOCK(mutex_buf[n]);
    }
}

static unsigned long id_function() {  // NOLINT
    // Use old-style C cast because THREAD_ID's type varies with the platform,
    // in some cases requiring static_cast, and in others requiring
    // reinterpret_cast.
    return (unsigned long)THREAD_ID; // NOLINT
}

static CRYPTO_dynlock_value* dyn_create_function(const char* file, int line) {
    (void)file;
    (void)line;

    CRYPTO_dynlock_value* value = new CRYPTO_dynlock_value;
    if (!value) {
        return NULL;
    }
    MUTEX_SETUP(value->mutex);
    return value;
}

static void dyn_lock_function(int mode, CRYPTO_dynlock_value* l,
        const char* file, int line) 
{
    (void)mode;
    (void)file;
    (void)line;

    if (mode & CRYPTO_LOCK) {
        MUTEX_LOCK(l->mutex);
    } else {
        MUTEX_UNLOCK(l->mutex);
    }
}

static void dyn_destroy_function(CRYPTO_dynlock_value* l,
        const char* file, int line) 
{
    (void)file;
    (void)line;

    MUTEX_CLEANUP(l->mutex);
    delete l;
}

VerificationCallback OpenSSLAdapter::_custom_verify_callback = NULL;

bool OpenSSLAdapter::initialize_SSL(VerificationCallback callback) {
    if (!initialize_SSL_thread() || !SSL_library_init()) {
        return false;
    }
#if !defined(ADDRESS_SANITIZER) || !defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
    // Loading the error strings crashes mac_asan.  Omit this debugging aid there.
    SSL_load_error_strings();
#endif
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
    RAND_poll();
    _custom_verify_callback = callback;
    return true;
}

bool OpenSSLAdapter::initialize_SSL_thread() {
    // BoringSSL is doing the locking internally, so the callbacks are not used
    // in this case (and are no-ops anyways).
    mutex_buf = new MUTEX_TYPE[CRYPTO_num_locks()];
    if (!mutex_buf) {
        return false;
    }
    for (int i = 0; i < CRYPTO_num_locks(); ++i) {
        MUTEX_SETUP(mutex_buf[i]);
    }

    // we need to cast our id_function to return an unsigned long -- pthread_t is
    // a pointer
    CRYPTO_set_id_callback(id_function);
    CRYPTO_set_locking_callback(locking_function);
    CRYPTO_set_dynlock_create_callback(dyn_create_function);
    CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
    CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
    return true;
}

bool OpenSSLAdapter::cleanup_SSL() {
    if (!mutex_buf) {
        return false;
    }
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    CRYPTO_set_dynlock_create_callback(NULL);
    CRYPTO_set_dynlock_lock_callback(NULL);
    CRYPTO_set_dynlock_destroy_callback(NULL);
    for (int i = 0; i < CRYPTO_num_locks(); ++i) {
        MUTEX_CLEANUP(mutex_buf[i]);
    }
    delete [] mutex_buf;
    mutex_buf = NULL;
    return true;
}

void OpenSSLAdapter::SSL_info_callback(const SSL* s, int where, int ret) {
    const char* str = "undefined";
    int w = where & ~SSL_ST_MASK;
    if (w & SSL_ST_CONNECT) {
        str = "SSL_connect";
    } else if (w & SSL_ST_ACCEPT) {
        str = "SSL_accept";
    }
    if (where & SSL_CB_LOOP) {
        LOG(LS_TRACE) <<  str << ":" << SSL_state_string_long(s);
    } else if (where & SSL_CB_ALERT) {
        str = (where & SSL_CB_READ) ? "read" : "write";
        LOG(LS_TRACE) <<  "SSL3 alert " << str
            << ":" << SSL_alert_type_string_long(ret)
            << ":" << SSL_alert_desc_string_long(ret);
    } else if (where & SSL_CB_EXIT) {
        if (ret == 0) {
            LOG(LS_TRACE) << str << ":failed in " << SSL_state_string_long(s);
        } else if (ret < 0) {
            LOG(LS_TRACE) << str << ":error in " << SSL_state_string_long(s);
        }
    }
}

} // namespace rtcbase


