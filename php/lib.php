<?php
/**
 * Contains a bunch of helper funcions
 */

function dir_list($directory, $dironly = false) 
{
    $results = array();
    if (false === ($handler = opendir($directory))) 
        return false;

    while ($file = readdir($handler)) {
        $path = $directory . "/" . $file;
        if ($file == "." or $file == "..")
            continue;

        if ($dironly === true && is_dir($path)) {
            $results[] = $file;
            continue;
        }

        if ($dironly === false)
            $results[] = $file;
    }
    closedir($handler);
    return $results;
}
?>
