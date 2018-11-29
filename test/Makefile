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
  -I../deps/libev/include \
  -I../output/include
DEP_INCPATH=


#BUILDMAKE UUID
BUILDMAKE_MD5=7da0d15aa3b5d6d4616efaab4b3b5d6e  BUILDMAKE


.PHONY:all
all:buildmake_makefile_check test 
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
	rm -rf test
	rm -rf ./output/bin/test
	rm -rf test_array_size_test.o
	rm -rf test_base64_test.o
	rm -rf test_network_test.o
	rm -rf test_test.o

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

test:test_array_size_test.o \
  test_base64_test.o \
  test_network_test.o \
  test_test.o \
  ../deps/libev/lib/libev.a \
  ../output/lib/*.a
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mtest[0m']"
	$(CXX) test_array_size_test.o \
  test_base64_test.o \
  test_network_test.o \
  test_test.o -Xlinker "-(" ../deps/libev/lib/libev.a \
  ../output/lib/*.a  -lpthread \
  -lcrypto \
  -lrt -Xlinker "-)" -o test
	mkdir -p ./output/bin
	cp -f --link test ./output/bin

test_array_size_test.o:array_size_test.cpp
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mtest_array_size_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test_array_size_test.o array_size_test.cpp

test_base64_test.o:base64_test.cpp
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mtest_base64_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test_base64_test.o base64_test.cpp

test_network_test.o:network_test.cpp
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mtest_network_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test_network_test.o network_test.cpp

test_test.o:test.cpp \
  test.h
	@echo "[[1;32;40mBUILDMAKE:BUILD[0m][Target:'[1;32;40mtest_test.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test_test.o test.cpp

endif #ifeq ($(shell uname -m), x86_64)


