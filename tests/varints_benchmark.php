<?php
// TODO
// * Change mt_rand to produce numbers in the full [0,2^64-1] range
//   "On both 32 and 64-bit systems (OS X and Linux), mt_getrandmax() returns 2147483647 for me, i.e. ~2^31."

require __DIR__ . '/vendor/autoload.php';

require '../protocolbuffers.inc.php';

define("TRIALS", 3);
define("LOOPS", 100000);

/**
 * Returns a random encoded varint using mt_rand
 * TODO Improve this to just generate a random string of varint looking bytes
 */
function mt_rand_varint() {
	return Protobuf::encode_varint_float(mt_rand());
}

/*
function run_return_avg(callable $callable, $args) {
	mt_srand(0);

	$results = array();
	for ($i = 0; $i < TRIALS; $i++) {
		$bench = new Ubench;
		$bench->run(function() {
			mt_rand_varint()
		});

		$results[] + $bench->getTime() . "\t";
	}
}
 */

// Execute a benchmark and writes out results
function run($name, callable $callable, $args = array()) {
	mt_srand(0);

	printf("%21s\t", $name);

	for ($i = 0; $i < TRIALS; $i++) {
		$bench = new Ubench;
		$bench->run($callable, $args);

		echo $bench->getTime() . "\t";
	}

	echo "\n";
}

function encode_tests() {

	run("encode_varint_int", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			Protobuf::encode_varint_int(mt_rand());
		}
	});

	run("encode_varint_slide", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			Protobuf::encode_varint_slide(mt_rand());
		}
	});

	run("encode_varint_float", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			Protobuf::encode_varint_float(mt_rand());
		}
	});

	run("encode_varint_bc", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			Protobuf::encode_varint_float(mt_rand());
		}
	});

	run("encode_varint_gmp", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			Protobuf::encode_varint_float(mt_rand());
		}
	});

	run("encode_varint", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			Protobuf::encode_varint(mt_rand());
		}
	});

	run("encode_neg_varint", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			Protobuf::encode_varint(-mt_rand());
		}
	});
}

function decode_tests() {

	run("decode_varint", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			Protobuf::decode_varint(mt_rand_varint());
		}
	});

	run("decode_varint_int", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			$v = mt_rand_varint();
			Protobuf::decode_varint_int($v, strlen($v));
		}
	});
	
	run("decode_varint_float", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			$v = mt_rand_varint();
			Protobuf::decode_varint_float($v, strlen($v));
		}
	});

	run("decode_varint_bc", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			$v = mt_rand_varint();
			Protobuf::decode_varint_bc($v, strlen($v));
		}
	});

	run("decode_varint_gmp", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			$v = mt_rand_varint();
			Protobuf::decode_varint_gmp($v, strlen($v));
		}
	});
}

function size_tests() {
	run("size_varint", function() {
		for ($i = 0; $i < LOOPS; $i++) {
			Protobuf::size_varint(mt_rand());
		}
	});
}

//encode_tests();
decode_tests();
//size_tests();

