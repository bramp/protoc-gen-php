<?php

/**
 * Class to aid in the parsing and creating of Protocol Buffer Messages
 * This class should be included by the developer before they use a
 * generated protobuf class.
 *
 * @author Andrew Brampton
 *
 */
class Protobuf {

	const TYPE_DOUBLE   = 1;   // double, exactly eight bytes on the wire.
	const TYPE_FLOAT    = 2;   // float, exactly four bytes on the wire.
	const TYPE_INT64    = 3;   // int64, varint on the wire.  Negative numbers
                               // take 10 bytes.  Use TYPE_SINT64 if negative
                               // values are likely.
	const TYPE_UINT64   = 4;   // uint64, varint on the wire.
	const TYPE_INT32    = 5;   // int32, varint on the wire.  Negative numbers
                               // take 10 bytes.  Use TYPE_SINT32 if negative
                               // values are likely.
	const TYPE_FIXED64  = 6;   // uint64, exactly eight bytes on the wire.
	const TYPE_FIXED32  = 7;   // uint32, exactly four bytes on the wire.
	const TYPE_BOOL     = 8;   // bool, varint on the wire.
	const TYPE_STRING   = 9;   // UTF-8 text.
	const TYPE_GROUP    = 10;  // Tag-delimited message.  Deprecated.
	const TYPE_MESSAGE  = 11;  // Length-delimited message.

	const TYPE_BYTES    = 12;  // Arbitrary byte array.
	const TYPE_UINT32   = 13;  // uint32, varint on the wire
	const TYPE_ENUM     = 14;  // Enum, varint on the wire
	const TYPE_SFIXED32 = 15;  // int32, exactly four bytes on the wire
	const TYPE_SFIXED64 = 16;  // int64, exactly eight bytes on the wire
	const TYPE_SINT32   = 17;  // int32, ZigZag-encoded varint on the wire
	const TYPE_SINT64   = 18;  // int64, ZigZag-encoded varint on the wire

	/**
	 * Returns a string representing this wiretype
	 */
	public static function get_wiretype($wire_type) {
		switch ($wire_type) {
			case 0: return 'varint';
			case 1: return '64-bit';
			case 2: return 'length-delimited';
			case 3: return 'group start';
			case 4: return 'group end';
			case 5: return '32-bit';
			default: return 'unknown';
		}
	}

	public static function skip_varint($fp) {
		$len = 0;
		do { // Keep reading until we find the last byte
			$b = fread($fp, 1);
			if ($b === false)
				throw new Exception("skip(varint): Error reading byte");
			$len++;
		} while ($b >= "\x80");
		return $len;
	}

	/**
	 * Tries to read a varint from $fp.
	 * @returns the Varint from the stream, or false if the stream has reached eof.
	 */
	public static function read_varint($fp, &$limit = null) {
		$value = '';
		$len = 0;
		do { // Keep reading until we find the last byte
			$b = fread($fp, 1);
			if ($b === false)
				throw new Exception("read_varint(): Error reading byte");
			if (strlen($b) < 1)
				break;

			$value .= $b;
			$len++;
		} while ($b >= "\x80");

		if ($len == 0) {
			if (feof($fp))
				return false;
			throw new Exception("read_varint(): Error reading byte");
		}

		if ($limit !== null)
			$limit -= $len;
			
		$i = 0;
		$shift = 0;
		for ($j = 0; $j < $len; $j++) {
			$i |= ((ord($value[$j]) & 0x7F) << $shift);
			$shift += 7;
		}

		return $i;
	}

	public static function skip($fp, $wire_type) {
		switch ($wire_type) {
			case 0: // varint
				return Protobuf::skip_varint($fp);

			case 1: // 64bit
				if (fseek($fp, 8, SEEK_CUR) === -1)
					throw new Exception('skip(' . get_wiretype(1) . '): Error seeking');
				return 8;

			case 2: // length delimited
				$varlen = 0;
				$len = Protobuf::read_varint($fp, &$varlen);
				if (fseek($fp, $len, SEEK_CUR) === -1)
					throw new Exception('skip(' . get_wiretype(2) . '): Error seeking');
				return $len - $varlen;

			//case 3: // Start group TODO we must keep looping until we find the closing end grou

			//case 4: // End group - We should never skip a end group!
			//	return 0; // Do nothing

			case 5: // 32bit
				if (fseek($fp, 4, SEEK_CUR) === -1)
					throw new Exception('skip('. get_wiretype(5) . '): Error seeking');
				return 4;

			default:
				throw new Exception('skip('. get_wiretype($wire_type) . '): Unsupported wire_type');
		}
	}

	public static function read_unknown($fp, $wire_type, &$limit = null) {
		switch ($wire_type) {
			case 0: // varint
				return Protobuf::read_varint($fp, $limit);

			case 1: // 64bit
				$limit -= 8;
				return fread($fp, 8);

			case 2: // length delimited
				$len = Protobuf::read_varint($fp, $limit);
				$limit -= $len;
				return fread($fp, $len);

			//case 3: // Start group TODO we must keep looping until we find the closing end grou

			//case 4: // End group - We should never skip a end group!
			//	return 0; // Do nothing

			case 5: // 32bit
				$limit -= 4;
				return fread($fp, 4);

			default:
				throw new Exception('read_unknown('. get_wiretype($wire_type) . '): Unsupported wire_type');
		}
	}


	/**
	 * Used to aid in pretty printing of Protobuf objects
	 */
	private static $print_depth = 0;
	private static $indent_char = "\t";
	private static $print_limit = 50;

	public static function toString($key, $value) {
		if (is_null($value))
			return;
		$ret = str_repeat(self::$indent_char, self::$print_depth) . "$key=>";
		if (is_array($value)) {
			$ret .= "array(\n";
			self::$print_depth++;
			foreach($value as $i => $v)
				$ret .= self::toString("[$i]", $v);
			self::$print_depth--;
			$ret .= str_repeat(self::$indent_char, self::$print_depth) . ")\n";
		} else {
			if (is_object($value)) {
				self::$print_depth++;
				$ret .= get_class($value) . "(\n";
				$ret .= $value->__toString() . "\n";
				self::$print_depth--;
				$ret .= str_repeat(self::$indent_char, self::$print_depth) . ")\n";
			} elseif (is_string($value)) {
				$safevalue = addcslashes($value, "\0..\37\177..\377");
				if (strlen($safevalue) > self::$print_limit) {
					$safevalue = substr($safevalue, 0, self::$print_limit) . '...';
				}

				$ret .= '"' . $safevalue . '" (' . strlen($value) . " bytes)\n";
				
			} elseif (is_bool($value)) {
				$ret .= ($value ? 'true' : 'false') . "\n";
			} else {
				$ret .= (string)$value . "\n";
			}
		}
		return $ret;
	}
}

$varint_tests  = array(
	1   => "\x01",
	2   => "\x02",
	127 => "\x7F",
	128 => "\x80\x01",
	300 => "\xAC\x02",
);

function test_varint() {
	global $varint_tests;

	$fp = fopen('php://memory', 'r+b');
	if ($fp === false)
		exit('Unable to open stream');

	foreach ($varint_tests as $i => $enc) {

		// Write the answer into the buffer
		fwrite($fp, $enc);
		fseek($fp, 0, SEEK_SET);

		$a = Protobuf::read_varint($fp);
		if ($a != $i)
			exit("Failed to decode varint($i) got $a\n");

		$len = Protobuf::write_varint($fp, $i);
		fseek($fp, 0, SEEK_SET);

		echo "$i OK\n";
	}
	fclose($fp);
}

//	test_varint();
?>
