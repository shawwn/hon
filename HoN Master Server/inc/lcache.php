<?php
  $memcache = new Memcache;

  $memcache->addServer('cache01', 11211, TRUE, 1);
  $memcache->addServer('cache02', 11211, TRUE, 1);
  $memcache->addServer('cache03', 11211, TRUE, 1);
?>
