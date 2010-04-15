<?php
	require('protocolbuffers.inc.php');
	require('addressbook.proto.php');

	$fp = fopen('test.book', 'rb');

	$m = new tutorial_AddressBook($fp);

	var_dump($m);

	fclose($fp);
?>
