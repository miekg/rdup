<?php
error_reporting(E_ALL);
require_once("web.php");
$web = new web("NL", "lang.txt");
$web->header("config");

#$web->htpasswd();
$web->system_network();
$web->footer();
?>
