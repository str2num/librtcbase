#BUILDMAKE edit-mode: -*- Makefile -*-
####################64Bit Mode####################
ifeq ($(shell uname -m), x86_64)
CC=gcc
CXX=g++
CPPFLAGS=-D_GNU_SOURCE \
  -D__STDC_LIMIT_MACROS \
  -DVERSION=\"1.9.8.7\"
CFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
CXXFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC \
  -std=gnu++11
INCPATH=-I. \
  -I./include \
  -I./output \
  -I./output/include \
  -I./deps/libev/include
DEP_INCPATH=


#BUILDMAKE UUID
BUILDMAKE_MD5=f862eb618313cf5c88d33bccecb42124  BUILDMAKE


.PHONY:all
all:buildmake_makefile_check librtcbase.a 
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

PHONY:buildmake_makefile_check
buildmake_makefile_check:
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mbuildmake_makefile_check[0m']"
	#in case of error, update "Makefile" by "buildmake"
	@echo "$(BUILDMAKE_MD5)" > buildmake.md5
	@md5sum -c --status buildmake.md5
	@rm -f buildmake.md5

.PHONY:clean
clean:
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mclean[0m']"
	rm -rf librtcbase.a
	rm -rf ./output/lib/librtcbase.a
	rm -rf ./output/include/rtcbase/array_size.h
	rm -rf ./output/include/rtcbase/array_view.h
	rm -rf ./output/include/rtcbase/async_packet_socket.h
	rm -rf ./output/include/rtcbase/async_socket.h
	rm -rf ./output/include/rtcbase/async_udp_socket.h
	rm -rf ./output/include/rtcbase/atomicops.h
	rm -rf ./output/include/rtcbase/base64.h
	rm -rf ./output/include/rtcbase/basic_types.h
	rm -rf ./output/include/rtcbase/buffer.h
	rm -rf ./output/include/rtcbase/buffer_queue.h
	rm -rf ./output/include/rtcbase/byte_buffer.h
	rm -rf ./output/include/rtcbase/byte_order.h
	rm -rf ./output/include/rtcbase/constructor_magic.h
	rm -rf ./output/include/rtcbase/crc32.h
	rm -rf ./output/include/rtcbase/critical_section.h
	rm -rf ./output/include/rtcbase/dscp.h
	rm -rf ./output/include/rtcbase/event.h
	rm -rf ./output/include/rtcbase/event_loop.h
	rm -rf ./output/include/rtcbase/format_macros.h
	rm -rf ./output/include/rtcbase/function_view.h
	rm -rf ./output/include/rtcbase/ifaddrs_converter.h
	rm -rf ./output/include/rtcbase/ipaddress.h
	rm -rf ./output/include/rtcbase/location.h
	rm -rf ./output/include/rtcbase/log_trace_id.h
	rm -rf ./output/include/rtcbase/logging.h
	rm -rf ./output/include/rtcbase/md5.h
	rm -rf ./output/include/rtcbase/md5_digest.h
	rm -rf ./output/include/rtcbase/memcheck.h
	rm -rf ./output/include/rtcbase/message_digest.h
	rm -rf ./output/include/rtcbase/moving_median_filter.h
	rm -rf ./output/include/rtcbase/net_helpers.h
	rm -rf ./output/include/rtcbase/network.h
	rm -rf ./output/include/rtcbase/network_constants.h
	rm -rf ./output/include/rtcbase/openssl.h
	rm -rf ./output/include/rtcbase/openssl_adapter.h
	rm -rf ./output/include/rtcbase/openssl_digest.h
	rm -rf ./output/include/rtcbase/openssl_identity.h
	rm -rf ./output/include/rtcbase/openssl_stream_adapter.h
	rm -rf ./output/include/rtcbase/optional.h
	rm -rf ./output/include/rtcbase/percentile_filter.h
	rm -rf ./output/include/rtcbase/physical_socket_server.h
	rm -rf ./output/include/rtcbase/platform_thread.h
	rm -rf ./output/include/rtcbase/platform_thread_types.h
	rm -rf ./output/include/rtcbase/ptr_utils.h
	rm -rf ./output/include/rtcbase/random.h
	rm -rf ./output/include/rtcbase/rate_statistics.h
	rm -rf ./output/include/rtcbase/ref_count.h
	rm -rf ./output/include/rtcbase/ref_counted_base.h
	rm -rf ./output/include/rtcbase/ref_counted_object.h
	rm -rf ./output/include/rtcbase/ref_counter.h
	rm -rf ./output/include/rtcbase/rtccertificate.h
	rm -rf ./output/include/rtcbase/rtccertificate_generator.h
	rm -rf ./output/include/rtcbase/safe_compare.h
	rm -rf ./output/include/rtcbase/safe_conversions.h
	rm -rf ./output/include/rtcbase/safe_conversions_impl.h
	rm -rf ./output/include/rtcbase/safe_minmax.h
	rm -rf ./output/include/rtcbase/sanitizer.h
	rm -rf ./output/include/rtcbase/sha1.h
	rm -rf ./output/include/rtcbase/sha1_digest.h
	rm -rf ./output/include/rtcbase/sigslot.h
	rm -rf ./output/include/rtcbase/sigslot_repeater.h
	rm -rf ./output/include/rtcbase/socket.h
	rm -rf ./output/include/rtcbase/socket_address.h
	rm -rf ./output/include/rtcbase/socket_factory.h
	rm -rf ./output/include/rtcbase/ssl_adapter.h
	rm -rf ./output/include/rtcbase/ssl_fingerprint.h
	rm -rf ./output/include/rtcbase/ssl_identity.h
	rm -rf ./output/include/rtcbase/ssl_stream_adapter.h
	rm -rf ./output/include/rtcbase/stream.h
	rm -rf ./output/include/rtcbase/string_encode.h
	rm -rf ./output/include/rtcbase/string_to_number.h
	rm -rf ./output/include/rtcbase/string_utils.h
	rm -rf ./output/include/rtcbase/stringize_macros.h
	rm -rf ./output/include/rtcbase/thread_annotations.h
	rm -rf ./output/include/rtcbase/time_utils.h
	rm -rf ./output/include/rtcbase/type_traits.h
	rm -rf ./output/include/rtcbase/zmalloc.h
	rm -rf ./output/include/rtcbase/zmalloc_define.h
	rm -rf src/rtcbase_async_packet_socket.o
	rm -rf src/rtcbase_async_socket.o
	rm -rf src/rtcbase_async_udp_socket.o
	rm -rf src/rtcbase_base64.o
	rm -rf src/rtcbase_buffer_queue.o
	rm -rf src/rtcbase_byte_buffer.o
	rm -rf src/rtcbase_crc32.o
	rm -rf src/rtcbase_critical_section.o
	rm -rf src/rtcbase_event.o
	rm -rf src/rtcbase_event_loop.o
	rm -rf src/rtcbase_ifaddrs_converter.o
	rm -rf src/rtcbase_ipaddress.o
	rm -rf src/rtcbase_location.o
	rm -rf src/rtcbase_logging.o
	rm -rf src/rtcbase_md5.o
	rm -rf src/rtcbase_md5_digest.o
	rm -rf src/rtcbase_message_digest.o
	rm -rf src/rtcbase_net_helpers.o
	rm -rf src/rtcbase_network.o
	rm -rf src/rtcbase_openssl_adapter.o
	rm -rf src/rtcbase_openssl_digest.o
	rm -rf src/rtcbase_openssl_identity.o
	rm -rf src/rtcbase_openssl_stream_adapter.o
	rm -rf src/rtcbase_physical_socket_server.o
	rm -rf src/rtcbase_platform_thread.o
	rm -rf src/rtcbase_random.o
	rm -rf src/rtcbase_rate_statistics.o
	rm -rf src/rtcbase_rtccertificate.o
	rm -rf src/rtcbase_rtccertificate_generator.o
	rm -rf src/rtcbase_sha1.o
	rm -rf src/rtcbase_sha1_digest.o
	rm -rf src/rtcbase_sigslot.o
	rm -rf src/rtcbase_socket_address.o
	rm -rf src/rtcbase_ssl_adapter.o
	rm -rf src/rtcbase_ssl_fingerprint.o
	rm -rf src/rtcbase_ssl_identity.o
	rm -rf src/rtcbase_ssl_stream_adapter.o
	rm -rf src/rtcbase_stream.o
	rm -rf src/rtcbase_string_encode.o
	rm -rf src/rtcbase_string_to_number.o
	rm -rf src/rtcbase_string_utils.o
	rm -rf src/rtcbase_time_utils.o
	rm -rf src/rtcbase_zmalloc.o

.PHONY:dist
dist:
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mdist[0m']"
	tar czvf output.tar.gz output
	@echo "make dist done"

.PHONY:distclean
distclean:clean
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mdistclean[0m']"
	rm -f output.tar.gz
	@echo "make distclean done"

.PHONY:love
love:
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mlove[0m']"
	@echo "make love done"

librtcbase.a:src/rtcbase_async_packet_socket.o \
  src/rtcbase_async_socket.o \
  src/rtcbase_async_udp_socket.o \
  src/rtcbase_base64.o \
  src/rtcbase_buffer_queue.o \
  src/rtcbase_byte_buffer.o \
  src/rtcbase_crc32.o \
  src/rtcbase_critical_section.o \
  src/rtcbase_event.o \
  src/rtcbase_event_loop.o \
  src/rtcbase_ifaddrs_converter.o \
  src/rtcbase_ipaddress.o \
  src/rtcbase_location.o \
  src/rtcbase_logging.o \
  src/rtcbase_md5.o \
  src/rtcbase_md5_digest.o \
  src/rtcbase_message_digest.o \
  src/rtcbase_net_helpers.o \
  src/rtcbase_network.o \
  src/rtcbase_openssl_adapter.o \
  src/rtcbase_openssl_digest.o \
  src/rtcbase_openssl_identity.o \
  src/rtcbase_openssl_stream_adapter.o \
  src/rtcbase_physical_socket_server.o \
  src/rtcbase_platform_thread.o \
  src/rtcbase_random.o \
  src/rtcbase_rate_statistics.o \
  src/rtcbase_rtccertificate.o \
  src/rtcbase_rtccertificate_generator.o \
  src/rtcbase_sha1.o \
  src/rtcbase_sha1_digest.o \
  src/rtcbase_sigslot.o \
  src/rtcbase_socket_address.o \
  src/rtcbase_ssl_adapter.o \
  src/rtcbase_ssl_fingerprint.o \
  src/rtcbase_ssl_identity.o \
  src/rtcbase_ssl_stream_adapter.o \
  src/rtcbase_stream.o \
  src/rtcbase_string_encode.o \
  src/rtcbase_string_to_number.o \
  src/rtcbase_string_utils.o \
  src/rtcbase_time_utils.o \
  src/rtcbase_zmalloc.o \
  src/array_size.h \
  src/array_view.h \
  src/async_packet_socket.h \
  src/async_socket.h \
  src/async_udp_socket.h \
  src/atomicops.h \
  src/base64.h \
  src/basic_types.h \
  src/buffer.h \
  src/buffer_queue.h \
  src/byte_buffer.h \
  src/byte_order.h \
  src/constructor_magic.h \
  src/crc32.h \
  src/critical_section.h \
  src/dscp.h \
  src/event.h \
  src/event_loop.h \
  src/format_macros.h \
  src/function_view.h \
  src/ifaddrs_converter.h \
  src/ipaddress.h \
  src/location.h \
  src/log_trace_id.h \
  src/logging.h \
  src/md5.h \
  src/md5_digest.h \
  src/memcheck.h \
  src/message_digest.h \
  src/moving_median_filter.h \
  src/net_helpers.h \
  src/network.h \
  src/network_constants.h \
  src/openssl.h \
  src/openssl_adapter.h \
  src/openssl_digest.h \
  src/openssl_identity.h \
  src/openssl_stream_adapter.h \
  src/optional.h \
  src/percentile_filter.h \
  src/physical_socket_server.h \
  src/platform_thread.h \
  src/platform_thread_types.h \
  src/ptr_utils.h \
  src/random.h \
  src/rate_statistics.h \
  src/ref_count.h \
  src/ref_counted_base.h \
  src/ref_counted_object.h \
  src/ref_counter.h \
  src/rtccertificate.h \
  src/rtccertificate_generator.h \
  src/safe_compare.h \
  src/safe_conversions.h \
  src/safe_conversions_impl.h \
  src/safe_minmax.h \
  src/sanitizer.h \
  src/sha1.h \
  src/sha1_digest.h \
  src/sigslot.h \
  src/sigslot_repeater.h \
  src/socket.h \
  src/socket_address.h \
  src/socket_factory.h \
  src/ssl_adapter.h \
  src/ssl_fingerprint.h \
  src/ssl_identity.h \
  src/ssl_stream_adapter.h \
  src/stream.h \
  src/string_encode.h \
  src/string_to_number.h \
  src/string_utils.h \
  src/stringize_macros.h \
  src/thread_annotations.h \
  src/time_utils.h \
  src/type_traits.h \
  src/zmalloc.h \
  src/zmalloc_define.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mlibrtcbase.a[0m']"
	ar crs librtcbase.a src/rtcbase_async_packet_socket.o \
  src/rtcbase_async_socket.o \
  src/rtcbase_async_udp_socket.o \
  src/rtcbase_base64.o \
  src/rtcbase_buffer_queue.o \
  src/rtcbase_byte_buffer.o \
  src/rtcbase_crc32.o \
  src/rtcbase_critical_section.o \
  src/rtcbase_event.o \
  src/rtcbase_event_loop.o \
  src/rtcbase_ifaddrs_converter.o \
  src/rtcbase_ipaddress.o \
  src/rtcbase_location.o \
  src/rtcbase_logging.o \
  src/rtcbase_md5.o \
  src/rtcbase_md5_digest.o \
  src/rtcbase_message_digest.o \
  src/rtcbase_net_helpers.o \
  src/rtcbase_network.o \
  src/rtcbase_openssl_adapter.o \
  src/rtcbase_openssl_digest.o \
  src/rtcbase_openssl_identity.o \
  src/rtcbase_openssl_stream_adapter.o \
  src/rtcbase_physical_socket_server.o \
  src/rtcbase_platform_thread.o \
  src/rtcbase_random.o \
  src/rtcbase_rate_statistics.o \
  src/rtcbase_rtccertificate.o \
  src/rtcbase_rtccertificate_generator.o \
  src/rtcbase_sha1.o \
  src/rtcbase_sha1_digest.o \
  src/rtcbase_sigslot.o \
  src/rtcbase_socket_address.o \
  src/rtcbase_ssl_adapter.o \
  src/rtcbase_ssl_fingerprint.o \
  src/rtcbase_ssl_identity.o \
  src/rtcbase_ssl_stream_adapter.o \
  src/rtcbase_stream.o \
  src/rtcbase_string_encode.o \
  src/rtcbase_string_to_number.o \
  src/rtcbase_string_utils.o \
  src/rtcbase_time_utils.o \
  src/rtcbase_zmalloc.o
	mkdir -p ./output/lib
	cp -f --link librtcbase.a ./output/lib
	mkdir -p ./output/include/rtcbase
	cp -f --link src/array_size.h src/array_view.h src/async_packet_socket.h src/async_socket.h src/async_udp_socket.h src/atomicops.h src/base64.h src/basic_types.h src/buffer.h src/buffer_queue.h src/byte_buffer.h src/byte_order.h src/constructor_magic.h src/crc32.h src/critical_section.h src/dscp.h src/event.h src/event_loop.h src/format_macros.h src/function_view.h src/ifaddrs_converter.h src/ipaddress.h src/location.h src/log_trace_id.h src/logging.h src/md5.h src/md5_digest.h src/memcheck.h src/message_digest.h src/moving_median_filter.h src/net_helpers.h src/network.h src/network_constants.h src/openssl.h src/openssl_adapter.h src/openssl_digest.h src/openssl_identity.h src/openssl_stream_adapter.h src/optional.h src/percentile_filter.h src/physical_socket_server.h src/platform_thread.h src/platform_thread_types.h src/ptr_utils.h src/random.h src/rate_statistics.h src/ref_count.h src/ref_counted_base.h src/ref_counted_object.h src/ref_counter.h src/rtccertificate.h src/rtccertificate_generator.h src/safe_compare.h src/safe_conversions.h src/safe_conversions_impl.h src/safe_minmax.h src/sanitizer.h src/sha1.h src/sha1_digest.h src/sigslot.h src/sigslot_repeater.h src/socket.h src/socket_address.h src/socket_factory.h src/ssl_adapter.h src/ssl_fingerprint.h src/ssl_identity.h src/ssl_stream_adapter.h src/stream.h src/string_encode.h src/string_to_number.h src/string_utils.h src/stringize_macros.h src/thread_annotations.h src/time_utils.h src/type_traits.h src/zmalloc.h src/zmalloc_define.h ./output/include/rtcbase

src/rtcbase_async_packet_socket.o:src/async_packet_socket.cpp \
  src/async_packet_socket.h \
  src/constructor_magic.h \
  src/dscp.h \
  src/sigslot.h \
  src/socket.h \
  src/basic_types.h \
  src/socket_address.h \
  src/ipaddress.h \
  src/byte_order.h \
  src/time_utils.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_async_packet_socket.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_async_packet_socket.o src/async_packet_socket.cpp

src/rtcbase_async_socket.o:src/async_socket.cpp \
  src/async_socket.h \
  src/sigslot.h \
  src/socket.h \
  src/basic_types.h \
  src/constructor_magic.h \
  src/socket_address.h \
  src/ipaddress.h \
  src/byte_order.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_async_socket.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_async_socket.o src/async_socket.cpp

src/rtcbase_async_udp_socket.o:src/async_udp_socket.cpp \
  src/logging.h \
  src/constructor_magic.h \
  src/event_loop.h \
  src/async_udp_socket.h \
  src/memcheck.h \
  src/async_packet_socket.h \
  src/dscp.h \
  src/sigslot.h \
  src/socket.h \
  src/basic_types.h \
  src/socket_address.h \
  src/ipaddress.h \
  src/byte_order.h \
  src/time_utils.h \
  src/socket_factory.h \
  src/async_socket.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_async_udp_socket.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_async_udp_socket.o src/async_udp_socket.cpp

src/rtcbase_base64.o:src/base64.cpp \
  src/base64.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_base64.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_base64.o src/base64.cpp

src/rtcbase_buffer_queue.o:src/buffer_queue.cpp \
  src/buffer_queue.h \
  src/buffer.h \
  src/memcheck.h \
  src/logging.h \
  src/constructor_magic.h \
  src/array_view.h \
  src/type_traits.h \
  src/critical_section.h \
  src/atomicops.h \
  src/thread_annotations.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_buffer_queue.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_buffer_queue.o src/buffer_queue.cpp

src/rtcbase_byte_buffer.o:src/byte_buffer.cpp \
  src/basic_types.h \
  src/byte_order.h \
  src/byte_buffer.h \
  src/memcheck.h \
  src/logging.h \
  src/constructor_magic.h \
  src/buffer.h \
  src/array_view.h \
  src/type_traits.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_byte_buffer.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_byte_buffer.o src/byte_buffer.cpp

src/rtcbase_crc32.o:src/crc32.cpp \
  src/array_size.h \
  src/crc32.h \
  src/basic_types.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_crc32.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_crc32.o src/crc32.cpp

src/rtcbase_critical_section.o:src/critical_section.cpp \
  src/logging.h \
  src/constructor_magic.h \
  src/critical_section.h \
  src/atomicops.h \
  src/thread_annotations.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_critical_section.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_critical_section.o src/critical_section.cpp

src/rtcbase_event.o:src/event.cpp \
  src/event.h \
  src/constructor_magic.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_event.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_event.o src/event.cpp

src/rtcbase_event_loop.o:src/event_loop.cpp \
  deps/libev/include/ev.h \
  src/logging.h \
  src/constructor_magic.h \
  src/time_utils.h \
  src/basic_types.h \
  src/event_loop.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_event_loop.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_event_loop.o src/event_loop.cpp

src/rtcbase_ifaddrs_converter.o:src/ifaddrs_converter.cpp \
  src/ifaddrs_converter.h \
  src/memcheck.h \
  src/logging.h \
  src/constructor_magic.h \
  src/ipaddress.h \
  src/byte_order.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_ifaddrs_converter.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_ifaddrs_converter.o src/ifaddrs_converter.cpp

src/rtcbase_ipaddress.o:src/ipaddress.cpp \
  src/net_helpers.h \
  src/ipaddress.h \
  src/byte_order.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_ipaddress.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_ipaddress.o src/ipaddress.cpp

src/rtcbase_location.o:src/location.cpp \
  src/location.h \
  src/stringize_macros.h \
  src/string_utils.h \
  src/basic_types.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_location.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_location.o src/location.cpp

src/rtcbase_logging.o:src/logging.cpp \
  src/critical_section.h \
  src/atomicops.h \
  src/constructor_magic.h \
  src/thread_annotations.h \
  src/time_utils.h \
  src/basic_types.h \
  src/string_encode.h \
  src/logging.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_logging.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_logging.o src/logging.cpp

src/rtcbase_md5.o:src/md5.cpp \
  src/byte_order.h \
  src/md5.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_md5.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_md5.o src/md5.cpp

src/rtcbase_md5_digest.o:src/md5_digest.cpp \
  src/md5_digest.h \
  src/md5.h \
  src/message_digest.h \
  src/memcheck.h \
  src/logging.h \
  src/constructor_magic.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_md5_digest.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_md5_digest.o src/md5_digest.cpp

src/rtcbase_message_digest.o:src/message_digest.cpp \
  src/basic_types.h \
  src/md5_digest.h \
  src/md5.h \
  src/message_digest.h \
  src/memcheck.h \
  src/logging.h \
  src/constructor_magic.h \
  src/sha1_digest.h \
  src/sha1.h \
  src/string_encode.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_message_digest.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_message_digest.o src/message_digest.cpp

src/rtcbase_net_helpers.o:src/net_helpers.cpp \
  src/byte_order.h \
  src/net_helpers.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_net_helpers.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_net_helpers.o src/net_helpers.cpp

src/rtcbase_network.o:src/network.cpp \
  src/ifaddrs_converter.h \
  src/memcheck.h \
  src/logging.h \
  src/constructor_magic.h \
  src/ipaddress.h \
  src/byte_order.h \
  src/network.h \
  src/network_constants.h \
  src/sigslot.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_network.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_network.o src/network.cpp

src/rtcbase_openssl_adapter.o:src/openssl_adapter.cpp \
  src/logging.h \
  src/constructor_magic.h \
  src/openssl.h \
  src/openssl_adapter.h \
  src/ssl_adapter.h \
  src/ssl_stream_adapter.h \
  src/event_loop.h \
  src/stream.h \
  src/sigslot.h \
  src/ssl_identity.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_openssl_adapter.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_openssl_adapter.o src/openssl_adapter.cpp

src/rtcbase_openssl_digest.o:src/openssl_digest.cpp \
  src/openssl_digest.h \
  src/message_digest.h \
  src/memcheck.h \
  src/logging.h \
  src/constructor_magic.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_openssl_digest.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_openssl_digest.o src/openssl_digest.cpp

src/rtcbase_openssl_identity.o:src/openssl_identity.cpp \
  src/logging.h \
  src/constructor_magic.h \
  src/random.h \
  src/basic_types.h \
  src/openssl.h \
  src/openssl_digest.h \
  src/message_digest.h \
  src/memcheck.h \
  src/openssl_identity.h \
  src/ssl_identity.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_openssl_identity.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_openssl_identity.o src/openssl_identity.cpp

src/rtcbase_openssl_stream_adapter.o:src/openssl_stream_adapter.cpp \
  src/logging.h \
  src/constructor_magic.h \
  src/openssl.h \
  src/openssl_stream_adapter.h \
  src/event_loop.h \
  src/buffer.h \
  src/memcheck.h \
  src/array_view.h \
  src/type_traits.h \
  src/ssl_stream_adapter.h \
  src/stream.h \
  src/sigslot.h \
  src/ssl_identity.h \
  src/openssl_identity.h \
  src/openssl_digest.h \
  src/message_digest.h \
  src/openssl_adapter.h \
  src/ssl_adapter.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_openssl_stream_adapter.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_openssl_stream_adapter.o src/openssl_stream_adapter.cpp

src/rtcbase_physical_socket_server.o:src/physical_socket_server.cpp \
  src/logging.h \
  src/constructor_magic.h \
  src/time_utils.h \
  src/basic_types.h \
  src/physical_socket_server.h \
  src/memcheck.h \
  src/socket_factory.h \
  src/socket.h \
  src/socket_address.h \
  src/ipaddress.h \
  src/byte_order.h \
  src/async_socket.h \
  src/sigslot.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_physical_socket_server.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_physical_socket_server.o src/physical_socket_server.cpp

src/rtcbase_platform_thread.o:src/platform_thread.cpp \
  src/logging.h \
  src/constructor_magic.h \
  src/atomicops.h \
  src/platform_thread.h \
  src/platform_thread_types.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_platform_thread.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_platform_thread.o src/platform_thread.cpp

src/rtcbase_random.o:src/random.cpp \
  src/logging.h \
  src/constructor_magic.h \
  src/basic_types.h \
  src/random.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_random.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_random.o src/random.cpp

src/rtcbase_rate_statistics.o:src/rate_statistics.cpp \
  src/rate_statistics.h \
  src/optional.h \
  src/array_view.h \
  src/type_traits.h \
  src/sanitizer.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_rate_statistics.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_rate_statistics.o src/rate_statistics.cpp

src/rtcbase_rtccertificate.o:src/rtccertificate.cpp \
  src/time_utils.h \
  src/basic_types.h \
  src/rtccertificate.h \
  src/ssl_identity.h \
  src/constructor_magic.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_rtccertificate.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_rtccertificate.o src/rtccertificate.cpp

src/rtcbase_rtccertificate_generator.o:src/rtccertificate_generator.cpp \
  src/rtccertificate_generator.h \
  src/rtccertificate.h \
  src/ssl_identity.h \
  src/constructor_magic.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_rtccertificate_generator.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_rtccertificate_generator.o src/rtccertificate_generator.cpp

src/rtcbase_sha1.o:src/sha1.cpp \
  src/sha1.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_sha1.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_sha1.o src/sha1.cpp

src/rtcbase_sha1_digest.o:src/sha1_digest.cpp \
  src/sha1_digest.h \
  src/message_digest.h \
  src/memcheck.h \
  src/logging.h \
  src/constructor_magic.h \
  src/sha1.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_sha1_digest.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_sha1_digest.o src/sha1_digest.cpp

src/rtcbase_sigslot.o:src/sigslot.cpp \
  src/sigslot.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_sigslot.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_sigslot.o src/sigslot.cpp

src/rtcbase_socket_address.o:src/socket_address.cpp \
  src/byte_order.h \
  src/logging.h \
  src/constructor_magic.h \
  src/net_helpers.h \
  src/socket_address.h \
  src/basic_types.h \
  src/ipaddress.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_socket_address.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_socket_address.o src/socket_address.cpp

src/rtcbase_ssl_adapter.o:src/ssl_adapter.cpp \
  src/ssl_adapter.h \
  src/ssl_stream_adapter.h \
  src/event_loop.h \
  src/stream.h \
  src/sigslot.h \
  src/constructor_magic.h \
  src/ssl_identity.h \
  src/openssl_adapter.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_ssl_adapter.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_ssl_adapter.o src/ssl_adapter.cpp

src/rtcbase_ssl_fingerprint.o:src/ssl_fingerprint.cpp \
  src/logging.h \
  src/constructor_magic.h \
  src/message_digest.h \
  src/memcheck.h \
  src/string_encode.h \
  src/ssl_fingerprint.h \
  src/ssl_identity.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_ssl_fingerprint.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_ssl_fingerprint.o src/ssl_fingerprint.cpp

src/rtcbase_ssl_identity.o:src/ssl_identity.cpp \
  src/time_utils.h \
  src/basic_types.h \
  src/openssl_identity.h \
  src/constructor_magic.h \
  src/ssl_identity.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_ssl_identity.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_ssl_identity.o src/ssl_identity.cpp

src/rtcbase_ssl_stream_adapter.o:src/ssl_stream_adapter.cpp \
  src/openssl_stream_adapter.h \
  src/event_loop.h \
  src/buffer.h \
  src/memcheck.h \
  src/logging.h \
  src/constructor_magic.h \
  src/array_view.h \
  src/type_traits.h \
  src/ssl_stream_adapter.h \
  src/stream.h \
  src/sigslot.h \
  src/ssl_identity.h \
  src/openssl_identity.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_ssl_stream_adapter.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_ssl_stream_adapter.o src/ssl_stream_adapter.cpp

src/rtcbase_stream.o:src/stream.cpp \
  src/stream.h \
  src/sigslot.h \
  src/constructor_magic.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_stream.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_stream.o src/stream.cpp

src/rtcbase_string_encode.o:src/string_encode.cpp \
  src/string_utils.h \
  src/basic_types.h \
  src/string_encode.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_string_encode.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_string_encode.o src/string_encode.cpp

src/rtcbase_string_to_number.o:src/string_to_number.cpp \
  src/string_to_number.h \
  src/optional.h \
  src/array_view.h \
  src/type_traits.h \
  src/sanitizer.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_string_to_number.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_string_to_number.o src/string_to_number.cpp

src/rtcbase_string_utils.o:src/string_utils.cpp \
  src/string_utils.h \
  src/basic_types.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_string_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_string_utils.o src/string_utils.cpp

src/rtcbase_time_utils.o:src/time_utils.cpp \
  src/time_utils.h \
  src/basic_types.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_time_utils.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_time_utils.o src/time_utils.cpp

src/rtcbase_zmalloc.o:src/zmalloc.cpp \
  src/zmalloc_define.h \
  src/zmalloc.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40msrc/rtcbase_zmalloc.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o src/rtcbase_zmalloc.o src/zmalloc.cpp

endif #ifeq ($(shell uname -m), x86_64)


