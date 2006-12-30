<?php
/**
 * @Class: web for generating the website
 */

require_once("i18n.php");

final class web
{
    private $x;

    public function __construct($lang, $langfile)
    {
        $this->x = new i18n($lang, $langfile);
        return true;
    }

    /* print the website's header */
    public function header($curpage)
    {
        $pagename = $this->x->T($curpage);
        /* pages */
        $info     = $this->x->T("infopage");
        $backup   = $this->x->T("backups");
        $config   = $this->x->T("configuration");
        $system   = $this->x->T("system");
        $home     = $this->x->T("homepage rdup");
    
        echo <<<HEADER
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
  <style type="text/css" media="screen">
    <!--
        @import url("css/rdup.css");
    -->
</style>
<title>rdup - $pagename</title>
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
    <li id="m0"><a href="http://www.miek.nl/">$info</a></li>
    <li id="m1"><a href="http://www.miek.nl/projects/">$backup</a></li>
    <li id="m2"><a href="http://www.miek.nl/projects/">$config</a></li>
    <li id="m3"><a href="http://www.miek.nl/linux/">$system</a></li>
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
2005 - 2007
 &copy; Miek Gieben
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
        echo <<<FOOTER
</div> <!-- content -->

</div><!-- contentIndent -->
</div><!-- content1Col -->
</div><!-- contentWrap -->
<p id="footer"> Copyright &copy; 2005 - 2007 Miek Gieben<br/>
  <a href="http://www.miek.nl/projects/rdup">$home</a> :
  <a href="http://www.miek.nl/about/about.html">$author</a>
</p>
</div> <!-- end innerWrap -->

FOOTER;
    }
}
?>
