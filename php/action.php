<?php
/* performs all the disered actions 
 * authentication is done with .htaccess
 */

switch ($_REQUEST['action']) {

    case "backup.dir":
        $a = $_REQUEST['dirs'];
        print_r($a);
        break;
    case "system.network":
    default:
         echo $_REQUEST['action'];
        /* go to home page */
}
?>
