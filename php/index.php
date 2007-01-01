<?php
error_reporting(E_ALL);
require_once("web.php");
require_once("rc.php");
/* setup language */
$L = "NL";
$w = new web($L, "lang.txt");
$conf = new rc("rdup.rc");

$w->header("infopage");
$w->infopage_info($conf->backup);
$w->infopage_backup();
print "<hr/>";
$w->infopage_dirs($conf->backup);
$w->footer();
?>
