<?
#############################################################
##    client_request_class.php
##    class designed to handle game client requests
##    for all web related content including clans (general),
##    member info, authentication, and items_db
#############################################################

class ClientRequest {
    var $error;
    var $ret_array;
    var $cli_info;

    ## CONSTRUCT ##
    function ClientRequest(){
        $this->ret_array = array();
        $this->error = array();
    }

    ## OUTPUT VIEW ##
    function output(){
        if(count($this->error) > 0)
        {   $false = array(false);
            $return = array_merge($false, $this->error);
            echo serialize($return);
        } else {
            //format output
            $true = array(true);
            $return = array_merge($true, $this->ret_array);

            $data = serialize($return);
            echo $data;
        }
    }

    ## PROXIED LOGINS ##
    function get_proxy_status(){
        if (
             $_SERVER['HTTP_X_FORWARDED_FOR']
          || $_SERVER['HTTP_X_FORWARDED']
          || $_SERVER['HTTP_FORWARDED_FOR']
          || $_SERVER['HTTP_CLIENT_IP']
          || $_SERVER['HTTP_VIA']
          || in_array($_SERVER['REMOTE_PORT'], array(8080,80,6588,8000,3128,553,554))
          || @fsockopen($_SERVER['REMOTE_ADDR'], 80, $errno, $errstr, 1))
        {
            return true;
        }   else   {
            return false;
        }
    }

    ## LOGIN ATTEMPTS ##
    function get_auth_attempts($key){
        require('inc/lcache.php');
        $attempts = $memcache->get($key);
        if($attempts == null){
          $attempts == false;
        }
        return $attempts;
    }

    ## UPDATE LOGIN ATTEMPTS ##
    function update_auth_attempts($key, $attempts, $replace=false){
        require('inc/lcache.php');
        if($replace){
          $result = $memcache->replace($key, $attempts, false, 300);
        } else {
          $result = $memcache->set($key, $attempts, false, 300);
        }
        return $result;
    }

    ## PERMISSION CHECK ##
    function check_perms() {
        include('inc/l2.php');
        $account_id = mysql_real_escape_string($this->cli_info['account_id']);
        $select = "SELECT account.* FROM hon_accounts.account WHERE account_id='{$account_id}' LIMIT 1";
	    if($result = mysql_query($select, $l2) )
        {   $row = mysql_fetch_assoc($result);
            // garena account permissions update
            if($row['garena_id'] != null AND !empty($_REQUEST['token']) )
            {   if( $garInfo = $this->garena_info($_REQUEST['token'] ) )
                {
                    $type = $this->garena_account_type($garInfo);   //var_dump($garInfo);
                    $row = array_merge($row, $type);
                }
            }

            $this->ret_array['check_perms'] = $row;
        }

        @mysql_close($l2);
    }

    ## CLIENT COOKIE VALIDATION ##
    function validate_cookie($cookie) {
        include('inc/l1.php');
        $cookie = mysql_real_escape_string($cookie);
        if (empty($cookie)) return 0;
        $select = "SELECT in_game.*, account.account_type, susp_id FROM hon_tracking.in_game JOIN hon_accounts.account USING (account_id) JOIN hon_accounts.super_account USING (super_id) WHERE cookie = '{$cookie}' LIMIT 1";     
        if($result = mysql_query($select, $l1) )
        {   if(mysql_num_rows($result) == 1)
            {   $this->cli_info = mysql_fetch_assoc($result);
                // suspension check
                if($this->cli_info['susp_id'] == 0){

                $update = "UPDATE hon_tracking.in_game SET last_activity = CURRENT_TIMESTAMP WHERE account_id = '{$this->cli_info['account_id']}' LIMIT 1";
                mysql_query($update, $l1);


                return 1;
                } else {return 0; }    // suspended
            } else {return 0; }
        } else {return 0; }
        @mysql_close($l1);
    }

    ## REQUEST COOKIE VALIDATION ##
    function validate_user_cookie($cookie) {
        include('inc/l1.php');
        $cookie = mysql_real_escape_string($cookie);
        if (empty($cookie)) return 0;
        $select = "SELECT * FROM hon_tracking.in_game WHERE cookie = '{$cookie}' LIMIT 1";
        if($result = mysql_query($select, $l1) )
        {   if(mysql_num_rows($result) == 1)
            {   $row = mysql_fetch_assoc($result);
                return $row['account_id'];
            } else { return 0; }
        } else { return 0; }
        @mysql_close($l1);
    }

    ## LAN IP REQUEST
    function check_lan_ip(){
        include('inc/l2.php');
        $ip = mysql_real_escape_string($_SERVER['REMOTE_ADDR']);
        $select = "SELECT lan_id FROM hon_accounts.ip_lan WHERE ip='{$ip}' LIMIT 1";
        if($result = mysql_query($select, $l2) )
        {   if(mysql_num_rows($result) > 0)
            {return true; } else {return false; }
        } else {return false; }
        @mysql_close($l2);
    }

    ## CHAT SERVER REQUESTED INFOS ##
    function grab_chatserv_info() {
        include('inc/l2.php');

        $account_id = mysql_real_escape_string($this->cli_info['account_id']);
        $select = "SELECT account.account_id, roster.clan_id, nickname, name, tag, rank FROM hon_accounts.account LEFT JOIN hon_clans.roster USING (account_id) LEFT JOIN hon_clans.clan_info USING (clan_id) WHERE account.account_id='{$account_id}' LIMIT 1";
        if($result = mysql_query($select, $l2) )
        {
            $this->ret_array = mysql_fetch_assoc($result);
            $aid[]=$this->cli_info['account_id'];

            // get buddy list
            $this->buddy_list($aid);

            // get clan list
            $this->clan_rosterByClan_id($this->ret_array['clan_id']);

            $this->get_stats($aid);

        } else {$this->error['connect'] = "Could not connect. Try again."; }
        @mysql_close($l2);
    }

    ## SERVER LIST REQUEST ##
    function get_server_list($cookie, $gametype, $region = "", $limit = "") {
        include('inc/l1.php');

        $select = "SELECT online, in_game FROM hon_tracking.chat_online";
        if($result = mysql_query($select, $l1) )
        {
            $this->ret_array['svr_stats'] = mysql_fetch_assoc($result);
        }

        if(ereg('[0-9]{2}', $gametype) )
        {

            if (!empty($region))
            {
              $region = mysql_real_escape_string($region);
              // US Region Filter
        			if (strcasecmp($region, "US") == 0)
        			{
     				    $region_filter = "AND location IN ('USE','USC','USS','USW')";
        			}
        			// EU Region Filters
        			else if (strcasecmp($region, "EU") == 0)
        			{
     				    $region_filter = "AND location IN ('EU','DE','NL','UK')";
        			}
        			// All Other Region Filters
        			else
        			{
        				$region_filter = "AND location = '$region'";
        			}
            }
            else
            {
              $region_filter = "";
            }

            // private
            switch(substr($gametype, 1, 1) )
            {   case "0":
                    $private = "AND private='0'";    // public match
                break;
                case "1": // neutral
                    $private = "AND private='1'";    // reserved / password protected
                break;
                case "2":
                default:
                    $private = "";
                break;
            }


            // server list
            switch(substr($gametype, 0, 1) )
            {   case "0":       // all servers
                    $match_class = "AND class='0'";
                    $c_state = "2";
                    $select = "SELECT game_type.*, game_options.*, ip, port, num_conn, c_state, last_upd, location FROM hon_server.info JOIN hon_server.game_type ON (info.server_id=game_type.server_id AND info.session=game_type.session) JOIN hon_server.game_options ON (info.server_id=game_options.server_id AND info.session=game_options.session)WHERE c_state='{$c_state}' {$match_class} {$private}";
                break;

                default:
                case "1":       // quick match
                    $match_class = "AND class='1'";
                    $c_state = "2";
                    $select = "SELECT game_type.*, game_options.*, ip, port, location, num_conn, c_state, last_upd, location FROM hon_server.info JOIN hon_server.game_type ON (info.server_id=game_type.server_id AND info.session=game_type.session) JOIN hon_server.game_options ON (info.server_id=game_options.server_id AND info.session=game_options.session)WHERE c_state='{$c_state}' {$match_class} {$private} {$region_filter}";
                break;
                case "2":       // ranked ladder match
                    $match_class = "AND class='2'";
                    $c_state = "2";
                    $select = "SELECT game_type.*, game_options.*, ip, port, num_conn, c_state, last_upd, location FROM hon_server.info JOIN hon_server.game_type ON (info.server_id=game_type.server_id AND info.session=game_type.session) JOIN hon_server.game_options ON (info.server_id=game_options.server_id AND info.session=game_options.session)WHERE c_state='{$c_state}' {$match_class} {$private}";
                break;
                case "3":       // active games
                    $c_state = "3";
                    $select = "SELECT game_type.*, game_options.*, ip, port, num_conn, c_state, last_upd, location FROM hon_server.info JOIN hon_server.game_type ON (info.server_id=game_type.server_id AND info.session=game_type.session) JOIN hon_server.game_options ON (info.server_id=game_options.server_id AND info.session=game_options.session)WHERE c_state='{$c_state}' {$private}";
                break;

                case "9":       // zhombie list
                    $c_state = "1";

                    if (!empty($limit))
                    {
                      $limit = mysql_real_escape_string($limit);
                      $limit_filter = "LIMIT $limit";
                    }
                    else
                    {
                      $limit_filter = "";
                    }

                    $select = "SELECT server_id, session, ip, port, location FROM hon_server.info WHERE c_state='{$c_state}' {$region_filter} ORDER BY server_id ASC {$limit_filter}";

                    //include('inc/l1.php');

                    $ins_cookie = mysql_real_escape_string($cookie);
                    // GENERATE KEY
                    $salt = '';       // make salt
                    for ($i = 0; $i < 3; $i++)
                    {$salt .= chr(rand(33, 126)); }
                    // return key
                    $this->ret_array['acc_key'] = md5($_SERVER['REMOTE_ADDR'].$salt);
                    $tmp['key'] = mysql_real_escape_string($this->ret_array['acc_key']);
                    // store key
                    $insert = "INSERT INTO hon_server.match_request (cookie, acc_key) VALUES ('{$ins_cookie}', '{$tmp['key']}') ON DUPLICATE KEY UPDATE acc_key=VALUES(acc_key)";
                    if(!$result = mysql_query($insert, $l1) )
                    { $this->error['acc_key'] = "Failed to create key.";  }

                    //@mysql_close($l1);
                break;
            }

        }

        // grab server list
        if($result = mysql_query($select, $l1) )
        {   $this->ret_array['server_list'] = array();
            while($row = mysql_fetch_assoc($result) )
            {
              $this->ret_array['server_list'][$row['server_id']]=$row;
            }
        } else {$this->error['server_list'] = "Failed to return game server list."; }
        @mysql_close($l1);
    }

    ## COOKIE FACTORY ##
    function make_cookie($account_id){
        include('inc/l1.php');
        $cookie = md5(time().md5($account_id));

        $account_id = mysql_real_escape_string($account_id);
        $this->ret_array['cookie'] = $cookie;
        $this->ret_array['ip'] = $_SERVER['REMOTE_ADDR'];

	    $country = mysql_real_escape_string($_SERVER['GEOIP_COUNTRY_CODE']);

	    $ipv4address = sprintf("%u", ip2long($_SERVER['REMOTE_ADDR']));
	    $ipv4address = mysql_real_escape_string($ipv4address);

        // save cookie
        $replace = "REPLACE INTO hon_tracking.in_game(account_id, last_login, cookie, ip) VALUES ('{$account_id}', NOW(), '{$cookie}', '{$ipv4address}' )";
        if(!mysql_query($replace, $l1) )
        {$this->error['error']="Connect: Could not connect."; }

        // login tracking
        $insert = "INSERT INTO hon_tracking.player_logins (account_id, first, latest, total) VALUES ('{$account_id}', NOW(), NOW(), 1) ON DUPLICATE KEY UPDATE latest=VALUES(latest), total=total + VALUES(total)";
        if(!mysql_query($insert, $l1) )
        {$this->error['error']="Connect: Could not connect."; }

        // ip tracking
        $insert = "INSERT IGNORE INTO hon_tracking.ip_tracking (account_id, ip, country) VALUES ('{$account_id}', '{$ipv4address}', '{$country}')";
        if(!mysql_query($insert, $l1) )
        {$this->error['error']="Connect: Could not connect."; }

        // logins (unique)
        $insert = "INSERT INTO hon_tracking.logins (account_id, time, date) VALUES ('{$account_id}', NOW(), CURDATE())";
        if(!mysql_query($insert, $l1) )
        {$this->error['error']="Connect: Could not connect."; }


        @mysql_close($l1);
    }

    ## GET COOKIE ##
    function get_cookie($account_id){
        include('inc/l1.php');
        $account_id = mysql_real_escape_string($account_id);

        //retrieve cookie
        $get_q = "SELECT * FROM hon_tracking.in_game WHERE account_id = '{$account_id}' LIMIT 1";
        if($res_get = mysql_query($get_q, $l1) )
        { if(mysql_num_rows($res_get) > 0)
          {
            $row = mysql_fetch_assoc($res_get);
            return $row['cookie'];
          } else {$this->error['error']="Cookie:  No cookie found."; return 0; }
        } else {$this->error['error']="Connect: Could not connect."; return 0; }
        @mysql_close($l1);
    }

    ## COOKIE COMPARE ##
    function compare_cookie($account_id, $cookie){
        if($cookie === $this->get_cookie($account_id) )
        { return 1;
        } else {return 0; }
    }

    ## STANDARD AUTHENTICATION ##
    function alpha_auth($login, $pass){


        include('inc/l1.php');

        $login = mysql_real_escape_string($login, $l1);

        $query = "SELECT account_id, nickname, email, account_type, trial, susp_id, salt, password, pass_exp, lan, unblock_ip FROM hon_accounts.account JOIN hon_accounts.super_account USING (super_id) WHERE (nickname = '{$login}') LIMIT 1";
        if($result = mysql_query($query, $l1) )
        {
            if($row_auth = mysql_fetch_assoc($result))
            {
                if($this->allow_ip($_SERVER['REMOTE_ADDR']) OR (bool) $row_auth['unblock_ip'])
                {


                // Brute force check
                $bfckey = $row_auth['account_id']."@".$_SERVER['REMOTE_ADDR'];
                $attempts = $this->get_auth_attempts($bfckey);
                if($attempts >= 10){
                  $this->error['auth']="Max login attempts exceeded. Wait 5 minutes and try again.";
                  return 0;
                }

                // suspension check
                if($row_auth['susp_id'] == 0) {

                $passCheck = md5($pass.$row_auth['salt']."[!~esTo0".chr(125) );

                // mb str to arr function
                function mbStringToArray ($string) {
                    $strlen = mb_strlen($string);
                    while ($strlen) {
                        $array[] = mb_substr($string,0,1,"UTF-8");
                        $string = mb_substr($string,1,$strlen,"UTF-8");
                        $strlen = mb_strlen($string);
                    }
                    return $array;
                }

                // pass decryption
                $ePass = '';
                $aPass = mbStringToArray($pass); // print_r($aPass);

                $aPass = preg_split('//', $pass, -1, PREG_SPLIT_NO_EMPTY);
                foreach($aPass as $char)
                {$ePass .= chr(ord($char)^127); }

                $encPass = md5($ePass.$row_auth['salt']."[!~esTo0".chr(125));
                if($passCheck === $row_auth['password'] OR $encPass === $row_auth['password'] ) {

                // check lan account
                if($row_auth['lan'] == '1' AND $row_auth['account_type'] == 1 )
                {$lan_check = $this->check_lan_ip(); } else {$lan_check = false; }

		        if($lan_check == true) {$row_auth['account_type']=3; }

                // TRIAL ACCOUNTS
                $trial_check = false;
                $trial_expired = false;
                // active
                if($row_auth['account_type'] == 1 AND $row_auth['trial'] == 1)
                {$trial_check = true; }
                // expired
                if($row_auth['account_type'] == 1 AND $row_auth['trial'] == 2)
                {$trial_expired = true; }  // only true if account is still type 1

	            if(!ereg("UnNamedNewbie[0-9]{1,10}", $row_auth['nickname']) )
	            {  // DISABLED CHECK
                   if( $row_auth['account_type'] != 0) {
                   // TRIAL EXPIRED CHECK
		           if( $trial_expired != true) {
                   // PAID/ACTIVE ACCOUNT CHECKS
                   if( $row_auth['account_type'] > 1 OR $lan_check OR $trial_check )
                   {
                      ## AUTHORIZED ##
                    // return only authKeys
                    $authKeys = array ('account_id', 'nickname', 'email', 'account_type', 'trial', 'susp_id', 'prepay_only');
                    foreach($authKeys as $key)
                    {   $this->ret_array[$key] = $row_auth[$key];  }

                    // return null for no pass change prompt, > 0 for passive change prompt, 0 for forced change prompt
                    if (is_null($row_auth['pass_exp']))
                    {   $this->ret_array['pass_exp'] = $row_auth['pass_exp'];
                    } else {
                        $this->ret_array['pass_exp'] = $row_auth['pass_exp'] - time();
                        if($this->ret_array['pass_exp'] < 0)
                        { $this->ret_array['pass_exp'] = 0; }
                    }

                    // make cookie
                    $this->make_cookie($this->ret_array['account_id']);

                    // for multi-id functions
                    $tmp['account_id'] = $this->ret_array['account_id'];

                    $select = "SELECT * FROM hon_web.chat_server LIMIT 1";
                    if( $chat_result = mysql_query($select, $l1) )
                    {   if(mysql_num_rows($chat_result) > 0)
			            {	$chat = mysql_fetch_assoc($chat_result);
			  	            $this->ret_array['chat_url'] = $chat['url'];
			            } else {$this->ret_array['chat_url'] = 'http://chatserver.heroesofnewerth.com'; }
                    } else {$this->ret_array['chat_url'] = 'http://chatserver.heroesofnewerth.com'; }

                    $aid[]=$this->ret_array['account_id'];

                    // get buddy list
                    $this->buddy_list($aid);
                    $this->ignored_list($aid);
                    $this->ban_list($aid);

                    // get leaver and level
                    $this->get_initStats($this->ret_array['account_id']);
                    // get clan info
  	                $this->clan_infoByAccount($this->ret_array['account_id']);
  	                // get auto join channels
  	                $this->grab_rooms($this->ret_array['account_id']);
  	                // get notifications
  	                $this->grab_notifications($this->ret_array['account_id']);

                    if(isset($this->ret_array['clan_member_info']['clan_id']) )
                    { $this->clan_rosterByClan_id($this->ret_array['clan_member_info']['clan_id']);
                    } else { $this->ret_array['clan_roster']['error']="No clan members found."; }

                    // upgrades
                    $upgrades = $this->getUpgrades($this->ret_array['account_id']);
                    if(!empty($upgrades))
                    { $this->ret_array['infos'][$this->ret_array['account_id']]['upgrades'] = $upgrades; }

                   // UNPAID / DEACTIVATED
                   } else {$this->error['auth']="#You must purchase an account to log in."; $this->error['account_id']=$row_auth['account_id']; $this->error['nickname']=$row_auth['nickname']; $this->error['cookie'] = "00000000000000000000000000000000"; }
                   // EXPIRED TRIAL
                   } else {$this->error['auth']="Trial account expired."; $this->error['account_id']=$row_auth['account_id']; $this->error['nickname']=$row_auth['nickname']; $this->error['cookie'] = "00000000000000000000000000000000"; }
                   // DISABLED
                   } else {$this->error['auth']="Login is not available for your account.";}
		        } else {$this->error['auth']="Login is not available for your account. Nickname must be changed."; }
                // pass check
                } else {
                    //Valid Nickname, but bad password occured here. Do account_id limiting.
                    if($row_auth['account_id']) {
			            $bfckey = $row_auth['account_id']."@".$_SERVER['REMOTE_ADDR'];
                        $attempts = null;
                        $attempts = $this->get_auth_attempts($bfckey);
                        if($attempts) {
                           $attempts = $attempts + 1;
                           $this->update_auth_attempts($bfckey, $attempts, true);
                        } else { $this->update_auth_attempts($bfckey, 1); }
                        $this->error['auth']="Invalid Nickname or Password.";
                    }
                }
                // account suspension
                } else { $this->error['auth']="Account suspended.\n For more information, visit the Customer Service page."; }

                // IP BLOCK
                } else {$this->error['auth']="Regional access is restricted. Visit hon.garena.com for more details."; }

            // Bad Nickname occured here. Do IP limiting.
            } else { $this->error['auth']="Invalid Nickname or Password.";  }

        }

        @mysql_close($l1);

    }

    ## GET BUDDY LIST ##
    function buddy_list($account_id){
        include('inc/l2.php');
        $ids = "";
        foreach($account_id as $k => $v)
        {$ids .= "buddy.account_id = '".mysql_real_escape_string($v)."' OR "; }
        $ids = trim($ids, 'OR ');

        $select = "SELECT buddy.*, nickname, name AS clan_name, tag AS clan_tag FROM hon_chat.buddy JOIN hon_accounts.account ON (buddy_id = account.account_id) LEFT JOIN hon_clans.roster ON (buddy_id = roster.account_id) LEFT JOIN hon_clans.clan_info USING(clan_id) WHERE {$ids} AND hon_chat.buddy.status IN (0, 2)";
        if($result = mysql_query($select, $l2) )
        {   if(mysql_num_rows($result) > 0)
            {   while($row = mysql_fetch_assoc($result) )
                {   $this->ret_array['buddy_list'][$row['account_id']][]=$row;  }
            } else { $this->ret_array['buddy']['error'][1] = "No buddies found."; }
        } else { $this->error['error']="Connect: Could not connect."; }
        @mysql_close($l2);
    }

    ## GET IGNORED LIST ##
    function ignored_list($account_id){
        include('inc/l2.php');
        $ids = "";
        foreach($account_id as $k => $v)
        {$ids .= "ignored.account_id = '".mysql_real_escape_string($v)."' OR "; }
        $ids = trim($ids, 'OR ');

        $select = "SELECT ignored.*, nickname FROM hon_chat.ignored JOIN hon_accounts.account ON (ignored_id = account.account_id)  WHERE {$ids}";
        if($result = mysql_query($select, $l2) )
        {   if(mysql_num_rows($result) > 0)
            {   while($row = mysql_fetch_assoc($result) )
                {   $this->ret_array['ignored_list'][$row['account_id']][]=$row;  }
            } else { $this->ret_array['ignored']['error'][1] = "No ignored users found."; }
        } else { $this->error['error']="Connect: Could not connect."; }
        @mysql_close($l2);
    }

    ## GET BANNED LIST ##
    function ban_list($account_id){
        include('inc/l2.php');
        $ids = "";
        foreach($account_id as $k => $v)
        {$ids .= "banned.account_id = '".mysql_real_escape_string($v)."' OR "; }
        $ids = trim($ids, 'OR ');

        $select = "SELECT banned.*, nickname FROM hon_chat.banned JOIN hon_accounts.account ON (banned_id = account.account_id) WHERE {$ids}";
        if($result = mysql_query($select, $l2) )
        {   if(mysql_num_rows($result) > 0)
            {   while($row = mysql_fetch_assoc($result) )
                {   $this->ret_array['banned_list'][$row['account_id']][]=$row;  }
            } else { $this->ret_array['banned']['error'][1] = "No ban list found."; }
        } else { $this->error['error']="Connect: Could not connect."; }
        @mysql_close($l2);
    }

    ## ADD BUDDY ##
    function new_buddy($id, $buddy_id){
		if($buddy_id < 2000000){
			include('inc/l1.php');

			$id = mysql_real_escape_string($this->cli_info['account_id']);
			$buddy_id = mysql_real_escape_string($buddy_id);

			// add buddy to list
			$insert = "INSERT IGNORE INTO hon_chat.buddy (account_id, buddy_id) VALUES ('{$id}', '{$buddy_id}')";
			if($result = mysql_query($insert, $l1) )
			{   $this->ret_array['new_buddy'] = "OK";
			} else {$this->error['error'] = "Connect: Could not connect."; }
			@mysql_close($l1);
		} else {$this->error['error'] = "Connect: Could not connect."; }
    }

    ## NEW BUDDY REQUEST ##
    function new_buddy2($account_id, $buddynickname){
        include('inc/l1.php');
        $account_id = mysql_real_escape_string($account_id);
        $account_info = $this->oneid2onenick($account_id);
        $this->ret_array['account_id'] = $account_id;
        $this->ret_array['account_nickname'] = $account_info['nickname'];

        $buddy_info = $this->onenick2oneid($buddynickname);
        $buddy_id = $buddy_info['account_id'];
        $this->ret_array['buddy_id'] = $buddy_id;
        $this->ret_array['buddy_nickname'] = $buddy_info['nickname'];

        $notify_id = 0;

        if($account_id == 0 || $buddy_id == 0)
        {
	        $this->ret_array['error'] = "Connect: Could not connect.";
	        return;
        } else {
  			// is the buddy requester on the buddy requesteds ignore list?
  			$sql = "SELECT COUNT(account_id) AS Count FROM hon_chat.ignored WHERE account_id = '{$buddy_id}' AND ignored_id = '{$account_id}' LIMIT 1;";
  			$result = mysql_query($sql, $l1);
  			$row = mysql_fetch_assoc($result);

  			if ($row['Count'] == 1)
  			{
  			  $this->ret_array['new_buddy'] = "IGN";
  			  return;
  			}

  			$create_time = mktime();
	  		$expire_time = $create_time + 60; // 1 minute

  			// has the buddy requester recently requested this buddy as a friend?
  			$sql = "SELECT COUNT(account_id) AS Count FROM hon_chat.buddy WHERE (account_id = '{$account_id}' AND buddy_id = '{$buddy_id}') AND expire_time > '{$create_time}' LIMIT 1;";
  			$result = mysql_query($sql, $l1);
  			$row = mysql_fetch_assoc($result);

    	    if ($row['Count'] >= 1)
    	    {
    	        $this->ret_array['new_buddy'] = "DUP";
    	        return;
            }

            // remove old entries from BOTH buddies lists
            $insert = "DELETE FROM hon_chat.buddy WHERE account_id='{$account_id}' AND buddy_id='{$buddy_id}';";
            if ($result = mysql_query($insert, $l1) )
            {
                $insert = "DELETE FROM hon_chat.buddy WHERE account_id='{$buddy_id}' AND buddy_id='{$account_id}';";
                if ($result = mysql_query($insert, $l1))
                {  $this->ret_array['new_buddy'] = "REM";
                } else {
                  $this->ret_array['new_buddy'] = "REM1";
                  return;
                }
            } else {
                $this->ret_array['new_buddy'] = "REM2";
                return;
            }


  			// add buddy to the buddy adders list
  			$insert = "INSERT IGNORE INTO hon_chat.buddy (account_id, buddy_id, status, create_time, expire_time) VALUES ('{$account_id}', '{$buddy_id}', '1', '{$create_time}', '{$expire_time}')";
  			if($result = mysql_query($insert, $l1) )
  			{
  				// make the entry to the buddy added list too
    			$insert = "INSERT IGNORE INTO hon_chat.buddy (account_id, buddy_id, status, create_time, expire_time) VALUES ('{$buddy_id}', '{$account_id}', '1', '{$create_time}', '{$expire_time}')";
  	    	    if($result = mysql_query($insert, $l1) )
  			    {
                    // create the notification entries
                    $account_notification = $this->simple_base_notify(22, $buddy_info['nickname'], "", "", $create_time, $expire_time, $l1);
                    $this->ret_array['account_notify_id'] = $account_notification['notify_id'];
                    $this->ret_array['account_params'] = $account_notification['params'];

                    $buddy_notification = $this->simple_base_notify(23, $account_info['nickname'], "", "", $create_time, $expire_time, $l1);
                    $this->ret_array['buddy_notify_id'] = $buddy_notification['notify_id'];
                    $this->ret_array['buddy_params'] = $buddy_notification['params'];

          			if ($this->ret_array['account_notify_id'] == 0 || $this->ret_array['buddy_notify_id'] == 0)
          			{  $this->ret_array['error'] = "Connect: Could not connect.";
          			} else {
        			    // add the notifications to the table
                        $this->simple_accounts_notify(array($account_id), $this->ret_array['account_notify_id'], $l1);
                        $this->simple_accounts_notify(array($buddy_id), $this->ret_array['buddy_notify_id'], $l1);

      			        $this->ret_array['new_buddy'] = "OK";
                    }
  			    } else { $this->ret_array['error'] = "Connect: Could not connect."; }
  			} else { $this->error['error'] = "Connect: Could not connect."; }
  	    }

		@mysql_close($l1);
    }

    ## APPROVE BUDDY ##
    function approve_buddy2($account_id, $buddynickname){
        include('inc/l1.php');
        $account_id = mysql_real_escape_string($account_id);
        $account_info = $this->oneid2onenick($account_id);
        $this->ret_array['account_id'] = $account_id;
        $this->ret_array['account_nickname'] = $account_info['nickname'];

        $buddy_info = $this->onenick2oneid($buddynickname);
        $buddy_id = $buddy_info['account_id'];
        $this->ret_array['buddy_id'] = $buddy_id;
        $this->ret_array['buddy_nickname'] = $buddy_info['nickname'];

        $create_time = mktime();
        $expire_time = $create_time + 1209600;  // 2 weeks

        if ($account_id == 0 || $buddy_id == 0)
        {
  			$this->ret_array['error'] = "Connect: Could not connect.";
        } else {
            // approve buddy for the buddy who was the added
            $sql = "UPDATE hon_chat.buddy SET status = 2, expire_time = NULL WHERE account_id = '{$account_id}' AND buddy_id = '{$buddy_id}' LIMIT 1;";
            if($result = mysql_query($sql, $l1) )
            {
                // approve the buddy who was the adder too
                $sql = "UPDATE hon_chat.buddy SET status = 2, expire_time = NULL WHERE account_id = '{$buddy_id}' AND buddy_id = '{$account_id}' LIMIT 1;";
                if($result = mysql_query($sql, $l1) )
                {
                    // create the notification entries
                    $account_notification = $this->simple_base_notify(2, $buddy_info['nickname'], "", "", $create_time, $expire_time, $l1);
                    $this->ret_array['account_notify_id'] = $account_notification['notify_id'];
                    $this->ret_array['account_params'] = $account_notification['params'];

                    $buddy_notification = $this->simple_base_notify(1, $account_info['nickname'], "", "", $create_time, $expire_time, $l1);
                    $this->ret_array['buddy_notify_id'] = $buddy_notification['notify_id'];
                    $this->ret_array['buddy_params'] = $buddy_notification['params'];

                    if ($this->ret_array['account_notify_id'] == 0 || $this->ret_array['buddy_notify_id'] == 0)
                    {  $this->ret_array['error'] = "Connect: Could not connect.";
                    } else {
                        // add the notifications to the table
                        $this->simple_accounts_notify(array($account_id), $this->ret_array['account_notify_id'], $l1);
                        $this->simple_accounts_notify(array($buddy_id), $this->ret_array['buddy_notify_id'], $l1);

                        $this->ret_array['approve_buddy'] = "OK";
                    }
                } else { $this->ret_array['error'] = "Connect: Could not connect."; }
            } else { $this->error['error'] = "Connect: Could not connect."; }
        }
        @mysql_close($l1);
    }

    ## ADD IGNORED USER ##
    function new_ignored($id, $ignored_id){
        include('inc/l1.php');
        $tmp[]=$this->cli_info['account_id'];  // for nick retrieval
        $tmp[]=$ignored_id;  // for buddy nick retrieval

        $id = mysql_real_escape_string($this->cli_info['account_id']);
        $ignored_id = mysql_real_escape_string($ignored_id);

        // add buddy to list
        $insert = "INSERT IGNORE INTO hon_chat.ignored (account_id, ignored_id) VALUES ('{$id}', '{$ignored_id}')";
        if($result = mysql_query($insert, $l1) )
        {   $this->ret_array['new_ignored'] = "OK";
        } else {$this->error['error'] = "Connect: Could not connect."; }
        @mysql_close($l1);
    }

    ## ADD NEW BANNED USER ##
    function new_banned($id, $banned_id, $reason){
        include('inc/l1.php');
        $tmp[]=$this->cli_info['account_id'];  // for nick retrieval
        $tmp[]=$banned_id;  // for buddy nick retrieval

        $id = mysql_real_escape_string($this->cli_info['account_id']);
        $banned_id = mysql_real_escape_string($banned_id);
        $reason = mysql_real_escape_string($reason);

        // add buddy to list
        $insert = "INSERT IGNORE INTO hon_chat.banned (account_id, banned_id, reason) VALUES ('{$id}', '{$banned_id}', '{$reason}')";
        if($result = mysql_query($insert, $l1) )
        {   $this->ret_array['new_banned'] = "OK";
        } else {$this->error['error'] = "Connect: Could not connect."; }
        @mysql_close($l1);
    }

    ## REMOVE BUDDY ##
    function remove_buddy($id, $buddy_id ){
        include('inc/l1.php');
        $tmp[]=$this->cli_info['account_id'];  // for nick retrieval
        $tmp[]=$buddy_id;

        $id = mysql_real_escape_string($this->cli_info['account_id']);
        $buddy_id = mysql_real_escape_string($buddy_id);

        // add buddy to list
        $insert = "DELETE FROM hon_chat.buddy WHERE account_id='{$id}' AND buddy_id='{$buddy_id}' LIMIT 1";
        if($result = mysql_query($insert, $l1) )
        {   $nicknames = $this->id2nick($tmp);
            // user notification
            $options[0] = $this->cli_info['account_id'];
            $options[1] = @$nicknames[$buddy_id];
            $this->notification(1, 'REM', $options);
            // target notification
            $options[0] = $buddy_id;
            $options[1] = @$nicknames[$id];
            $this->notification(2, 'B_REM', $options);

            $this->ret_array['remove_buddy'] = "OK";
        } else {$this->error['error'] = "Connect: Could not connect."; }
        @mysql_close($l1);
    }

    ## REMOVE BUDDY ##
    function remove_buddy2($account_id, $buddy_id ){
        include('inc/l1.php');

        $account_id = mysql_real_escape_string($this->cli_info['account_id']);
        $buddy_id = mysql_real_escape_string($buddy_id);

        // remove buddy from BOTH lists
        $insert = "DELETE FROM hon_chat.buddy WHERE account_id='{$account_id}' AND buddy_id='{$buddy_id}' LIMIT 1";
        if($result = mysql_query($insert, $l1) )
        {
            $insert = "DELETE FROM hon_chat.buddy WHERE account_id='{$buddy_id}' AND buddy_id='{$account_id}' LIMIT 1";
            if($result = mysql_query($insert, $l1) )
            {
                $this->ret_array['remove_buddy'] = "OK";
            } else { $this->error['error'] = "Connect: Could not connect."; }
        } else {$this->error['error'] = "Connect: Could not connect."; }
        @mysql_close($l1);
    }

    ## REMOVE IGNORED USER ##
    function remove_ignored($id, $ignored_id ){
        include('inc/l1.php');

        $id = mysql_real_escape_string($id);
        $ignored_id = mysql_real_escape_string($ignored_id);

        // add buddy to list
        $sql = "DELETE FROM hon_chat.ignored WHERE account_id='{$id}' AND ignored_id='{$ignored_id}' LIMIT 1;";
        if($result = mysql_query($sql, $l1) )
        {   $this->ret_array['remove_ignored'] = "OK";
        } else {$this->error['error'] = "Connect: Could not connect."; }
        @mysql_close($l1);
    }
    ## REMOVE BANNED USER ##
    function remove_banned($id, $banned_id ){
        include('inc/l1.php');
        $tmp[]=$this->cli_info['account_id'];  // for nick retrieval
        $tmp[]=$banned_id;

        $id = mysql_real_escape_string($this->cli_info['account_id']);
        $banned_id = mysql_real_escape_string($banned_id);

        // remove from banned list
        $delete = "DELETE FROM hon_chat.banned WHERE account_id='{$id}' AND banned_id='{$banned_id}' LIMIT 1";
        if($result = mysql_query($delete, $l1) )
        {   $this->ret_array['remove_banned'] = "OK";
        } else {$this->error['error'] = "Connect: Could not connect."; }
        @mysql_close($l1);
    }

    ## RETRIEVE NICKNAMES FROM IDS ##
    function id2nick($account_id){
        include('inc/l2.php');
        $out = array();
        $ids = "";
        foreach($account_id as $k => $v)
        {$ids .= "account_id = '".mysql_real_escape_string($v)."' OR "; }
        $ids = trim($ids, 'OR ');
        $limit = sizeof($account_id);

        $query = "SELECT account_id, nickname FROM hon_accounts.account WHERE {$ids} LIMIT {$limit}";
        if($res = mysql_query($query, $l2) )
        {   while($row = mysql_fetch_assoc($res) )
            { $out[$row['account_id']]=$row['nickname'];  }
        } else {$this->error['error'] = "Connect: Could not connect."; }

        return $out;

        @mysql_close($l2);
    }

    ## RETRIEVE IDS FROM NICKNAMES ##
    function nick2id($nickname){
        include('inc/l2.php');
        $out = array();
        $nicks = "";
        foreach($nickname as $k => $v)
        {   $v = preg_replace('/\[(.*)\]/', '', $v);
		    $nicks .= "nickname = '".mysql_real_escape_string($v)."' OR ";
        }

        $limit = sizeof($nickname);
        $nicks = trim($nicks, 'OR ');
        $query = "SELECT account_id, nickname FROM hon_accounts.account WHERE {$nicks} LIMIT {$limit}";
        if($res = mysql_query($query, $l2) )
        {   while($row = mysql_fetch_assoc($res) )
            { $out[$row['nickname']]=$row['account_id'];  }
        } else {$this->error['error'] = "Connect: Could not connect."; }

        @mysql_close($l2);

        return $out;
    }

    ## GET ONE NICK FROM ID ##
    function oneid2onenick($account_id){
        include('inc/l2.php');
        $out = array();

        $account_id = mysql_real_escape_string($account_id);

        $query = "SELECT nickname FROM hon_accounts.account WHERE account_id = {$account_id} LIMIT 1";
        $res = mysql_query($query, $l2);
        if($row = mysql_fetch_assoc($res))
        { $out['nickname'] = $row['nickname'];
        } else { $this->error['error'] = "Connect: Could not connect."; }

        @mysql_close($l2);

        return $out;
    }

    ## GET ONE ID FROM NICK ##
    function onenick2oneid($nickname){
        include('inc/l2.php');
        $out = array();
        $nickname = mysql_real_escape_string($nickname);

        $query = "SELECT account_id, nickname FROM hon_accounts.account WHERE nickname = '{$nickname}' LIMIT 1";
        $res = mysql_query($query, $l2);
        if($row = mysql_fetch_assoc($res))
        {
          $out['account_id'] = $row['account_id'];
          $out['nickname'] = $row['nickname'];
        }
        else
        {
          $this->error['error'] = "Connect: Could not connect.";
        }

        @mysql_close($l2);

        return $out;
    }

    ## SET INITIAL RATINGS ##
    function set_rating($type, $rate){
        include("inc/l1.php");
        $account_id = mysql_real_escape_string($this->cli_info['account_id']);
        switch($type)
        {
            case "solo":
                $table = "ranked_stats";
                $col_pset = "rnk_amm_solo_pset";
                $col_rating = "rnk_amm_solo_rating";
            break;
            case "team":
                $table = "ranked_stats";
                $col_pset = "rnk_amm_team_pset";
                $col_rating = "rnk_amm_team_rating";
            break;
            case "pub":
                $table = "player_stats";
                $col_pset = "acc_pub_pset";
                $col_rating = "acc_pub_skill";
            break;
        }
        switch($rate)
        {
            case "0":
                $value = "1350";
            break;
            case "1":
                $value = "1500";
            break;
            case "2":
                $value = "1650";
            break;
        }

        // check if set
        $select = "SELECT {$col_pset} FROM hon_stats.{$table} WHERE account_id='{$account_id}' LIMIT 1";
        if($result = mysql_query($select, $l1) )
        {
            if(mysql_num_rows($result) == 1)
            {   $row = mysql_fetch_assoc($result);
                if(!$row[$col_pset])
                {   // update stats
                    $update = "UPDATE hon_stats.{$table} SET {$col_rating} = '{$value}', {$col_pset}=1 WHERE account_id='{$account_id}' LIMIT 1";
                    if(mysql_query($update, $l1) )
                    {$this->ret_array['pset']="OK"; } else {$this->error['pset'] = "Could not update."; } // update error
                } else {$this->error['pset'] = "Already set."; } // column already set
            } else {$this->error['pset'] = "Account not found."; } // no account found
        } else {$this->error['connect'] = "Could not connect. Try again."; } // connect error
        @mysql_close($l1);
    }

    ## LAST 40 MATCHES ##
    function grab_last_matches($aid){
        if(ereg('^[1-9][0-9]*$', $aid) ) {
            include('inc/ls.php');
            $aid = mysql_real_escape_string($aid);

          	// last matches
          	$select = "SELECT * FROM hon_stats.player_match_history WHERE account_id='{$aid}' LIMIT 1";
          	if($result = mysql_query($select, $ls) )
          	{
          		if( mysql_num_rows($result) > 0 )
          		{   $ms = array();
                    $hrow = mysql_fetch_assoc($result);
                    $hist = explode(',',$hrow['history']);
          			foreach($hist AS $k => $v)
          			{   $curr = explode('|', $v);
                        $ms[]=$curr[0];  // match id => date
          			}
                    if(sizeof($ms) > 0) {
                    $myMatches = "";
                    if(krsort($ms, SORT_NUMERIC)){
                        $i=1;
                        foreach($ms as $k => $mid){
                            if($i > 40) {break; }
                            $mid = mysql_real_escape_string($mid);
                            $myMatches .= "('{$mid}'),";
                            $i++;
                        } $myMatches = trim($myMatches, ",");
                    }
                    $create = "CREATE TEMPORARY TABLE hon_stats.matchList (match_id int(12) unsigned not null primary key) ENGINE=MEMORY";
                    @mysql_query($create, $ls);
                    $insert = "INSERT INTO hon_stats.matchList (match_id) VALUES {$myMatches}";
                    if(mysql_query($insert, $ls) )
                    {
                        $select = "SELECT match_stats.*, cli_name FROM hon_stats.matchList JOIN hon_stats.match_stats USING (match_id) JOIN hon_stats.hero USING (hero_id) WHERE account_id='{$aid}' ORDER BY matchList.match_id DESC LIMIT 40";
                        if($result = mysql_query($select, $ls) )
                        {
                            while($row = mysql_fetch_assoc($result) )
                            {$this->ret_array['last_stats'][$row['match_id']] = $row; }
                        } else {$this->ret_array['last_stats']['error']="Unable to retrieve match stats."; }

                    } else {$this->ret_array['last_stats']['error']="Unable to retrieve match stats."; }
                    $drop = "DROP TABLE hon_stats.matchList";
                    @mysql_query($drop, $ls);
                    } else {$this->ret_array['last_stats']['error']="Unable to retrieve match stats."; }
          		} else {$this->ret_array['last_stats']['error']="Unable to retrieve match stats."; }
          	} else {$this->ret_array['last_stats']['error']="Unable to retrieve match stats."; }

            @mysql_close($ls);
        } else {$this->ret_array['last_stats']['error']="Unable to retrieve match stats."; }
    }

    // match specific stats
    function grab_match_stats($aid, $mid){
        if(ereg('^[1-9][0-9]*$', $aid) AND ereg('^[1-9][0-9]*$', $mid) ) {
            include('inc/ls.php');
            $aid = mysql_real_escape_string($aid);
            $mid = mysql_real_escape_string($mid);
            $select = "SELECT match_stats.*, cli_name FROM hon_stats.match_stats JOIN hon_stats.hero USING (hero_id) WHERE account_id = '{$aid}' AND match_id='{$mid}'";
            if($result = mysql_query($select, $ls) )
            {   while($row = mysql_fetch_assoc($result) )
                { $this->ret_array['match_stats'][$row['match_id']] = $row; }
            } else {$this->ret_array['match_stats']['error']="Unable to retrieve match stats."; }
            @mysql_close($ls);
        } else {$this->ret_array['match_stats']['error']="Unable to retrieve match stats."; }
    }

    // initial login stats leaver% and level
    function get_initStats($id){
        include('inc/l2.php');
        $aid = mysql_real_escape_string($id);
        $select = "SELECT player_rank.account_id, level, acc_trial_games_played, acc_pub_skill, acc_games_played, acc_no_stats_played, rnk_games_played, acc_wins, acc_losses, acc_discos, rnk_discos, IFNULL((acc_discos+rnk_discos)/(acc_games_played+rnk_games_played),0) AS leaverpct FROM hon_stats.player_rank JOIN hon_stats.player_stats USING (account_id) JOIN hon_stats.ranked_stats USING (account_id) WHERE account_id = '{$aid}' LIMIT 1";

        if($result = mysql_query($select, $l2) )
        {
            while($row = mysql_fetch_assoc($result) )
            {
                $this->ret_array['infos'][$row['account_id']] = $row;
            }
        } else {$this->ret_array['infos']['error']="Unable to retrieve ranked stats."; }


        @mysql_close($l2);
    }

    // retrieve ranks
    function get_rank($ids){     //ids = account_id array
        include('inc/l2.php');

        $aids = "account_id = '". implode("' OR account_id = '", $ids) . "'";

        $select = "SELECT * FROM hon_stats.player_rank WHERE {$aids}";
        if($result = mysql_query($select, $l2) )
        {
            while($row = mysql_fetch_assoc($result) )
            {
                $this->ret_array['ranks'][$row['account_id']] = $row;
            }
        } else {$this->ret_array['ranks']['error']="Unable to retrieve ranked stats."; }

        @mysql_close($l2);
    }

    // retrieve stats
    function get_stats($ids){     //ids = account_id array
        include('inc/l2.php');
        $aids = "player_stats.account_id = '". implode("' OR player_stats.account_id = '", $ids) . "'";

        $select = "SELECT * FROM hon_stats.player_stats JOIN hon_stats.ranked_stats USING (account_id) WHERE {$aids}";
        if($result = mysql_query($select, $l2) )
        {
            while($row = mysql_fetch_assoc($result) )
            {
                $this->ret_array['stats'][$row['account_id']] = $row;
            }
        } else {$this->ret_array['stats']['error']="Unable to retrieve ranked stats."; }

        @mysql_close($l2);
    }

    // retrieve ranks and stats
    function get_field_stats($ids, $fields){     //ids = account_id array
        include('inc/l2.php');
        $this->ret_array['field_stats'] = array();

        $aids = "player_rank.account_id = '". implode("' OR player_rank.account_id = '", $ids)."'";

        $lookups = "player_rank.account_id";
        // valid fields
        $valids = array("nickname","acc_games_played","acc_wins","acc_losses","acc_concedes","acc_concedevotes","acc_buybacks","acc_discos","acc_kicked","acc_pub_skill","acc_pub_count","acc_pub_pset","acc_avg_score","acc_herokills","acc_herodmg","acc_heroexp","acc_herokillsgold","acc_heroassists","acc_deaths","acc_goldlost2death","acc_secs_dead","acc_teamcreepkills","acc_teamcreepdmg","acc_teamcreepexp","acc_teamcreepgold","acc_neutralcreepkills","acc_neutralcreepdmg","acc_neutralcreepexp","acc_neutralcreepgold","acc_bdmg","acc_bdmgexp","acc_razed","acc_bgold","acc_denies","acc_exp_denied","acc_gold","acc_gold_spent","acc_exp","acc_actions","acc_secs","acc_consumables","acc_wards","acc_em_played","acc_time_earning_exp","acc_level","AR","AREM","AP","APEM","rnk_games_played","rnk_wins","rnk_losses","rnk_concedes","rnk_concedevotes","rnk_buybacks","rnk_discos","rnk_kicked","rnk_amm_solo_rating","rnk_amm_solo_count","rnk_amm_solo_conf","rnk_amm_solo_prov","rnk_amm_solo_pset","rnk_amm_team_rating","rnk_amm_team_count","rnk_amm_team_conf","rnk_amm_team_prov","rnk_amm_team_pset","rnk_herokills","rnk_herodmg","rnk_heroexp","rnk_herokillsgold","rnk_heroassists","rnk_deaths","rnk_goldlost2death","rnk_secs_dead","rnk_teamcreepkills","rnk_teamcreepdmg","rnk_teamcreepexp","rnk_teamcreepgold","rnk_neutralcreepkills","rnk_neutralcreepdmg","rnk_neutralcreepexp","rnk_neutralcreepgold","rnk_bdmg","rnk_bdmgexp","rnk_razed","rnk_bgold","rnk_denies","rnk_exp_denied","rnk_gold","rnk_gold_spent","rnk_exp","rnk_actions","rnk_secs","rnk_consumables","rnk_wards","rnk_em_played","rnk_level","rnk_time_earning_exp");
        foreach($fields AS $k => $v)
        {   if(in_array($v, $valids) )
            {$lookups .= ", ".$v; }
        }

        $select = "SELECT {$lookups} FROM hon_stats.player_rank JOIN hon_stats.player_stats USING (account_id) JOIN hon_stats.ranked_stats USING (account_id) JOIN hon_accounts.account USING (account_id) JOIN hon_accounts.account_info USING (account_id) JOIN hon_tracking.in_game USING (account_id) JOIN hon_stats.level USING (level) WHERE {$aids}";
        if($result = mysql_query($select, $l2) )
        {   while($row = mysql_fetch_assoc($result) )
            {
                $this->ret_array['field_stats'][$row['account_id']] = $row;
            }
        } else {$this->ret_array['field_stats']['error']="Unable to retrieve ranked stats."; }

        @mysql_close($l2);
    }

    // retrieve match stats
    function get_match_stats($ids){     //ids = account_id array
        include('inc/ls.php');

	    $limit = sizeof($ids);

        // ids is already clean ^[1-9][0-9]*$
        $select = "SELECT match_summ.*, file_loc.url, file_loc.size, DATE_FORMAT(mdt, '%c/%e/%Y') AS date, DATE_FORMAT(mdt, '%r') AS time, info.name, match_type.*, match_options.* FROM hon_stats.match_summ JOIN hon_stats.match_type USING (match_id) JOIN hon_stats.match_options USING (match_id) LEFT JOIN hon_replays.file_loc USING (match_id) LEFT JOIN hon_server.info ON (match_summ.server_id=info.server_id) WHERE match_summ.match_id = '". implode("' OR match_summ.match_id = '", $ids) . "' LIMIT {$limit}";
        if($result = mysql_query($select, $ls) )
        {
            while($row = mysql_fetch_assoc($result) )
            {
                $this->ret_array['match_summ'][$row['match_id']] = $row;
            }
        } else {$this->ret_array['match_summ']['error']="Unable to retrieve match summ."; }

        $select = "SELECT match_stats.*, nickname, hero.cli_name FROM hon_stats.match_stats JOIN hon_accounts.account USING (account_id) JOIN hon_stats.hero ON (match_stats.hero_id = hero.hero_id) WHERE match_id = '". implode("' OR match_id = '", $ids) . "'";
        if($result = mysql_query($select, $ls) )
        {
            while($row = mysql_fetch_assoc($result) )
            {
                $this->ret_array['match_player_stats'][$row['match_id']][$row['account_id']] = $row;
            }
        } else {$this->ret_array['match_player_stats']['error']="Unable to retrieve match stats."; }

        $select = "SELECT * FROM hon_stats.team_stats WHERE match_id = '". implode("' OR match_id = '", $ids) . "'";
        if($result = mysql_query($select, $ls) )
        {
            while($row = mysql_fetch_assoc($result) )
            {
                $this->ret_array['team_summ'][$row['match_id']][$row['team']] = $row;
            }
        } else {$this->ret_array['team_summ']['error']="Unable to retrieve team stats."; }

        $select = "(SELECT match_inventory.account_id, match_inventory.match_id, cli_name AS slot_1, null AS slot_2, null AS slot_3, null AS slot_4, null AS slot_5, null AS slot_6 FROM hon_stats.match_inventory LEFT JOIN hon_stats.items ON (slot_1 = item_id) WHERE match_id = '". implode("' OR match_id = '", $ids) . "') UNION (SELECT match_inventory.account_id, match_inventory.match_id, null AS slot_1, cli_name AS slot_2, null AS slot_3, null AS slot_4, null AS slot_5, null AS slot_6 FROM hon_stats.match_inventory LEFT JOIN hon_stats.items ON (slot_2 = item_id) WHERE match_id = '". implode("' OR match_id = '", $ids) . "') UNION (SELECT match_inventory.account_id, match_inventory.match_id, null AS slot_1, null AS slot_2, cli_name AS slot_3, null AS slot_4, null AS slot_5, null AS slot_6 FROM hon_stats.match_inventory LEFT JOIN hon_stats.items ON (slot_3 = item_id) WHERE match_id = '". implode("' OR match_id = '", $ids) . "') UNION (SELECT match_inventory.account_id, match_inventory.match_id, null AS slot_1, null AS slot_2, null AS slot_3, cli_name AS slot_4, null AS slot_5, null AS slot_6 FROM hon_stats.match_inventory LEFT JOIN hon_stats.items ON (slot_4 = item_id) WHERE match_id = '". implode("' OR match_id = '", $ids) . "') UNION (SELECT match_inventory.account_id, match_inventory.match_id, null AS slot_1, null AS slot_2, null AS slot_3, null AS slot_4, cli_name AS slot_5, null AS slot_6 FROM hon_stats.match_inventory LEFT JOIN hon_stats.items ON (slot_5 = item_id) WHERE match_id = '". implode("' OR match_id = '", $ids) . "') UNION (SELECT match_inventory.account_id, match_inventory.match_id, null AS slot_1, null AS slot_2, null AS slot_3, null AS slot_4, null AS slot_5, cli_name AS slot_6 FROM hon_stats.match_inventory LEFT JOIN hon_stats.items ON (slot_6 = item_id) WHERE match_id = '". implode("' OR match_id = '", $ids) . "')";
        if($result = mysql_query($select, $ls) )
        {	$slots = array("slot_1", "slot_2", "slot_3", "slot_4", "slot_5", "slot_6");
            	while($row = mysql_fetch_assoc($result) )
            	{
                if(isset($this->ret_array['inventory'][$row['match_id']][$row['account_id']])){
			    foreach($slots AS $x){
			    if(!is_null($row[$x]) ){
				    $this->ret_array['inventory'][$row['match_id']][$row['account_id']][$x] = $row[$x];
			    }}
                } else {$this->ret_array['inventory'][$row['match_id']][$row['account_id']] = $row;}
            	}
        } else {$this->ret_array['inventory']['error']="Unable to retrieve match stats."; }
        @mysql_close($ls);
    }

    // retrieve ranks and stats
    function get_all_stats($ids){     //ids = account_id array
        include('inc/l2.php');
        $this->ret_array['all_stats'] = array();
        $this->ret_array['last_match'] = array();
        $this->ret_array['clan_info'] = array();
	    $limit = sizeof($ids);
        $aids = "player_rank.account_id = '". implode("' OR player_rank.account_id = '", $ids)."'";
        $mids = "match_stats.account_id = '".implode("' OR match_stats.account_id='", $ids)."'";
        $cids = "roster.account_id = '".implode("' OR roster.account_id='", $ids)."'";

        $select = "SELECT player_rank.*, player_stats.*, ranked_stats.*, minXP, maxXP, DATE_FORMAT(last_login, '%m/%d/%Y %T') AS last_login, DATE_FORMAT(last_activity, '%m/%d/%Y %T') AS last_activity, nickname, DATE_FORMAT(create_date, '%m/%d/%Y') AS create_date FROM hon_stats.player_rank JOIN hon_stats.player_stats USING (account_id) JOIN hon_stats.ranked_stats USING (account_id) JOIN hon_accounts.account USING (account_id) JOIN hon_accounts.account_info USING (account_id) JOIN hon_tracking.in_game USING (account_id) JOIN hon_stats.level USING (level) WHERE {$aids} LIMIT {$limit}";
        if($result = mysql_query($select, $l2) )
        {   while($row = mysql_fetch_assoc($result) )
            {
                $this->ret_array['all_stats'][$row['account_id']] = $row;
            }
        } else {$this->ret_array['all_stats']['error']="Unable to retrieve ranked stats."; }

        @mysql_close($l2);
    }

    ## CHECK CLAN NAME ##
    function clan_nameCheck($clan_name, $tag){
        include('inc/l2.php');
        $clan_name = ltrim(rtrim($clan_name) );
        if(ereg('^[a-zA-Z0-9 ]{1,32}$', $clan_name) )
        {   $clan_name = mysql_real_escape_string($clan_name);
            if(ereg('^[a-zA-Z0-9]{1,4}$', $tag) )
            {   $tag = mysql_real_escape_string($tag);
                $select = "SELECT name, tag FROM hon_clans.clan_info WHERE name='{$clan_name}' OR tag='{$tag}'";
                if($result = mysql_query($select, $l2) )
                {
                    if(mysql_num_rows($result) == 0)
    		        { $this->ret_array['clan_check']="OK";
    		        } else {
                        while($row = mysql_fetch_assoc($result) )
                        {   if($row['name'] == $clan_name){$this->error['clan_name'] = "Clan name already taken."; }
                            if($row['tag'] == $tag){$this->error['clan_tag'] = "Clan tag already taken.";}
                        }
                    }
                } else {$this->error['error'] = "Connect: Could not connect."; }
            } else {$this->error['clan_tag']= "Invalid clan tag format. 1-4 alphanum chars."; }
        } else {$this->error['clan_name']= "Invalid clan name format. 1-32 alphanum or spaces chars."; }
        @mysql_close($l2);
    }

    ## GET CLAN INFO ##
    function clan_infoByAccount($account_id){
	    include('inc/l2.php');
        $account_id = mysql_real_escape_string($account_id);
	    // clan info
        $select = "SELECT roster.*, name, tag FROM hon_clans.roster JOIN hon_clans.clan_info USING (clan_id) WHERE roster.account_id='{$account_id}'";
        if($result = mysql_query($select, $l2) )
        {   if(mysql_num_rows($result) == 0){ $this->ret_array['clan_member_info']['error']="No clan members found."; }
            while($row = mysql_fetch_assoc($result) )
            {  $this->ret_array['clan_member_info']=$row;  }
        } else {$this->error['error'] = "Connect: Could not connect."; }
        @mysql_close($l2);
    }

    ## GET CLAN ROSTER ##
    function clan_rosterByClan_id($clan_id){
	    if(ereg('^[1-9][0-9]*$', $clan_id) ){
            include('inc/l2.php');
            $clan_id = mysql_real_escape_string($clan_id);

            // clan info
            $select = "SELECT roster.*, nickname FROM hon_clans.roster JOIN hon_accounts.account USING (account_id) WHERE roster.clan_id = '{$clan_id}'";
            if($result = mysql_query($select, $l2) )
            {   if(mysql_num_rows($result) == 0){ $this->ret_array['clan_roster']['error']="No clan members found."; }
                while($row = mysql_fetch_assoc($result) )
                {  $this->ret_array['clan_roster'][$row['account_id']]=$row;}
            } else {$this->error['error'] = "Connect: Could not connect."; }
            @mysql_close($l2);
	    }

    }

    ## CREATE NEW CLAN ##
    function create_clan($clan_name, $tag, $member1, $member2, $member3, $member4){
        include("inc/l1.php");

        $clan_name = ltrim(rtrim($clan_name) );
        if(ereg('^[a-zA-Z0-9 ]{1,32}$', $clan_name) )
        {   $clan_name = mysql_real_escape_string($clan_name);
        } else {$this->error['clan_name']= "Invalid clan name format. 1-32 alphanum or spaces chars.";}
        if(ereg('^[a-zA-Z0-9]{1,4}$', $tag) )
        {   $tag = mysql_real_escape_string($tag);
        } else {$this->error['tag']= "Invalid clan tag format. 1-4 alphanum chars.";}
        if(ereg('^.+$', $this->cli_info['account_id']) )
        {   $leader = mysql_real_escape_string($this->cli_info['account_id']);
        } else {$this->error['leader']= "Must provide a leader.";}
        if($member1 != 0 )
        {   $member1 = mysql_real_escape_string($member1);
        } else {$this->error['member1']= "Invalid Cookie."; }
        if($member2 != 0 )
        {   $member2 = mysql_real_escape_string($member2);
        } else {$this->error['member2']= "Invalid Cookie."; }
        if($member3 != 0 )
        {   $member13 = mysql_real_escape_string($member3);
        } else {$this->error['member3']= "Invalid Cookie."; }
        if($member4 != 0 )
        {   $member4 = mysql_real_escape_string($member4);
        } else {$this->error['member4']= "Invalid Cookie."; }

        // post info error check
        if(sizeof($this->error) == 0 )
        {   // leader info
            $select = "SELECT account_id, clan_id, name, nickname, rank FROM hon_clans.clan_info JOIN hon_clans.roster USING(clan_id) RIGHT JOIN hon_accounts.account USING (account_id) WHERE account_id='{$leader}'";
            if($result = mysql_query($select, $l1) )
            {   // check nickname
                if(mysql_num_rows($result) == 0)
                {   $this->error['leader'] = "Nickname does not exist.";
                } else {

                    $row = mysql_fetch_assoc($result);
                    // clan check
                    if(!is_null($row['clan_id']) ) {
                        $this->error['leader'] = "{$row['nickname']} is already in a clan: {$row['name']}.";

                    } else {
                        // clan info query
                        $select = "SELECT clan_id, name, tag FROM hon_clans.clan_info WHERE name='{$clan_name}' OR tag='{$tag}'";
                        if($result = mysql_query($select, $l1) )
                        {   // CLAN INFO TAKEN
                            if(mysql_num_rows($result) > 0)
                            {   while($row = mysql_fetch_assoc($result) )
                                {   // clan_name and tag check
                                    if($clan_name == $row['name'])
                                    {   $this->error['clan_name']="Clan name is already taken.";
                                    } if($tag == $row['tag']) {
                                        $this->error['tag']="Tag is already taken.";
                                    }
                                }
                            } else {
                                // add clan
                                $insert = "INSERT INTO hon_clans.clan_info (name, tag, creator, active) VALUES ('{$clan_name}', '{$tag}', '{$row['account_id']}', 1)";
                                if($result = mysql_query($insert, $l1) )
                                {
                                    $clan_id = mysql_insert_id($l1);
                                    $this->ret_array['clan_id'] = $clan_id;

                                    $clan_id = mysql_real_escape_string($clan_id);
                                    // add leader
                                    $insert = "INSERT INTO hon_clans.roster (account_id, clan_id, rank, join_date) VALUES ('{$leader}', '{$clan_id}', 'Leader', NOW()), ('{$member1}', '{$clan_id}', 'Member', NOW()), ('{$member2}', '{$clan_id}', 'Member', NOW()), ('{$member3}', '{$clan_id}', 'Member', NOW()), ('{$member4}', '{$clan_id}', 'Member', NOW())";
                                    if($result = mysql_query($insert, $l1) )
                                    {   $this->ret_array['create_clan'] = "OK";
										$forumChange1 = file_get_contents('http://www.heroesofnewerth.com/forumTagUpdate.php?account_id='.$member1);
										$forumChange2 = file_get_contents('http://www.heroesofnewerth.com/forumTagUpdate.php?account_id='.$member2);
										$forumChange3 = file_get_contents('http://www.heroesofnewerth.com/forumTagUpdate.php?account_id='.$member3);
										$forumChange4 = file_get_contents('http://www.heroesofnewerth.com/forumTagUpdate.php?account_id='.$member4);
                                    } else { $this->error['connect']="Connection error. Leader not set."; }
                                } else { $this->error['connect']="Connection error. Try again."; }
                            }
                        } else { $this->error['connect']="Connection error. Try again."; }
                    }
                }
            } else { $this->error['connect']="Connection error. Try again."; }
        }
        @mysql_close($l1);
    }

    ## ADD CLAN MEMBER ##
    function add_member($member_id, $clan_id){
        include("inc/l1.php");
        $member_id = mysql_real_escape_string($member_id);
        $clan_id = mysql_real_escape_string($clan_id);

        // check if already clan member
        $select = "SELECT * FROM hon_clans.roster WHERE account_id='{$member_id}' LIMIT 1";
        if($result = mysql_query($select, $l1) )
        {   if(mysql_num_rows($result) == 0)
            {   // add member
                $insert = "INSERT INTO hon_clans.roster (account_id, clan_id, rank, join_date) VALUES ('{$member_id}', '{$clan_id}', 'Member', NOW()) ON DUPLICATE KEY UPDATE clan_id=VALUES(clan_id), rank=VALUES(rank), join_date=VALUES(join_date)";
                if($result = mysql_query($insert, $l1) )
                {
                    $this->ret_array['add_member'][$member_id] = "Member added";

                    // grab tag
                    $select = "SELECT name, tag FROM hon_clans.clan_info WHERE clan_id='{$clan_id}' LIMIT 1";
                    if($result = mysql_query($select, $l1) )
                    {   $frow = mysql_fetch_assoc($result);
                        $tag = mysql_real_escape_string($frow['tag']);
                        $name = mysql_real_escape_string($frow['name']);

                        // add event
                        $memberArray = array($member_id);
                        $nick = $this->id2nick($memberArray);
                        $insertComment = "INSERT INTO hon_clans.message_text (msg) VALUES ('{$nick[$member_id]} has joined clan {$frow['name']}!')";
                        $doInsertComment = mysql_query($insertComment,$l1);
                        $GetLast = "SELECT LAST_INSERT_ID() AS lastId FROM hon_clans.message_text LIMIT 1";
                        $doGetLast = mysql_query($GetLast,$l1);

                        $last = mysql_fetch_assoc($doGetLast);
                        $insertInfo = "INSERT INTO hon_clans.clan_events (clan_id, creator, class, message_id, create_date) VALUES ('{$clan_id}', '45691', 'joining', '{$last['lastId']}', NOW())";
                        $result = mysql_query($insertInfo, $l1);
                        //include("inc/l16.php");
						$forumChange = file_get_contents('http://www.heroesofnewerth.com/forumTagUpdate.php?account_id='.$member_id);
                        //$forum_query = "UPDATE hon_vb.userfield SET field5 = '{$tag}' WHERE userid = '{$member_id}' LIMIT 1";
                        //@mysql_query($forum_query, $l16);

                    } else { $this->error['connect']="Connection failed. Try again.1"; }
                } else { $this->error['connect']="Connection failed. Try again."; }
            } else { $this->error['member']="Member is already in a clan."; }
        } else { $this->error['connect']="Connection failed. Try again."; }
        if(isset($l16)){ mysql_close($l16); }
        @mysql_close($l1);
    }

    ## RANK/REMOVE CLAN MEMBER ##
    function set_rank($account_id, $target_id, $clan_id, $set_rank){
        if(ereg("^[1-9][0-9]*$", $account_id) AND ereg("^[1-9][0-9]*$", $target_id)) {
        include("inc/l1.php");
        include("inc/l2.php");
        $account_id = mysql_real_escape_string($account_id);
        $target_id = mysql_real_escape_string($target_id);
        $clan_id = mysql_real_escape_string($clan_id);
        $users = array();
        // check permissions
        $select = "SELECT * FROM hon_clans.roster WHERE clan_id='{$clan_id}'AND (account_id='{$account_id}' OR account_id='{$target_id}') LIMIT 2";
        if($result = mysql_query($select, $l2) )
        {
            if(mysql_num_rows($result) == 2 OR $account_id == $target_id) {
            while($row = mysql_fetch_assoc($result) )
            {$users[$row['account_id']] = $row['rank']; }
            if($users[$account_id] == 'Leader') {
                $rank = mysql_real_escape_string($set_rank);
            } elseif($users[$account_id] == 'Officer') {
                if(($set_rank == 'Member' OR $set_rank == 'Remove') AND ($users[$target_id] != 'Officer' OR $users[$target_id] != 'Leader') )
                {$rank = mysql_real_escape_string($set_rank); }
            } elseif($account_id == $target_id AND $set_rank == 'Remove'){
                {$rank = mysql_real_escape_string($set_rank); }
            }
            switch($rank)
            {
                case "Remove":
                    $query = "DELETE FROM hon_clans.roster WHERE account_id='{$target_id}' AND clan_id='{$clan_id}' LIMIT 1";
                    $forum_query = "UPDATE hon_vb.userfield SET field5 = '' WHERE userid = '{$target_id}' LIMIT 1";
                    // event
                    $select = "SELECT name FROM hon_clans.clan_info WHERE clan_id='{$clan_id}' LIMIT 1";
                    if($result = mysql_query($select, $l1) )
                    {   $frow = mysql_fetch_assoc($result);

                        $nickArray = array($target_id);
                        $nick = $this->id2nick($nickArray);
                        $insertComment = "INSERT INTO hon_clans.message_text (msg) VALUES ('{$nick[$target_id]} has left clan {$frow['name']}')";
                        $doInsertComment = mysql_query($insertComment);
                        $GetLast = "SELECT LAST_INSERT_ID() AS lastId FROM hon_clans.message_text LIMIT 1";
                        $doGetLast = mysql_query($GetLast);
                        $last = mysql_fetch_assoc($doGetLast);

                        $insertInfo = "INSERT INTO hon_clans.clan_events (clan_id, creator, class, message_id, create_date) VALUES ('{$clan_id}', '45691', 'leaving', '{$last['lastId']}', NOW())";
                        @mysql_query($insertInfo, $l1);
                    }


                break;

                case "Member":
                case "Officer":
                case "Leader":
                    $query = "INSERT INTO hon_clans.roster (account_id, clan_id, rank, join_date) VALUES ('{$target_id}', '{$clan_id}', '{$rank}', NOW()) ON DUPLICATE KEY UPDATE clan_id=VALUES(clan_id), rank=VALUES(rank)";
                break;

            }
            if(isset($query) )
            {   if($result = mysql_query($query, $l1) )
                {
                    $this->ret_array['set_rank'] = "Member updated.";
					$forumChange = file_get_contents('http://www.heroesofnewerth.com/forumTagUpdate.php?account_id='.$target_id);
                } else { $this->error['roster']['connect']="Connection failed. Try again."; }
            } else { $this->error['rank'] = "Invalid options given."; }

            } else { $this->error['clan'] = "Both users must be of the same clan.{$select}"; }
        } else { $this->error['roster']['connect']="Connection failed. Try again."; }
        @mysql_close($l1);
        @mysql_close($l2);
        } else { $this->error['account'] = "Invalid account given."; }
    }

    ## LOGOUT ##
    function logout(){
        include('inc/l1.php');
        $account_id = mysql_real_escape_string($this->cli_info['account_id']);
        $update = "UPDATE hon_tracking.in_game SET cookie='' WHERE account_id='{$account_id}' LIMIT 1";
        if($result = mysql_query($update, $l1) )
        { $this->ret_array['client_disco'] = "OK";
        } else {$this->error['error'] = "Connect: Could not connect."; }
        @mysql_close($l1);
    }

    ## AUTOCOMPLETE NICKNAMES ##
    function autocompleteNicks($nickname)
    {
        if(ereg("^[a-zA-Z0-9\`\_]{3,12}$", $nickname)) {
        include("inc/l2.php");
	      $nickname = str_replace('_', '\_', $nickname);
        $nickname = mysql_real_escape_string($nickname);
        $select = "SELECT nickname FROM hon_accounts.account WHERE nickname LIKE '{$nickname}%' ORDER BY nickname ASC LIMIT 100";
        if($result = mysql_query($select, $l2))
        {   while($row = mysql_fetch_assoc($result))
            {
                $this->ret_array['nicks'][] = $row['nickname'];
            }
        } else {$this->error['error'] = "FALSE"; }
        @mysql_close($l2);
        } else {$this->error['error'] = "FALSE"; }
    }

    ## ANOTHER AUTOCOMPLETE NICKNICKNAMES ##
    function autocompleteOnlineNicks($nickname)
    {
        if(ereg("^[a-zA-Z0-9\`\_]{3,12}$", $nickname)) {
        include("inc/l2.php");
	      $nickname = str_replace('_', '\_', $nickname);
        $nickname = mysql_real_escape_string($nickname);
        // only grab accounts active in the last 60 minutes
        $activity_threshold  = date("Y-m-d H:i:s", mktime(date("H"), date("i") - 60, date("s"), date("m"), date("d"), date("Y")));
        $select = "SELECT hon_accounts.account.nickname FROM hon_accounts.account
                    JOIN hon_tracking.in_game
                    ON hon_accounts.account.account_id = hon_tracking.in_game.account_id
                    WHERE hon_accounts.account.nickname LIKE '{$nickname}%'
                    AND hon_tracking.in_game.last_activity > '$activity_threshold'
                    ORDER BY nickname ASC LIMIT 100;";

        if($result = mysql_query($select, $l2))
        {   while($row = mysql_fetch_assoc($result))
            {
                $this->ret_array['nicks'][] = $row['nickname'];
            }
        } else {$this->error['error'] = "FALSE"; }
        @mysql_close($l2);
        } else {$this->error['error'] = "FALSE"; }
    }

    ## CURRENT GAME TIME ##
    function get_cgt($match_id, $nickname) {
        include('inc/l2.php');

        $match_id = mysql_real_escape_string($match_id);
        $nickname = mysql_real_escape_string($nickname);

        // if the match_summ.c_state = 4, the match is still in progress
        $select = "SELECT hon_server.info.cgt, hon_stats.match_type.mname AS matchname, hon_server.info.c_state AS gamestate FROM hon_server.info JOIN hon_stats.match_summ ON hon_server.info.server_id = hon_stats.match_summ.server_id JOIN hon_stats.match_type ON hon_stats.match_summ.match_id = hon_stats.match_type.match_id WHERE hon_stats.match_summ.match_id = '{$match_id}' AND hon_stats.match_summ.c_state = 4 LIMIT 1";

        if($result = mysql_query($select, $l2))
        {
          if($row = mysql_fetch_assoc($result))
          {
            // 2 = in game lobby, 3 = in game
            if(strcasecmp($row['gamestate'], "2") == 0){
                $this->ret_array['cgt'] = "Lobby";
            } else { $this->ret_array['cgt'] = $row['cgt']; }

            $this->ret_array['matchname'] = $row['matchname'];
            $this->ret_array['nickname'] = $nickname;
          } else {  // match_summ.c_state = 5, the match is finished
            $this->ret_array['cgt'] = "Finished";
          }
        } else { $this->error['connect']="Could not get cgt."; }
        @mysql_close($l2);
    }

    ## HERO USAGE ##
    function get_hero_usage(){
        include("inc/l2.php");

        $select = "SELECT * FROM hon_web.hero_ms_usage LIMIT 1";
        if($result = mysql_query($select, $l2) )
        {   $hu = mysql_fetch_assoc($result);
            $hero_usage = unserialize($hu['hero_usage']);
            $this->ret_array['total'] = $hu['total'];
        } else {$this->error['error'] = "Connect: Could not connect."; }

        $select = "SELECT hero_id, cli_name FROM hon_stats.hero WHERE active=1";
        if($result = mysql_query($select, $l2) )
        {   while($row = mysql_fetch_assoc($result) )
            {
                 $this->ret_array['hero_usage'][$row['cli_name']] = $hero_usage[$row['hero_id']];
            }
        } else {$this->error['error'] = "Connect: Could not connect."; }

        @mysql_close($l2);
    }

    ## LAST LOGIN ##
    function get_last_login($account_id) {
      include('inc/l2.php');
      $account_id = mysql_real_escape_string($account_id);

      $select = "SELECT last_login FROM hon_tracking.in_game WHERE account_id = '{$account_id}' LIMIT 1";
      if($result = mysql_query($select, $l2))
      {     if($row = mysql_fetch_assoc($result))
            { $this->ret_array['last_login'] = $row['last_login'];
            } else { $this->ret_array['last_login'] = "N/A"; }
      } else { $this->error['connect'] = "Could not get last login time."; }
      @mysql_close($l2);
    }

    ## GET ROOMS ##
    function grab_rooms($account_id) {
        if(ereg('^[1-9][0-9]*$', $account_id) ) {
            include('inc/l2.php');
            $account_id = mysql_real_escape_string($account_id);

            $select = "SELECT * FROM hon_chat.autojoin WHERE account_id = '{$account_id}'";
            if($result = mysql_query($select, $l2))
            {   if(mysql_num_rows($result) > 0)
                {   while($row = mysql_fetch_assoc($result))
                    { $this->ret_array['chatrooms'][$row['autojoin_id']] = $row['channel_name']; }
                } else { $this->ret_array['chatrooms'] = "N/A"; }
            } else { $this->error['connect'] = "Connect: Could not connect."; }

            @mysql_close($l2);
        } else {$this->error['chatrooms'] = "Invalid account id."; }
    }

    ## ADD ROOM ##
    function add_room($account_id, $room) {
        $this->grab_rooms($account_id);
        if( sizeof($this->ret_array['chatrooms']) < 9) {
            unset($this->ret_array['chatrooms']);
            if(ereg('^[1-9][0-9]*$', $account_id) ) {
                include('inc/l1.php');
                $account_id = mysql_real_escape_string($account_id);
                $room = mysql_real_escape_string($room);

                $insert = "INSERT IGNORE INTO hon_chat.autojoin (account_id, channel_name) VALUES ('{$account_id}','{$room}')";
                if($result = mysql_query($insert, $l1) )
                {   if (mysql_affected_rows() > 0){
                        $this->ret_array['add_room'] = "OK";
                    } else { $this->ret_array['add_room'] = "NONE"; }
                } else { $this->error['error'] = "Connect: Could not connect."; }

                @mysql_close($l1);
            } else { $this->error['add_room'] = "Invalid account id."; }
        } else { $this->error['add_room'] = "Room limit (8) exceeded."; }
    }

    ## REMOVE ROOM ##
    function remove_room($account_id, $room) {
        if(ereg('^[1-9][0-9]*$', $account_id) ) {
            include('inc/l1.php');
            $account_id = mysql_real_escape_string($account_id);
            $room = mysql_real_escape_string($room);

            $delete = "DELETE FROM hon_chat.autojoin WHERE account_id='{$account_id}' AND channel_name='{$room}' LIMIT 1";
            if($result = mysql_query($delete, $l1))
            {
              if (mysql_affected_rows() > 0)
                $this->ret_array['remove_room'] = "OK";
              else
                $this->ret_array['remove_room'] = "NONE";
            } else { $this->error['error'] = "Connect: Could not connect."; }

            @mysql_close($l1);
        } else { $this->error['remove_room'] = "Invalid account id."; }
    }

    ## CLEAR ALL ROOMS ##
    function clear_rooms($account_id) {
        if(ereg('^[1-9][0-9]*$', $account_id) ) {
            include('inc/l1.php');
            $account_id = mysql_real_escape_string($account_id);

            $delete = "DELETE FROM hon_chat.autojoin WHERE account_id='{$account_id}'";
            if($result = mysql_query($delete, $l1) )
            {
              if (mysql_affected_rows() > 0)
                $this->ret_array['clear_rooms'] = "OK";
              else
                $this->ret_array['clear_rooms'] = "NONE";
            } else {$this->error['error'] = "Connect: Could not connect."; }

            @mysql_close($l1);
        } else {$this->error['clear_rooms'] = "Invalid account id."; }
    }



    // BEGIN NEW NOTIFICATION STUFF
    ## CREATE NOTIFICATION ##
    function create_notification($type, $account_ids, $params) {
    	$bads = array(20, 11, 21, 15);
    	if(!in_array($type, $bads) ) {
            include('inc/l1.php');
            include('notification_class.php');
            $this->notify = new Notification($type);
            $this->notify->base_notify($params['notification'], $l1);
            if($this->notify->accounts_notify($account_ids, $l1))
            {
              $this->ret_array['notify_id'] = $this->notify->notify_id;
              $this->ret_array['notification'] = $params['notification'];
              $this->ret_array['status'] = "OK";
            }
            else
            {
              $this->ret_array['status'] = "FAILED";
            }
            @mysql_close($l1);
    	}
    }

    ## GET NOTIFICATIONS ##
    function grab_notifications($account_id) {
        if(ereg('^[1-9][0-9]*$', $account_id) ) {
            include('inc/l2.php');
            $account_id = mysql_real_escape_string($account_id);

            $select = "SELECT hon_chat.notify.notify_id, hon_chat.notify.data FROM hon_chat.notify_account JOIN hon_chat.notify ON hon_chat.notify_account.notify_id = hon_chat.notify.notify_id WHERE hon_chat.notify_account.account_id = '{$account_id}' LIMIT 25";
            if($result = mysql_query($select, $l2))
            {
                if(mysql_num_rows($result) > 0)
                {
                    $index = 0;
                    while($row = mysql_fetch_assoc($result))
                    {
                      $this->ret_array['notifications'][$index]['notification'] = $row['data'];
                      $this->ret_array['notifications'][$index]['notify_id'] = $row['notify_id'];
                      $index++;
                    }
                    //print_r($this->ret_array);
                }
            } else { $this->error['error'] = "Error retreiving notifications."; }

            @mysql_close($l2);
        } else {$this->error['notifications'] = "Invalid account id."; }
    }

    ## REMOVE A NOTIFICATION ##
    function delete_notification($account_id, $internal_id, $notify_id) {
        // temporarily made all requests return "OK" so notifications can be deleted properly
        if(ereg('^[1-9][0-9]*$', $account_id) ) {
            include('inc/l1.php');
            $account_id = mysql_real_escape_string($account_id);
            $notify_id = mysql_real_escape_string($notify_id);

            // always save these for use in error messages
            $this->ret_array['internal_id'] = $internal_id;
            $this->ret_array['notify_id'] = $notify_id;

            $delete1 = "DELETE FROM hon_chat.notify_account WHERE notify_account.notify_id={$notify_id} AND notify_account.account_id={$account_id} LIMIT 1";
            if($result1 = mysql_query($delete1, $l1))
            {
              if (mysql_affected_rows() > 0)
              {
                $delete2 = "DELETE FROM hon_chat.notify WHERE notify.notify_id={$notify_id} LIMIT 1";
                if($result2 = mysql_query($delete2, $l1))
                {
                  if (mysql_affected_rows() > 0)
                  {
                    $this->ret_array['status'] = "OK";
                  }
                  else
                    $this->ret_array['status'] = "OK"; //"Error, no rows deleted. (2) ($notify_id)";
                }
                else
                {
                  $this->error['status'] = "OK"; //"Error removing notification. (2) ($notify_id)";
                }
              }
              else
                $this->ret_array['status'] = "OK"; //"Error, no rows deleted. (1) ($notify_id)";
            }
            else
            {
              $this->error['status'] = "OK"; //"Error removing notification. (1) ($notify_id)";
            }

            @mysql_close($l1);
        } else { $this->error['status'] = "Invalid account id."; }
    }

    ## BASE NOTIFICATION ##
    function simple_base_notify($type, $param1, $param2, $param3, $create_time, $expire_time, $link){
        $type = mysql_real_escape_string($type, $link);
        $param1 = mysql_real_escape_string($param1, $link);
        $param2 = mysql_real_escape_string($param2, $link);
        $param3 = mysql_real_escape_string($param3, $link);
        $create_time = mysql_real_escape_string($create_time, $link);
        $expire_time = mysql_real_escape_string($expire_time, $link);

        $insert = "INSERT INTO hon_chat.notify (type, create_time, expire_time) VALUES ('{$type}', '{$create_time}', '{$expire_time}')";
        if($result = mysql_query($insert, $link))
        {
          $notify_id = mysql_insert_id($link);
          $current_time = date("m/d")."  ".date("H:i A");

            switch($type)
            {
                // use 99 just to pass the notification in exactly as it is
                case "99":
                  $params = $param1;
                  break;
                case "1":
                  $params = "$param1||1|notify_buddy_added|notification_generic_info||$current_time|$notify_id";
                  break;
                case "2":
                  $params = "$param1||2|notify_buddy_added|notification_generic_info||$current_time|$notify_id";
                  break;
                case "22":
                  $params = "$param1||22|notify_buddy_requested_adder|notification_generic_info||$current_time|$notify_id";
                  break;
                case "23":
                  $params = "$param1||23|notify_buddy_requested_added|notification_generic_action|action_friend_request|$current_time|$notify_id";
                  break;
                default:
                  break;
            }
          $update = "UPDATE hon_chat.notify SET data = '{$params}' WHERE notify_id = '{$notify_id}' LIMIT 1";
          $result = mysql_query($update, $link);
        }
        else
        {
          $notify_id = 0;
        }

        $notification['notify_id'] = $notify_id;
        $notification['params'] = $params;
        return $notification;
    }

    ## ADD NOTIFICATION ##
    function simple_accounts_notify($aids, $notify_id, $link){
        if($notify_id)
        {   $id = mysql_real_escape_string($notify_id, $link);
            // setup notifications for all linked account_ids
            $insert = "INSERT INTO hon_chat.notify_account (notify_id, account_id) VALUES ";
            if(sizeof($aids) > 0){
            foreach($aids AS $k => $v)
            {   if(ereg('^[1-9][0-9]*$', $v))
                {
                    $this->remove_excess_notifications($v);
                    $v = mysql_real_escape_string($v, $link);
                    $insert .= "('{$id}', '{$v}'), ";
                }
            } $insert = trim($insert, ", ");

            if($result = mysql_query($insert, $link))
            {   return true;
            } else { return false; }
            } else { return false; }
        } else { return false; }
    }

    ## REMOVE EXCESS NOTIFICATIONS ##
    function remove_excess_notifications($account_id) {
      include('inc/l1.php');

      // see if any notifications need to be cleared out
      $sql = "SELECT COUNT(account_id) AS Count FROM hon_chat.notify_account WHERE account_id = $account_id";
      $result = mysql_query($sql, $l1);
      if ($row = mysql_fetch_assoc($result))
      {
        if ($row['Count'] > 24)
        {
          // grab the oldest row
          $sql = "SELECT notify_id FROM hon_chat.notify_account WHERE account_id = $account_id ORDER BY notify_id ASC LIMIT 1;";
          $result = mysql_query($sql, $l1);
          $row = mysql_fetch_assoc($result);

          if ($row['notify_id'] > 0)
          {
            // remove it to make space
            $sql = "DELETE FROM hon_chat.notify_account WHERE notify_id = '{$row['notify_id']}' LIMIT 1;";
            $result = mysql_query($sql, $l1);

            $sql = "DELETE FROM hon_chat.notify WHERE notify_id = '{$row['notify_id']}' LIMIT 1;";
            $result = mysql_query($sql, $l1);
          }
        }
      }
    }

    ## REMOVE ALL NOTIFICATIONS
    function remove_all_notifications($account_id) {
      include('inc/l1.php');
      mysql_select_db('hon_chat', $l1);
      $sql = "DELETE na, n FROM hon_chat.notify_account AS na INNER JOIN hon_chat.notify AS n ON na.notify_id = n.notify_id WHERE account_id = $account_id;";
      $result = mysql_query($sql, $l1);
      if (!$result)
        $this->ret_array['status'] = "ERROR";
      else
        $this->ret_array['status'] = "OK";
      @mysql_close($l1);
    }
    // END NEW NOTIFICATION STUFF

    ## CHAT SERVER CAPACITY UPDATE ##
    function ip_capacity($ip, $paid, $free){
        if(ereg('^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$', $ip) AND ereg('^[0-9]+$', $paid) AND ereg('^[0-9]+$', $free) ){
            include('inc/l1.php');
            $ip = mysql_real_escape_string($ip);
            $paid = mysql_real_escape_string($paid);
            $free = mysql_real_escape_string($free);
            $insert = "INSERT INTO hon_tracking.ip_capacity (ip, free, paid) VALUES ('{$ip}', '{$free}', '{$paid}') ON DUPLICATE KEY UPDATE free=VALUES(free), paid=VALUES(paid)";
            if($result = mysql_query($insert, $l1) )
            {
              $this->ret_array['ip_update'] = "OK";
            } else { $this->error['ip_update'] = "Connection error."; }

            @mysql_close($l1);
        } else { $this->error['ip_update'] = "Invalid input."; }
    }

    ## LOGIN CHECK ##
    function login_check($account_id, $password){
        if(ereg('^[1-9][0-9]*$', $account_id) AND ereg('^[a-zA-Z0-9]{32}$', $password) ){
            include('inc/l1.php');
            $account_id = mysql_real_escape_string($account_id);

            $select = "SELECT account_id, nickname, email, account_type, susp_id, salt, password, pass_exp, lan FROM hon_accounts.account JOIN hon_accounts.super_account USING (super_id) WHERE account_id='{$account_id}' LIMIT 1";
            if($result = mysql_query($select, $l1) )
            {	if(mysql_num_rows($result) > 0)
        		{	$row = mysql_fetch_assoc($result);
        			if($row['password'] == $password) {
                      		$this->ret_array['accepted'] = "OK";
        			} else { $this->error['accepted'] = "FAIL"; }
        		} else { $this->error['accepted'] = "FAIL"; }
            } else { $this->error['accepted'] = "FAIL"; }

            @mysql_close($l1);
        } else { $this->error['accepted'] = "FAIL"; }
    }

    ## IP REGIONAL FILTER ##
    function allow_ip($ip){
        $allowdisbitch = true; // default allow
        $EXCEPTIONS = array('178.67.103.113', '210.23.133.41', '210.23.132.55', '210.23.137.244', '213.108.172.195');
        if(!in_array($ip, $EXCEPTIONS) ) {

            $COUNTRY_RESTRICT = array("MYS","MY", "PHL","PH", "VNM","VN", "SGP","SG");

            if(in_array($_SERVER['GEOIP_COUNTRY_CODE'], $COUNTRY_RESTRICT) )
            { $allowdisbitch = false; }

        }
        return $allowdisbitch;
    }

    ## GRAB UPGRADES ##
    function getUpgrades($account_id){
        include('inc/l2.php');
        
        if (empty($account_id))
          return "";
          
        // try to query the super_id based on the account_id
        $sql = "SELECT super_id FROM hon_accounts.account WHERE account_id = $account_id LIMIT 1;";
        $result = mysql_query($sql, $l2);

        if (mysql_num_rows($result) == 0)
          return "";
        
        $row = mysql_fetch_assoc($result);    
                                        
        // query all the upgrade names available for this super_id
        $sql = "SELECT hon_micro.products.name FROM hon_micro.upgrades JOIN hon_micro.products ON hon_micro.upgrades.product_id = hon_micro.products.product_id WHERE hon_micro.upgrades.super_id = " . $row['super_id'];
        $result = mysql_query($sql, $l2);
        
        $upgrades = "";
        
        if (mysql_num_rows($result))
        {
          while($row = mysql_fetch_assoc($result))
          {
             $upgrades .= $row['name'] . "|";
          }
          // strip off the last pipe
          $upgrades = substr($upgrades, 0, (strlen($upgrades) - 1));      
        }
        
        
        @mysql_close($l2);  
        
        return $upgrades;
    }     
    

}

?>
