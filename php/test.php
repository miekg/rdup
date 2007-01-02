#!/usr/bin/php -q
<?php
error_reporting(E_ALL);

require_once("web.php");
require_once("rc.php");

$conf = new rc("rdup.rc");

print $conf->fifo;


?>
