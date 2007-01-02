<?php
error_reporting(E_ALL);
require_once("web.php");
require_once("rc.php");

global $conf;
$conf = new rc("rdup.rc");

$web->header("infopage");
$web->infopage_info($conf->backup);
$web->infopage_backup();
print "<hr/>";
$web->infopage_dirs($conf->backup);
$web->footer();
?>
