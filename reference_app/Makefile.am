AUTOMAKE_OPTIONS = foreign

MAKEFLAGS = --no-builtin-rules
SUFFIXES:

LDFLAGS   =
LIBS      = -lprotobuf

PROTO_INCLUDES=$(shell pkg-config protobuf --cflags-only-I)

bin_PROGRAMS = reference
reference_SOURCES = reference.cc conformance.pb.cc conformance.pb.h 

%.pb.cc %.pb.h: %.proto
	$(PROTOC) --proto_path=$(srcdir) --cpp_out=$(builddir) $(PROTO_INCLUDES) $^
# protoc --cpp_out . $<;
