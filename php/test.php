#!/usr/bin/php -q
<?php
error_reporting(E_ALL);

require_once("i18n.php");

$L = "NL";

$x = new i18n($L, "lang.txt");
$x->show("Configuration", "Configuratie");

echo $x->_T("Hello") . "\n";
echo $x->_T("Configuration") . "\n";

?>
