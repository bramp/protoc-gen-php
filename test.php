<?php
	require('protocolbuffers.inc.php');

	$test = $argv[1];
	require("$test.php");

	if ($test == 'addressbook.proto') {
		$fp = fopen('test.book', 'rb');

		$m = new tutorial_AddressBook($fp);

		var_dump($m);

		fclose($fp);
	}
?>
