#!/usr/bin/php -q
<?php
error_reporting(E_ALL);

require_once("web.php");

$a =  dir_list("/tmp", false);

print_r($a);

?>
