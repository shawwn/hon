<?php

//db to offset
function get_time($time){
    $unix_time = strtotime($time);
    return $unix_time+($_COOKIE['tz']*3600);
}

//offset to db
function set_unixtime($unix_time){
    return $unix_time-($_COOKIE['tz']*3600);
}

//time zones array
$zones = array(
//multiplier            integer sign      zone         3 char code            long zone description
   '-12' => array( 'sign' => '-', 'n' => "12:00", 'disp' => "IDL", 'long' => "International Date Line West"),
   '-11' => array( 'sign' => '-', 'n' => "1100", 'disp' => "Sam", 'long' => "Midway Island, Samoa"),
   '-10' => array( 'sign' => '-', 'n' => "1000", 'disp' => "Haw", 'long' => "Hawaii"),
   '-9'  => array( 'sign' => '-', 'n' => "0900", 'disp' => "Ala", 'long' => "Alaska"),
   '-8'  => array( 'sign' => '-', 'n' => "0800", 'disp' => "PST", 'long' => "Pacific Time (US & Canada); Tijuana"),
   '-7'  => array( 'sign' => '-', 'n' => "0700", 'disp' => "MST", 'long' => "Mountain Time (US & Canada); Chihuahua"),
   '-6'  => array( 'sign' => '-', 'n' => "0600", 'disp' => "CST", 'long' => "Central Time (US & Canada); Central America"),
   '-5'  => array( 'sign' => '-', 'n' => "0500", 'disp' => "EST", 'long' => "Eastern Time (US & Canada); Bogota"),
   '-4'  => array( 'sign' => '-', 'n' => "0400", 'disp' => "Atl", 'long' => "Atlantic Time (Canada); Caracas"),
   '-3.5'=> array( 'sign' => '-', 'n' => "0330", 'disp' => "Nfd", 'long' => "Newfoundland"),
   '-3'  => array( 'sign' => '-', 'n' => "0300", 'disp' => "Bra", 'long' => "Brasilia, Greenland"),
   '-2'  => array( 'sign' => '-', 'n' => "0200", 'disp' => "MdA", 'long' => "Mid-Atlantic"),
   '-1'  => array( 'sign' => '-', 'n' => "0100", 'disp' => "Azo", 'long' => "Azores"),
   '0'   => array( 'sign' => '+', 'n' => "0000", 'disp' => "GMT", 'long' => "Greenwich Mean Time"),
   '1'   => array( 'sign' => '+', 'n' => "0100", 'disp' => "Ber", 'long' => "Berlin, Rome, Stockholm, Brussels"),
   '2'   => array( 'sign' => '+', 'n' => "0200", 'disp' => "Ath", 'long' => "Athens, Beirut, Cairo, Jerusalem"),
   '3'   => array( 'sign' => '+', 'n' => "0300", 'disp' => "Mos", 'long' => "Baghdad, Moscow, Nairobi"),
   '3.5' => array( 'sign' => '+', 'n' => "0330", 'disp' => "Teh", 'long' => "Tehran"),
   '4'   => array( 'sign' => '+', 'n' => "0400", 'disp' => "Mus", 'long' => "Muscat, Baku"),
   '4'   => array( 'sign' => '+', 'n' => "0430", 'disp' => "Kab", 'long' => "Kabul"),
   '5'   => array( 'sign' => '+', 'n' => "0500", 'disp' => "Isl", 'long' => "Islamabad, Karachi, Tashkent"),
   '5.5' => array( 'sign' => '+', 'n' => "0530", 'disp' => "Mum", 'long' => "Chennai, Kolkata, Mumbai, New Delhi"),
   '5.75'=> array( 'sign' => '+', 'n' => "0545", 'disp' => "Kat", 'long' => "Kathmandu"),
   '6'   => array( 'sign' => '+', 'n' => "0600", 'disp' => "Dha", 'long' => "Almaty, Astana, Dhaka"),
   '6.5' => array( 'sign' => '+', 'n' => "0630", 'disp' => "Ran", 'long' => "Rangoon"),
   '7'   => array( 'sign' => '+', 'n' => "0700", 'disp' => "Ban", 'long' => "Bangkok, Hanoi, Jakarta"),
   '8'   => array( 'sign' => '+', 'n' => "0800", 'disp' => "Bei", 'long' => "Beijing, Hong Kong, Irkutsk, Taipei"),
   '9'   => array( 'sign' => '+', 'n' => "0900", 'disp' => "Seo", 'long' => "Tokyo, Seoul, Yakutsk"),
   '9.5' => array( 'sign' => '+', 'n' => "0930", 'disp' => "Ade", 'long' => "Adelaide, Darwin"),
   '10'  => array( 'sign' => '+', 'n' => "1000", 'disp' => "Bri", 'long' => "Brisbane, Sydney, Guam"),
   '11'  => array( 'sign' => '+', 'n' => "1100", 'disp' => "Mag", 'long' => "Magadan, Solomon Is."),
   '12'  => array( 'sign' => '+', 'n' => "1200", 'disp' => "Fij", 'long' => "Auckland, Wellington, Fiji"),
);
?>