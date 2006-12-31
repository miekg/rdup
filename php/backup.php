<?php
error_reporting(E_ALL);
require_once("web.php");
$L = "NL";
$w = new web($L, "lang.txt");
$w->header("system");

$w->backup_start();
$w->backup_freespace();
$w->footer();
?>
