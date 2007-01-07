<?php
error_reporting(E_ALL);
require_once("web.php");
require_once("rc.php");

$conf = new rc("rdup.rc");
$web =  new web($conf->lang, "lang.txt");

$web->header("system");
$web->system_shutdown();
$web->footer();
?>
