<?php
if (
      $_SERVER['HTTP_X_FORWARDED_FOR']
   || $_SERVER['HTTP_X_FORWARDED']
   || $_SERVER['HTTP_FORWARDED_FOR']
   || $_SERVER['HTTP_CLIENT_IP']
   || $_SERVER['HTTP_VIA']
   || in_array($_SERVER['REMOTE_PORT'], array(8080,80,6588,8000,3128,553,554))
   || @fsockopen($_SERVER['REMOTE_ADDR'], 80, $errno, $errstr, 1))
{
    exit('Proxy detected');
}   else   {
    exit('Not a proxy');
}
?>
