<?php
error_reporting(E_ALL);
require_once("web.php");
$web = new web("NL", "lang.txt");
$web->header("system");
$web->system_shutdown();
$web->footer();
?>
