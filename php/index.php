<?php
error_reporting(E_ALL);
require_once("web.php");
$L = "NL";
$w = new web($L, "lang.txt");
$w->header("infopage");

/* gig */
$df = (disk_free_space("/tmp") / 1024 / 1024 / 1024);
$dt = (disk_total_space("/tmp") / 1024 / 1024 / 1024);
$per = (($dt - $df) / $dt) * 100;

?>

<h2>Process</h2>
<form name="backup" action="http://www.miek.nl/php/action.php" method="post">
<input type="hidden" value="backup" name="action">
<input class="form-submit" type="submit" value="Maak nu een backup">
</form>

<p/>
Laatste backup: 
<input class="form-submit" readonly value="1912u">
<p>
Totaal/vrij: 
<?php
print "<input class=\"form-submit\" readonly value=\"";
printf("%.2f", $dt); print "/";
printf("%.2f", $df); print "/";
printf("%.2f", $per); 
print "% \">\n";
?>

<p/>

<?php

print "<h2>Backups</h2>\n\n";
$a = (dirlist("/home/miekg/miek.nl/php/A", true, 2));
foreach(array_keys($a) as $topdir) {
    # rdup syntax
    $month = $w->T(month_name(substr($topdir, -2, 2)));

    print "<div class=\"mC\">\n";
#    print "<form name=\"remove\" action=\"http://www.miek.nl/php/action.php\" method=\"post\">";
#    print "<input type=\"hidden\" value=\"remove\" name=\"action\">";
#    print "<input type=\"hidden\" value=$topdir\" name=\"arg\">";
#    print "<input type=\"submit\" value=\"X\">";
#    print "</form>";
    print "<div class=\"mH\" onclick=\"toggleMenu('$topdir')\">$month:&nbsp $topdir/\n";
    print "</div>\n";
    print "<div id=\"$topdir\" class=\"mL\">\n";

    print "<p>";
    print "<form name=\"remove\" action=\"http://www.miek.nl/php/action.php\" method=\"post\">";
    print "<input type=\"hidden\" value=\"remove\" name=\"action\">";
    print "<input type=\"hidden\" value=$topdir name=\"arg\">";
    print "<input type=\"submit\" value=\"Verwijder alles\">";
    print "</form>\n";
    $i = 0;
    print "<table>\n";
    foreach(array_keys($a{$topdir}) as $dir) {
        $dir = basename($dir);
    if ($i % 5 == 0) {
        if ($i == 0) 
            print "<tr>\n";
        else
            print "</tr><tr>\n";
    }

    print "<td>";
    print "<span class=\"mO\">\n";
    print "<form name=\"remove\" action=\"http://www.miek.nl/php/action.php\" method=\"post\">\n";
    print "<input type=\"hidden\" value=\"remove\" name=\"action\">\n";
    print "<input type=\"hidden\" value=\"$topdir/$dir\" name=\"arg\">\n";
    print "<input type=\"submit\" value=\"Verwijder\">\n";
    print "&nbsp; $dir/</span></form>";
    print "</td>";


    $i++;
    }
    print "</table>\n";


    print "</div>\n</div>";
}


$w->footer();
?>
