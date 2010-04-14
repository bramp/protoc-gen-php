#CXX=g++
CXX=clang

all:
	$(CXX) -lprotobuf -lprotoc -pthread -Wall protoc-gen-php.cc -o protoc-gen-php

test: all
	protoc --php_out . --plugin=protoc-gen-php=./protoc-gen-php addressbook.proto
	cat test.php
