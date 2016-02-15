<?php

function stringToStream($str) {
	// TODO Consider replacing with ProtobufIO::toStream
	$fp = fopen('php://temp', 'w+b');
	fwrite($fp, $str);
	rewind($fp);
	return $fp;
}


/**
 * Magic logger
 * If you want to trace function calls just prefix them with L::. For example:
 *   $fp = L::fopen("somefile");
 *   L::fread($fp, 10);
 * Will print out
 *   fopen("somefile") = Resource id #97
 *   fread(Resource id #97, 10) = "....."
 */
abstract class L {
	public static function __callStatic($name, $arguments) {
		echo "$name(" . implode(', ', $arguments). ") = ";
		$ret = forward_static_call_array($name, $arguments);
		echo print_r($ret, true) . "\n";
		return $ret;
	}
}

function print_bytes($bytes) {
	$len = strlen($bytes);
	for ($i = 0; $i < $len; $i++) {
		echo sprintf("%02X ", ord($bytes[$i]));
	}
	echo "\n";
}

function print_varint_binary($bytes) {
	$len = strlen($bytes);
	for ($i = 0; $i < $len; $i++) {
		$c = ord($bytes[$i]);
		if ($c >= 0x80) {
			echo sprintf("%07b ", $c & 0x7F);
		} else {
			echo sprintf("%07b ", $c);
		}
	}
	echo "\n";
}


/**
 * Tests if the number represented by this string has an exact float representation.
 *
 * @param string $value
 *
 * @return bool
 */
function hasExactRepresentation($value) {
	return number_format((float)$value, 0, '.', '') === $value;
}

class IsBinaryEqual extends PHPUnit_Framework_Constraint {
	protected $value;

	public function __construct($value) {
		parent::__construct();

		if (!is_string($value)) {
            throw PHPUnit_Util_InvalidArgumentHelper::factory(1, 'string');
        }

        $this->value = $value;
	}

	private static function toHexString($string) {
		$result = '';
		$len = strlen($string);
		for ($i = 0; $i < $len; $i++) {
			$c = ord($string[$i]);
			$result .= (($c < 16) ? '0' : '') . dechex($c);
		}
		return $result;
	}

	/**
	 *  @return bool
	 *  @throws PHPUnit_Framework_ExpectationFailedException
	 */
    public function evaluate($other, $description = '', $returnResult = false) {
			
		if ($this->value === $other) {
			return true;
		}

		if ($returnResult) {
            return false;
        }

        throw new PHPUnit_Framework_ExpectationFailedException(
            $description . "\n" . 
                "Expected: len:" . strlen($this->value) . " " . self::toHexString($this->value) . "\n" .
                "  Actual: len:" . strlen($this->value) . " " . self::toHexString($other)
        );
	}

	public function toString() {
		return 'is equal to <text>';
	}
}

abstract class ProtobufTestCase extends PHPUnit_Framework_TestCase {

	protected $fp;

	public function setUp() {
		$this->fp = fopen('php://memory', 'r+b');
		$this->assertTrue($this->fp !== false, 'Unable to open stream');
    }

	public function tearDown() {
		fclose($this->fp);
    }

	protected function assertBinaryEqual($expected, $actual, $message = '') {
		$constraint = new IsBinaryEqual($expected);
		PHPUnit_Framework_TestCase::assertThat($actual, $constraint, $message);
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
