<?php
/**
 * Contains a bunch of helper funcions
 */

function dirlist($dir, $dironly = false, $depth = 1) 
{
    foreach(scandir($dir) as $entry)
        if($entry != '.' && $entry != '..') {
            $entry  = $dir . '/' .$entry;
            if(is_dir($entry)) {
                $path = pathinfo($entry);
                if ($depth > 1) 
                    $listarray[$path['basename']] = dirlist($entry, $dironly, ($depth - 1));
                else
                    $listarray[$path['basename']] = $entry;
            } else {
                if ($dironly) 
                    continue;
                $path = pathinfo($entry);
                $listarray[] = $path['basename'];
            }
        }
    return($listarray);
}

function month_name($num)
{
    if (!is_numeric($num))
        return "";

    switch($num) {
        case 1:
            return "Jan";
        case 2:
            return "Feb";
        case 3:
            return "Mar";
        case 4:
            return "Apr";
        case 5:
            return "May";
        case 6:
            return "Jun";
        case 7:
            return "Jul";
        case 8:
            return "Aug";
        case 9:
            return "Sep";
        case 10:
            return "Oct";
        case 11:
            return "Nov";
        case 12:
            return "Dec";
    }
}
?>
