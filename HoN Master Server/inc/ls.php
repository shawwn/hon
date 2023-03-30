<?php
if(!isset($ls) ){
    $ls = mysql_pconnect("hondb1.heroesofnewerth.com","s2","lunlenabs5") or die();
    mysql_query("SET character_set_results = 'utf8', character_set_client = 'utf8', character_set_connection = 'utf8'", $ls);
}

?>