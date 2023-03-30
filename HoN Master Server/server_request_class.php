<?php
//
//    input from server of all statistics
//

class ServerRequest {
    var $error;                         //errors
    var $input;                         //unserialized array of input data
    var $output;

    function ServerRequest(){
        $this->match_summ = array();
        $this->input = array();
    }

    function to_server(){
        if(count($this->error) > 0)
        {   echo serialize($this->error);
        } else {
           echo serialize($this->output);
        }
    }

    //## AUTHENTICATE SERVER
    // by login credentials
    function auth(){
        include('inc/l1.php');
        $login = mysql_real_escape_string($this->input['login']);

        $query = "SELECT account_id, nickname, password, salt, official_svr FROM hon_accounts.account JOIN hon_accounts.super_account USING (super_id) WHERE nickname='{$login}' AND host_svr = 1 LIMIT 1";
        if($result = mysql_query($query, $l1) )
        { if(mysql_num_rows($result) == 1 )
          { $row = mysql_fetch_assoc($result);
            // case sensitive login/pass check
            $passCheck = md5($this->input['pass'].$row['salt']."[!~esTo0}");
            $passCheck2 = md5(md5($this->input['pass']).$row['salt']."[!~esTo0}");
            if($this->input['login'] === $row['nickname'] AND ($passCheck == $row['password'] OR $passCheck2 == $row['password'] OR $this->input['pass'] == md5($row['password']) ) )
            {
                $this->output['server_id']= $row['account_id'];
                $this->output['official_svr'] = $row['official_svr'];
                return 1;
            } else {$this->error['authenticate'] = "Failed to authenticate A2";  return 0; }
          } else {$this->error['authenticate'] = "Failed to authenticate A1"; return 0; }
        } else {$this->error['connect']="Could not connect. A0"; return 0; }
        @mysql_close($l1);
    }
    // by session
    function auth_session(){
        include('inc/l1.php');
        $session = mysql_real_escape_string($this->input['session']);
        // grab infos with 30 MINUTE TIMEOUT
        $query = "SELECT server_id FROM hon_server.info WHERE session='{$session}' LIMIT 1";
        if($result = mysql_query($query, $l1) )
        { if(mysql_num_rows($result) == 1 )
          { $row = mysql_fetch_assoc($result);

            $this->output['server_id']= $row['server_id'];
            return 1;
          } else {$this->error['authenticate'] = "Failed to authenticate B1"; return 0; }
        } else {$this->error['connect']="Could not connect. B0"; return 0; }
        @mysql_close($l1);
    }

    //## NEW SERVER SESSION
    // create a new session for a server
    function new_session(){
        include('inc/l1.php');

        $ip = mysql_real_escape_string($this->input['ip']);
        $port = mysql_real_escape_string($this->input['port']);

        $name = mysql_real_escape_string($this->input['name']);
        $desc = mysql_real_escape_string($this->input['desc']);
        $location = mysql_real_escape_string($this->input['location']);
        $official = mysql_real_escape_string($this->output['official_svr']);
        // make salt
        $salt = '';
        for ($i = 0; $i < 3; $i++)
        {$salt .= chr(rand(33, 126)); }

        // create session
        $temp_session = md5(md5($this->output['server_id']).$salt);
        $this->output['session'] = $temp_session;
        $session = mysql_real_escape_string($temp_session);

        //escape salt
        $salt = mysql_real_escape_string($salt);

        if(isset($this->input['init_state']) )
        { $c_state=mysql_real_escape_string($this->input['init_state']); } else { $c_state=1; }
        // threshold
        $this->output['leaverthreshold'] = '0.50';
        $this->output['tiers'] = array(0 => array('min'=>0, 'max'=>5), 1=> array('min'=>0, 'max'=>70), 2 => array('min'=>15, 'max'=>70));

        // make server available in list
        $query = "INSERT INTO hon_server.info SET server_id='{$this->output['server_id']}', session='{$session}', official='{$official}', salt='{$salt}', ip='{$ip}', port='{$port}', location='{$location}', name='{$name}', description='{$desc}', last_upd=NOW() ON DUPLICATE KEY UPDATE session=VALUES(session), official='{$official}', salt=VALUES(salt), ip=VALUES(ip), port=VALUES(port), location=VALUES(location), name=VALUES(name), description=VALUES(description), cgt=DEFAULT, c_state='{$c_state}', last_upd=VALUES(last_upd), num_conn=DEFAULT";
        if($result = mysql_query($query, $l1) )
        {   return 1;
        } else {return 0; }
        @mysql_close($l1);
    }

    //## SHUT DOWN
    // disconnect server with CURRENT STATE update
    function shutdown(){
        include('inc/l1.php');
        $query = "UPDATE hon_server.info SET num_conn=0, c_state=0 WHERE server_id='{$this->output['server_id']}' LIMIT 1";
        if($result = mysql_query($query, $l1) )
        {   $this->output['shutdown'] = "OK";
        } else {$this->error['connect']="Could not connect. A1"; }

        $query = "UPDATE hon_server.game_type SET session=MD5(CONCAT(session,server_id)), mname=DEFAULT, class=DEFAULT, private=DEFAULT, nm=DEFAULT, sd=DEFAULT, rd=DEFAULT, dm=DEFAULT, bd=DEFAULT, cd=DEFAULT, cm=DEFAULT, league=DEFAULT, max_players=DEFAULT, tier=DEFAULT, priv_key=DEFAULT WHERE server_id='{$this->output['server_id']}' LIMIT 1";
        if($result = mysql_query($query, $l1) )
        {   $this->output['shutdown'] = "OK";
        } else {$this->error['connect']="Could not connect. A1"; }
        @mysql_close($l1);
    }


    //## CLIENT CONNECTION FUNCS
    // key accepting
    function accept_key(){
        include('inc/l1.php');
        $acc_key = mysql_real_escape_string($this->input['acc_key']);
        $query = "SELECT gametype, options FROM hon_server.match_request WHERE acc_key='{$acc_key}' LIMIT 1";
        if($result = mysql_query($query, $l1) )
        {   if(mysql_num_rows($result) > 0 )
            {   $row = mysql_fetch_assoc($result);
                $update = "UPDATE hon_server.info SET c_state='2' WHERE server_id = '{$this->output['server_id']}' LIMIT 1";
                if($result = mysql_query($update, $l1) )
                {   $this->output['match_info'] = $row;
                } else {$this->error['match_info'] = "Current state update failed."; }
            } else {$this->error['match_info'] = "No request found.";  }
        } else {$this->error['connect']="Could not connect. A1"; }

        @mysql_close($l1);
    }

    // check number online with db
    function num_check(){
        include('inc/l1.php');
        $query ="SELECT num_conn FROM hon_server.info WHERE server_id='{$this->output['server_id']}' LIMIT 1";
        if($result = mysql_query($query, $l1) )
        {
            $row = mysql_fetch_assoc($result);
            if($row['num_conn'] == $this->input['num_conn'])
            {return "OK"; } else {return "UPD"; }
        } else {return "UPD"; }
        @mysql_close($l1);
    }

    //set server to online status
    function set_online($_ok ){
        include('inc/l1.php');

        if(isset($this->input['c_state']) ){
            $c_state = "'".mysql_real_escape_string($this->input['c_state'])."'";
        } else {$c_state = "DEFAULT"; }

        if(isset($this->input['cgt']) ){
            $cgt = "'".mysql_real_escape_string($this->input['cgt'])."'";
        } else {$cgt = "DEFAULT"; }

        if(isset($this->input['num_conn']) ){
            $num_conn = "'".mysql_real_escape_string($this->input['num_conn'])."'";
        } else {$num_conn = "DEFAULT"; }

        // make server available
        $insert = "INSERT INTO hon_server.info SET server_id='{$this->output['server_id']}', num_conn={$num_conn}, c_state={$c_state}, cgt={$cgt}, last_upd=NOW() ON DUPLICATE KEY UPDATE num_conn=VALUES(num_conn), cgt=VALUES(cgt), c_state=VALUES(c_state), last_upd=VALUES(last_upd)";
        if($result = mysql_query($insert, $l1) )
        {   // assign private match key
            if(isset($this->input['priv_key']) )
            {
              $priv_key = mysql_real_escape_string($this->input['priv_key']);
            } else {$priv_key = ''; }
            if(isset($this->input['clear']) )
            {
                $insert = "INSERT INTO hon_server.game_type SET server_id='{$this->output['server_id']}', session=MD5(CONCAT(session,server_id)), mname=DEFAULT, class=DEFAULT, private=DEFAULT, priv_key=DEFAULT, nm=DEFAULT, sd=DEFAULT, rd=DEFAULT, dm=DEFAULT, bd=DEFAULT, cd=DEFAULT, cm=DEFAULT, league=DEFAULT, max_players=DEFAULT, tier=DEFAULT ON DUPLICATE KEY UPDATE session=VALUES(session), mname=VALUES(mname), class=VALUES(class), private=VALUES(private), priv_key=VALUES(priv_key), nm=VALUES(nm), sd=VALUES(sd), rd=VALUES(rd), dm=VALUES(dm), bd=VALUES(bd), cd=VALUES(cd), cm=VALUES(cm), league=VALUES(league), max_players=VALUES(max_players), tier=VALUES(tier)";
                if($result = mysql_query($insert, $l1) )
                {
                    return 1;
                } else {return 0; }
            }

            // toggle private match
	     	if(!isset($this->input['new']) AND isset($this->input['private']) )
		    {	$private = mysql_real_escape_string($this->input['private']);
			    $session = mysql_real_escape_string($this->input['session']);
			    $insert = "INSERT INTO hon_server.game_type SET server_id='{$this->output['server_id']}', session='{$session}', private='{$private}', priv_key='{$priv_key}' ON DUPLICATE KEY UPDATE session=VALUES(session), private=VALUES(private), priv_key=VALUES(priv_key)";
		        if($result = mysql_query($insert, $l1) )
                {
                    return 1;
                } else {return 0; }
		    }

            // initial request - game type
            if(isset($this->input['new']) )
            {   $session = mysql_real_escape_string($this->input['session']);
                $mname = mysql_real_escape_string($this->input['mname']);

                // match class
                $class = mysql_real_escape_string($this->input['new']);

                // modes
                $nm = 0; $sd = 0; $rd = 0; $dm = 0; $bd = 0; $bp = 0; $cd = 0; $cm = 0;
                ${$this->input['mode']} = 1;

                // options
                $opts = array ('ap', 'ar', 'em', 'no_repick', 'no_agi', 'drp_itm', 'no_timer', 'rev_hs', 'no_swap', 'no_int', 'alt_pick', 'veto', 'shuf', 'no_str', 'no_pups', 'dup_h', 'rs', 'nl', 'officl', 'no_stats', 'ab', 'hardcore', 'dev_heroes');
                foreach($opts AS $v)
                {   if(!empty($this->input['option'][$v]) )
                    {${$v} = 1;} else {${$v} = 0; }
                }

                // settings
                $league = mysql_real_escape_string($this->input['league']);
                $private = mysql_real_escape_string($this->input['private']);
                $max_players = mysql_real_escape_string($this->input['max_players']);
                $tier = mysql_real_escape_string($this->input['tier']);

                // server list queries
                // game type
                $insert = "INSERT INTO hon_server.game_type SET server_id='{$this->output['server_id']}', session='{$session}', mname='{$mname}', class='{$class}', private='{$private}', priv_key='{$priv_key}', nm='{$nm}', sd='{$sd}', rd='{$rd}', dm='{$dm}', bd='{$bd}', bp='{$bp}', cd='{$cd}', cm='{$cm}', league='{$league}', max_players='{$max_players}', tier='{$tier}' ON DUPLICATE KEY UPDATE session=VALUES(session), mname=VALUES(mname), class=VALUES(class), private=VALUES(private), priv_key=VALUES(priv_key), nm=VALUES(nm), sd=VALUES(sd), rd=VALUES(rd), dm=VALUES(dm), bd=VALUES(bd), bp=VALUES(bp), cd=VALUES(cd), cm=VALUES(cm), league=VALUES(league), max_players=VALUES(max_players), tier=VALUES(tier)";
                @mysql_query($insert, $l1);
                // game options
                $insert = "INSERT INTO hon_server.game_options SET server_id='{$this->output['server_id']}', session='{$session}', no_repick='{$no_repick}', no_agi='{$no_agi}', drp_itm='{$drp_itm}', no_timer='{$no_timer}', rev_hs='{$rev_hs}', no_swap='{$no_swap}', no_int='{$no_int}', alt_pick='{$alt_pick}', veto='{$veto}', shuf='{$shuf}', no_str='{$no_str}', no_pups='{$no_pups}', dup_h='{$dup_h}', ap='{$ap}', ar='{$ar}', em='{$em}', rs='{$rs}', nl='{$nl}', officl='{$officl}', no_stats='{$no_stats}', ab='{$ab}', hardcore='{$hardcore}', dev_heroes='{$dev_heroes}' ON DUPLICATE KEY UPDATE session=VALUES(session), no_repick=VALUES(no_repick), no_agi=VALUES(no_agi), drp_itm=VALUES(drp_itm), no_timer=VALUES(no_timer), rev_hs=VALUES(rev_hs), no_swap=VALUES(no_swap), no_int=VALUES(no_int), alt_pick=VALUES(alt_pick), veto=VALUES(veto), shuf=VALUES(shuf), no_str=VALUES(no_str), no_pups=VALUES(no_pups), dup_h=VALUES(dup_h), ap=VALUES(ap), ar=VALUES(ar), em=VALUES(em), rs=VALUES(rs), nl=VALUES(nl), officl=VALUES(officl), no_stats=VALUES(no_stats), ab=VALUES(ab), hardcore=VALUES(hardcore), dev_heroes=VALUES(dev_heroes)";
                @mysql_query($insert, $l1);



                // replay queries
                $match_id = mysql_real_escape_string($this->input['match_id']);
                // match type
                $insert = "INSERT INTO hon_stats.match_type (match_id, mname, class, private, nm, sd, rd, dm, bd, bp, cd, cm, league, max_players, tier)(SELECT '{$match_id}' AS match_id, mname, class, private, nm, sd, rd, dm, bd, bp, cd, cm, league, max_players, tier FROM hon_server.game_type WHERE server_id='{$this->output['server_id']}' AND session='{$session}')";
                @mysql_query($insert, $l1);
                // match options
                $insert = "INSERT INTO hon_stats.match_options (match_id, no_repick, no_agi, drp_itm, no_timer, rev_hs, no_swap, no_int, alt_pick, veto, shuf, no_str, no_pups, dup_h, ap, ar, em, rs, nl, officl, no_stats, ab, hardcore, dev_heroes) (SELECT '{$match_id}' AS match_id, no_repick, no_agi, drp_itm, no_timer, rev_hs, no_swap, no_int, alt_pick, veto, shuf, no_str, no_pups, dup_h, ap, ar, em, rs, nl, officl, no_stats, ab, hardcore, dev_heroes FROM hon_server.game_options WHERE server_id='{$this->output['server_id']}' AND session='{$session}')";
                if($result = mysql_query($insert, $l1) )
                {   return 1;
                } else {return 0; }

            } else {return 1; }
        } else {return 0; }
        @mysql_close($l1);
    }

    //
    function set_online_ids(){
        include('inc/l1.php');
        $c = "";
        $num_conn = mysql_real_escape_string($this->input['num_conn']);

        foreach($this->input['account_id'] as $k => $v)
        {   $v = mysql_real_escape_string($v);
            $c .="('{$v}', '{$this->output['server_id']}', NOW()),";
        }

        // connected table update
        $query = "REPLACE INTO hon_server.connected (account_id, server_id, dt) VALUES {$c}";
        if($result = mysql_query($query, $l1) )
        {   $this->output['set_online_ids'] = "OK";
        } else {$this->error['connect']="Could not connect."; }

        // info table update
        $query = "UPDATE hon_server.info SET num_conn='{$num_conn}', last_upd = NOW() WHERE server_id='{$this->output['server_id']}' LIMIT 1";
        if($result = mysql_query($query, $l1) )
        {
            $this->output['set_online_ids'] = "OK";
        } else {$this->error['connect']="Could not connect."; }
        @mysql_close($l1);
    }

    function c_conn(){
        include('inc/l1.php');

        $cookie = mysql_real_escape_string($this->input['cookie']);
        $ip = $this->input['ip'];

        $select = "SELECT account_id, nickname, nicktype, account_type, INET_NTOA(ip), lan, trial FROM hon_tracking.in_game JOIN hon_accounts.account USING (account_id) JOIN hon_accounts.super_account USING (super_id) WHERE cookie='{$cookie}' LIMIT 1";
	    if($result = mysql_query($select, $l1) )
        {   if(mysql_num_rows($result) )
            {
                $row = mysql_fetch_assoc($result);
                //echo $row['ip'] . ":". ip2long($ip);
                //if($row['ip'] == $ip )
                //{
		      if($row['lan'] == 1) {$row['account_type'] = 3; }

                    $update = "UPDATE hon_tracking.in_game SET last_svr='{$this->output['server_id']}' WHERE account_id='{$row['account_id']}' LIMIT 1";
                    if(!$result = mysql_query($update, $l1) )
                    { $this->error['cli_status'] = "Could not update client.";  }

                    $replace = "REPLACE INTO hon_server.connected (account_id, server_id) VALUES ('{$row['account_id']}', '{$this->output['server_id']}')";
                    if(!$result = mysql_query($replace, $l1) )
                    { $this->error['conn_status'] = "Could not update connection.";  }

                    $this->output['account_id'] = $row['account_id'];
                    $this->output['nickname'] = $row['nickname'];
                    $this->output['account_type'] = $row['account_type'];
                    $this->output['trial'] = $row['trial'];
                    $this->output['nicktype'] = $row['nicktype'];

                    //$select = "SELECT player_rank.account_id, clan_id, tag, level, acc_games_played, acc_pub_skill, acc_pub_count, rnk_amm_solo_rating, rnk_amm_solo_count, rnk_amm_solo_conf, rnk_amm_team_rating, rnk_amm_team_count, rnk_amm_team_conf, rnk_discos, rnk_games_played, acc_wins, acc_losses, acc_discos, IFNULL(acc_discos/(acc_wins+acc_losses),0) AS leaverpct FROM hon_stats.player_rank JOIN hon_stats.player_stats USING (account_id) JOIN hon_stats.ranked_stats USING (account_id)LEFT JOIN hon_clans.roster USING (account_id) LEFT JOIN hon_clans.clan_info USING (clan_id) WHERE account_id = '{$row['account_id']}' LIMIT 1";
                    $select = "SELECT player_rank.account_id, clan_id, tag, level, acc_wards, acc_games_played, acc_trial_games_played, acc_pub_skill, acc_pub_count, acc_herokills, acc_heroassists, acc_deaths, acc_em_played, acc_secs, acc_time_earning_exp, acc_gold, acc_exp, rnk_wards, rnk_amm_solo_rating, rnk_amm_solo_count, rnk_amm_solo_conf, rnk_amm_team_rating, rnk_amm_team_count, rnk_amm_team_conf, rnk_wins, rnk_losses, rnk_discos, rnk_herokills, rnk_heroassists, rnk_deaths, rnk_em_played, rnk_games_played, rnk_secs, rnk_gold, rnk_exp, rnk_time_earning_exp, acc_wins, acc_losses, acc_discos, IFNULL(acc_discos/(acc_wins+acc_losses),0) AS leaverpct FROM hon_stats.player_rank JOIN hon_stats.player_stats USING (account_id) JOIN hon_stats.ranked_stats USING (account_id)LEFT JOIN hon_clans.roster USING (account_id) LEFT JOIN hon_clans.clan_info USING (clan_id) WHERE account_id = '{$row['account_id']}' LIMIT 1";
                    if($result = mysql_query($select, $l1) )
                    {
                        $row = mysql_fetch_assoc($result);
                        $this->output['infos'][$row['account_id']] = $row;
                        $this->output['infos'][$row['account_id']]['trial'] = $this->output['trial'];
                        $this->output['infos'][$row['account_id']]['account_type']  = $this->output['account_type']; 
                    }
                    
					$account_id = $row['account_id'];
					
					@mysql_close($l1);
					
					$quick_stats = $this->quick_stats($account_id);
					
					if(!empty($quick_stats)){
						
						if(isset($quick_stats['match_history'])){
							
							$this->output['infos'][$account_id]['match_history'] = $quick_stats['match_history'];
							
						}
						
						if(isset($quick_stats['fav_heroes'])){
							
							$this->output['infos'][$account_id]['fav_heroes'] = $quick_stats['fav_heroes'];
						
						}
					
					}
					
                    $upgrades = $this->getUpgrades($this->output['account_id']);
                    
                    if (!empty($upgrades))
                        $this->output['infos'][$this->output['account_id']]['upgrades'] = $upgrades;


                    $this->output['c_conn'] = "OK";
                //} else { $this->error['account'] = "Authorization failed."; }
            } else { $this->error['cookie'] = "Invalid cookie."; }
        } else { $this->error['connect']="Could not connect."; }
       
    }
	
	public function quick_stats($account_id, $ln = false){
	
		$return = false;
		$matches = array();
		$selecter = 'WHERE (';
		
		if(isset($account_id)){
		
			if(!$ln){
				include('inc/ls.php');
				$ln = $ls;
			}
			
			$account_id = mysql_real_escape_string($account_id,$ln);

			$select = "SELECT history FROM hon_stats.player_match_history WHERE account_id = '".$account_id."' LIMIT 1";
			if($result = mysql_query($select,$ln)){
			
				if(mysql_num_rows($result) > 0){
					
					$row = mysql_fetch_assoc($result);
					
					$exp = explode(',',$row['history']);
					
					foreach($exp as $v){
						
						$data = explode('|',$v);
						if(is_numeric($data[0])){
						
							$matches[$data[0]] = $data[0];
							
						}
					}
					
					if(krsort($matches, SORT_NUMERIC)){
						if(count($matches) > 0){
							$matches = array_slice($matches,0,7,true);
							$e=0;
							foreach($matches as $mid){
								
								$selecter .= "match_id = '".mysql_real_escape_string($mid,$ln)."'";
								if($mid != end($matches)){
									$selecter .= " OR ";
								} else {
									$selecter .= " )";
								}
								
								if($e > 6){
									break;
								}
								
								$e++;
								
							}
						
						}
					}
					
					
					
					
					
					$select = "SELECT cli_name, wins, herokills, deaths, heroassists, match_id FROM hon_stats.match_stats JOIN hon_stats.hero USING(hero_id) ".$selecter." AND account_id = '".$account_id."' LIMIT 7";
					
					if($doSelect = mysql_query($select,$ln)){
					
						while($sRow = mysql_fetch_assoc($doSelect)){
							
							$match_history[$sRow['match_id']] = $sRow;
			
						}
						
						krsort($match_history, SORT_NUMERIC);
						
						$i = 0;
						foreach($match_history as $v){
							$return['match_history']['match'.$i] = $v;
							$i++;
						}
						
					}
					
					$select = "SELECT h.cli_name, phs.ph_secs FROM hon_stats.hero AS h JOIN (select hero_id, ph_secs from hon_stats.player_hero_stats where account_id = '{$account_id}') AS phs ON phs.hero_id = h.hero_id ORDER BY phs.ph_secs DESC LIMIT 5";
			
					if($result = mysql_query($select, $ln))
					{
						if(mysql_num_rows($result) > 0)
						{
							$i=0;
							while($row = mysql_fetch_assoc($result))
							{
								$return['fav_heroes']['hero'.$i] = $row;
								$i++;
							}
						}
					}
				
				}
			
			}
		
		}
		
		return $return;
	
	}

    function c_disco(){
        include('inc/l1.php');

        $account_id = mysql_real_escape_string($this->input['account_id']);

        $update = "UPDATE hon_tracking.in_game SET last_activity=CURRENT_TIMESTAMP WHERE account_id='{$row['account_id']}' LIMIT 1";
        if(!$result = mysql_query($update, $l1) )
        { $this->error['cli_status'] = "Could not update client.";  }

        $delete = "DELETE FROM hon_server.connected WHERE account_id='{$account_id}' AND server_id='{$this->output['server_id']}' LIMIT 1";
        if(!$result = mysql_query($delete, $l1) )
        { $this->error['conn_status'] = "Could not update connection.";  }

        $this->output['c_disco'] = "OK";

        @mysql_close($l1);
    }

    function start_game(){
        include('inc/l1.php');
        $session = mysql_real_escape_string($this->input['session']);
        $server_id = mysql_real_escape_string($this->output['server_id']);
        $version = mysql_real_escape_string($this->input['version']);
        $map = mysql_real_escape_string($this->input['map']);
        $c_state = 4;


        $insert = "INSERT INTO hon_stats.match_summ (server_id, map, c_state, version) VALUES ('{$server_id}','{$map}','{$c_state}','{$version}')";
        if($result = mysql_query($insert, $l1) )
        {
            $this->output['match_id'] = mysql_insert_id($l1);
            $this->output['date']=date("Ymd");  // for replay folder

            // grab current replays server info
            $select = "SELECT nickname AS name, file_host, volume FROM hon_accounts.account, hon_server.replaydl WHERE account_id='{$server_id}' LIMIT 1";
            if($result = mysql_query($select, $l1) )
            {
                $row = mysql_fetch_assoc($result);
                $this->output['file_host'] = $row['file_host'].":31336";
                $this->output['file_dir'] = "/saves/replays/{$this->output['date']}";
                // set replay table entry
                $match_id = mysql_real_escape_string($this->output['match_id']);
                $host = mysql_real_escape_string($this->output['file_host']);
                $dir = mysql_real_escape_string($this->output['file_dir']);
                $url = mysql_real_escape_string("http://{$row['file_host']}/{$this->input['mstr']}/saves/replays/{$this->output['date']}/M{$this->output['match_id']}.honreplay");
                $insert = "INSERT INTO hon_replays.file_loc (match_id, host, dir, url) VALUES ('{$match_id}','{$host}','{$dir}','{$url}')";
                @mysql_query($insert, $l1);
            }


        } else { $this->error['connect']="Could not connect."; }

        @mysql_close($l1);
    }

    function update_trial(){

        if(sizeof($this->input['trial_ids']) > 0) {
            include('inc/l1.php');
            $insert = "INSERT INTO hon_stats.player_stats (account_id, acc_trial_games_played) VALUES ";
            foreach($this->input['trial_ids'] AS $k => $v)
            {
                if(ereg('^[1-9][0-9]*$', $v) ){
                    $aid = mysql_real_escape_string($v);
                   $insert .= "('{$aid}', 1),";
                }
            } $insert = trim($insert, ",");
            $insert .= " ON DUPLICATE KEY UPDATE acc_trial_games_played = acc_trial_games_played + VALUES(acc_trial_games_played)";
            if($result = mysql_query($insert, $l1) )
            {
                $this->output['trial_update'] = "OK";
            }

            @mysql_close($l1);
        }
    }

    function chat_online(){
        include('inc/l1.php');
        $num = 0;
        $online = mysql_real_escape_string($this->input['online']);
        $in_game = mysql_real_escape_string($this->input['in_game']);
        $seen = mysql_real_escape_string($this->input['seen']);

        $insert = "INSERT INTO hon_tracking.chat_online (num, date, time, online, in_game, seen) VALUES ('{$num}', NOW(), NOW(), '{$online}', '{$in_game}', '{$seen}') ON DUPLICATE KEY UPDATE date=VALUES(date), time=VALUES(time), online=VALUES(online), in_game=VALUES(in_game), seen=VALUES(seen)";
        if($result = mysql_query($insert, $l1) )
        { $this->output['chat_online'] = "OK"; } else { $this->error['connect']="Could not connect."; }
        @mysql_close($l1);
    }

    function replay_auth(){
        $url = "https://admin.heroesofnewerth.com/catchInfo.php";
        $data = '<?xml version="1.0" encoding="utf-8"?>
<server>
    <server_id>'.$this->output['server_id'].'</server_id>
    <name>'.$this->input['login'].'</name>
    <hash>'.$this->input['pass'].'</hash>
</server>';

        $port = 443;

        $user_agent = $_SERVER['HTTP_USER_AGENT'];
        $ch = curl_init(); // initialize curl handle
        curl_setopt($ch, CURLOPT_URL, $url); // set url to post to
        curl_setopt($ch, CURLOPT_FAILONERROR, 1); // Fail on errors
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1); // allow redirects
        curl_setopt($ch, CURLOPT_RETURNTRANSFER,1); // return into a variable
        curl_setopt($ch, CURLOPT_PORT, $port); //Set the port number
        curl_setopt($ch, CURLOPT_TIMEOUT, 15); // times out after 15s
        curl_setopt($ch, CURLOPT_POSTFIELDS, $data); // add POST fields
        curl_setopt($ch, CURLOPT_USERAGENT, $user_agent);
        if($port==443)
        {
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
        }
        $ret = curl_exec($ch);
        curl_close($ch);
    }
    
    function getUpgrades($account_id)
    { 
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