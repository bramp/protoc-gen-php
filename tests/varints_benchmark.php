<?php
require __DIR__ . '/vendor/autoload.php';

require 'protocolbuffers.inc.php';

define("TRIALS", 3);
define("LOOPS", 100000);


// Execute some code
// mt_getrandmax() 
// mt_srand
// 
function run($name, callable $callable, $args) {
	mt_srand(0);

	printf("%21s\t", $name);

	for ($i = 0; $i < TRIALS; $i++) {
		$bench = new Ubench;
		$bench->run($callable, $args);

		echo $bench->getTime() . "\t";
	}

	echo "\n";
}

run("encode_varint_int", function($loops) {
	for ($i = 0; $i < $loops; $i++) {
		Protobuf::encode_varint_int(mt_rand());
	}
}, LOOPS);

run("encode_varint_slide", function($loops) {
	for ($i = 0; $i < $loops; $i++) {
		Protobuf::encode_varint_slide(mt_rand());
	}
}, LOOPS);

run("encode_varint_float", function($loops) {
	for ($i = 0; $i < $loops; $i++) {
		Protobuf::encode_varint_float(mt_rand());
	}
}, LOOPS);

run("encode_varint", function($loops) {
	for ($i = 0; $i < $loops; $i++) {
		Protobuf::encode_varint(mt_rand());
	}
}, LOOPS);

run("encode_varint_neg", function($loops) {
	for ($i = 0; $i < $loops; $i++) {
		Protobuf::encode_varint(-mt_rand());
	}
}, LOOPS);