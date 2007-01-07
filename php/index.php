<?php
error_reporting(E_ALL);
require_once("web.php");
require_once("rc.php");

$conf = new rc("rdup.rc");
$web =  new web($conf->lang, "lang.txt");

$web->header("infopage");
$web->infopage_group1($conf->backup);
$web->infopage_dirs($conf->backup);
$web->footer();
?>
