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
        $a = $_REQUEST['arg'];
        print_r($a);
        $req = "$req $a";
    case "BACKUP":
    case "SHUTDOWN":
        fwrite($fd, $req);
        break;
    default:
        /* go to home page */
         break;
}
fclose($fd);
?>
