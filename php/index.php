<?php
error_reporting(E_ALL);
require_once("web.php");
$L = "NL";
$w = new web($L, "lang.txt");
$w->header("infopage");
?>

<script src="/php/progressbar.js" type="text/javascript" language="javascript1.2"></script>

<h2>rdup 0.3.5</h2>
<div id="info">Fri Dec 27 19:53:59 CET 2006</div>
An exclude (-E) option was added, rdup-restore is
refactored.  See the 
<a href="projects/rdup/index.html">project page</a> for details. 
<p/>
<a href="projects/rdup/rdup.tar.bz2">download</a>

    <script type="text/javascript" language="javascript1.2"><!--
        var myProgBar = new progressBar(
            1,         //border thickness
            '#000000', //border colour
            '#a5f3b1', //background colour
            '#043db2', //bar colour
            400,       //width of bar (excluding border)
            20,        //height of bar (excluding border)
            1          //direction of progress: 1 = right, 2 = down, 3 = left, 4 = up
        );
    //--></script>

<script type="text/javascript" language="javascript1.2"><!--

myProgBar.setBar(0.5);       //set the progress bar to indicate a progress of 50%
myProgBar.setBar(0.1,true);  //add 10% to the progress bar's progress
myProgBar.setBar(0.1,false); //subtract 10% from the progress bar's progress
myProgBar.setCol('#ff0000'); //change the colour of the progress bar
alert( 'Current progress is ' + myProgBar.amt ) // read current progress

    //--></script>


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
