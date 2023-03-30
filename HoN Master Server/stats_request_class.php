<?
    include("server_request_class.php");

    class Stats extends ServerRequest {
        var $input;
        var $output;
        var $match_id;
        var $server_id;
        var $match_info;                      //match info
        var $match_stats;                     //match stats
        var $hero_stats;
        var $team_stats;
        var $abilities;
        var $read;                            //db read source
        var $write;                           //db write source
        var $heroes;                          //hero list
        var $items;                           //item list
        var $inventory;                       //inventory list
        // construct class
        function Stats(){
            // load input
            foreach($_REQUEST as $key => $val)
            {   $this->input[$key] = $val; }

            // db links
            include("inc/l1.php");
            $this->write = $l1;
            include("inc/l2.php");
            $this->read = $l2;

            // server authorization
            if($this->auth_session() AND $this->checkAccountIDs())
            {   // match and server ids
                if(ereg('^[1-9][0-9]*$', $this->input['match_stats']['match_id']) )
                {   $this->match_id = mysql_real_escape_string($this->input['match_stats']['match_id']);
                } else {$this->error['match_id']="Match id inproperly formatted."; }

                $this->server_id = mysql_real_escape_string($this->output['server_id']);

                // grab heroes
                $select = "SELECT hero_id, cli_name FROM hon_stats.hero";
                if($result = mysql_query($select, $this->read) )
                {   while($row = mysql_fetch_assoc($result) )
                    { $this->heroes[$row['cli_name']]=$row['hero_id']; }
                }

                // grab items
                $select = "SELECT item_id, cli_name FROM hon_stats.items";
                if($result = mysql_query($select, $this->read) )
                {   while($row = mysql_fetch_assoc($result) )
                    { $this->items[$row['cli_name']]=$row['item_id']; }
                }

                // grab match info
                $select = "SELECT * FROM hon_stats.match_summ JOIN hon_stats.match_type USING (match_id) JOIN hon_stats.match_options USING (match_id) WHERE match_id='{$this->match_id}' AND server_id='{$this->server_id}' LIMIT 1";
                if($result = mysql_query($select, $this->read) )
                { if(mysql_num_rows($result) > 0 )
                  { $this->output['match_info'] = "OK";
                    $this->match_info = mysql_fetch_assoc($result);

                  } else { $this->output['match_info']="Match info not found."; }
                } else { $this->output['match_info']="Could not retrieve match info."; }
            } else {$this->error['connect'] = "Could not authenticate."; }
        }

        // destruct class
        function __destruct(){
            @mysql_close($this->write);
            @mysql_close($this->read);
        }

        // write to match cache
        function pre_ins(){
            $value = mysql_real_escape_string(serialize($this->input));
            $insert = "INSERT INTO hon_stats.match_cache VALUES ('{$this->match_id}', '{$value}') ON DUPLICATE KEY UPDATE val=VALUES(val)";
            if(mysql_query($insert, $this->write) )
            { $this->output['cache'] = "OK";
            } else {$this->output['cache'] = "Could not connect."; }
        }

        // check account ids
        function checkAccountIDs(){
            if(isset($this->input['player_stats']) ) {
            foreach($this->input['player_stats'] AS $k => $v)
            {   if(!ereg('^[1-9]+[0-9]*$', $k))
                {$this->error['accounts'] = "Could not complete submission. Invalid account ids."; return 0; }
            } } else {$this->input['player_stats'] = array(); }
            return 1;
        }

        // update match info
        function set_match_summ(){
            $ma = array("map", "map_version", "time_played", "file_host", "file_size", "file_name", "c_state", "version");
            if(!isset($this->input['match_stats']['file_host']) ){$this->input['match_stats']['file_host']="replaydl.heroesofnewerth.com"; }
            foreach($ma as $k)
            { ${$k} = mysql_real_escape_string($this->input['match_stats'][$k]); }
            $update = "UPDATE hon_stats.match_summ SET map='{$map}', map_version='{$map_version}', time_played='{$time_played}', file_host='{$file_host}', file_size='{$file_size}', file_name='{$file_name}', c_state='{$c_state}', version='{$version}' WHERE match_id='{$this->match_id}' LIMIT 1";
            if($result = mysql_query($update, $this->write) )
            { if(mysql_affected_rows($this->write) > 0)
              { $this->output['match_summ'] = "OK";
              } else { $this->output['match_summ']="No update.";}
            } else { $this->output['match_summ']="Could not connect."; }
            // replay upd
            $update = "UPDATE hon_replays.file_loc SET size='{$file_size}' WHERE match_id='{$this->match_id}' LIMIT 1";
            @mysql_query($update, $this->write);
        }

        // banned heroes into db
		function set_banned_heroes(){
			if(isset($this->input['match_stats']['banned_heroes']))
			{   $banned_heroes = mysql_real_escape_string($this->input['match_stats']['banned_heroes']);
                $insert = "INSERT INTO hon_stats.match_banned_heroes (match_id, heroes) VALUES('{$this->match_id}', '{$banned_heroes}')";
				if($result = mysql_query($insert, $this->write) )
				{ 	if(mysql_affected_rows($this->write) > 0)
					{ 	$this->output['banned_heroes'] = "OK";
					} else { $this->output['banned_heroes']="No update."; }
				} else { $this->output['banned_heroes']="Could not connect."; }
			}
		}

        // format player stats
        function fmt_player_stats(){
            $sa = array("wins", "losses", "concedes", "concedevotes", "buybacks", "discos", "kicked", "pub_skill", "pub_count", "amm_solo_rating", "amm_solo_count", "amm_team_rating", "amm_team_count", "avg_score", "herokills", "herodmg", "heroexp", "herokillsgold", "heroassists", "deaths", "goldlost2death", "secs_dead", "teamcreepkills", "teamcreepdmg", "teamcreepexp", "teamcreepgold", "neutralcreepkills", "neutralcreepdmg", "neutralcreepexp", "neutralcreepgold", "bdmg", "bdmgexp", "razed", "bgold", "denies", "exp_denied", "gold", "gold_spent", "exp", "actions", "secs", "consumables", "wards", "time_earning_exp", "bloodlust", "doublekill", "triplekill", "quadkill", "annihilation", "ks3", "ks4", "ks5", "ks6", "ks7", "ks8", "ks9", "ks10", "ks15", "smackdown", "humiliation", "nemesis", "retribution");

            foreach($this->input['player_stats'] AS $aid => $h)
            {   foreach($this->input['player_stats'][$aid] AS $hid => $v)
                {   foreach($sa AS $k)   // numeric stats
                    {   if(isset($v[$k]) ) {
				            if(is_numeric($v[$k]) ){$tmp=$v[$k]; } else {$tmp = 0; }
			            } else {$tmp = 0; }
                        $this->hero_stats[$aid][$this->heroes[$hid]][$k] = $tmp;
	                    if(isset($this->match_stats[$aid][$k]) ){
                            $this->match_stats[$aid][$k] += $tmp;
	                    } else {$this->match_stats[$aid][$k] = $tmp; }
	                    if(isset($this->team_stats[$v['team']][$k]) ){
                            $this->team_stats[$v['team']][$k] += $tmp;
    	                } else {$this->team_stats[$v['team']][$k] = $tmp; }
                    }
                    // clan_id
                    $this->match_stats[$aid]['clan_id']=$v['clan_id'];
                    // hero_id
                    $this->match_stats[$aid]['hero_id']=$this->heroes[$hid];
                    // position
                    $this->match_stats[$aid]['position']=$v['position'];
                    // team
                    $this->match_stats[$aid]['team']=$v['team'];
                    // level
                    $this->match_stats[$aid]['level']=$v['level'];
                }  // just in case
                $this->match_stats[$aid]['games_played']=1;
                if($this->match_stats[$aid]['wins'] > 1) {$this->match_stats[$aid]['wins']=1; }
                if($this->match_stats[$aid]['losses'] > 1) {$this->match_stats[$aid]['losses']=1; }
                if($this->match_stats[$aid]['concedes'] > 1) {$this->match_stats[$aid]['concedes']=1; }
                if($this->match_stats[$aid]['kicked'] > 1) {$this->match_stats[$aid]['kicked']=1; }
                // SETUP EM STATS
                if($this->match_info['em'] == 1){
                    $this->match_stats[$aid]['em_played'] = 1;
                    $this->match_stats[$aid]['em_gold'] = $this->match_stats[$aid]['gold'];
                    $this->match_stats[$aid]['em_exp'] = $this->match_stats[$aid]['exp'];
                    $this->match_stats[$aid]['em_secs'] = $this->match_stats[$aid]['secs'];
                } else { $this->match_stats[$aid]['em_played'] = $this->match_stats[$aid]['em_gold'] = $this->match_stats[$aid]['em_exp'] = $this->match_stats[$aid]['em_secs'] = 0; }

            }

        }

        // set player stats
        function set_player_stats(){
            $insert = "INSERT INTO hon_stats.player_stats (account_id, acc_games_played, acc_wins, acc_losses, acc_concedes, acc_concedevotes, acc_buybacks, acc_discos, acc_kicked, acc_pub_skill, acc_pub_count, acc_pub_pset, acc_herokills, acc_herodmg, acc_heroexp, acc_herokillsgold, acc_heroassists, acc_deaths, acc_goldlost2death, acc_secs_dead, acc_teamcreepkills, acc_teamcreepdmg, acc_teamcreepexp, acc_teamcreepgold, acc_neutralcreepkills, acc_neutralcreepdmg, acc_neutralcreepexp, acc_neutralcreepgold, acc_bdmg, acc_bdmgexp, acc_razed, acc_bgold, acc_denies, acc_exp_denied, acc_gold, acc_gold_spent, acc_exp, acc_actions, acc_secs, acc_consumables, acc_wards, acc_em_played, acc_time_earning_exp, acc_bloodlust, acc_doublekill, acc_triplekill, acc_quadkill, acc_annihilation, acc_ks3, acc_ks4, acc_ks5, acc_ks6, acc_ks7, acc_ks8, acc_ks9, acc_ks10, acc_ks15, acc_smackdown, acc_humiliation, acc_nemesis, acc_retribution) VALUES ";
            foreach($this->match_stats AS $aid => $v)
            {   $account_id = mysql_real_escape_string($aid);
                $insert .="('{$account_id}', ";
                $insert .="'{$v['games_played']}', ";
                $insert .="'{$v['wins']}', ";
                $insert .="'{$v['losses']}', ";
                $insert .="'{$v['concedes']}', ";
                $insert .="'{$v['concedevotes']}', ";
                $insert .="'{$v['buybacks']}', ";
                $insert .="'{$v['discos']}', ";
                $insert .="'{$v['kicked']}', ";
                $insert .="'{$v['pub_skill']}', ";
                $insert .="'{$v['pub_count']}', ";
                $insert .="'1', ";  // acc_pub_pset
                $insert .="'{$v['herokills']}', ";
                $insert .="'{$v['herodmg']}', ";
                $insert .="'{$v['heroexp']}', ";
                $insert .="'{$v['herokillsgold']}', ";
                $insert .="'{$v['heroassists']}', ";
                $insert .="'{$v['deaths']}', ";
                $insert .="'{$v['goldlost2death']}', ";
                $insert .="'{$v['secs_dead']}', ";
                $insert .="'{$v['teamcreepkills']}', ";
                $insert .="'{$v['teamcreepdmg']}', ";
                $insert .="'{$v['teamcreepexp']}', ";
                $insert .="'{$v['teamcreepgold']}', ";
                $insert .="'{$v['neutralcreepkills']}', ";
                $insert .="'{$v['neutralcreepdmg']}', ";
                $insert .="'{$v['neutralcreepexp']}', ";
                $insert .="'{$v['neutralcreepgold']}', ";
                $insert .="'{$v['bdmg']}', ";
                $insert .="'{$v['bdmgexp']}', ";
                $insert .="'{$v['razed']}', ";
                $insert .="'{$v['bgold']}', ";
                $insert .="'{$v['denies']}', ";
                $insert .="'{$v['exp_denied']}', ";
                $insert .="'{$v['gold']}', ";
                $insert .="'{$v['gold_spent']}', ";
                $insert .="'{$v['exp']}', ";
                $insert .="'{$v['actions']}', ";
                $insert .="'{$v['secs']}', ";
                $insert .="'{$v['consumables']}', ";
                $insert .="'{$v['wards']}', ";
                $insert .="'{$v['em_played']}', ";
                $insert .="'{$v['time_earning_exp']}', ";
                $insert .="'{$v['bloodlust']}', ";
                $insert .="'{$v['doublekill']}', ";
                $insert .="'{$v['triplekill']}', ";
                $insert .="'{$v['quadkill']}', ";
                $insert .="'{$v['annihilation']}', ";
                $insert .="'{$v['ks3']}', ";
                $insert .="'{$v['ks4']}', ";
                $insert .="'{$v['ks5']}', ";
                $insert .="'{$v['ks6']}', ";
                $insert .="'{$v['ks7']}', ";
                $insert .="'{$v['ks8']}', ";
                $insert .="'{$v['ks9']}', ";
                $insert .="'{$v['ks10']}', ";
                $insert .="'{$v['ks15']}', ";
                $insert .="'{$v['smackdown']}', ";
                $insert .="'{$v['humiliation']}', ";
                $insert .="'{$v['nemesis']}', ";
                $insert .="'{$v['retribution']}' ";  // last
                $insert .="),";
            } $insert = trim($insert, ",");
            $insert .= " ON DUPLICATE KEY UPDATE ";
            $insert .= "acc_games_played=acc_games_played+VALUES(acc_games_played), ";
            $insert .= "acc_wins=acc_wins+VALUES(acc_wins), ";
            $insert .= "acc_losses=acc_losses+VALUES(acc_losses), ";
            $insert .= "acc_concedes=acc_concedes+VALUES(acc_concedes), ";
            $insert .= "acc_concedevotes=acc_concedevotes+VALUES(acc_concedevotes), ";
            $insert .= "acc_buybacks=acc_buybacks+VALUES(acc_buybacks), ";
            $insert .= "acc_discos=acc_discos+VALUES(acc_discos), ";
            $insert .= "acc_kicked=acc_kicked+VALUES(acc_kicked), ";
            $insert .= "acc_pub_skill=acc_pub_skill+VALUES(acc_pub_skill), ";
            $insert .= "acc_pub_count=acc_pub_count+VALUES(acc_pub_count), ";
            $insert .= "acc_pub_pset=VALUES(acc_pub_pset), ";
            $insert .= "acc_herokills=acc_herokills+VALUES(acc_herokills), ";
            $insert .= "acc_herodmg=acc_herodmg+VALUES(acc_herodmg), ";
            $insert .= "acc_heroexp=acc_heroexp+VALUES(acc_heroexp), ";
            $insert .= "acc_herokillsgold=acc_herokillsgold+VALUES(acc_herokillsgold), ";
            $insert .= "acc_heroassists=acc_heroassists+VALUES(acc_heroassists), ";
            $insert .= "acc_deaths=acc_deaths+VALUES(acc_deaths), ";
            $insert .= "acc_goldlost2death=acc_goldlost2death+VALUES(acc_goldlost2death), ";
            $insert .= "acc_secs_dead=acc_secs_dead+VALUES(acc_secs_dead), ";
            $insert .= "acc_teamcreepkills=acc_teamcreepkills+VALUES(acc_teamcreepkills), ";
            $insert .= "acc_teamcreepdmg=acc_teamcreepdmg+VALUES(acc_teamcreepdmg), ";
            $insert .= "acc_teamcreepexp=acc_teamcreepexp+VALUES(acc_teamcreepexp), ";
            $insert .= "acc_teamcreepgold=acc_teamcreepgold+VALUES(acc_teamcreepgold), ";
            $insert .= "acc_neutralcreepkills=acc_neutralcreepkills+VALUES(acc_neutralcreepkills), ";
            $insert .= "acc_neutralcreepdmg=acc_neutralcreepdmg+VALUES(acc_neutralcreepdmg), ";
            $insert .= "acc_neutralcreepexp=acc_neutralcreepexp+VALUES(acc_neutralcreepexp), ";
            $insert .= "acc_neutralcreepgold=acc_neutralcreepgold+VALUES(acc_neutralcreepgold), ";
            $insert .= "acc_bdmg=acc_bdmg+VALUES(acc_bdmg), ";
            $insert .= "acc_bdmgexp=acc_bdmgexp+VALUES(acc_bdmgexp), ";
            $insert .= "acc_razed=acc_razed+VALUES(acc_razed), ";
            $insert .= "acc_bgold=acc_bgold+VALUES(acc_bgold), ";
            $insert .= "acc_denies=acc_denies+VALUES(acc_denies), ";
            $insert .= "acc_exp_denied=acc_exp_denied+VALUES(acc_exp_denied), ";
            $insert .= "acc_gold=acc_gold+VALUES(acc_gold), ";
            $insert .= "acc_gold_spent=acc_gold_spent+VALUES(acc_gold_spent), ";
            $insert .= "acc_exp=acc_exp+VALUES(acc_exp), ";
            $insert .= "acc_actions=acc_actions+VALUES(acc_actions), ";
            $insert .= "acc_secs=acc_secs+VALUES(acc_secs), ";
            $insert .= "acc_consumables=acc_consumables+VALUES(acc_consumables), ";
            $insert .= "acc_wards=acc_wards+VALUES(acc_wards), ";
            $insert .= "acc_em_played=acc_em_played+VALUES(acc_em_played), ";
            $insert .= "acc_time_earning_exp=acc_time_earning_exp+VALUES(acc_time_earning_exp), ";
            $insert .= "acc_bloodlust=acc_bloodlust+VALUES(acc_bloodlust), ";
            $insert .= "acc_doublekill=acc_doublekill+VALUES(acc_doublekill), ";
            $insert .= "acc_triplekill=acc_triplekill+VALUES(acc_triplekill), ";
            $insert .= "acc_quadkill=acc_quadkill+VALUES(acc_quadkill), ";
            $insert .= "acc_annihilation=acc_annihilation+VALUES(acc_annihilation), ";
            $insert .= "acc_ks3=acc_ks3+VALUES(acc_ks3), ";
            $insert .= "acc_ks4=acc_ks4+VALUES(acc_ks4), ";
            $insert .= "acc_ks5=acc_ks5+VALUES(acc_ks5), ";
            $insert .= "acc_ks6=acc_ks6+VALUES(acc_ks6), ";
            $insert .= "acc_ks7=acc_ks7+VALUES(acc_ks7), ";
            $insert .= "acc_ks8=acc_ks8+VALUES(acc_ks8), ";
            $insert .= "acc_ks9=acc_ks9+VALUES(acc_ks9), ";
            $insert .= "acc_ks10=acc_ks10+VALUES(acc_ks10), ";
            $insert .= "acc_ks15=acc_ks15+VALUES(acc_ks15), ";
            $insert .= "acc_smackdown=acc_smackdown+VALUES(acc_smackdown), ";
            $insert .= "acc_humiliation=acc_humiliation+VALUES(acc_humiliation), ";
            $insert .= "acc_nemesis=acc_nemesis+VALUES(acc_nemesis), ";
            $insert .= "acc_retribution=acc_retribution+VALUES(acc_retribution)";

            if($result = mysql_query($insert, $this->write) )
            {    $this->output['player_stats']="OK";
            } else { $this->output['player_stats']="Could not connect."; }
        }

        // set player stats
        function set_ranked_stats(){
            $insert = "INSERT INTO hon_stats.ranked_stats (account_id, rnk_games_played, rnk_wins, rnk_losses, rnk_concedes, rnk_concedevotes, rnk_buybacks, rnk_discos, rnk_kicked, rnk_amm_solo_rating, rnk_amm_solo_count, rnk_amm_solo_conf, rnk_amm_solo_prov, rnk_amm_solo_pset, rnk_amm_team_rating, rnk_amm_team_count, rnk_amm_team_conf, rnk_amm_team_prov, rnk_amm_team_pset, rnk_herokills, rnk_herodmg, rnk_heroexp, rnk_herokillsgold, rnk_heroassists, rnk_deaths, rnk_goldlost2death, rnk_secs_dead, rnk_teamcreepkills, rnk_teamcreepdmg, rnk_teamcreepexp, rnk_teamcreepgold, rnk_neutralcreepkills, rnk_neutralcreepdmg, rnk_neutralcreepexp, rnk_neutralcreepgold, rnk_bdmg, rnk_bdmgexp, rnk_razed, rnk_bgold, rnk_denies, rnk_exp_denied, rnk_gold, rnk_gold_spent, rnk_exp, rnk_actions, rnk_secs, rnk_consumables, rnk_wards, rnk_em_played, rnk_time_earning_exp, rnk_bloodlust, rnk_doublekill, rnk_triplekill, rnk_quadkill, rnk_annihilation, rnk_ks3, rnk_ks4, rnk_ks5, rnk_ks6, rnk_ks7, rnk_ks8, rnk_ks9, rnk_ks10, rnk_ks15, rnk_smackdown, rnk_humiliation, rnk_nemesis, rnk_retribution) VALUES ";
            foreach($this->match_stats AS $aid => $v)
            {   if($v['amm_solo_count'] > 0) {$v['amm_solo_pset'] = 1; $v['amm_solo_prov'] = 1; }  // first game always set pset for ranked matches
                if($v['amm_team_count'] > 0) {$v['amm_team_pset'] = 1; $v['amm_team_prov'] = 1;  } // always add provisional game
                $account_id = mysql_real_escape_string($aid);
                $insert .="('{$account_id}', ";
                $insert .="'{$v['games_played']}', ";
                $insert .="'{$v['wins']}', ";
                $insert .="'{$v['losses']}', ";
                $insert .="'{$v['concedes']}', ";
                $insert .="'{$v['concedevotes']}', ";
                $insert .="'{$v['buybacks']}', ";
                $insert .="'{$v['discos']}', ";
                $insert .="'{$v['kicked']}', ";
                $insert .="'{$v['amm_solo_rating']}', ";
                $insert .="'{$v['amm_solo_count']}', ";
                $insert .="'{$v['amm_solo_conf']}', ";
                $insert .="'{$v['amm_solo_prov']}', ";
                $insert .="'{$v['amm_solo_pset']}', ";
                $insert .="'{$v['amm_team_rating']}', ";
                $insert .="'{$v['amm_team_count']}', ";
                $insert .="'{$v['amm_team_conf']}', ";
                $insert .="'{$v['amm_team_prov']}', ";
                $insert .="'{$v['amm_team_pset']}', ";
                $insert .="'{$v['herokills']}', ";
                $insert .="'{$v['herodmg']}', ";
                $insert .="'{$v['heroexp']}', ";
                $insert .="'{$v['herokillsgold']}', ";
                $insert .="'{$v['heroassists']}', ";
                $insert .="'{$v['deaths']}', ";
                $insert .="'{$v['goldlost2death']}', ";
                $insert .="'{$v['secs_dead']}', ";
                $insert .="'{$v['teamcreepkills']}', ";
                $insert .="'{$v['teamcreepdmg']}', ";
                $insert .="'{$v['teamcreepexp']}', ";
                $insert .="'{$v['teamcreepgold']}', ";
                $insert .="'{$v['neutralcreepkills']}', ";
                $insert .="'{$v['neutralcreepdmg']}', ";
                $insert .="'{$v['neutralcreepexp']}', ";
                $insert .="'{$v['neutralcreepgold']}', ";
                $insert .="'{$v['bdmg']}', ";
                $insert .="'{$v['bdmgexp']}', ";
                $insert .="'{$v['razed']}', ";
                $insert .="'{$v['bgold']}', ";
                $insert .="'{$v['denies']}', ";
                $insert .="'{$v['exp_denied']}', ";
                $insert .="'{$v['gold']}', ";
                $insert .="'{$v['gold_spent']}', ";
                $insert .="'{$v['exp']}', ";
                $insert .="'{$v['actions']}', ";
                $insert .="'{$v['secs']}', ";
                $insert .="'{$v['consumables']}', ";
                $insert .="'{$v['wards']}', ";
                $insert .="'{$v['em_played']}', ";
                $insert .="'{$v['time_earning_exp']}', ";
                $insert .="'{$v['bloodlust']}', ";
                $insert .="'{$v['doublekill']}', ";
                $insert .="'{$v['triplekill']}', ";
                $insert .="'{$v['quadkill']}', ";
                $insert .="'{$v['annihilation']}', ";
                $insert .="'{$v['ks3']}', ";
                $insert .="'{$v['ks4']}', ";
                $insert .="'{$v['ks5']}', ";
                $insert .="'{$v['ks6']}', ";
                $insert .="'{$v['ks7']}', ";
                $insert .="'{$v['ks8']}', ";
                $insert .="'{$v['ks9']}', ";
                $insert .="'{$v['ks10']}', ";
                $insert .="'{$v['ks15']}', ";
                $insert .="'{$v['smackdown']}', ";
                $insert .="'{$v['humiliation']}', ";
                $insert .="'{$v['nemesis']}', ";
                $insert .="'{$v['retribution']}' ";  // last
                $insert .="),";
            } $insert = trim($insert, ",");
            $insert .= " ON DUPLICATE KEY UPDATE ";
            $insert .= "rnk_games_played=rnk_games_played+VALUES(rnk_games_played), ";
            $insert .= "rnk_wins=rnk_wins+VALUES(rnk_wins), ";
            $insert .= "rnk_losses=rnk_losses+VALUES(rnk_losses), ";
            $insert .= "rnk_concedes=rnk_concedes+VALUES(rnk_concedes), ";
            $insert .= "rnk_concedevotes=rnk_concedevotes+VALUES(rnk_concedevotes), ";
            $insert .= "rnk_buybacks=rnk_buybacks+VALUES(rnk_buybacks), ";
            $insert .= "rnk_discos=rnk_discos+VALUES(rnk_discos), ";
            $insert .= "rnk_kicked=rnk_kicked+VALUES(rnk_kicked), ";
            $insert .= "rnk_amm_solo_rating=rnk_amm_solo_rating+VALUES(rnk_amm_solo_rating), ";
            $insert .= "rnk_amm_solo_count=rnk_amm_solo_count+VALUES(rnk_amm_solo_count), ";
            $insert .= "rnk_amm_solo_conf=rnk_amm_solo_conf+VALUES(rnk_amm_solo_conf), ";
            $insert .= "rnk_amm_solo_prov=rnk_amm_solo_prov+VALUES(rnk_amm_solo_prov), ";
            $insert .= "rnk_amm_solo_pset=rnk_amm_solo_pset+VALUES(rnk_amm_solo_pset), ";
            $insert .= "rnk_amm_team_rating=rnk_amm_team_rating+VALUES(rnk_amm_team_rating), ";
            $insert .= "rnk_amm_team_count=rnk_amm_team_count+VALUES(rnk_amm_team_count), ";
            $insert .= "rnk_amm_team_conf=rnk_amm_team_conf+VALUES(rnk_amm_team_conf), ";
            $insert .= "rnk_amm_team_prov=rnk_amm_team_prov+VALUES(rnk_amm_team_prov), ";
            $insert .= "rnk_amm_team_pset=rnk_amm_team_pset+VALUES(rnk_amm_team_pset), ";
            $insert .= "rnk_herokills=rnk_herokills+VALUES(rnk_herokills), ";
            $insert .= "rnk_herodmg=rnk_herodmg+VALUES(rnk_herodmg), ";
            $insert .= "rnk_heroexp=rnk_heroexp+VALUES(rnk_heroexp), ";
            $insert .= "rnk_herokillsgold=rnk_herokillsgold+VALUES(rnk_herokillsgold), ";
            $insert .= "rnk_heroassists=rnk_heroassists+VALUES(rnk_heroassists), ";
            $insert .= "rnk_deaths=rnk_deaths+VALUES(rnk_deaths), ";
            $insert .= "rnk_goldlost2death=rnk_goldlost2death+VALUES(rnk_goldlost2death), ";
            $insert .= "rnk_secs_dead=rnk_secs_dead+VALUES(rnk_secs_dead), ";
            $insert .= "rnk_teamcreepkills=rnk_teamcreepkills+VALUES(rnk_teamcreepkills), ";
            $insert .= "rnk_teamcreepdmg=rnk_teamcreepdmg+VALUES(rnk_teamcreepdmg), ";
            $insert .= "rnk_teamcreepexp=rnk_teamcreepexp+VALUES(rnk_teamcreepexp), ";
            $insert .= "rnk_teamcreepgold=rnk_teamcreepgold+VALUES(rnk_teamcreepgold), ";
            $insert .= "rnk_neutralcreepkills=rnk_neutralcreepkills+VALUES(rnk_neutralcreepkills), ";
            $insert .= "rnk_neutralcreepdmg=rnk_neutralcreepdmg+VALUES(rnk_neutralcreepdmg), ";
            $insert .= "rnk_neutralcreepexp=rnk_neutralcreepexp+VALUES(rnk_neutralcreepexp), ";
            $insert .= "rnk_neutralcreepgold=rnk_neutralcreepgold+VALUES(rnk_neutralcreepgold), ";
            $insert .= "rnk_bdmg=rnk_bdmg+VALUES(rnk_bdmg), ";
            $insert .= "rnk_bdmgexp=rnk_bdmgexp+VALUES(rnk_bdmgexp), ";
            $insert .= "rnk_razed=rnk_razed+VALUES(rnk_razed), ";
            $insert .= "rnk_bgold=rnk_bgold+VALUES(rnk_bgold), ";
            $insert .= "rnk_denies=rnk_denies+VALUES(rnk_denies), ";
            $insert .= "rnk_exp_denied=rnk_exp_denied+VALUES(rnk_exp_denied), ";
            $insert .= "rnk_gold=rnk_gold+VALUES(rnk_gold), ";
            $insert .= "rnk_gold_spent=rnk_gold_spent+VALUES(rnk_gold_spent), ";
            $insert .= "rnk_exp=rnk_exp+VALUES(rnk_exp), ";
            $insert .= "rnk_actions=rnk_actions+VALUES(rnk_actions), ";
            $insert .= "rnk_secs=rnk_secs+VALUES(rnk_secs), ";
            $insert .= "rnk_consumables=rnk_consumables+VALUES(rnk_consumables), ";
            $insert .= "rnk_wards=rnk_wards+VALUES(rnk_wards), ";
            $insert .= "rnk_em_played=rnk_em_played+VALUES(rnk_em_played), ";
            $insert .= "rnk_time_earning_exp=rnk_time_earning_exp+VALUES(rnk_time_earning_exp), ";
            $insert .= "rnk_bloodlust=rnk_bloodlust+VALUES(rnk_bloodlust), ";
            $insert .= "rnk_doublekill=rnk_doublekill+VALUES(rnk_doublekill), ";
            $insert .= "rnk_triplekill=rnk_triplekill+VALUES(rnk_triplekill), ";
            $insert .= "rnk_quadkill=rnk_quadkill+VALUES(rnk_quadkill), ";
            $insert .= "rnk_annihilation=rnk_annihilation+VALUES(rnk_annihilation), ";
            $insert .= "rnk_ks3=rnk_ks3+VALUES(rnk_ks3), ";
            $insert .= "rnk_ks4=rnk_ks4+VALUES(rnk_ks4), ";
            $insert .= "rnk_ks5=rnk_ks5+VALUES(rnk_ks5), ";
            $insert .= "rnk_ks6=rnk_ks6+VALUES(rnk_ks6), ";
            $insert .= "rnk_ks7=rnk_ks7+VALUES(rnk_ks7), ";
            $insert .= "rnk_ks8=rnk_ks8+VALUES(rnk_ks8), ";
            $insert .= "rnk_ks9=rnk_ks9+VALUES(rnk_ks9), ";
            $insert .= "rnk_ks10=rnk_ks10+VALUES(rnk_ks10), ";
            $insert .= "rnk_ks15=rnk_ks15+VALUES(rnk_ks15), ";
            $insert .= "rnk_smackdown=rnk_smackdown+VALUES(rnk_smackdown), ";
            $insert .= "rnk_humiliation=rnk_humiliation+VALUES(rnk_humiliation), ";
            $insert .= "rnk_nemesis=rnk_nemesis+VALUES(rnk_nemesis), ";
            $insert .= "rnk_retribution=rnk_retribution+VALUES(rnk_retribution)";
            if($result = mysql_query($insert, $this->write) )
            {    $this->output['ranked_stats']="OK";
            } else { $this->output['ranked_stats']="Could not connect."; }
        }


        // set player stats
        function set_player_hero_stats(){
            $insert = "INSERT INTO hon_stats.player_hero_stats (account_id, hero_id, ph_wins, ph_losses, ph_concedes, ph_concedevotes, ph_buybacks, ph_discos, ph_kicked, ph_pub_skill, ph_pub_count, ph_amm_solo_rating, ph_amm_solo_count, ph_amm_team_rating, ph_amm_team_count, ph_herokills, ph_herodmg, ph_heroexp, ph_herokillsgold, ph_heroassists, ph_deaths, ph_goldlost2death, ph_secs_dead, ph_teamcreepkills, ph_teamcreepdmg, ph_teamcreepexp, ph_teamcreepgold, ph_neutralcreepkills, ph_neutralcreepdmg, ph_neutralcreepexp, ph_neutralcreepgold, ph_bdmg, ph_bdmgexp, ph_razed, ph_bgold, ph_denies, ph_exp_denied, ph_gold, ph_gold_spent, ph_exp, ph_actions, ph_secs, ph_consumables, ph_wards, ph_time_earning_exp, ph_bloodlust, ph_doublekill, ph_triplekill, ph_quadkill, ph_annihilation, ph_ks3, ph_ks4, ph_ks5, ph_ks6, ph_ks7, ph_ks8, ph_ks9, ph_ks10, ph_ks15, ph_smackdown, ph_humiliation, ph_nemesis, ph_retribution) VALUES ";
            foreach($this->hero_stats AS $aid => $h)
            {   $account_id = mysql_real_escape_string($aid);
                foreach($this->hero_stats[$aid] AS $hid => $v)
                {   $hero_id = mysql_real_escape_string($hid);
                    $insert .="('{$account_id}', ";
                    $insert .="'{$hero_id}', ";
                    $insert .="'{$v['wins']}', ";
                    $insert .="'{$v['losses']}', ";
                    $insert .="'{$v['concedes']}', ";
                    $insert .="'{$v['concedevotes']}', ";
                    $insert .="'{$v['buybacks']}', ";
                    $insert .="'{$v['discos']}', ";
                    $insert .="'{$v['kicked']}',";
                    $insert .="'{$v['pub_skill']}', ";
                    $insert .="'{$v['pub_count']}', ";
                    $insert .="'{$v['amm_solo_rating']}', ";
                    $insert .="'{$v['amm_solo_count']}', ";
                    $insert .="'{$v['amm_team_rating']}', ";
                    $insert .="'{$v['amm_team_count']}', ";
                    $insert .="'{$v['herokills']}', ";
                    $insert .="'{$v['herodmg']}', ";
                    $insert .="'{$v['heroexp']}', ";
                    $insert .="'{$v['herokillsgold']}', ";
                    $insert .="'{$v['heroassists']}', ";
                    $insert .="'{$v['deaths']}', ";
                    $insert .="'{$v['goldlost2death']}', ";
                    $insert .="'{$v['secs_dead']}', ";
                    $insert .="'{$v['teamcreepkills']}', ";
                    $insert .="'{$v['teamcreepdmg']}', ";
                    $insert .="'{$v['teamcreepexp']}', ";
                    $insert .="'{$v['teamcreepgold']}', ";
                    $insert .="'{$v['neutralcreepkills']}', ";
                    $insert .="'{$v['neutralcreepdmg']}', ";
                    $insert .="'{$v['neutralcreepexp']}', ";
                    $insert .="'{$v['neutralcreepgold']}', ";
                    $insert .="'{$v['bdmg']}', ";
                    $insert .="'{$v['bdmgexp']}', ";
                    $insert .="'{$v['razed']}', ";
                    $insert .="'{$v['bgold']}', ";
                    $insert .="'{$v['denies']}', ";
                    $insert .="'{$v['exp_denied']}', ";
                    $insert .="'{$v['gold']}', ";
                    $insert .="'{$v['gold_spent']}', ";
                    $insert .="'{$v['exp']}', ";
                    $insert .="'{$v['actions']}', ";
                    $insert .="'{$v['secs']}', ";
                    $insert .="'{$v['consumables']}', ";
                    $insert .="'{$v['wards']}', ";
                    $insert .="'{$v['time_earning_exp']}', ";
                    $insert .="'{$v['bloodlust']}', ";
                    $insert .="'{$v['doublekill']}', ";
                    $insert .="'{$v['triplekill']}', ";
                    $insert .="'{$v['quadkill']}', ";
                    $insert .="'{$v['annihilation']}', ";
                    $insert .="'{$v['ks3']}', ";
                    $insert .="'{$v['ks4']}', ";
                    $insert .="'{$v['ks5']}', ";
                    $insert .="'{$v['ks6']}', ";
                    $insert .="'{$v['ks7']}', ";
                    $insert .="'{$v['ks8']}', ";
                    $insert .="'{$v['ks9']}', ";
                    $insert .="'{$v['ks10']}', ";
                    $insert .="'{$v['ks15']}', ";
                    $insert .="'{$v['smackdown']}', ";
                    $insert .="'{$v['humiliation']}', ";
                    $insert .="'{$v['nemesis']}', ";
                    $insert .="'{$v['retribution']}' ";  // last
                    $insert .="),";
                }
            } $insert = trim($insert, ",");
            $insert .= " ON DUPLICATE KEY UPDATE ";
            $insert .= "ph_used=ph_used+1, ";
            $insert .= "ph_wins=ph_wins+VALUES(ph_wins), ";
            $insert .= "ph_losses=ph_losses+VALUES(ph_losses), ";
            $insert .= "ph_concedes=ph_concedes+VALUES(ph_concedes), ";
            $insert .= "ph_concedevotes=ph_concedevotes+VALUES(ph_concedevotes), ";
            $insert .= "ph_buybacks=ph_buybacks+VALUES(ph_buybacks), ";
            $insert .= "ph_discos=ph_discos+VALUES(ph_discos), ";
            $insert .= "ph_kicked=ph_kicked+VALUES(ph_kicked), ";
            $insert .= "ph_pub_skill=ph_pub_skill+VALUES(ph_pub_skill), ";
            $insert .= "ph_pub_count=ph_pub_count+VALUES(ph_pub_count), ";
            $insert .= "ph_amm_solo_rating=ph_amm_solo_rating+VALUES(ph_amm_solo_rating), ";
            $insert .= "ph_amm_solo_count=ph_amm_solo_count+VALUES(ph_amm_solo_count), ";
            $insert .= "ph_amm_team_rating=ph_amm_team_rating+VALUES(ph_amm_team_rating), ";
            $insert .= "ph_amm_team_count=ph_amm_team_count+VALUES(ph_amm_team_count), ";
            $insert .= "ph_herokills=ph_herokills+VALUES(ph_herokills), ";
            $insert .= "ph_herodmg=ph_herodmg+VALUES(ph_herodmg), ";
            $insert .= "ph_heroexp=ph_heroexp+VALUES(ph_heroexp), ";
            $insert .= "ph_herokillsgold=ph_herokillsgold+VALUES(ph_herokillsgold), ";
            $insert .= "ph_heroassists=ph_heroassists+VALUES(ph_heroassists), ";
            $insert .= "ph_deaths=ph_deaths+VALUES(ph_deaths), ";
            $insert .= "ph_goldlost2death=ph_goldlost2death+VALUES(ph_goldlost2death), ";
            $insert .= "ph_secs_dead=ph_secs_dead+VALUES(ph_secs_dead), ";
            $insert .= "ph_teamcreepkills=ph_teamcreepkills+VALUES(ph_teamcreepkills), ";
            $insert .= "ph_teamcreepdmg=ph_teamcreepdmg+VALUES(ph_teamcreepdmg), ";
            $insert .= "ph_teamcreepexp=ph_teamcreepexp+VALUES(ph_teamcreepexp), ";
            $insert .= "ph_teamcreepgold=ph_teamcreepgold+VALUES(ph_teamcreepgold), ";
            $insert .= "ph_neutralcreepkills=ph_neutralcreepkills+VALUES(ph_neutralcreepkills), ";
            $insert .= "ph_neutralcreepdmg=ph_neutralcreepdmg+VALUES(ph_neutralcreepdmg), ";
            $insert .= "ph_neutralcreepexp=ph_neutralcreepexp+VALUES(ph_neutralcreepexp), ";
            $insert .= "ph_neutralcreepgold=ph_neutralcreepgold+VALUES(ph_neutralcreepgold), ";
            $insert .= "ph_bdmg=ph_bdmg+VALUES(ph_bdmg), ";
            $insert .= "ph_bdmgexp=ph_bdmgexp+VALUES(ph_bdmgexp), ";
            $insert .= "ph_razed=ph_razed+VALUES(ph_razed), ";
            $insert .= "ph_bgold=ph_bgold+VALUES(ph_bgold), ";
            $insert .= "ph_denies=ph_denies+VALUES(ph_denies), ";
            $insert .= "ph_exp_denied=ph_exp_denied+VALUES(ph_exp_denied), ";
            $insert .= "ph_gold=ph_gold+VALUES(ph_gold), ";
            $insert .= "ph_gold_spent=ph_gold_spent+VALUES(ph_gold_spent), ";
            $insert .= "ph_exp=ph_exp+VALUES(ph_exp), ";
            $insert .= "ph_actions=ph_actions+VALUES(ph_actions), ";
            $insert .= "ph_secs=ph_secs+VALUES(ph_secs), ";
            $insert .= "ph_consumables=ph_consumables+VALUES(ph_consumables), ";
            $insert .= "ph_wards=ph_wards+VALUES(ph_wards), ";
            $insert .= "ph_time_earning_exp=ph_time_earning_exp+VALUES(ph_time_earning_exp), ";
            $insert .= "ph_bloodlust=ph_bloodlust+VALUES(ph_bloodlust), ";
            $insert .= "ph_doublekill=ph_doublekill+VALUES(ph_doublekill), ";
            $insert .= "ph_triplekill=ph_triplekill+VALUES(ph_triplekill), ";
            $insert .= "ph_quadkill=ph_quadkill+VALUES(ph_quadkill), ";
            $insert .= "ph_annihilation=ph_annihilation+VALUES(ph_annihilation), ";
            $insert .= "ph_ks3=ph_ks3+VALUES(ph_ks3), ";
            $insert .= "ph_ks4=ph_ks4+VALUES(ph_ks4), ";
            $insert .= "ph_ks5=ph_ks5+VALUES(ph_ks5), ";
            $insert .= "ph_ks6=ph_ks6+VALUES(ph_ks6), ";
            $insert .= "ph_ks7=ph_ks7+VALUES(ph_ks7), ";
            $insert .= "ph_ks8=ph_ks8+VALUES(ph_ks8), ";
            $insert .= "ph_ks9=ph_ks9+VALUES(ph_ks9), ";
            $insert .= "ph_ks10=ph_ks10+VALUES(ph_ks10), ";
            $insert .= "ph_ks15=ph_ks15+VALUES(ph_ks15), ";
            $insert .= "ph_smackdown=ph_smackdown+VALUES(ph_smackdown), ";
            $insert .= "ph_humiliation=ph_humiliation+VALUES(ph_humiliation), ";
            $insert .= "ph_nemesis=ph_nemesis+VALUES(ph_nemesis), ";
            $insert .= "ph_retribution=ph_retribution+VALUES(ph_retribution)";

            if($result = mysql_query($insert, $this->write) )
            {    $this->output['player_hero_stats']="OK";
            } else { $this->output['player_hero_stats']="Could not connect.$insert"; }

        }
        // set match stats
        function set_match_stats(){
            $usage = "INSERT INTO hon_stats.usage_heroes (date, hero_id, used, wins, losses, herokills, deaths, assists, teamcreepkills, neutralcreepkills, denies, gold, em_gold, exp, em_exp, secs, em_secs) VALUES ";
            $insert = "INSERT INTO hon_stats.match_stats (match_id, account_id, clan_id, hero_id, position, team, level, wins, losses, concedes, concedevotes, buybacks, discos, kicked, pub_skill, pub_count, amm_solo_rating, amm_solo_count, amm_team_rating, amm_team_count, herokills, herodmg, heroexp, herokillsgold, heroassists, deaths, goldlost2death, secs_dead, teamcreepkills, teamcreepdmg, teamcreepexp, teamcreepgold, neutralcreepkills, neutralcreepdmg, neutralcreepexp, neutralcreepgold, bdmg, bdmgexp, razed, bgold, denies, exp_denied, gold, gold_spent, exp, actions, secs, consumables, wards, time_earning_exp, bloodlust, doublekill, triplekill, quadkill, annihilation, ks3, ks4, ks5, ks6, ks7, ks8, ks9, ks10, ks15, smackdown, humiliation, nemesis, retribution) VALUES ";
            foreach($this->match_stats AS $aid => $v)
            {   $usage .= "(CURDATE(), '{$v['hero_id']}', 1, '{$v['wins']}','{$v['losses']}','{$v['herokills']}','{$v['deaths']}','{$v['heroassists']}','{$v['teamcreepkills']}','{$v['neutralcreepkills']}','{$v['denies']}','{$v['gold']}','{$v['em_gold']}','{$v['exp']}','{$v['em_exp']}','{$v['secs']}','{$v['em_secs']}'),";
                $account_id = mysql_real_escape_string($aid);
                $insert .="('{$this->match_id}', ";
                $insert .="'{$account_id}', ";
                $insert .="'{$v['clan_id']}', ";
                $insert .="'{$v['hero_id']}', ";
                $insert .="'{$v['position']}', ";
                $insert .="'{$v['team']}', ";
                $insert .="'{$v['level']}', ";
                $insert .="'{$v['wins']}', ";
                $insert .="'{$v['losses']}', ";
                $insert .="'{$v['concedes']}', ";
                $insert .="'{$v['concedevotes']}', ";
                $insert .="'{$v['buybacks']}', ";
                $insert .="'{$v['discos']}', ";
                $insert .="'{$v['kicked']}', ";
                $insert .="'{$v['pub_skill']}', ";
                $insert .="'{$v['pub_count']}', ";
                $insert .="'{$v['amm_solo_rating']}', ";
                $insert .="'{$v['amm_solo_count']}', ";
                $insert .="'{$v['amm_team_rating']}', ";
                $insert .="'{$v['amm_team_count']}', ";
                $insert .="'{$v['herokills']}', ";
                $insert .="'{$v['herodmg']}', ";
                $insert .="'{$v['heroexp']}', ";
                $insert .="'{$v['herokillsgold']}', ";
                $insert .="'{$v['heroassists']}', ";
                $insert .="'{$v['deaths']}', ";
                $insert .="'{$v['goldlost2death']}', ";
                $insert .="'{$v['secs_dead']}', ";
                $insert .="'{$v['teamcreepkills']}', ";
                $insert .="'{$v['teamcreepdmg']}', ";
                $insert .="'{$v['teamcreepexp']}', ";
                $insert .="'{$v['teamcreepgold']}', ";
                $insert .="'{$v['neutralcreepkills']}', ";
                $insert .="'{$v['neutralcreepdmg']}', ";
                $insert .="'{$v['neutralcreepexp']}', ";
                $insert .="'{$v['neutralcreepgold']}', ";
                $insert .="'{$v['bdmg']}', ";
                $insert .="'{$v['bdmgexp']}', ";
                $insert .="'{$v['razed']}', ";
                $insert .="'{$v['bgold']}', ";
                $insert .="'{$v['denies']}', ";
                $insert .="'{$v['exp_denied']}', ";
                $insert .="'{$v['gold']}', ";
                $insert .="'{$v['gold_spent']}', ";
                $insert .="'{$v['exp']}', ";
                $insert .="'{$v['actions']}', ";
                $insert .="'{$v['secs']}', ";
                $insert .="'{$v['consumables']}', ";
                $insert .="'{$v['wards']}', ";
                $insert .="'{$v['time_earning_exp']}', ";
                $insert .="'{$v['bloodlust']}', ";
                $insert .="'{$v['doublekill']}', ";
                $insert .="'{$v['triplekill']}', ";
                $insert .="'{$v['quadkill']}', ";
                $insert .="'{$v['annihilation']}', ";
                $insert .="'{$v['ks3']}', ";
                $insert .="'{$v['ks4']}', ";
                $insert .="'{$v['ks5']}', ";
                $insert .="'{$v['ks6']}', ";
                $insert .="'{$v['ks7']}', ";
                $insert .="'{$v['ks8']}', ";
                $insert .="'{$v['ks9']}', ";
                $insert .="'{$v['ks10']}', ";
                $insert .="'{$v['ks15']}', ";
                $insert .="'{$v['smackdown']}', ";
                $insert .="'{$v['humiliation']}', ";
                $insert .="'{$v['nemesis']}', ";
                $insert .="'{$v['retribution']}' ";  // last
                $insert .="),";
            } $insert = trim($insert, ",");  $usage = trim($usage, ",");
            $insert .= " ON DUPLICATE KEY UPDATE ";
            $insert .= "clan_id=VALUES(clan_id), ";
            $insert .= "hero_id=VALUES(hero_id), ";
            $insert .= "position=VALUES(position), ";
            $insert .= "team=VALUES(team), ";
            $insert .= "level=VALUES(level), ";
            $insert .= "wins=VALUES(wins), ";
            $insert .= "losses=VALUES(losses), ";
            $insert .= "concedes=VALUES(concedes), ";
            $insert .= "concedevotes=VALUES(concedevotes), ";
            $insert .= "buybacks=VALUES(buybacks), ";
            $insert .= "discos=VALUES(discos), ";
            $insert .= "kicked=VALUES(kicked), ";
            $insert .= "pub_skill=VALUES(pub_skill), ";
            $insert .= "pub_count=VALUES(pub_count), ";
            $insert .= "amm_solo_rating=VALUES(amm_solo_rating), ";
            $insert .= "amm_solo_count=VALUES(amm_solo_count), ";
            $insert .= "amm_team_rating=VALUES(amm_team_rating), ";
            $insert .= "amm_team_count=VALUES(amm_team_count), ";
            $insert .= "herokills=VALUES(herokills), ";
            $insert .= "herodmg=VALUES(herodmg), ";
            $insert .= "heroexp=VALUES(heroexp), ";
            $insert .= "herokillsgold=VALUES(herokillsgold), ";
            $insert .= "heroassists=VALUES(heroassists), ";
            $insert .= "deaths=VALUES(deaths), ";
            $insert .= "goldlost2death=VALUES(goldlost2death), ";
            $insert .= "secs_dead=VALUES(secs_dead), ";
            $insert .= "teamcreepkills=VALUES(teamcreepkills), ";
            $insert .= "teamcreepdmg=VALUES(teamcreepdmg), ";
            $insert .= "teamcreepexp=VALUES(teamcreepexp), ";
            $insert .= "teamcreepgold=VALUES(teamcreepgold), ";
            $insert .= "neutralcreepkills=VALUES(neutralcreepkills), ";
            $insert .= "neutralcreepdmg=VALUES(neutralcreepdmg), ";
            $insert .= "neutralcreepexp=VALUES(neutralcreepexp), ";
            $insert .= "neutralcreepgold=VALUES(neutralcreepgold), ";
            $insert .= "bdmg=VALUES(bdmg), bdmgexp=VALUES(bdmgexp), ";
            $insert .= "razed=VALUES(razed), ";
            $insert .= "bgold=VALUES(bgold), ";
            $insert .= "denies=VALUES(denies), ";
            $insert .= "exp_denied=VALUES(exp_denied), ";
            $insert .= "gold_spent=VALUES(gold_spent), ";
            $insert .= "gold=VALUES(gold_spent), ";
            $insert .= "exp=VALUES(exp), ";
            $insert .= "actions=VALUES(actions), ";
            $insert .= "secs=VALUES(secs), ";
            $insert .= "consumables=VALUES(consumables), ";
            $insert .= "wards=VALUES(wards), ";
            $insert .= "time_earning_exp=VALUES(time_earning_exp), ";
            $insert .= "bloodlust=VALUES(bloodlust), ";
            $insert .= "doublekill=VALUES(doublekill), ";
            $insert .= "triplekill=VALUES(triplekill), ";
            $insert .= "quadkill=VALUES(quadkill), ";
            $insert .= "annihilation=VALUES(annihilation), ";
            $insert .= "ks3=VALUES(ks3), ";
            $insert .= "ks4=VALUES(ks4), ";
            $insert .= "ks5=VALUES(ks5), ";
            $insert .= "ks6=VALUES(ks6), ";
            $insert .= "ks7=VALUES(ks7), ";
            $insert .= "ks8=VALUES(ks8), ";
            $insert .= "ks9=VALUES(ks9), ";
            $insert .= "ks10=VALUES(ks10), ";
            $insert .= "ks15=VALUES(ks15), ";
            $insert .= "smackdown=VALUES(smackdown), ";
            $insert .= "humiliation=VALUES(humiliation), ";
            $insert .= "nemesis=VALUES(nemesis), ";
            $insert .= "retribution=VALUES(retribution)";

            if($result = mysql_query($insert, $this->write) )
            {    $this->output['match_stats']="OK";
            } else { $this->output['match_stats']="Could not connect."; }

            // hero usage
            $usage .= " ON DUPLICATE KEY UPDATE used=used+1, wins=wins+VALUES(wins), losses=losses+VALUES(losses), herokills=herokills+VALUES(herokills), deaths=deaths+VALUES(deaths), assists=assists+VALUES(assists), teamcreepkills=teamcreepkills+VALUES(teamcreepkills), neutralcreepkills=neutralcreepkills+VALUES(neutralcreepkills), denies=denies+VALUES(denies), gold=gold+VALUES(gold), em_gold=em_gold+VALUES(em_gold), exp=exp+VALUES(exp), em_exp=em_exp+VALUES(em_exp), secs=secs+VALUES(secs), em_secs=em_secs+VALUES(em_secs)";
            if($result = mysql_query($usage, $this->write) )
            {    $this->output['hero_usage']="OK";
            } else { $this->output['hero_usage']="Could not connect.";  }
        }
                // set match stats
        function set_match_hero_stats(){
            $insert = "INSERT INTO hon_stats.match_hero_stats (match_id, account_id, hero_id, mh_wins, mh_losses, mh_concedes, mh_concedevotes, mh_buybacks, mh_discos, mh_kicked, mh_pub_skill, mh_pub_count, mh_amm_solo_rating, mh_amm_solo_count, mh_amm_team_rating, mh_amm_team_count, mh_herokills, mh_herodmg, mh_heroexp, mh_herokillsgold, mh_heroassists, mh_deaths, mh_goldlost2death, mh_secs_dead, mh_teamcreepkills, mh_teamcreepdmg, mh_teamcreepexp, mh_teamcreepgold, mh_neutralcreepkills, mh_neutralcreepdmg, mh_neutralcreepexp, mh_neutralcreepgold, mh_bdmg, mh_bdmgexp, mh_razed, mh_bgold, mh_denies, mh_exp_denied, mh_gold, mh_gold_spent, mh_exp, mh_actions, mh_secs, mh_consumables, mh_wards, mh_time_earning_exp, mh_bloodlust, mh_doublekill, mh_triplekill, mh_quadkill, mh_annihilation, mh_ks3, mh_ks4, mh_ks5, mh_ks6, mh_ks7, mh_ks8, mh_ks9, mh_ks10, mh_ks15, mh_smackdown, mh_humiliation, mh_nemesis, mh_retribution) VALUES ";
            foreach($this->hero_stats AS $aid => $v)
            {   $account_id = mysql_real_escape_string($aid);
                foreach($this->hero_stats[$aid] AS $hid => $v)
                {   $hero_id = mysql_real_escape_string($hid);
                    $insert .="('{$this->match_id}', ";
                    $insert .="'{$account_id}', ";
                    $insert .="'{$hero_id}', ";
                    $insert .="'{$v['wins']}', ";
                    $insert .="'{$v['losses']}', ";
                    $insert .="'{$v['concedes']}', ";
                    $insert .="'{$v['concedevotes']}', ";
                    $insert .="'{$v['buybacks']}', ";
                    $insert .="'{$v['discos']}', ";
                    $insert .="'{$v['kicked']}', ";
                    $insert .="'{$v['pub_skill']}', ";
                    $insert .="'{$v['pub_count']}', ";
                    $insert .="'{$v['amm_solo_rating']}', ";
                    $insert .="'{$v['amm_solo_count']}', ";
                    $insert .="'{$v['amm_team_rating']}', ";
                    $insert .="'{$v['amm_team_count']}', ";
                    $insert .="'{$v['herokills']}', ";
                    $insert .="'{$v['herodmg']}', ";
                    $insert .="'{$v['heroexp']}', ";
                    $insert .="'{$v['herokillsgold']}', ";
                    $insert .="'{$v['heroassists']}', ";
                    $insert .="'{$v['deaths']}', ";
                    $insert .="'{$v['goldlost2death']}', ";
                    $insert .="'{$v['secs_dead']}', ";
                    $insert .="'{$v['teamcreepkills']}', ";
                    $insert .="'{$v['teamcreepdmg']}', ";
                    $insert .="'{$v['teamcreepexp']}', ";
                    $insert .="'{$v['teamcreepgold']}', ";
                    $insert .="'{$v['neutralcreepkills']}', ";
                    $insert .="'{$v['neutralcreepdmg']}', ";
                    $insert .="'{$v['neutralcreepexp']}', ";
                    $insert .="'{$v['neutralcreepgold']}', ";
                    $insert .="'{$v['bdmg']}', ";
                    $insert .="'{$v['bdmgexp']}', ";
                    $insert .="'{$v['razed']}', ";
                    $insert .="'{$v['bgold']}', ";
                    $insert .="'{$v['denies']}', ";
                    $insert .="'{$v['exp_denied']}', ";
                    $insert .="'{$v['gold']}', ";
                    $insert .="'{$v['gold_spent']}', ";
                    $insert .="'{$v['exp']}', ";
                    $insert .="'{$v['actions']}', ";
                    $insert .="'{$v['secs']}',";
                    $insert .="'{$v['consumables']}', ";
                    $insert .="'{$v['wards']}', ";
                    $insert .="'{$v['time_earning_exp']}', ";
                    $insert .="'{$v['bloodlust']}', ";
                    $insert .="'{$v['doublekill']}', ";
                    $insert .="'{$v['triplekill']}', ";
                    $insert .="'{$v['quadkill']}', ";
                    $insert .="'{$v['annihilation']}', ";
                    $insert .="'{$v['ks3']}', ";
                    $insert .="'{$v['ks4']}', ";
                    $insert .="'{$v['ks5']}', ";
                    $insert .="'{$v['ks6']}', ";
                    $insert .="'{$v['ks7']}', ";
                    $insert .="'{$v['ks8']}', ";
                    $insert .="'{$v['ks9']}', ";
                    $insert .="'{$v['ks10']}', ";
                    $insert .="'{$v['ks15']}', ";
                    $insert .="'{$v['smackdown']}', ";
                    $insert .="'{$v['humiliation']}', ";
                    $insert .="'{$v['nemesis']}', ";
                    $insert .="'{$v['retribution']}' ";  // last
                    $insert .="),";
                }
            } $insert = trim($insert, ",");
            $insert .= " ON DUPLICATE KEY UPDATE ";
            $insert .= "mh_wins=VALUES(mh_wins), ";
            $insert .= "mh_losses=VALUES(mh_losses), ";
            $insert .= "mh_concedes=VALUES(mh_concedes), ";
            $insert .= "mh_concedevotes=VALUES(mh_concedevotes), ";
            $insert .= "mh_buybacks=VALUES(mh_buybacks), ";
            $insert .= "mh_discos=VALUES(mh_discos), ";
            $insert .= "mh_kicked=VALUES(mh_kicked), ";
            $insert .= "mh_pub_skill=VALUES(mh_pub_skill), ";
            $insert .= "mh_pub_count=mh_pub_count+VALUES(mh_pub_count), ";
            $insert .= "mh_amm_solo_rating=mh_amm_solo_rating+VALUES(mh_amm_solo_rating), ";
            $insert .= "mh_amm_solo_count=mh_amm_solo_count+VALUES(mh_amm_solo_count), ";
            $insert .= "mh_amm_team_rating=mh_amm_team_rating+VALUES(mh_amm_team_rating), ";
            $insert .= "mh_amm_team_count=mh_amm_team_count+VALUES(mh_amm_team_count), ";
            $insert .= "mh_herokills=VALUES(mh_herokills), ";
            $insert .= "mh_herodmg=VALUES(mh_herodmg), ";
            $insert .= "mh_heroexp=VALUES(mh_heroexp), ";
            $insert .= "mh_herokillsgold=VALUES(mh_herokillsgold), ";
            $insert .= "mh_heroassists=VALUES(mh_heroassists), ";
            $insert .= "mh_deaths=VALUES(mh_deaths), ";
            $insert .= "mh_goldlost2death=VALUES(mh_goldlost2death), ";
            $insert .= "mh_secs_dead=VALUES(mh_secs_dead), ";
            $insert .= "mh_teamcreepkills=VALUES(mh_teamcreepkills), ";
            $insert .= "mh_teamcreepdmg=VALUES(mh_teamcreepdmg), ";
            $insert .= "mh_teamcreepexp=VALUES(mh_teamcreepexp), ";
            $insert .= "mh_teamcreepgold=VALUES(mh_teamcreepgold), ";
            $insert .= "mh_neutralcreepkills=VALUES(mh_neutralcreepkills), ";
            $insert .= "mh_neutralcreepdmg=VALUES(mh_neutralcreepdmg), ";
            $insert .= "mh_neutralcreepexp=VALUES(mh_neutralcreepexp), ";
            $insert .= "mh_neutralcreepgold=VALUES(mh_neutralcreepgold), ";
            $insert .= "mh_bdmg=VALUES(mh_bdmgexp), ";
            $insert .= "mh_bdmg=VALUES(mh_bdmgexp), ";
            $insert .= "mh_razed=VALUES(mh_razed), ";
            $insert .= "mh_bgold=VALUES(mh_bgold), ";
            $insert .= "mh_denies=VALUES(mh_denies), ";
            $insert .= "mh_exp_denied=VALUES(mh_exp_denied), ";
            $insert .= "mh_gold=VALUES(mh_gold), ";
            $insert .= "mh_gold_spent=VALUES(mh_gold_spent), ";
            $insert .= "mh_exp=VALUES(mh_exp), ";
            $insert .= "mh_actions=VALUES(mh_actions), ";
            $insert .= "mh_secs=VALUES(mh_secs), ";
            $insert .= "mh_consumables=VALUES(mh_consumables), ";
            $insert .= "mh_wards=VALUES(mh_wards), ";
            $insert .= "mh_time_earning_exp=VALUES(mh_time_earning_exp), ";
            $insert .= "mh_bloodlust=VALUES(mh_bloodlust), ";
            $insert .= "mh_doublekill=VALUES(mh_doublekill), ";
            $insert .= "mh_triplekill=VALUES(mh_triplekill), ";
            $insert .= "mh_quadkill=VALUES(mh_quadkill), ";
            $insert .= "mh_annihilation=VALUES(mh_annihilation), ";
            $insert .= "mh_ks3=VALUES(mh_ks3), ";
            $insert .= "mh_ks4=VALUES(mh_ks4), ";
            $insert .= "mh_ks5=VALUES(mh_ks5), ";
            $insert .= "mh_ks6=VALUES(mh_ks6), ";
            $insert .= "mh_ks7=VALUES(mh_ks7), ";
            $insert .= "mh_ks8=VALUES(mh_ks8), ";
            $insert .= "mh_ks9=VALUES(mh_ks9), ";
            $insert .= "mh_ks10=VALUES(mh_ks10), ";
            $insert .= "mh_ks15=VALUES(mh_ks15), ";
            $insert .= "mh_smackdown=VALUES(mh_smackdown), ";
            $insert .= "mh_humiliation=VALUES(mh_humiliation), ";
            $insert .= "mh_nemesis=VALUES(mh_nemesis), ";
            $insert .= "mh_retribution=VALUES(mh_retribution)";

            if($result = mysql_query($insert, $this->write) )
            {    $this->output['match_hero_stats']="OK";
            } else { $this->output['match_hero_stats']="Could not connect.";  }

        }

        function set_match_history(){
            $date = strftime("%m/%d/%Y", strtotime($this->match_info['mdt']));
            $history = mysql_real_escape_string("{$this->match_id}|{$this->match_info['class']}|{$date}");
            $insert = "INSERT INTO hon_stats.player_match_history (account_id, history) VALUES ";
            foreach($this->match_stats AS $aid => $v)
            {   $account_id = mysql_real_escape_string($aid);
                $insert .="('{$account_id}','{$history}'),";
            } $insert = trim($insert, ",");
            $insert .= " ON DUPLICATE KEY UPDATE history=CONCAT(history,',',VALUES(history))";
            if($result = mysql_query($insert, $this->write) )
            {    $this->output['player_match_history']="OK";
            } else { $this->output['player_match_history']="Could not connect.";  }

        }

        // format abilities
        function fmt_ability_stamps(){
            // grab abilities from db
            $abilities = array();
            $select = "SELECT hero_id, 0 AS slot, IF(ab1=ability_id, ab1, null) AS ab FROM hon_stats.hero_abilities JOIN hon_stats.abilities ON (ab1=ability_id) UNION SELECT hero_id, 1 AS slot, IF(ab2=ability_id, ab2, null) AS ab FROM hon_stats.hero_abilities JOIN hon_stats.abilities ON (ab2=ability_id) UNION SELECT hero_id, 2 AS slot, IF(ab3=ability_id, ab3, null) AS ab FROM hon_stats.hero_abilities JOIN hon_stats.abilities ON (ab3=ability_id) UNION SELECT hero_id, 3 AS slot, IF(ab4=ability_id, ab4, null) AS ab FROM hon_stats.hero_abilities JOIN hon_stats.abilities ON (ab4=ability_id) UNION SELECT hero_id, 4 AS slot, IF(ab5=ability_id, ab5, null) AS ab FROM hon_stats.hero_abilities JOIN hon_stats.abilities ON (ab5=ability_id)";
            if($result = mysql_query($select, $this->read) )
            {
                while($row = mysql_fetch_assoc($result) )
                {
                    $abilities[$row['hero_id']][$row['slot']] = $row['ab'];
                }
            }


            foreach($this->input['abilities'] AS $aid => $s)
            {   foreach($this->input['abilities'][$aid] AS $sel_id => $v)
                {   $sel_order = mysql_real_escape_string($sel_id);
                    $hero_id = mysql_real_escape_string($this->heroes[$v['hero_cli_name']]);
                    $ability_id = mysql_real_escape_string($abilities[$this->heroes[$v['hero_cli_name']]][$v['slot']]);
                    $ab_slot = mysql_real_escape_string($v['slot']);
                    $secs = mysql_real_escape_string($v['secs']);

                    $this->abilities[$aid][$sel_id]['sel_order'] = $sel_order;
                    $this->abilities[$aid][$sel_id]['hero_id'] = $this->heroes[$v['hero_cli_name']];
                    $this->abilities[$aid][$sel_id]['ability_id'] = $ability_id;
                    $this->abilities[$aid][$sel_id]['ab_slot'] = $ab_slot;
                    $this->abilities[$aid][$sel_id]['secs'] = $secs;

                }
            }

        }

        // set levels
        function set_levels(){
            $account_ids = "";
            foreach($this->input['player_stats'] AS $k => $v)
            {   $aid = mysql_real_escape_string($k);
                $account_ids .= "player_rank.account_id='{$aid}' OR ";
            } $account_ids = trim($account_ids, "OR ");
            $update = "UPDATE hon_stats.player_rank JOIN hon_stats.player_stats USING (account_id) JOIN hon_stats.level ON (acc_exp BETWEEN minXP AND maxXP) SET player_rank.level=level.level WHERE {$account_ids}";
            if($result = mysql_query($update, $this->write) )
            {    $this->output['player_levels']="OK";
            } else { $this->output['player_levels']="Could not connect.";  }
        }

        // set abilities
        function set_ability_stamps(){
            $insert = "INSERT INTO hon_stats.match_ability_stamp (account_id, match_id, ability) VALUES ";
            foreach($this->abilities AS $aid => $s)
            {   $account_id = mysql_real_escape_string($aid);
                $ability = serialize($s);
                $insert .= "('{$account_id}', '{$this->match_id}', COMPRESS('{$ability}')),";
            } $insert = trim($insert, ",");

            if($result = mysql_query($insert, $this->write) )
            {    $this->output['ability_stamps']="OK";
            } else { $this->output['ability_stamps']="Could not connect.";  }

        }


        function set_item_stamps(){
            if(sizeof($this->input['items']) > 0) {
                $items = array();
                $usage = "INSERT INTO hon_stats.usage_items (date, item_id, used) VALUES ";
                foreach($this->input['items'] AS $num => $v)
                {   if($v['action'] == 1 OR $v['action'] == 2){ $usage .= "(CURDATE(), '{$this->items[$v['cli_name']]}', 1),";  }
                    $items[$v['account_id']][] = array('item_id'=>$this->items[$v['cli_name']], 'secs'=>$v['secs'], 'action'=>$v['action']);
                } $usage = trim($usage, ",");
                $usage .= " ON DUPLICATE KEY UPDATE used=used+1";
                if($result = mysql_query($usage, $this->write) )
                {    $this->output['item_usage']="OK";
                } else { $this->output['item_usage']="Could not connect.";  }

                $insert = "INSERT INTO hon_stats.match_item_stamp (account_id, match_id, item) VALUES ";
                foreach($items AS $account_id => $v)
                {   $account_id = mysql_real_escape_string($account_id);
                    $item = serialize($items[$account_id]);
                    $insert .= "('{$account_id}', '{$this->match_id}', COMPRESS('{$item}')),";
                } $insert = trim($insert, ",");

                if($result = mysql_query($insert, $this->write) )
                {    $this->output['item_stamps']="OK";
                } else { $this->output['item_stamps']="Could not connect.";  }
            } else {$this->output['item_stamps']="NONE"; }

        }


        function set_final_inventory(){
            if(isset($this->input['inventory']) ) {
            if(sizeof($this->input['inventory']) > 0) {
                $insert = "INSERT INTO hon_stats.match_inventory (account_id, match_id, slot_1, slot_2, slot_3, slot_4, slot_5, slot_6) VALUES ";
                foreach($this->input['inventory'] AS $k => $v)
                {
                    $account_id = mysql_real_escape_string($k);

                    if(isset($v[0]) ){$slot1 = (isset($this->items[$v[0]])?"'".mysql_real_escape_string($this->items[$v[0]])."'":"DEFAULT"); } else {$slot1 = "DEFAULT"; }
                    if(isset($v[1]) ){$slot2 = (isset($this->items[$v[1]])?"'".mysql_real_escape_string($this->items[$v[1]])."'":"DEFAULT"); } else {$slot2 = "DEFAULT"; }
                    if(isset($v[2]) ){$slot3 = (isset($this->items[$v[2]])?"'".mysql_real_escape_string($this->items[$v[2]])."'":"DEFAULT"); } else {$slot3 = "DEFAULT"; }
                    if(isset($v[3]) ){$slot4 = (isset($this->items[$v[3]])?"'".mysql_real_escape_string($this->items[$v[3]])."'":"DEFAULT"); } else {$slot4 = "DEFAULT"; }
                    if(isset($v[4]) ){$slot5 = (isset($this->items[$v[4]])?"'".mysql_real_escape_string($this->items[$v[4]])."'":"DEFAULT"); } else {$slot5 = "DEFAULT"; }
                    if(isset($v[5]) ){$slot6 = (isset($this->items[$v[5]])?"'".mysql_real_escape_string($this->items[$v[5]])."'":"DEFAULT"); } else {$slot6 = "DEFAULT"; }

                    $insert .= "('{$account_id}', '{$this->match_id}', {$slot1}, {$slot2}, {$slot3}, {$slot4}, {$slot5}, {$slot6}),";
                } $insert = trim($insert, ",");

                if($result = mysql_query($insert, $this->write) )
                {    $this->output['inventory']="OK";
                } else { $this->output['inventory']="Could not connect.";  }
            } else {$this->output['inventory']="NONE"; }
            }

        }


        function set_frag_stamps(){
            if(sizeof($this->input['frags']) > 0) {
                $insert = "INSERT INTO hon_stats.match_frag_stamp (match_id, frag) VALUES ";
                foreach($this->input['frags'] AS $num => $v)
                {   if(!isset($v['assisters']) ){$v['assisters'] = array(); }
                    $frag = mysql_real_escape_string(serialize($v));
                    $insert .= "('{$this->match_id}', COMPRESS('{$frag}')),";
                } $insert = trim($insert, ",");

                if($result = mysql_query($insert, $this->write) )
                {    $this->output['frag_stamps']="OK";
                } else { $this->output['frag_stamps']="Could not connect.";  }
            } else {$this->output['frag_stamps']="NONE"; }
        }

        // grab frag stamps and set match frags
        function set_match_frags(){
            $select = "SELECT frag_id, UNCOMPRESS(frag) AS f FROM hon_stats.match_frag_stamp WHERE match_id='{$this->match_id}'";
            if($result = mysql_query($select, $this->write) )
            {   $frag_id_list = array();
                while($row = mysql_fetch_assoc($result) )
                {   $frag_id = mysql_real_escape_string($row['frag_id']);
                    $f = unserialize($row['f']);
                    // killer
                    $frag_id_list[$f['killer_id']] = (isset($frag_id_list[$f['killer_id']])?$frag_id_list[$f['killer_id']].",".$frag_id:$frag_id);

                    // target
                    $target_id = mysql_real_escape_string($f['target_id']);
                    $frag_id_list[$f['target_id']] = (isset($frag_id_list[$f['target_id']])?$frag_id_list[$f['target_id']].",".$frag_id:$frag_id);

                    // assisters
                    if(sizeof($f['assisters']) > 0){
                        foreach($f['assisters'] AS $k => $v)
                        {   $v = mysql_real_escape_string($v);
                            $frag_id_list[$v] = (isset($frag_id_list[$v])?$frag_id_list[$v].",".$frag_id:$frag_id);
                        }
                    }
                }

                $frags = "";
                if(sizeof($frag_id_list) > 0) {
                foreach($frag_id_list AS $k => $v)
                {   $account_id = mysql_real_escape_string($k);
                    $frag_list = mysql_real_escape_string($v);
                    $frags .="('{$this->match_id}', '{$account_id}', '{$frag_list}'), ";


                } $frags = trim($frags, ", ");

                // insert rows
                $insert = "INSERT INTO hon_stats.match_frag (match_id, account_id, frag_id_list) VALUES ".$frags;
                if(!$result = mysql_query($insert, $this->write) )
                {$this->output['frags']="ERROR"; }

                }
            }
        }


        // set team stats
        function set_team_summ(){
            $insert = "INSERT INTO hon_stats.team_stats (match_id, team, tm_wins, tm_losses, tm_concedes, tm_concedevotes, tm_buybacks, tm_kicked, tm_herokills, tm_herodmg, tm_heroexp, tm_herokillsgold, tm_heroassists, tm_deaths, tm_goldlost2death, tm_secs_dead, tm_teamcreepkills, tm_teamcreepdmg, tm_teamcreepexp, tm_teamcreepgold, tm_neutralcreepkills, tm_neutralcreepdmg, tm_neutralcreepexp, tm_neutralcreepgold, tm_bdmg, tm_bdmgexp, tm_razed, tm_bgold, tm_denies, tm_exp_denied, tm_gold, tm_gold_spent, tm_exp, tm_actions, tm_secs, tm_consumables, tm_wards, tm_time_earning_exp) VALUES ";
            foreach($this->team_stats AS $tm => $v)
            {   $team = mysql_real_escape_string($tm);
                $insert .= "('{$this->match_id}', ";
                $insert .= "'{$team}', ";
                $insert .="'{$v['wins']}', ";
                $insert .="'{$v['losses']}', ";
                $insert .="'{$v['concedes']}', ";
                $insert .="'{$v['concedevotes']}', ";
                $insert .="'{$v['buybacks']}', ";
                $insert .="'{$v['consumables']}', ";
                $insert .="'{$v['herokills']}', ";
                $insert .="'{$v['herodmg']}', ";
                $insert .="'{$v['heroexp']}', ";
                $insert .="'{$v['herokillsgold']}', ";
                $insert .="'{$v['heroassists']}', ";
                $insert .="'{$v['deaths']}', ";
                $insert .="'{$v['goldlost2death']}', ";
                $insert .="'{$v['secs_dead']}', ";
                $insert .="'{$v['teamcreepkills']}', ";
                $insert .="'{$v['teamcreepdmg']}', ";
                $insert .="'{$v['teamcreepexp']}', ";
                $insert .="'{$v['teamcreepgold']}', ";
                $insert .="'{$v['neutralcreepkills']}', ";
                $insert .="'{$v['neutralcreepdmg']}', ";
                $insert .="'{$v['neutralcreepexp']}', ";
                $insert .="'{$v['neutralcreepgold']}', ";
                $insert .="'{$v['bdmg']}', ";
                $insert .="'{$v['bdmgexp']}', ";
                $insert .="'{$v['razed']}', ";
                $insert .="'{$v['bgold']}', ";
                $insert .="'{$v['denies']}', ";
                $insert .="'{$v['exp_denied']}', ";
                $insert .="'{$v['gold']}', ";
                $insert .="'{$v['gold_spent']}', ";
                $insert .="'{$v['exp']}', ";
                $insert .="'{$v['actions']}', ";
                $insert .="'{$v['secs']}', ";
                $insert .="'{$v['consumables']}', ";
                $insert .="'{$v['wards']}', ";
		        $insert .="'{$v['time_earning_exp']}'),";

            } $insert = trim($insert, ",");
            $insert .= " ON DUPLICATE KEY UPDATE tm_wins=VALUES(tm_wins), tm_losses=VALUES(tm_losses), tm_concedes=VALUES(tm_concedes), tm_concedevotes=VALUES(tm_concedevotes), tm_buybacks=VALUES(tm_buybacks), tm_kicked=VALUES(tm_kicked), tm_herokills=VALUES(tm_herokills), tm_herodmg=VALUES(tm_herodmg), tm_heroexp=VALUES(tm_heroexp), tm_herokillsgold=VALUES(tm_herokillsgold), tm_heroassists=VALUES(tm_heroassists), tm_deaths=VALUES(tm_deaths), tm_goldlost2death=VALUES(tm_goldlost2death), tm_secs_dead=VALUES(tm_secs_dead), tm_teamcreepkills=VALUES(tm_teamcreepkills), tm_teamcreepdmg=VALUES(tm_teamcreepdmg), tm_teamcreepexp=VALUES(tm_teamcreepexp), tm_teamcreepgold=VALUES(tm_teamcreepgold), tm_neutralcreepkills=VALUES(tm_neutralcreepkills), tm_neutralcreepdmg=VALUES(tm_neutralcreepdmg), tm_neutralcreepexp=VALUES(tm_neutralcreepexp), tm_neutralcreepgold=VALUES(tm_neutralcreepgold), tm_bdmg=VALUES(tm_bdmg), tm_bdmgexp=VALUES(tm_bdmgexp), tm_razed=VALUES(tm_razed), tm_bgold=VALUES(tm_bgold), tm_denies=VALUES(tm_denies), tm_exp_denied=VALUES(tm_exp_denied), tm_gold=VALUES(tm_gold), tm_gold_spent=VALUES(tm_gold_spent), tm_exp=VALUES(tm_exp), tm_actions=VALUES(tm_actions), tm_secs=VALUES(tm_secs), tm_consumables=VALUES(tm_consumables), tm_wards=VALUES(tm_wards), tm_time_earning_exp=VALUES(tm_time_earning_exp)";
            if($result = mysql_query($insert, $this->write) )
            {    $this->output['team_stats']="OK";
            } else { $this->output['team_stats']="Could not connect.";  }

        }


        function set_comments(){
            // check for comments
            if(isset($this->input['comments']) ) {
            if(sizeof($this->input['comments']) > 0)
            {   $insert = "INSERT INTO hon_replays.comments (account_id, match_id, comment, dt) VALUES ";
                foreach($this->input['comments'] AS $k => $v)
                {  $account_id = mysql_real_escape_string($k);

                   $v = strip_tags($v, '<p><br><u><i><b>');
                   $v = nl2br($v);
                   $v = htmlspecialchars($v, ENT_QUOTES, "UTF-8");
                   $val = mysql_real_escape_string($v);
                   $insert .="('{$account_id}', '{$this->match_id}', '{$val}', NOW()),";
                } $insert = trim($insert, ",");

                if($result = mysql_query($insert, $this->write) )
                {  $this->output['comments'] = "OK";
                } else {$this->output['comments'] = "Could not connect.";  }

            } else {$this->output['comments'] = "NONE"; }
            }
        }

        function set_ratings(){
            // check for votes
            if(isset($this->input['ratings']) ) {
            if(sizeof($this->input['ratings']) > 0)
            {
                $insert = "INSERT INTO hon_replays.votes (account_id, match_id, val) VALUES ";
                foreach($this->input['ratings'] AS $k => $v)
                {  $account_id = mysql_real_escape_string($k);
                   $val = mysql_real_escape_string($v);
                   $insert .="('{$account_id}', '{$this->match_id}', '{$val}'),";
                } $insert = trim($insert, ",");

                if($result = mysql_query($insert, $this->write) )
                {  $this->output['votes'] = "OK";
                } else {$this->output['votes'] = "Could not connect.";  }


                $insert = "INSERT INTO hon_replays.rating (match_id, votes, avg) SELECT match_id, COUNT(*) AS votes, AVG(val) AS avg FROM hon_replays.votes WHERE match_id='{$this->match_id}' GROUP BY match_id ON DUPLICATE KEY UPDATE votes=VALUES(votes), avg=VALUES(avg)";
                if($result = mysql_query($insert, $this->write) )
                {  $this->output['rating'] = "OK";
                } else {$this->output['rating'] = "Could not connect.";  }
            } else {    // no votes ... initialize rating anyway
                $insert = "INSERT INTO hon_replays.rating (match_id) VALUES ('{$this->match_id}') ON DUPLICATE KEY UPDATE match_id=VALUES(match_id)";
                if($result = mysql_query($insert, $this->write) )
                { $this->output['rating'] = "NONE";
                } else {$this->output['rating'] = "Could not connect."; }
            }
            }


        }

        function addEventToClan(){
			$winnerIDs = NULL;
			$loserIDs = NULL;
			$cArray = array();
            foreach($this->match_stats AS $aid => $v){ // loop through match stats with clan_ids - load em up
                if($v['clan_id'] && $v['clan_id'] != 0){
                    if($v['wins'] == 1){
                        $cArray[1][$v['clan_id']][] = $aid;
                    } else {
                        $cArray[0][$v['clan_id']][] = $aid;
                    }
                }
            }

            if(is_array($cArray)){ // if clans
                $descript = array();
                foreach($cArray as $key => $var){
                    foreach($var as $k => $v){
                        if(is_array($v)){
                            switch(count($v)){ // determine each side's involvement
                                case 3:
                                $descript[$k] = 'public game';
                                $key == 1 ? $winnerIDs = $v : $loserIDs = $v;
                                break;
                                case 4:
                                $descript[$k] = 'scrimmage';
                                $key == 1 ? $winnerIDs = $v : $loserIDs = $v;
                                break;
                                case 5:
                                $descript[$k] = 'scrimmage';
                                $key == 1 ? $winnerIDs = $v : $loserIDs = $v;
                                break;
                            }
                        }
                    }
                }
                if(is_array($descript)){ // determine event type from involvement
                    if(count($descript) == 2){
                        if(current($descript) === "scrimmage" && next($descript) === "scrimmage"){
                            $type = "scrimmage";
                        } else if(reset($descript) === "scrimmage" or next($descript) === "scrimmage"){
                            $type = "public game";
                        } else if(reset($descript) === "public game" xor next($descript) === "public game"){
                            $type = "public game";
                        } else { $type = "public game"; }
                    } else {
                        if(count($descript) == 1){
                            $type = "public game";
                        }
                    }
                }
            }
            $match_id = $this->match_id;
            if($winnerIDs && count($winnerIDs) >= 3){ // write out and insert event for winners
                $typeM = "match";
                $match_id ? mysql_real_escape_string($match_id) : $match_id = null;
                foreach($winnerIDs as $v){
                    $getName = "SELECT a.nickname, ci.name, ci.clan_id FROM hon_accounts.account AS a
                    LEFT JOIN hon_clans.roster AS r ON  r.account_id = a.account_id
                    LEFT JOIN hon_clans.clan_info AS ci ON ci.clan_id = r.clan_id
                    WHERE a.account_id = '".mysql_real_escape_string($v)."' LIMIT 1";
                    $doGetName = mysql_query($getName,$this->read);
                    $row = mysql_fetch_assoc($doGetName);
                    $players[] = $row;
                    $clan_id = $row['clan_id'];
                }
                $i=1;
                $c=count($players);
				$string = '';
                foreach($players as $key => $var){
                    $string = $string.$var['nickname'];
                    if($i != $c){
                        $string = $string.", ";
                    }
                    $i++;
                }

                if($match_id){
                    $event = $string." won ".$type." ".$match_id;
                } else {
                    $event = $string." won a ".$type;
                }


                $insertComment = "INSERT INTO hon_clans.message_text (msg) VALUES ('".htmlspecialchars(mysql_real_escape_string($event))."')";
                $doInsertComment = mysql_query($insertComment,$this->write);
                $GetLast = "SELECT LAST_INSERT_ID() AS lastId FROM hon_clans.message_text LIMIT 1";
                $doGetLast = mysql_query($GetLast, $this->write);
                $last = mysql_fetch_assoc($doGetLast);

                $insertInfo = "INSERT INTO hon_clans.clan_events (clan_id, creator, class, message_id, create_date) VALUES ('".$clan_id."', '45691', '".$type."', '".$last['lastId']."', NOW())";
                $doInserter = mysql_query($insertInfo,$this->write);
            }
            if($loserIDs && count($loserIDs) >= 3){ // write out and insert event for losers
                unset($event);
                //unset($string);
                $typeM = "match";
                $match_id ? mysql_real_escape_string($match_id) : $match_id = null;
                foreach($loserIDs as $v){
                    $getName = "SELECT a.nickname, ci.name, ci.clan_id FROM hon_accounts.account AS a
                    LEFT JOIN hon_clans.roster AS r ON  r.account_id = a.account_id
                    LEFT JOIN hon_clans.clan_info AS ci ON ci.clan_id = r.clan_id
                    WHERE a.account_id = '".mysql_real_escape_string($v)."' LIMIT 1";
                    $doGetName = mysql_query($getName,$this->read);
                    $row = mysql_fetch_assoc($doGetName);
                    $losers[] = $row;
                    $clan_id = $row['clan_id'];
                }
                $i=1;
                $c=count($losers);
				$string = '';
                foreach($losers as $key => $var){
                    $string = $string.$var['nickname'];
                    if($i != $c){
                        $string = $string.", ";
                    }
                    $i++;
                }

                if($match_id){
                    $event = $string." lost ".$type." ".$match_id;
                } else {
                    $event = $string." lost a ".$type;
                }

                $insertComment = "INSERT INTO hon_clans.message_text (msg) VALUES ('".htmlspecialchars(mysql_real_escape_string($event))."')";
                $doInsertComment = mysql_query($insertComment,$this->write);
                $GetLast = "SELECT LAST_INSERT_ID() AS lastId FROM hon_clans.message_text LIMIT 1";
                $doGetLast = mysql_query($GetLast,$this->write);
                $last = mysql_fetch_assoc($doGetLast);

                $insertInfo = "INSERT INTO hon_clans.clan_events (clan_id, creator, class, message_id, create_date) VALUES ('".$clan_id."', '45691', '".$type."', '".$last['lastId']."', NOW())";
                $doInserter = mysql_query($insertInfo,$this->write);
            }
        }


    }


?>