<?php

function checkArgument($exp, $message) {
	if (!$exp) {
    	throw new InvalidArgumentException($message);
	}
}

class ProtobufEnum {

	public static function toString($value) {
		checkArgument(is_int($value), 'value must be a integer');

		if (array_key_exists($value, self::$_values))
			return self::$_values[$value];

		return 'UNKNOWN';
	}
}

class ProtobufMessage {
	private $_unknown; // TODO Should this be here?

	function __construct($fp = NULL, &$limit = PHP_INT_MAX) {
		// Zero arguments, so construct a empty one
		if($fp === NULL) {
			return;
		}

		// If the input is a string, turn it into a stream and decode it
		if (is_string($fp)) {
			$str = $fp;
			$limit = min($limit, strlen($str));
			$fp = fopen('php://temp', 'w+b');
			fwrite($fp, $str, $limit);
			rewind($fp);

		} else {
			checkArgument(get_resource_type($fp) === 'stream', 'fp must be a string or file resource');
		}

		// Decode
		$this->read($fp, $limit);

		// If we opened the stream, then close it
		if (isset($str))
			fclose($fp);
	}

	// Reads the protobuf
	public function read($fp, &$limit = PHP_INT_MAX) {
		while(!feof($fp) && $limit > 0) {
			$tag = Protobuf::read_varint($fp, $limit);
			if ($tag === false)
				break;
			$wire  = $tag & 0x07;
			$field = $tag >> 3;

			$field_idx = $field . '-' . Protobuf::get_wiretype($wire);
			$this->_unknown[$field_idx][] = Protobuf::read_field($fp, $wire, $limit);
		}
	}

	// TODO Rename this
	public function toProtobuf() {
		$fp = fopen('php://temp', 'w+b');
		$this->write($fp);
		rewind($fp);
		return stream_get_contents($fp);
	}
}

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
	 * // TODO Rename to something enum related
	 */
	public static function get_wiretype($wire_type) {
		checkArgument(is_int($wire_type), "wire_type must be a integer");

		switch ($wire_type) {
			case 0: return '(0) varint';
			case 1: return '(1) 64-bit';
			case 2: return '(2) length-delimited';
			case 3: return '(3) group start';
			case 4: return '(4) group end';
			case 5: return '(5) 32-bit';
			default: return 'unknown';
		}
	}


	/**
	 * Returns how big (in bytes) this number would be as a varint
	 */
	public static function size_varint($value) {
		checkArgument(is_int($value), "value must be a integer");

/*		$len = 0;
		do {
			$i = $i >> 7;
			$len++;
		} while ($i != 0);
		return $len;
*/
		// TODO Rearrange to make a binary search
		if ($value < 0x80)
			return 1;
		if ($value < 0x4000)
			return 2;
		if ($value < 0x200000)
			return 3;
		if ($value < 0x10000000)
			return 4;
		if ($value < 0x800000000)
			return 5;
		if ($value < 0x40000000000)
			return 6;
		if ($value < 0x2000000000000)
			return 7;
		if ($value < 0x100000000000000)
			return 8;
		if ($value < 0x8000000000000000)
			return 9;
	}

	/**
	 * Parses the raw bytes of a varint, and returns the number it represents
	 * @param value the raw bytes int to decode
	 * @returns the decoded int
	 */
	public static function decode_varint($encoded) {
		checkArgument(is_string($encoded), "encoded value must be a string of bytes");
		checkArgument($encoded !== '', "encoded value contains no bytes");

		$len = strlen($encoded);

		$result = 0;
		$shift = 0;
		for ($i = 0; $i < $len; $i++) {
			$result |= ((ord($encoded[$i]) & 0x7F) << $shift);
			$shift += 7;
		}

		return $result;
	}

	public static function read_bytes($fp, $len, &$limit = PHP_INT_MAX) {
		checkArgument(get_resource_type($fp) === 'stream', "fp must be a file resource");
		checkArgument(is_integer($len) && $len >= 0, 'len must be a postitive integer');
		checkArgument(is_integer($limit) && $limit >= 0, 'limit must be a postitive integer');

		if ($limit < $len) {
			throw new Exception("read_bytes(): Unexpected end of stream");
		}

		$bytes = fread($fp, $len);
		if ($bytes === '' && feof($fp)) {
			return false;
		}

		if ($bytes === false) {
			throw new Exception('read_bytes(): Error reading byte');
		}

		if (strlen($bytes) !== $len) {
			throw new Exception("read_bytes(): Unexpected end of stream");
		}

		$limit -= $len;
		return $bytes;
	}

	/**
	 * Tries to read a varint from $fp.
	 * @returns the decoded varint from the stream, or false if the stream has reached eof.
	 * @throws exception if stream error occurs
	 */
	public static function read_varint($fp, &$limit = PHP_INT_MAX) {
		$value = '';
		do { // Keep reading until we find the last byte
			$b = Protobuf::read_bytes($fp, 1, $limit);
			if ($b === false) {
				return false;
			}

			$value .= $b;

		} while ($b >= "\x80");

		return Protobuf::decode_varint($value);
	}

	/**
	 * @returns a integer as specified by the pack variable.
	 * On EOF false is returned, otherwise Exception is thrown on stream error or invalid argument.
	 */
	protected static function read_unpack($fp, $len, $pack, &$limit = PHP_INT_MAX) {
		checkArgument(is_string($pack), 'pack must be a valid string');

		$bytes = Protobuf::read_bytes($fp, $len, $limit);
		if ($bytes === false) {
			return false;
		}

		return unpack($pack, $bytes)[1];
	}

	public static function read_float ($fp, &$limit = PHP_INT_MAX) { return Protobuf::read_unpack($fp, 4, 'f', $limit); } // BUG We need to convert from little order to machine order first
	public static function read_double($fp, &$limit = PHP_INT_MAX) { return Protobuf::read_unpack($fp, 8, 'd', $limit); } // BUG We need to convert from little order to machine order first
	public static function read_int32 ($fp, &$limit = PHP_INT_MAX) { return Protobuf::read_unpack($fp, 4, 'l', $limit); } // BUG We need to convert from little order to machine order first
	public static function read_uint32($fp, &$limit = PHP_INT_MAX) { return Protobuf::read_unpack($fp, 4, 'V', $limit); }

	 // unpack 'q' and 'P' are only supported since PHP 5.6.3
	public static function read_int64 ($fp, &$limit = PHP_INT_MAX) { return Protobuf::read_unpack($fp, 8, 'q', $limit); } // BUG We need to convert from little order to machine order first
	public static function read_uint64($fp, &$limit = PHP_INT_MAX) { return Protobuf::read_unpack($fp, 8, 'P', $limit); }

	public static function read_zint32($fp, &$limit = PHP_INT_MAX) {
		$i = Protobuf::read_varint($fp, $limit);
		if ($i === false) {
			return false;
		}

		return (!($i & 0x1) ? ($i >> 1) : (($i >> 1) ^ (~0)));
	}

	public static function read_zint64($fp, &$limit = PHP_INT_MAX) {
		$i = Protobuf::read_varint($fp, $limit);
		if ($i === false) {
			return false;
		}

		return (!($i & 0x1) ? ($i >> 1) : (($i >> 1) ^ (~0)));
	}

	/**
	 * Read a unknown field from the stream and decodes the value if possible.
	 */
	public static function read_field($fp, $wire_type, &$limit = PHP_INT_MAX) {
		checkArgument(get_resource_type($fp) === 'stream', 'fp must be a file resource');
		checkArgument(is_integer($limit) && $limit >= 0, 'limit must be a postitive integer');

		switch ($wire_type) {
			case 0: // varint
				return Protobuf::read_varint($fp, $limit); // Decoded value

			case 1: // 64bit
				return Protobuf::read_bytes($fp, 8, $limit);

			case 2: // length delimited
				$len = Protobuf::read_varint($fp, $limit);
				if ($len <= 0) {
					throw new Exception('read_field('. ProtoBuf::get_wiretype($wire_type) . "): Invalid length: $len it must be >= 0");
				}
				return Protobuf::read_bytes($fp, $len, $limit);

			//case 3: // Start group TODO we must keep looping until we find the closing end grou
			//case 4: // End group - We should never skip a end group!
			//	return 0; // Do nothing

			case 5: // 32bit
				return Protobuf::read_bytes($fp, 4, $limit);

			default:
				throw new Exception('read_field('. ProtoBuf::get_wiretype($wire_type) . '): Unsupported wire_type');
		}
	}

	/**
	 * Encodes a int
	 * @param $value The int to encode
	 * @returns the bytes of the encoded int
	 */
	public static function encode_varint($value) {
		checkArgument(is_int($value), 'value must be a integer');

		$buf = '';
		do {
			$encoded = $value & 0x7F;
			$value = $value >> 7;

			if ($value != 0)
				$encoded |= 0x80;

			$buf .= chr($encoded);
		} while ($value != 0);

		return $buf;
	}

	/**
	 * Writes a varint to $fp
	 * returns the number of bytes written
	 * @param $fp
	 * @param $value The int to encode and write
	 * @return The number of bytes written
	 */
	public static function write_varint($fp, $value) {
		checkArgument(get_resource_type($fp) === 'stream', 'fp must be a file resource');
		// TODO Check it was int/double/float/etc checkArgument(is_int($value), "value must be a integer");

		$buf = Protobuf::encode_varint($value);
		$len = strlen($buf);

		if (fwrite($fp, $buf) !== $len)
			throw new Exception('write_varint(): Error writing byte');

		return $len;
	}

	/**
	 * Writes a packed value to $fp
	 * returns the number of bytes written
	 * @param $fp
	 * @param $value The int to encode and write
	 * @param $pack The pack format
	 * @return The number of bytes written
	 */
	protected static function write_pack($fp, $value, $pack) {
		checkArgument(get_resource_type($fp) === 'stream', 'fp must be a file resource');
		checkArgument(is_integer($value) || is_float($value), 'value must be numeric');
		checkArgument(is_string($pack), 'pack must be a valid string');

		$encoded = pack($pack, $value);
		$len = strlen($encoded);

		$wrote = fwrite($fp, $encoded, $len);
		if ($wrote === false || $wrote !== $len) {
			throw new Exception('write_pack(): Error writing byte');
		}

		return $len;
	}

	public static function write_float ($fp, $f) { return Protobuf::write_pack($fp, $f, 'f'); } // BUG We need to convert from machine order to little order first
	public static function write_double($fp, $d) { return Protobuf::write_pack($fp, $d, 'd'); } // BUG We need to convert from machine order to little order first
	public static function write_int32 ($fp, $i) { return Protobuf::write_pack($fp, $i, 'l'); } // BUG We need to convert from machine order to little order first
	public static function write_uint32($fp, $i) { return Protobuf::write_pack($fp, $i, 'V'); }

	 // unpack 'q' and 'P' are only supported since PHP 5.6.3
	public static function write_int64 ($fp, $i) { return Protobuf::write_pack($fp, $i, 'q'); } // BUG We need to convert from machine order to little order first
	public static function write_uint64($fp, $i) { return Protobuf::write_pack($fp, $i, 'P'); }

	public static function write_zint32($fp, $i) { return Protobuf::write_varint($fp, ($i>= 0 ? ($i << 1) : (($i << 1) ^ ~0))); }
	public static function write_zint64($fp, $i) { return Protobuf::write_varint($fp, ($i>= 0 ? ($i << 1) : (($i << 1) ^ ~0))); }

	/**
	 * Seek past a varint
	 */
	public static function skip_varint($fp) {
		checkArgument(get_resource_type($fp) === 'stream', 'fp must be a file resource');

		$len = 0;
		do { // Keep reading until we find the last byte
			$b = fread($fp, 1);
			if ($b === false)
				throw new Exception('skip(varint): Error reading byte');
			$len++;
		} while ($b >= "\x80");

		return $len;
	}

	/**
	 * Seek past the current field
	 * TODO Rewrite this to be safer
	 */
	public static function skip_field($fp, $wire_type) {
		checkArgument(get_resource_type($fp) === 'stream', 'fp must be a file resource');
		checkArgument(is_int($wire_type), 'wire_type must be a integer');

		switch ($wire_type) {
			case 0: // varint
				return Protobuf::skip_varint($fp);

			case 1: // 64bit
				if (fseek($fp, 8, SEEK_CUR) === -1)
					throw new Exception('skip(' . ProtoBuf::get_wiretype(1) . '): Error seeking');
				return 8;

			case 2: // length delimited
				$varlen = 0;
				$len = Protobuf::read_varint($fp, $varlen);
				if (fseek($fp, $len, SEEK_CUR) === -1)
					throw new Exception('skip(' . ProtoBuf::get_wiretype(2) . '): Error seeking');
				return $len - $varlen;

			//case 3: // Start group TODO we must keep looping until we find the closing end grou
			//case 4: // End group - We should never skip a end group!
			//	return 0; // Do nothing

			case 5: // 32bit
				if (fseek($fp, 4, SEEK_CUR) === -1)
					throw new Exception('skip('. ProtoBuf::get_wiretype(5) . '): Error seeking');
				return 4;

			default:
				throw new Exception('skip('. ProtoBuf::get_wiretype($wire_type) . '): Unsupported wire_type');
		}
	}

	/**
	 * TODO Decide to delete toString(). It used when printing unknown fields
	 * Used to aid in pretty printing of Protobuf objects
	 * TODO Decode enums
	 */
	private static $print_depth = 0;
	private static $indent_char = "\t";
	private static $print_limit = 50;

	public static function toString($key, $value, $default) {
		if ($value === $default)
			return;

		$ret = str_repeat(self::$indent_char, self::$print_depth) . "$key=>";
		if (is_array($value)) {
			$ret .= "array(\n";
			self::$print_depth++;
			foreach($value as $i => $v)
				$ret .= self::toString("[$i]", $v);
			self::$print_depth--;
			$ret .= str_repeat(self::$indent_char, self::$print_depth) . ")\n";

		} elseif (is_object($value)) {
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
		return $ret;
	}
}
