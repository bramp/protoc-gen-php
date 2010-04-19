<?php

	/**
	 * Returns a string representing this wiretype
	 */
	function get_wiretype($wire_type) {
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

	function skip_varint($fp) {
		$len = 0;
		do { // Keep reading until we find the last byte
			$b = fread($fp, 1);
			if ($b === false)
				throw new Exception("skip(varint): Error reading byte");
			$len++;
		} while ($b >= "\x80");
		return $len;
	}

	function read_varint($fp, &$limit = null) {
		$value = '';
		$len = 0;
		do { // Keep reading until we find the last byte
			$b = fread($fp, 1);
			if ($b === false)
				throw new Exception("read_varint(): Error reading byte");

			$value .= $b;
			$len++;
		} while ($b >= "\x80");

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

	function skip($fp, $wire_type) {
		switch ($wire_type) {
			case 0: // varint
				return skip_varint($fp);

			case 1: // 64bit
				if (fseek($fp, 8, SEEK_CUR) === -1)
					throw new Exception('skip(' . get_wiretype(1) . '): Error seeking');
				return 8;

			case 2: // length delimited
				$varlen = 0;
				$len = read_varint($fp, &$varlen);
				if (fseek($fp, $len, SEEK_CUR) === -1)
					throw new Exception('skip(' . get_wiretype(2) . '): Error seeking');
				return $len - $varlen;

			//case 3: // Start group TODO we must keep looping until we find the closing end grou

			case 4: // End group
				return 0; // Do nothing

			case 5: // 32bit
				if (fseek($fp, 4, SEEK_CUR) === -1)
					throw new Exception('skip('. get_wiretype(5) . '): Error seeking');
				return 4;

			default:
				throw new Exception('skip('. get_wiretype($wire_type) . '): Unsupported wire_type');
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

			$a = read_varint($fp);
			if ($a != $i)
				exit("Failed to decode varint($i) got $a\n");

			$len = write_varint($fp, $i);
			fseek($fp, 0, SEEK_SET);

			echo "$i OK\n";
		}
		fclose($fp);
	}

//	test_varint();
?>
