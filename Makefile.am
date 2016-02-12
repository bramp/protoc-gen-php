AUTOMAKE_OPTIONS = foreign
SUBDIRS = src reference_app

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

test:
	phpunit tests/*_test.php

bench:
	php tests/varints_benchmark.php

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