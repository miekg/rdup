<?php
error_reporting(E_ALL);
require_once("web.php");
$web->header("config");

#$web->htpasswd();
$web->system_network();
$web->footer();
?>
