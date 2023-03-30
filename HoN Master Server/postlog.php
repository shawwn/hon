<?php
  if (isset($_SERVER['HTTP_USER_AGENT'])) {
    $user_agent = $_SERVER['HTTP_USER_AGENT'];
  } else {
    $user_agent = "NULL";
  }

  if (isset($_POST) && is_array($_POST) && count($_POST) > 0) {
    $request_type = "POST";
  } elseif (isset($_GET) && is_array($_GET) && count($_GET) > 0){
    $request_type = "GET";
  } else {
    $request_type = "NONE";
  }
    

  $log_dir = '/usr/local/www/websites/masterserver.hon.s2games.com/postlogs/'; 
  $log_name = "postlog.log";
  $log_entry = gmdate('r') . '  -  ' . $_SERVER['REMOTE_ADDR'] . '  -  ' . $request_type . "\t" . $user_agent .
               "\r\n" . $_SERVER['REQUEST_URI'] .
               "\r\n" . serialize($_REQUEST) .
               "\r\n\r\n";

  $fp=fopen( $log_dir . $log_name, 'a' ); 
  fwrite($fp, $log_entry); 
  fclose($fp); 
?>
