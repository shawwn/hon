<?php
    $user_agent = $_SERVER['HTTP_USER_AGENT'];
    $remote_ip  = $_SERVER['REMOTE_ADDR'];
    $allowed = false;

    $_ALLOWED_UA = array("^$", "login_check", "^S2 Games/Heroes of Newerth/[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}/[lwmatx1sc]{3}/(x86-biarch|universal|i686)$");
    //$_ALLOWED_IP = array("64.136.239.227");
    $_ALLOWED_IP = array("127.0.0.1");

    foreach($_ALLOWED_UA as $ereg){
      if(!ereg($ereg, $user_agent) && !in_array($remote_ip, $_ALLOWED_IP)){
         $allowed = false;
      } else {
         $allowed = true;
         break;
      }
    }

    if(!$allowed){
      print 'Invalid Connection: You do not appear to be a Heroes of Newerth client';
      exit;
    }
?>
