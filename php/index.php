<?php
error_reporting(E_ALL);
require_once("web.php");
$L = "NL";
$w = new web($L, "lang.txt");
$w->header("infopage");
?>

<h2>rdup 0.3.5</h2>
<div id="info">Fri Dec 27 19:53:59 CET 2006</div>
An exclude (-E) option was added, rdup-restore is
refactored.  See the 
<a href="projects/rdup/index.html">project page</a> for details. 
<p/>
<a href="projects/rdup/rdup.tar.bz2">download</a>


<h2>RAID on Linux, booting from USB</h2>
<div id="info">Tue Sep  5 09:09:10 CEST 2006</div>
I'm building a RAID server which boots from a USB stick.
This server will be low maintenance and high capacity.
<p/>
Read <a href="/linux/usb_raid.html">how I did it</a>.

<h2>New LaTeX class</h2>
<div id="info">Fri jun 23 08:38:41 CEST 2006</div>
I've created a new LaTeX layout, this time based on
the excellent <tt>memoir</tt> class. Check it
<a href="linux/blocks.html">out</a>.

<?php
$w->footer();
?>
