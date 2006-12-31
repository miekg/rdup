#!/usr/bin/php -q
<?php
error_reporting(E_ALL);

require_once("web.php");

$L = "NL";

$x = new i18n($L, "lang.txt");
$x->show("Configuration", "Configuratie");
$x->show("infopage", "infopagina");
#
echo $x->T("Hello") . "\n";
echo $x->T("Configuration") . "\n";
#

$w = new web($L, "lang.txt");
$w->header("infopage");



$w->footer();
echo date("D M d H:i:s T Y");

?>
