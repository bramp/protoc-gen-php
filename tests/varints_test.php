<?php
error_reporting(E_ALL);

require('phpunit_base.php');
require('protocolbuffers.inc.php');

class VarintProtobufTest extends ProtobufTestCase {

	var $tests = array(
		-9223372036854775808 => "\x80\x80\x80\x80\x80\x80\x80\x80\x80\x01", // -2^63
		-9223372036854775807 => "\x81\x80\x80\x80\x80\x80\x80\x80\x80\x01", // -2^63 + 1
		-9223372036854775800 => "\x88\x80\x80\x80\x80\x80\x80\x80\x80\x01", // -2^63 + 8

		-72057594037927937   => "\xff\xff\xff\xff\xff\xff\xff\xff\xfe\x01", // -2^56 - 1
		-72057594037927936   => "\x80\x80\x80\x80\x80\x80\x80\x80\xff\x01", // -2^56
		-72057594037927935   => "\x81\x80\x80\x80\x80\x80\x80\x80\xff\x01", // -2^56 + 1

		-2147483648          => "\x80\x80\x80\x80\xf8\xff\xff\xff\xff\x01", // -2^31
		-255                 => "\x81\xfe\xff\xff\xff\xff\xff\xff\xff\x01",
		-16                  => "\xf0\xff\xff\xff\xff\xff\xff\xff\xff\x01",
		-8                   => "\xf8\xff\xff\xff\xff\xff\xff\xff\xff\x01",
		-3                   => "\xfd\xff\xff\xff\xff\xff\xff\xff\xff\x01",
		-2                   => "\xfe\xff\xff\xff\xff\xff\xff\xff\xff\x01",
		-1                   => "\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01",
		0                    => "\x00",
		1                    => "\x01",
		2                    => "\x02",
		3                    => "\x03",
		127                  => "\x7F",
		128                  => "\x80\x01",
		255                  => "\xff\x01",
		300                  => "\xac\x02",
		1454260703           => "\xdf\x83\xb9\xb5\x05", // Previously reported broken: https://github.com/bramp/protoc-gen-php/pull/3

		// Below should work on 32bit PHP:
		2147483647             => "\xff\xff\xff\xff\x07", // 2^31-1

		// Below should work on 64bit PHP:
		2147483648             => "\x80\x80\x80\x80\x08", // 2^31

		// Below begins to lose precision as floats
		72057594037927935      => "\xff\xff\xff\xff\xff\xff\xff\x7f", // 2^56 - 1
		72057594037927936      => "\x80\x80\x80\x80\x80\x80\x80\x80\x01", // 2^56
		72057594037927937      => "\x81\x80\x80\x80\x80\x80\x80\x80\x01", // 2^56 + 1

		9223372036854775807    => "\xff\xff\xff\xff\xff\xff\xff\xff\x7f", // 2^63 - 1

		// The following will floats (stored as strings, since PHP associated arrays are keyed by int or string only):
		"9223372036854775808"  => "\x80\x80\x80\x80\x80\x80\x80\x80\x80\x01", // 2^63
		"18446744073709551615" => "\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01", // 2^64 - 1
	);


	function testReadVarint() {
		foreach ($this->tests as $i => $enc) {
			if (is_string($i))
				$i = (float)$i;

			$this->reset($enc);

			if ($i >= 0) {
				$a = Protobuf::read_varint($this->fp);
			} else {
				$a = Protobuf::read_signed_varint($this->fp);
			}
			$this->assertSame($i, $a, "Failed to read_varint($i)");
		}
	}

	function testSizeVarint() {
		foreach ($this->tests as $i => $enc) {
			if (is_string($i))
				$i = (float)$i;

			$len = Protobuf::size_varint($i);
			$this->assertSame(strlen($enc), $len, "Incorrect size_varint($i)");
		}
	}

	function testWriteVarint() {
		foreach ($this->tests as $i => $enc) {
			if (is_string($i)) {
				// Skip floats without exact reprentation, they will make the test
				// fail, and there is nothing we can do about it.
				if (!hasExactRepresentation($i)) {
					continue;
				}

				$i = (float)$i;
			}

			$this->reset();

			$len = Protobuf::write_varint($this->fp, $i);
			$this->assertBinaryEqual($enc, $this->get(), "Failed to write_varint($i)");
		}
	}

	function testEncodeVarintInt() {
		foreach ($this->tests as $i => $enc) {
			if (!is_int($i) || $i < 0) {
				continue;
			}

			// Only test positive ints
			$result = Protobuf::encode_varint_int($i);
			$this->assertBinaryEqual($enc, $result, "Failed to encode_varint_int($i)");
		}
	}

	function testEncodeVarintIntSlide() {
		foreach ($this->tests as $i => $enc) {
			if (!is_int($i) || $i < 0) {
				continue;
			}

			// Only test positive ints
			$result = Protobuf::encode_varint_slide($i);
			$this->assertBinaryEqual($enc, $result, "Failed to encode_varint_slide($i)");
		}
	}

	function testEncodeVarintFloat() {
		foreach ($this->tests as $i => $enc) {
			if (!hasExactRepresentation($i) || $i < 0) {
				continue;
			}

			if (is_string($i)) {
				$i = (float)$i;
			}

			$result = Protobuf::encode_varint_float($i);
			$this->assertBinaryEqual($enc, $result, "Failed to encode_varint_float($i)");
		}
	}
}


/*
	if ($argc > 1) {
		$test = $argv[1];
		require("$test.php");

		if ($test == 'addressbook.proto') {
			$fp = fopen('test.book', 'rb');

			$m = new tutorial_AddressBook($fp);

			var_dump($m);

			fclose($fp);

		} else if ($test == 'market.proto') {
			//$fp = fopen('market2-in-1.dec', 'rb');
			$fp = fopen('market2-in-2.dec', 'rb');
			//$fp = fopen('temp', 'rb');

			$m = new Response($fp);

			echo $m;

			//$mem = fopen('php://memory', 'wb');
			$mem = fopen('temp', 'wb');
			if ($mem === false)
				exit('Unable to open output stream');

			$s = fstat($fp);
			echo 'File size: ' . $s['size'] . "\n";
			echo 'Guested size: ' . $m->size() . "\n";
			$m->write($mem);
			echo 'Write size: ' . ftell($mem) . "\n";

			fclose($mem);
			fclose($fp);
		}
	}
*/