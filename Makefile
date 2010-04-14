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

debug: all
	gdb --args protoc --php_out . --plugin=protoc-gen-php=./protoc-gen-php addressbook.proto
	cat test.php

test: all
	protoc --php_out . --plugin=protoc-gen-php=./protoc-gen-php addressbook.proto
	cat test.php
