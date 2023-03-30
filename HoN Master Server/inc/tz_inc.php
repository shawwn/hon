<?php
    //timezone cookie include
    //set cookie and refresh
    if(isset($_POST['timezone']) )
    {
        setcookie('tz',$_POST['timezone'], time()+60*60*24*60);
        header("location: ".$_SERVER['PHP_SELF']);
        exit;
    } elseif(!isset($_COOKIE['tz']) ) {
        setcookie('tz','0', time()+60*60*24*60);
        header("location: ".$_SERVER['PHP_SELF']);
        exit;
    }
//time zones array
$zones = array(
   //multiplier            integer sign     zone       3 char code           long zone description
   '-12' => array( 'sign' => '-', 'n' => "12:00", 'disp' => "IDL", 'long' => "International Date Line West"),
   '-11' => array( 'sign' => '-', 'n' => "11:00", 'disp' => "Sam", 'long' => "Midway Island, Samoa"),
   '-10' => array( 'sign' => '-', 'n' => "10:00", 'disp' => "Haw", 'long' => "Hawaii"),
   '-9'  => array( 'sign' => '-', 'n' => "09:00", 'disp' => "Ala", 'long' => "Alaska"),
   '-8'  => array( 'sign' => '-', 'n' => "08:00", 'disp' => "PST", 'long' => "Pacific Time (US & Canada)"),
   '-7'  => array( 'sign' => '-', 'n' => "07:00", 'disp' => "MST", 'long' => "Mountain Time (US & Canada)"),
   '-6'  => array( 'sign' => '-', 'n' => "06:00", 'disp' => "CST", 'long' => "Central Time (US & Canada)"),
   '-5'  => array( 'sign' => '-', 'n' => "05:00", 'disp' => "EST", 'long' => "Eastern Time (US & Canada)"),
   '-4'  => array( 'sign' => '-', 'n' => "04:00", 'disp' => "Atl", 'long' => "Atlantic Time (Canada); Caracas"),
   '-3.5'=> array( 'sign' => '-', 'n' => "03:30", 'disp' => "Nfd", 'long' => "Newfoundland"),
   '-3'  => array( 'sign' => '-', 'n' => "03:00", 'disp' => "Bra", 'long' => "Brasilia, Greenland"),
   '-2'  => array( 'sign' => '-', 'n' => "02:00", 'disp' => "MdA", 'long' => "Mid-Atlantic"),
   '-1'  => array( 'sign' => '-', 'n' => "01:00", 'disp' => "Azo", 'long' => "Azores"),
   '0'   => array( 'sign' => '+', 'n' => "00:00", 'disp' => "GMT", 'long' => "Greenwich Mean Time"),
   '1'   => array( 'sign' => '+', 'n' => "01:00", 'disp' => "Ber", 'long' => "Berlin, Rome, Stockholm"),
   '2'   => array( 'sign' => '+', 'n' => "02:00", 'disp' => "Ath", 'long' => "Athens, Beirut, Cairo"),
   '3'   => array( 'sign' => '+', 'n' => "03:00", 'disp' => "Mos", 'long' => "Baghdad, Moscow, Nairobi"),
   '3.5' => array( 'sign' => '+', 'n' => "03:30", 'disp' => "Teh", 'long' => "Tehran"),
   '4'   => array( 'sign' => '+', 'n' => "04:00", 'disp' => "Mus", 'long' => "Muscat, Baku"),
   '4'   => array( 'sign' => '+', 'n' => "04:30", 'disp' => "Kab", 'long' => "Kabul"),
   '5'   => array( 'sign' => '+', 'n' => "05:00", 'disp' => "Isl", 'long' => "Islamabad, Karachi, Tashkent"),
   '5.5' => array( 'sign' => '+', 'n' => "05:30", 'disp' => "Mum", 'long' => "Kolkata, Mumbai, New Delhi"),
   '5.75'=> array( 'sign' => '+', 'n' => "05:45", 'disp' => "Kat", 'long' => "Kathmandu"),
   '6'   => array( 'sign' => '+', 'n' => "06:00", 'disp' => "Dha", 'long' => "Almaty, Astana, Dhaka"),
   '6.5' => array( 'sign' => '+', 'n' => "06:30", 'disp' => "Ran", 'long' => "Rangoon"),
   '7'   => array( 'sign' => '+', 'n' => "07:00", 'disp' => "Ban", 'long' => "Bangkok, Hanoi, Jakarta"),
   '8'   => array( 'sign' => '+', 'n' => "08:00", 'disp' => "Bei", 'long' => "Beijing, Hong Kong, Irkutsk, Taipei"),
   '9'   => array( 'sign' => '+', 'n' => "09:00", 'disp' => "Seo", 'long' => "Tokyo, Seoul, Yakutsk"),
   '9.5' => array( 'sign' => '+', 'n' => "09:30", 'disp' => "Ade", 'long' => "Adelaide, Darwin"),
   '10'  => array( 'sign' => '+', 'n' => "10:00", 'disp' => "Bri", 'long' => "Brisbane, Sydney, Guam"),
   '11'  => array( 'sign' => '+', 'n' => "11:00", 'disp' => "Mag", 'long' => "Magadan, Solomon Is."),
   '12'  => array( 'sign' => '+', 'n' => "12:00", 'disp' => "Fij", 'long' => "Auckland, Wellington, Fiji"),
);

?>