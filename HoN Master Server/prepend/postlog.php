<?php
//if ( isset($_REQUEST) && is_array($_REQUEST) && count($_REQUEST) > 0 ) { 
  $log_dir = '/var/log/httpd/postlog/'; 
  $log_name = "postlog.log";
  $log_entry = gmdate('r') . "\t" . $_SERVER['REQUEST_URI'] . "\r\n" . serialize($_REQUEST) . "\r\n\r\n"; 
  $fp=fopen( $log_dir . $log_name, 'a' ); 
  fwrite($fp, $log_entry); 
  fclose($fp); 
//}
?>
