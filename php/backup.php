<?php
error_reporting(E_ALL);
require_once("web.php");
$L = "NL";
$w = new web($L, "lang.txt");
$w->header("system");
?>

<h2>Select what to backup</h2>
Use control for multiple selections
<form name="backup.dir" action="http://www.miek.nl/php/action.php" method="post">
<input type="hidden" name="action" value="backup.dir">
<select name="dirs[]" multiple size="10">
<?php
$ar = dir_list("/home/miekg/miek.nl", true);
if ($ar === false)
    echo "Nothing found!\n";

foreach ($ar as $d) {
    echo "<option class=dir>$d</option>\n";
}
?>


</select>
<input type="submit" value="gaan met die banaan">
</form>





<h2>Start backup now!</h2>

<h2>Daily backup</h2>
Schedule: 02:00 ('s nachts) 
Schedule: 15:00 (overdag) 



<?php
$w->footer();
?>
