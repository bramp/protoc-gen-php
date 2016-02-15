.PHONY = test bench lint

AUTOMAKE_OPTIONS = foreign
SUBDIRS = src tests reference_app

EXTRA_DIST = autogen.sh

MAKEFLAGS = --no-builtin-rules
SUFFIXES:

# what flags you want to pass to the C compiler & linker
CXXFLAGS  = -std=c++11 -Wall -Wextra -pedantic -O2
CXXFLAGS += -Werror -Wshadow -Wstrict-overflow -fno-strict-aliasing
CXXFLAGS += -Wno-unused-local-typedef
export CXXFLAGS

noinst_SCRIPTS = protocolbuffers.inc.php

# Renable this, but `make check` doesn't play nice with phpunit
#TESTS = test

test bench:
	$(MAKE) -C tests $@

# Useful for testing
TEST_PROTOS = reference_app/addressbook2.proto reference_app/addressbook3.proto reference_app/conformance.proto 
TEST_PROTOS_PHP = $(TEST_PROTOS:.proto=.proto.php)
%.proto.php : %.proto src/protoc-gen-php
	$(PROTOC) --php_out . --plugin=protoc-gen-php=src/protoc-gen-php $<;

lint: protocolbuffers.inc.php $(TEST_PROTOS_PHP)
	$(PHP) --syntax-check protocolbuffers.inc.php; \
	for file in $(MYTESTS); do \
		$(PHP) --syntax-check $${file}.php; \
	done ;