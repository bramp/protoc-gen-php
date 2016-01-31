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
LIBS      = -lprotobuf

PROTO_INCLUDES=$(shell pkg-config protobuf --cflags-only-I)

bin_PROGRAMS = reference
reference_SOURCES = reference.cc conformance.pb.cc conformance.pb.h 

%.pb.cc %.pb.h: %.proto
	$(PROTOC) --proto_path=$(srcdir) --cpp_out=$(builddir) $(PROTO_INCLUDES) $^
# protoc --cpp_out . $<;
