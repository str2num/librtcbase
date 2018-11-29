/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file stream.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_STREAM_H_
#define  __RTCBASE_STREAM_H_

#include "sigslot.h"
#include "constructor_magic.h"

namespace rtcbase {

// The SS_OPENING state indicates that the stream will signal open or closed
// in the future.
enum StreamState { SS_CLOSED, SS_OPENING, SS_OPEN };

// Stream read/write methods return this value to indicate various success
// and failure conditions described below.
enum StreamResult { SR_ERROR, SR_SUCCESS, SR_BLOCK, SR_EOS };

// StreamEvents are used to asynchronously signal state transitionss.  The flags
// may be combined.
//  SE_OPEN: The stream has transitioned to the SS_OPEN state
//  SE_CLOSE: The stream has transitioned to the SS_CLOSED state
//  SE_READ: Data is available, so Read is likely to not return SR_BLOCK
//  SE_WRITE: Data can be written, so Write is likely to not return SR_BLOCK
enum StreamEvent { SE_OPEN = 1, SE_READ = 2, SE_WRITE = 4, SE_CLOSE = 8 };

class StreamInterface {
public: 
    virtual ~StreamInterface() {}

    virtual StreamState get_state() const = 0;
    
    // Read attempts to fill buffer of size buffer_len.  Write attempts to send
    // data_len bytes stored in data.  The variables read and write are set only
    // on SR_SUCCESS (see below).  Otherwise, error is only set on SR_ERROR.
    // Read and Write return a value indicating:
    //  SR_ERROR: an error occurred, which is returned in a non-null error
    //    argument.  Interpretation of the error requires knowledge of the
    //    stream's concrete type, which limits its usefulness.
    //  SR_SUCCESS: some number of bytes were successfully written, which is
    //    returned in a non-null read/write argument.
    //  SR_BLOCK: the stream is in non-blocking mode, and the operation would
    //    block, or the stream is in SS_OPENING state.
    //  SR_EOS: the end-of-stream has been reached, or the stream is in the
    //    SS_CLOSED state.
    virtual StreamResult read(void* buffer, size_t buffer_len,
            size_t* read, int* error) = 0;
    virtual StreamResult write(const void* data, size_t data_len,
            size_t* written, int* error) = 0;

    // Streams may signal one or more StreamEvents to indicate state changes.
    // The first argument identifies the stream on which the state change occured.
    // The second argument is a bit-wise combination of StreamEvents.
    // If SE_CLOSE is signalled, then the third argument is the associated error
    // code.  Otherwise, the value is undefined.
    // Note: Not all streams will support asynchronous event signalling.  However,
    // SS_OPENING and SR_BLOCK returned from stream member functions imply that
    // certain events will be raised in the future.
    rtcbase::Signal3<StreamInterface*, int, int> signal_event;

    
    // WriteAll is a helper function which repeatedly calls Write until all the
    // data is written, or something other than SR_SUCCESS is returned.  Note that
    // unlike Write, the argument 'written' is always set, and may be non-zero
    // on results other than SR_SUCCESS.  The remaining arguments have the
    // same semantics as Write.
    StreamResult write_all(const void* data, size_t data_len,
            size_t* written, int* error);

protected:
    StreamInterface() {}

private:
    RTC_DISALLOW_COPY_AND_ASSIGN(StreamInterface);
};

class StreamAdapterInterface : public StreamInterface,
                               public HasSlots<>
{
public:
    explicit StreamAdapterInterface(StreamInterface* stream, bool owned = true);
   
    // Core Stream Interface
    StreamState get_state() const override; 
    StreamResult read(void* buffer,
            size_t buffer_len,
            size_t* read,
            int* error) override;
    StreamResult write(const void* data,
            size_t data_len,
            size_t* written,
            int* error) override;

protected:
    ~StreamAdapterInterface() override;

    // Note that the adapter presents itself as the origin of the stream events,
    // since users of the adapter may not recognize the adapted object.
    virtual void on_event(StreamInterface* stream, int events, int err); 
    StreamInterface* stream() { return _stream; }    

private:
    StreamInterface* _stream;
    bool _owned;
    
    RTC_DISALLOW_COPY_AND_ASSIGN(StreamAdapterInterface);
};

} // namespace rtcbase

#endif  //__RTCBASE_STREAM_H_


