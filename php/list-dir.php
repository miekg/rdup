#!/usr/bin/php -q

<?php

$files = dirList("/tmp");

print_r($files);



function dirList($directory) 
{
    $results = array();
    if (false === ($handler = opendir($directory))) 
        return false;

    while ($file = readdir($handler)) {
        if ($file != '.' && $file != '..')
            $results[] = $file;
    }
    closedir($handler);
    return $results;
}
?>
