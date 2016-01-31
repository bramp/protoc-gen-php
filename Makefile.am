AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = autogen.sh

MAKEFLAGS = --no-builtin-rules
SUFFIXES:

# what flags you want to pass to the C compiler & linker
# TODO:
CXXFLAGS  = -std=c++11 -Wall -Wextra -pedantic -O2
CXXFLAGS += -Werror -Wshadow -Wstrict-overflow -fno-strict-aliasing
CXXFLAGS += -Wno-unused-local-typedef
LDFLAGS   =
LIBS      = -lprotobuf -lprotoc

CLEANFILES = php_options.pb.cc php_options.pb.h

PROTO_INCLUDES=$(shell pkg-config protobuf --cflags-only-I)

bin_PROGRAMS = protoc-gen-php
protoc_gen_php_SOURCES = protoc-gen-php.cc php_options.pb.cc strutil.cc strutil.h util.cc util.h php_util.cc php_util.h

noinst_SCRIPTS = protocolbuffers.inc.php

protoc-gen-php.cc: php_options.pb.cc php_options.pb.h

%.pb.cc %.pb.h: %.proto
	$(PROTOC) --proto_path=$(srcdir) --cpp_out=$(builddir) $(PROTO_INCLUDES) $^


# Useful for testing
MYTESTS = addressbook2.proto addressbook3.proto conformance/conformance.proto 
GENTESTS = $(MYTESTS:.proto=.proto.php)
%.proto.php : %.proto protoc-gen-php
	protoc --php_out . --plugin=protoc-gen-php=./protoc-gen-php $<;

mytest: $(GENTESTS)
	php --syntax-check protocolbuffers.inc.php; \
	for file in $(MYTESTS); do \
		php --syntax-check $${file}.php; \
	done ;