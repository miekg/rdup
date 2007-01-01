<?php
/**
 * @Class: web for generating the website
 */

require_once("i18n.php");
require_once("lib.php");

final class web
{
    private $x;
    public function __construct($lang, $langfile)
    {
        if (false === ($this->x = new i18n($lang, $langfile)))
            return false;
        return true;
    }

    public function T($e) 
    {
        return $this->x->T($e);
    }

    /* print the website's header */
    public function header($curpage)
    {
        $pagename = $this->x->T($curpage);
        /* pages */
        $info     = $this->x->T("infopage");
        $config   = $this->x->T("configuration");
        $system   = $this->x->T("system");
        $home     = $this->x->T("homepage rdup");

        /* mainLast is highlighted in the stylesheet */
        $hl_info = "m0";
        $hl_config = "m0";
        $hl_system = "m0";
        switch($curpage) {
            case "infopage":
                $hl_info = "mainLast";
                break;
            case "config":
                $hl_config = "mainLast";
                break;
            case "system":
                $hl_system = "mainLast";
                break;
        }
    
        echo <<<HEADER
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
  <style type="text/css" media="screen">
    <!--
        @import url("css/rdup.css");
        @import url("css/menu.css");
    -->
</style>
<script src="menu.js" type="text/javascript"></script>
<title>rdup - $pagename</title>
</head>
<body id="mainend">
<div id="wrap">
<div id="innerWrap">
<!-- header -->
<div id="header">
<div id="headerLeft">
</div>
<div>
</div>
</div><!-- end header -->

<!-- MAIN NAVIGATION -->
<div id="mainNavOuter">
<div id="mainNav">
<div id="mainNavInner">
<ul>
<!-- mainLast is highlighted -->
    <li id="$hl_info"><a href="http://www.miek.nl/php/index.php">$info</a></li>
    <li id="$hl_config"><a href="http://www.miek.nl/php/config.php">$config</a></li>
    <li id="$hl_system"><a href="http://www.miek.nl/php/system.php">$system</a></li>
    <li id="m4"><a href="http://www.miek.nl/projects/rdup/">$home</a></li>
</ul>
</div><!-- end mainNavInner -->
</div><!-- end mainNav -->
</div><!-- end mainNavOuter -->

<div id="secNavOuter">
<div id="secNav">
<div id="secNavInner">

</div><!-- end secNavInner -->
</div><!-- end secNav -->
</div><!-- end secNavOuter -->
<span id="login-info">
2005 - 2007 &copy; Miek Gieben
</span>
<div style="clear: both;"></div>

<!-- MAIN CONTENT AREA -->
<div id="contentWrap">

  <!-- CONTENT COLUMN -->
  <div id="content1Col">
  <div id="contentIndent">
    <h1>$pagename</h1>
 <div class="content-nav">
</div>
<div id="content">

HEADER;
    return true;
    }

    public function footer() 
    {
        $home   = $this->T("Homepage rdup");
        $author = $this->T("Contact author");
        $date   = date("D M d H:i:s T Y");

        echo <<<FOOTER
</div> <!-- content -->

</div><!-- contentIndent -->
</div><!-- content1Col -->
</div><!-- contentWrap -->
<p id="footer"> Copyright &copy; 2005 - 2007 Miek Gieben<br/>
  <a href="http://www.miek.nl/projects/rdup">$home</a> :
  <a href="http://www.miek.nl/about/about.html">$author</a> :
  $date
</p>
</div> <!-- end innerWrap -->
</div> <!-- end wrap --></body></html>

FOOTER;
        return true;
    }

    public function system_shutdown()
    {
        $server   = $this->T("Server");
        $shutdown = $this->T("Shutdown");
        $value = $this->T("Shutdown the server");
        echo <<<EOF
<h2>$server</h2>
<form name="system.shutdown" action="http://www.miek.nl/php/action.php" method="post">
<fieldset>
<legend>$value</legend>
<input type="hidden" name="action" value="shutdown"/>
<input class="form-submit" value="$value" type="submit"/>
</fieldset>
</form>
EOF;
        return true;
    }

    public function system_network()
    {
        $network = $this->T("Network");
        $leg_type = $this->T("Network type");
        $dyna = $this->T("Dynamic");
        $stat = $this->T("Static");
        $leg_stat = $this->x->T("Static network settings");
        $addr = $this->T("Address");
        $mask = $this->T("Network mask");
        $gate = $this->T("Gateway");
        $submit = $this->T("Submit");
        echo <<<EOF
<h2>$network</h2>
<form name="system.network" action="http://www.miek.nl/php/action.php" method="post">
<fieldset>
<legend>$leg_type</legend>
<input type="hidden" name="action" value="network"/>
<select name="network_type">
   <option value="DHCP">$dyna</option>
   <option value="STATIC">$stat</option>
</select>
<p>
<input class="form-submit" value="$submit" type="submit"/>
</fieldset>
<fieldset>
<legend>$leg_stat</legend>
<table>
<tr>
<td>$addr:</td>
<td><input type="text" name="ip_addr" value=""/></td>
</tr>
<tr>
<td>$mask:</td> 
<td><input type="text" name="ip_mask" value=""/></td>
</tr>
<tr>
<td>$gate:</td>
<td><input type="text" name="ip_gate" value=""/></td>
</tr>
</table>
<input class="form-submit" value="$submit" type="submit"/>
</fieldset>
</form>
EOF;
        return true;
    }

    public function infopage_backup()
    {
        $mk_backup = $this->T("Make a backup now");
        $process = $this->T("Process");
        echo <<<EOF
<h2>$process</h2>
<form name="backup" action="http://www.miek.nl/php/action.php" method="post">
<input type="hidden" value="backup" name="action">
<input class="form-submit" type="submit" value="$mk_backup">
</form>
EOF;
        return true;
    }
        
    public function infopage_info($dir) {
        $last = $this->T("Last backup");
        $free = $this->T("Free");
        $used = $this->T("Used");
        $info = $this->T("Info");
#        $last_backup = last_backup();
        $free_v = sprintf("%.2f", free($dir));
        $used_v = sprintf("%.2f", used($dir));
        $perc_v = sprintf("%.2f", percentage($dir));
        echo <<<EOF
<h2>$info</h2>
<table>
<tr>
<td>$last:</td>
<td>
<input class="form-submit" readonly value="blaat"></td>
</tr>
<tr><td>$free/$used:</td>
<td><input class="form-submit" readonly value="$free_v/$used_v"> ($perc_v %)</td>
</tr>
</table>
EOF;
        return true;
    }

    public function infopage_dirs($dir) 
    {
        $delete_all = $this->T("Delete all");
        $delete = $this->T("Delete");
        $list = dirlist($dir, true, 2);
        $perline = 10;

        print "<h2>Backups</h2>\n";
        foreach(array_keys($list) as $topdir) {
            # rdup syntax
            $month = $this->T(month_name(substr($topdir, -2, 2)));

            print "<div class=\"mC\">\n";
            print "<div class=\"mH\" onclick=\"toggleMenu('$topdir')\">$topdir/\n";
            print "<span class=\"right\">$month</span>";
            print "</div>\n";
            print "<div id=\"$topdir\" class=\"mL\">\n";
            print "<form name=\"remove\" action=\"http://www.miek.nl/php/action.php\" method=\"post\">";
            print "<input type=\"hidden\" value=\"remove\" name=\"action\">";
            print "<input type=\"hidden\" value=$topdir name=\"arg\">";
            print "<input type=\"submit\" value=\"$delete_all\">";
            print "</form>\n";

            $i = 0;
            print "<table>\n";
            foreach(array_keys($list{$topdir}) as $dir) {
                $dir = basename($dir);
            if ($i % $perline == 0) {
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
            print "<input type=\"submit\" value=\"$delete\">\n";
            print "&nbsp; $dir/</span></form>";
            print "</td>";

            $i++;
            }
            print "</table>\n";
            print "</div></div>\n";
        }
        return true;
    }

}
?>
