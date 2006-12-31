<?php
error_reporting(E_ALL);
require_once("web.php");
$L = "NL";
$w = new web($L, "lang.txt");
$w->header("system");

$w->system_network();
$w->system_shutdown();
$w->footer();
?>
