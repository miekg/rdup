#!/usr/bin/php -q
<?php
error_reporting(E_ALL);
require_once("web.php");

$L = "NL";
$x = new i18n($L, "lang.txt");

$x->show($argv[1], $argv[2]);
?>
