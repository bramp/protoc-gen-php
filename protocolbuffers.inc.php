<?php

	function skip_varint($fp) {
		do { // Keep reading until we find the last byte
			$b = fread($fp, 1);
			// TODO check for false
		} while ($b >= "\x80");
	}

	function read_varint($fp) {
		$value = '';
		$len = 0;
		do { // Keep reading until we find the last byte
			$b = fread($fp, 1);
			// TODO check for false
			$value .= $b;
			$len++;
		} while ($b >= "\x80");

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
			case 0:
				skip_varint($fp);
				break;
			case 1:
				fseek($fp, 8, SEEK_CUR);
				// TODO check return
				break;
			case 5:
				fseek($fp, 4, SEEK_CUR);
				// TODO check return
				break;
			default:
				throw new Exception("Unsupported wire_type($wire_type)");
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

		$data = "data://application/octet-stream,";
		foreach ($varint_tests as $i => $enc) {
			$data .= $enc;
		}
		$fp = fopen($data, 'rb');
		if ($fp === false)
			exit('Unable to open stream');

		foreach ($varint_tests as $i => $enc) {
			$a = read_varint($fp);
			if ($a != $i)
				exit("Failed to decode varint($i) got $a\n");
			echo "$i OK\n";
		}
		fclose($fp);
	}

	//test_varint();
?>
