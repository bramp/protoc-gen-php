PHP Protocol Buffer Generator
=============================

This is a plugin for Google's Protocol Buffer Generator, protoc. It generates PHP code from a .proto file.
by Andrew Brampton ([bramp.net](http://bramp.net)) (c) 2010,2016

[![Minimum PHP Version](https://img.shields.io/badge/php-%3E%3D%205.5-8892BF.svg?style=flat-square)](https://php.net/) [![Build Status](https://img.shields.io/travis/bramp/protoc-gen-php/master.svg?style=flat-square)](https://travis-ci.org/bramp/protoc-gen-php)

Supports:
 * Proto2 and Proto3
 * All features (Messages, Enums, Oneof, Maps)
 * Designed to be quick
 * Passes all compliance tests

Future:
 * JSON encoding
 * Support optimize_for=CODE_SIZE


Use
---

Once compiled and installed you can use it via protoc like so:

```
protoc --php_out=. your.proto
```

This should generate the file "your.proto.php", which should be able to encode and decode protocol buffer messages. When using the generated PHP code you must include the "protocolbuffers.inc.php" file.


Notes on Numbers
-----
PHP uses signed integers, and depending on the platform may be 32 bit or 64 bit in size. Additionally PHP is typically compiled with IEEE 754 double precision floating point numbers, but may be compiled with single precision. This causes multiple problems for PHP using large numbers. As such the best appropimation is used when an exact representation is not available. On 32 bit platforms, signed integers are correct between -2^31 and 2^31-1, outside of that range floats are used. Double precision floats maintain integer accurancy up to 2^53, beyond that integers may not be precisce.


| Proto type       | PHP type (32 bit)                              | PHP type (64 bit)
| --------------------------------------------------------------------------------------
| bool             | bool                                           | bool
| float / double   | float                                          | float
| int32            | int                                            | int
| int64 / uint32   | int (between -2^31 and 2^31-1) otherwise float | int
| uint64           | int (between 0 and 2^31-1) otherwise float     | int (between 0 and 2^63-1) otherwise float
| byte / string    | string                                         | string
| enum             | int                                            | int



Install
-------

This has only been tested on Linux and Mac. You will need the following libraries:

```
# For Debian based Linux
sudo apt-get install libprotobuf-dev protobuf-compiler

# For Mac
brew install protobuf
```

To build:
```
./autogen.sh
./configure
make
sudo make install
```


Licence (Simplified BSD License)
--------------------------------
Copyright (c) 2010-2013, Andrew Brampton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

