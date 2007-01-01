<?php
/* performs all the disered actions 
 * authentication is done with .htaccess
 */
$req = strtoupper($_REQUEST['action']);
echo $req;

if (false === ($fd = fopen("/home/miekg/miek.nl/php/rdupd.fifo", "w"))) {
    echo "cannot open";
    return;
}
switch ($req) {
    case "REMOVE":
        $dirs = $_REQUEST['arg'];
        $req = "$req $dirs";
    case "BACKUP":
    case "SHUTDOWN":
        fwrite($fd, $req);
        break;
    case "HTACCESS":
        $user = $_REQUEST['user'];
        $pass = $_REQUEST['pass'];
        fwrite($fd, $req . $user . $pass);
        break;
    default:
        /* go to home page */
         break;
}
fclose($fd);
?>
