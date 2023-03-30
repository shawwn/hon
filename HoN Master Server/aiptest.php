<?

    echo $_SERVER['GEOIP_COUNTRY_CODE']."<br /><br />";

// $SGP = '218.186.10.239';
// $PHL = '124.107.67.161';
// $MYL = '60.54.101.56'; 
// $VNM = '222.252.26.251';

    echo "<pre>";
    include("inc/countrymap.php");
    print_r($C2MAP3);

    echo "</pre><br /><br />";
    echo $_SERVER['GEOIP_COUNTRY_CODE']."<br /><br />"; 
    echo $C2MAP3[$_SERVER['GEOIP_COUNTRY_CODE']]."<br /><br />";


    echo "########################################";

    echo geoip_database_info(GEOIP_COUNTRY_EDITION);
    echo $_SERVER['REMOTE_ADDR'];
    echo geoip_country_code3_by_name($_SERVER['REMOTE_ADDR']);

    ## IP REGIONAL FILTER ##
    function allow_ip($ip){
        $restrict = 1; // default allow
        $EXCEPTIONS = array('178.67.103.113', '210.23.133.41', '210.23.132.55', '210.23.137.244', '213.108.172.195');

        if(!in_array($ip, $EXCEPTIONS) ) {
        if(ereg('^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$', $ip) ) {
            //include("inc/l1.php");

            $COUNTRY_RESTRICT = array("MYS","MY", "PHL","PH", "VNM","VN", "SGP","SG", "USA");

            include("inc/countrymap.php");

            if(in_array($_SERVER['GEOIP_COUNTRY_CODE'], $COUNTRY_RESTRICT) )
            { $restrict = 0; }

        } }
        return $restrict;
    }

    //$_SERVER['GEOIP_COUNTRY_CODE'] = 'US';
    //echo allow_ip('97.86.64.178');



    ?>



