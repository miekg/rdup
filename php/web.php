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
    -->
</style>
<script src="menu.js" type="text/javascript"></script>
<link href="menu.css" type="text/css" rel="stylesheet"/> 
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
        $home   = $this->x->T("Homepage rdup");
        $author = $this->x->T("Contact author");
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
        $server   = $this->x->T("Server");
        $shutdown = $this->x->T("Shutdown");
        $value = $this->x->T("Shutdown the server");
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
        $network = $this->x->T("Network");
        $leg_type = $this->x->T("Network type");
        $dyna = $this->x->T("Dynamic");
        $stat = $this->x->T("Static");
        $leg_stat = $this->x->T("Static network settings");
        $addr = $this->x->T("Address");
        $mask = $this->x->T("Network mask");
        $gate = $this->x->T("Gateway");
        $submit = $this->x->T("Submit");
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
}
?>
