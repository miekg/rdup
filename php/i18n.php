<?php

/**
 * @Class: i18n for translating string
 * Have a list of the language, a sha1 sum
 * of the english string and the translation
 */

final class i18n 
{
    private $lang;
    private $trans;
    private $file;

    public function __construct($lang, $langfile)
    {
        $this->lang  = strtoupper($lang);
        $this->trans = $this->read($langfile);
        return true;
    }

    public function read($file)
    {
        if (false === ($f = file($file)))
            return array();
       
        $tr = array();
        foreach($f as $line) {
            if (substr($line, 0, 1) == "#") 
                continue;
            list($l, $h, $t) = split(" ", rtrim($line, "\n"), 3);
            $tr{$l}{$h} = $t;
        }
        return $tr;
    }

    public function show($english, $translation)
    {
        echo "# " . $english . "\n";
        echo $this->lang . " ";
        echo sha1($english) . " ";
        echo $translation . "\n";
        return true;
    }

    /* The translate funcion, give the
     * english text and retrieve the language string
     */
    public function T($e) {
        if (isset($this->trans{$this->lang}{sha1($e)}))
            return $this->trans{$this->lang}{sha1($e)};
        return $e;
    }
}
?>
