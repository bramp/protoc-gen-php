#CXX=g++
CXX=/home/bramp/llvm/Debug/bin/clang

CXXFLAGS = -Wall -g

INCLUDES =
LFLAGS   =
LIBS     = -lprotobuf -lprotoc -pthread

SRCS = protoc-gen-php.cc strutil.cc
OBJS = $(SRCS:.cc=.o)

MAIN = protoc-gen-php

.PHONY: depend clean

all:    $(MAIN)
$(MAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.cc.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

valgrind: DEBUGCMD=gdb --args
debug: all
	gdb --args protoc --php_out . --plugin=protoc-gen-php=./protoc-gen-php addressbook.proto

valgrind: DEBUGCMD=valgrind --trace-children=yes --leak-check=full
valgrind: all test

TESTS = addressbook.proto market.proto

%.proto.php : %.proto $(MAIN)
	$(DEBUGCMD) protoc --php_out . --plugin=protoc-gen-php=./protoc-gen-php $<;

test: $(TESTS:.proto=.proto.php)
	for file in $(TESTS); do \
		echo | cat -n $${file}.php -; \
		php --syntax-check $${file}.php; \
		php test.php $${file}; \
	done ;
