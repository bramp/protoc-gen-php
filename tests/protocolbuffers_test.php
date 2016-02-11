<?php
error_reporting(E_ALL);

require('phpunit_utils.php');

require('protocolbuffers.inc.php');

function stringToStream($str) {
	$fp = fopen('php://temp', 'w+b');
	fwrite($fp, $str);
	rewind($fp);
	return $fp;
}


abstract class ProtobufTestCase extends PHPUnit_Framework_TestCase {
	var $fp;

	public function setUp() {
		$this->fp = fopen('php://memory', 'r+b');
		$this->assertTrue($this->fp !== false, 'Unable to open stream');
    }

	public function tearDown() {
		fclose($this->fp);
    }


    // Reset the resource to these bytes
    protected function reset($bytes = '') {
		$this->assertTrue(ftruncate($this->fp, 0));
		$this->assertEquals(strlen($bytes), fwrite($this->fp, $bytes));
		$this->assertTrue(rewind($this->fp));
    }

    // Gets the bytes written to the buffer
    protected function get() {
    	$this->assertTrue(rewind($this->fp));
    	return stream_get_contents($this->fp);
    }
}

/*
class SignedTest extends PHPUnit_Framework_TestCase {
	var $tests = array(
		0 => 0,
		1 => 1,
		2147483647 => 2147483647,
		2147483648 => -2147483648,
		4294967296 => -1,
	);

	function testSignedExtension() {
		foreach ($this->tests as $a => $b) {
			echo $a . " " . $b . " ~ " . Protobuf::signed_extension($b) . "\n";
			$this->assertEquals($a, Protobuf::signed_extension($b), "Failed signed_extension");
		}
	}
}
*/

class VarintProtobufTest extends ProtobufTestCase {

	var $tests = array(
		//−9223372036854775808 => "\x80\x80\x80\x80\x80\x80\x80\x80\x80\x01" // -2^63
		//-2147483648 => "\x80\x80\x80\x80\xf8\xff\xff\xff\xff\x01", // -2^31
		//-16         => "\xf0\xff\xff\xff\xff\xff\xff\xff\xff\x01",
		//-8          => "\xf8\xff\xff\xff\xff\xff\xff\xff\xff\x01",
		//-2          => "\xfe\xff\xff\xff\xff\xff\xff\xff\xff\x01",
		//-1          => "\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01",
		0           => "\x00",
		1           => "\x01",
		2           => "\x02",
		127         => "\x7F",
		128         => "\x80\x01",
		300         => "\xAC\x02",
		1454260703  => "\xdf\x83\xb9\xb5\x05", // Previously reported broken: https://github.com/bramp/protoc-gen-php/pull/3
		
		// Below should work on 32bit PHP:
		2147483647  => "\xff\xff\xff\xff\x07", // 2^31-1

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
			if ($i < 0) // Skipped the signed tests
				continue;

			if (is_string($i))
				$i = (float)$i;

			$this->reset($enc);

			$a = Protobuf::read_varint($this->fp);
			$this->assertSame($i, $a, "Failed to decode varint($i)");
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
				// Skip doubles without exact reprentation, they will make the test
				// fail, and there is nothing we can do about it.
				if (number_format((float)$i, 0, '.', '') !== $i) {
					continue;
				}

				$i = (float)$i;
			}

			$this->reset();

			$len = Protobuf::write_varint($this->fp, $i);
			assertBinaryEqual($enc, $this->get(), "Failed to encode varint($i)");
		}
	}
/*

	function testSignedReadVarint() {
		foreach ($this->tests as $i => $enc) {
			if ($i >= 0) // Skipped the signed tests
				continue;

			$this->reset($enc);

			$a = Protobuf::read_varint($this->fp);
			$this->assertSame($i, $a, "Failed to decode varint($i) got $a");
		}
	}





*/
/*
	function testWriteVarint() {

		$fp = $this->fp;

		foreach ($tests as $i => $enc) {
			$this->assertTrue(ftruncate($fp, 0));

			$len = Protobuf::write_varint($fp, $i);
			$this->assertEquals($len, Protobuf::size_varint($i));

			// Read it back to check it's good
			$this->assertTrue(rewind($fp));
			$b = fread($fp, $len);
			$this->assertEquals($enc, $b, "Failed to encode varint($i)");

			// Now try to decode it.
			$this->assertTrue(rewind($fp));
			$a = Protobuf::read_varint($fp, $len);
			$this->assertEquals($i, $a, "Failed to decode varint($i) got $a");
		}
	}
	function testSignedVarint() {

		$fp = $this->fp;

		foreach ($tests as $i => $enc) {
			$this->assertTrue(ftruncate($fp, 0));

			$len = Protobuf::write_signed_varint($fp, $i);
			$this->assertEquals($len, Protobuf::size_varint($i));

			// Read it back to check it's good
			$this->assertTrue(rewind($fp));
			$b = fread($fp, $len);
			$this->assertEquals($enc, $b, "Failed to encode varint($i)");

			// Now try to decode it.
			$this->assertTrue(rewind($fp));
			$a = Protobuf::read_signed_varint($fp, $len);
			$this->assertEquals($i, $a, "Failed to decode varint($i) got $a");
		}
	}
*/
/*
	// Test reading and writing many varints
	// TODO Turn this into a benchmark
	function testManyVarint() {
		$fp = $this->fp;

		$max = pow(2, 10);
		for ($i = -$max; $i < $max; $i++) {
			$this->assertTrue(ftruncate($fp, 0));

			$len = Protobuf::write_varint($fp, $i);
			$this->assertGreaterThan(0, $len);

			fseek($fp, 0, SEEK_SET);
			$a = Protobuf::read_varint($fp, $len);
			$this->assertEquals($i, $a);
		}
	}
*/

/*
	function testBrokenVarint() {
		$tests = array("", "\x80", "\x80\x80");

		for ($tests as $test) {
			$fp = stringToStream($test);
			Protobuf::read_varint($fp);
			fclose($fp);
		}
	}
*/
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