<?php
error_reporting(E_ALL);
require_once("web.php");
require_once("rc.php");

$conf = new rc("rdup.rc");
$web =  new web("NL", "lang.txt");

$web->header("infopage");
$web->infopage_info($conf->backup);
$web->infopage_backup();
print "<hr/>";
$web->infopage_dirs($conf->backup);
$web->footer();
?>
