AUTOMAKE_OPTIONS = foreign
SUBDIRS = reference_app

EXTRA_DIST = autogen.sh

MAKEFLAGS = --no-builtin-rules
SUFFIXES:

# what flags you want to pass to the C compiler & linker
CXXFLAGS  = -std=c++11 -Wall -Wextra -pedantic -O2
CXXFLAGS += -Werror -Wshadow -Wstrict-overflow -fno-strict-aliasing
CXXFLAGS += -Wno-unused-local-typedef
export CXXFLAGS

LDFLAGS   =
LIBS      = -lprotobuf -lprotoc


CLEANFILES = php_options.pb.cc php_options.pb.h

PROTO_INCLUDES=$(shell pkg-config protobuf --cflags-only-I)

bin_PROGRAMS = protoc-gen-php
protoc_gen_php_SOURCES = protoc-gen-php.cc php_generator.cc php_options.pb.cc strutil.cc strutil.h php_util.cc php_util.h
protoc_gen_php_SOURCES += php_enum.cc php_field.cc php_message.cc php_method.cc php_read.cc php_service.cc php_size.cc php_tostring.cc php_write.cc


noinst_SCRIPTS = protocolbuffers.inc.php

protoc-gen-php.cc: php_options.pb.cc php_options.pb.h

%.pb.cc %.pb.h: %.proto
	$(PROTOC) --proto_path=$(srcdir) --cpp_out=$(builddir) $(PROTO_INCLUDES) $^

# Renable this, but `make check` doesn't play nice with phpunit
#TESTS = test

test:
	phpunit tests/*_test.php

format: protoc-gen-php.cc php_util.cc php_util.h util.cc util.h
	clang-format -style=google -i $^

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