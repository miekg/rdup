#!/usr/bin/php -q
<?php
/* 
 * Small daemon that opens a unix socket and listens
 * for incoming requests
 * RUN - create a backup
 * DELETE <dir> - delete a backup
 * SHUTDOWN - shutdown the machine
 */
error_reporting(E_NONE);
require_once("rc.php");
$conf = new rc("rdup.rc");
declare(ticks = 1);
pcntl_signal(SIGTERM, "sig_handler");
pcntl_signal(SIGHUP,  "sig_handler");

/* create fifo */
$m = umask(0007);
$fifo = posix_mkfifo($conf->fifo, 0660); 
if (false === $fifo) {
    unlink($conf->fifo);
    if (false === ($fifo = posix_mkfifo($conf->fifo, 0660))) 
        exit(1);
}
umask($m);
chgrp($conf->fifo, "www-data");

if (false === ($fd = fopen($conf->fifo, "r")))
    exit(1);

$read = array($fifo);
while (!stream_select($read, $w = NULL, $e = NULL, 0)) {
    $in = fread($fd, 1024);
    if (strlen($in) == 0)
        continue;

    list($cmd, $arg) = preg_split("/ +/", rtrim($in,"\n"), 2);

    switch($cmd) {
        case "BACKUP":
            echo $conf->rdupsh . "\n";
            break;
        case "SHUTDOWN":
            if (!is_readable($conf->lockfile))
                echo "/sbin/poweroff\n";
            else
                echo "backup in progress\n";
            break;
        case "REMOVE":
            if (!check_backupdir($arg)) {
                echo "Illegal dir name\n";
                break;
            }
            echo "rm -rf " . $conf->backup . "/" . $arg . "\n";
            break;
        case "HTPASSWD":
            list($u, $p) = preg_split("/ /", $arg, 2);
            echo $conf->htaccess;
            print $u .":";
            print md5($p) . "\n";
            break;
        case "SMBSHARE":
            echo $conf->smbshare . "\n";
            break;
        default:
            echo "Unknown cmd: $cmd\n";
            break;
    }
    $read = array($fifo);
}

function sig_handler($signo) {
    switch ($signo) {
        case SIGTERM:
            fclose($fp);
            unlink($conf->fifo);
            exit;
        case SIGHUP:
            // handle restart tasks
            break;
    }
}

function check_backupdir($dir)
{
    /* only numbers and / are allowed in $dir */
    if (preg_match("/^[0-9]{6}$/", $dir))
        return true;
    if (preg_match("/^[0-9]{6}\/[0-9][0-9]?$/", $dir))
        return true;

    return false;
}
?>
