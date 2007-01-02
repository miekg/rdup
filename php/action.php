<?php
/* performs all the disered actions 
 * authentication is done with .htaccess
 */
require_once("web.php");
require_once("rc.php");

$conf = new rc("rdup.rc");
$req = strtoupper($_REQUEST['action']);

if (false === ($fd = fopen("/home/miekg/miek.nl/php/rdupd.fifo", "w"))) {
    echo "cannot open";
    return;
}

$web->header("Progress");
switch ($req) {
    case "SHUTDOWN":
    case "BACKUP":
        if (!is_readable($conf->lockfile)) {
            echo "$req\n";
            fwrite($fd, $req);
        } else {
            echo "backup in progress\n";
            break;
        }
    break;
    case "REMOVE":
        $dirs = $_REQUEST['arg'];
        echo "Removing: $dirs\n";
        $req = "$req $dirs";
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
$web->footer();
?>
