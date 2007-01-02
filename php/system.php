<?php
error_reporting(E_ALL);
require_once("web.php");
$web->header("system");
$web->system_shutdown();
$web->system_network();
$web->footer();
?>
