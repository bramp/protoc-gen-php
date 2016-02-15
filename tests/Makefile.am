.PHONY = all test bench

AUTOMAKE_OPTIONS = foreign

MAKEFLAGS = --no-builtin-rules
SUFFIXES:

PHPUNIT=vendor/bin/phpunit

all: test

composer.lock: composer.json
	$(COMPOSER) update
vendor: composer.lock
	$(COMPOSER) install

test: vendor *_test.php
	$(PHP_OR_HHVM) $(PHPUNIT) --verbose *_test.php

bench: vendor varints_benchmark.php
	$(PHP_OR_HHVM) varints_benchmark.php
