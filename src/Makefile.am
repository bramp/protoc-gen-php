AUTOMAKE_OPTIONS = foreign

MAKEFLAGS = --no-builtin-rules
SUFFIXES:

LDFLAGS   =
LIBS      = -lprotobuf -lprotoc

CLEANFILES = php_options.pb.cc php_options.pb.h

PROTO_INCLUDES=$(shell pkg-config protobuf --cflags-only-I)

bin_PROGRAMS = protoc-gen-php
protoc_gen_php_SOURCES = protoc-gen-php.cc php_generator.cc php_options.pb.cc strutil.cc strutil.h php_util.cc php_util.h
protoc_gen_php_SOURCES += php_enum.cc php_field.cc php_message.cc php_method.cc php_read.cc php_service.cc php_size.cc php_tostring.cc php_write.cc

protoc-gen-php.cc: php_options.pb.cc php_options.pb.h

%.pb.cc %.pb.h: %.proto
	$(PROTOC) --proto_path=$(srcdir) --cpp_out=$(builddir) $(PROTO_INCLUDES) $^

format: $(protoc_gen_php_SOURCES)
	clang-format -style=google -i $^