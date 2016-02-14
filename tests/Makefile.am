AUTOMAKE_OPTIONS = foreign

MAKEFLAGS = --no-builtin-rules
SUFFIXES:

.PHONY = test bench

PHPUNIT=vendor/bin/phpunit

all: test

composer.lock: composer.json 
vendor: composer.lock
	$(COMPOSER) install

test: vendor *_test.php
	$(PHPUNIT) *_test.php

bench: vendor varints_benchmark.php
	$(PHP) varints_benchmark.php
