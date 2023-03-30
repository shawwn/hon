<?php

//print serialize(array(0 => "failure", 1 => 1));
//exit;
$_REQUEST['table'] = "ranked";
include('inc/ls.php');
if(!empty($_REQUEST['nickname']) ){

	$nick = mysql_real_escape_string($_REQUEST['nickname']);
	$tagged = strrchr($nick, "]");
	if($tagged){
		$nick = substr($tagged,1);
	}

	$select = "SELECT account_id FROM hon_accounts.account WHERE nickname = '{$nick}' LIMIT 1";
	if(!$doSelect = mysql_query($select,$ls)){
		@mysql_close($ls);
		print serialize(array(0 => "failure", 1 => 1));
		exit;
	}

	$nRow = mysql_fetch_assoc($doSelect);
	$_REQUEST['aId'] = $nRow['account_id'];
}
if(!empty($_REQUEST['aId']) ){

    // stats request table switch (wouldnt kill us to join and pass both tables)
    if(isset($_REQUEST['table']) AND $_REQUEST['table'] =='ranked' ){
        $stats_table = "ranked_stats"; $prefix = "rnk_"; $namePrefix = "rnk_";
        $stats_table_alt = "player_stats"; $prefix_alt = "player_rank."; $namePrefix_alt = "acc_";
    } else {
        $stats_table = "player_stats"; $prefix = "player_rank."; $namePrefix = "acc_";
        $stats_table_alt = "ranked_stats"; $prefix_alt = "rnk_"; $namePrefix_alt = "rnk_";
    }

    // just clean account_id once
    $account_id = mysql_real_escape_string($_REQUEST['aId']);
	$matches = '';
	$matchDates = '';
       // match history
	$select = "SELECT * FROM hon_stats.player_match_history WHERE account_id='{$account_id}' LIMIT 1";
	if($result = mysql_query($select, $ls) )
	{
		if( mysql_num_rows($result) > 0 )
		{   $ms = array();
            $hrow = mysql_fetch_assoc($result);
            $hist = explode(',',$hrow['history']);
			foreach($hist AS $k => $v)
			{
				$curr = explode('|', $v);
				if($curr[1] > 1) {
                    $ms[$curr[0]]=$curr[2];  // match id => date
				}

			}
            if(krsort($ms, SORT_NUMERIC)){
                $i=1;
                foreach($ms as $mid => $d){
                    if($i > 20) {break; }
                    $matches .= str_pad($mid,10, " ");
			        $matchDates .= $d;
                    $i++;
                }
            }

		}
	}

    // stats rows request
    #+----+-------------+--------------+-------+------------------------------+--------------+---------+-------+------+-------+
    #| id | select_type | table        | type  | possible_keys                | key          | key_len | ref   | rows | Extra |
    #+----+-------------+--------------+-------+------------------------------+--------------+---------+-------+------+-------+
    #|  1 | SIMPLE      | account_info | const | PRIMARY                      | PRIMARY      | 4       | const |    1 |       |
    #|  1 | SIMPLE      | roster       | const | PRIMARY                      | PRIMARY      | 4       | const |    1 |       |
    #|  1 | SIMPLE      | clan_info    | const | PRIMARY                      | PRIMARY      | 4       | const |    1 |       |
    #|  1 | SIMPLE      | account      | const | PRIMARY                      | PRIMARY      | 4       | const |    1 |       |
    #|  1 | SIMPLE      | in_game      | const | account_id_2,account_id      | account_id_2 | 4       | const |    1 |       |
    #|  1 | SIMPLE      | ranked_stats | const | PRIMARY,account_id,rnk_level | PRIMARY      | 4       | const |    1 |       |
    #|  1 | SIMPLE      | level        | const | PRIMARY                      | PRIMARY      | 4       | const |    1 |       |
    #+----+-------------+--------------+-------+------------------------------+--------------+---------+-------+------+-------+


	$select = "SELECT nickname, name, rank, level.level, {$stats_table}.*, {$stats_table_alt}.{$namePrefix_alt}secs, maxXP, DATE_FORMAT(last_activity, '%m/%d/%Y') AS last_activity, DATE_FORMAT(account_info.create_date, '%m/%d/%Y') AS create_date, {$stats_table_alt}.{$namePrefix_alt}games_played, {$stats_table_alt}.{$namePrefix_alt}discos FROM hon_accounts.account ".
	    "JOIN hon_accounts.account_info USING (account_id) ".
	    "JOIN hon_tracking.in_game USING (account_id) ".
        "JOIN hon_stats.{$stats_table} USING (account_id) ".
        "JOIN hon_stats.{$stats_table_alt} USING (account_id) ".
        "JOIN hon_stats.player_rank USING (account_id) ".
        "JOIN hon_stats.level ON ({$prefix}level=level.level) ".
        "LEFT JOIN hon_clans.roster ON account_info.account_id=roster.account_id ".
	    "LEFT JOIN hon_clans.clan_info USING (clan_id) ".
	    "WHERE account_info.account_id = '{$account_id}' LIMIT 1";
	//print $select;
       //echo $select;
	$master_select = $select;

	if(!$doSelect = mysql_query($select,$ls)){
		@mysql_close($ls);
		print serialize(array(0 => "failure", 1 => 1));
		exit;
	}
	$stats = array();

	$fav = array();
	$row = mysql_fetch_assoc($doSelect);
	
	$games_played = $row[$namePrefix.'games_played'];
	
	$row['rnk_games_played'] += $row['acc_games_played'];
	$row['rnk_discos'] += $row['acc_discos'];
	
     /*
    //# will be rewritten for memcache support soon
    $select = "SELECT match_id FROM hon_stats.match_stats WHERE account_id='{$account_id}'";
    if($result = mysql_query($select, $ls) )
    {

        if(mysql_num_rows($result) > 0)
        {
            $mids = "(";
            while($row1 = mysql_fetch_assoc($result))
            {
                //print_r($row1);

                $mids .= "'{$row1['match_id']}', ";
            } $mids = trim($mids, ", "). ")";

                $select = "SELECT DATE_FORMAT(mdt, '%m/%d/%Y') AS date, match_id FROM hon_stats.match_summ WHERE match_id IN {$mids} ORDER BY match_id DESC LIMIT 20";
            	if(!$doSelect = mysql_query($select,$ls)){
              		@mysql_close($ls);
              		print serialize(array(0 => "failure", 1 => 1));
              		exit;
              	}

            	while($mRow = mysql_fetch_assoc($doSelect))
                {

		            $matches .= str_pad($mRow['match_id'],10, " ");
		            $matchDates .= $mRow['date'];
	            }
        }




    }
    */

    //# THIS WAS REMOVED ONLY BECAUSE WE HAD TO DEBUG A CORRUPT TABLE...
    //# ITS BETTER THAN THE ABOVE METHOD BUT WILL NOT BE REIMPLEMENTED
    //# UNTIL MEMCACHE IS INSTALLED ON THE DBS
    /*
	$create = "CREATE TEMPORARY TABLE hon_stats.statsRequester (match_id int(12) unsigned not null primary key) ENGINE=MEMORY";
	$doCreate = mysql_query($create,$ls);

	$insert = "INSERT INTO hon_stats.statsRequester (match_id) (SELECT match_id FROM hon_stats.match_stats WHERE account_id='{$account_id}')";
    //$insert = "INSERT INTO hon_stats.statsRequester (match_id) VALUES {$mids}";
	$doInsert = mysql_query($insert, $ls);

	$select = "SELECT DATE_FORMAT(mdt, '%m/%d/%Y') AS date, statsRequester.match_id FROM hon_stats.statsRequester JOIN hon_stats.match_summ USING (match_id) ORDER BY match_id DESC LIMIT 20";
	if(!$doSelect = mysql_query($select,$ls)){
		@mysql_close($ls);
		print serialize(array(0 => "failure", 1 => 1));
		exit;
	}

	while($mRow = mysql_fetch_assoc($doSelect)){
        //print_r($mRow);
		$matches .= str_pad($mRow['match_id'],10, " ");
		$matchDates .= $mRow['date'];
	}

	$drop_table = "DROP TABLE hon_stats.statsRequester";
	@mysql_query($drop_table, $ls);
    */



	$select = "SELECT h.cli_name, phs.ph_secs FROM hon_stats.hero AS h JOIN (select hero_id, ph_secs from hon_stats.player_hero_stats where account_id = '{$account_id}') AS phs ON phs.hero_id = h.hero_id ORDER BY phs.ph_secs DESC LIMIT 5";
	if(!$doSelect = mysql_query($select,$ls)){
		@mysql_close($ls);
		print serialize(array(0 => "failure", 1 => 1));
		exit;
	}
	$favHero1 = '';
	$favHero2 = '';
	$favHero3 = '';
	$favHero4 = '';
	$favHero5 = '';
	$favHero1Time = '';
	$favHero2Time = '';

	$favHero3Time = '';
	$favHero4Time = '';
	$favHero5Time = '';
	$i = 1;
	while($fRow = mysql_fetch_assoc($doSelect)){
		$vn = "favHero".$i;
		$vnT = "favHero".$i."Time";
		$vnD = "favHero".$i."_2";
		${$vn} = strtolower(substr($fRow['cli_name'], "5"));
		${$vnT} = $fRow['ph_secs'];
		${$vnD} = $fRow['cli_name'];
		$i++;
	}

	$final = array();
	$final = array_merge($final, $row);
	$final['matchIds'] = $matches;
	$final['matchDates'] = $matchDates;
	$final['favHero1'] = $favHero1;
	$final['favHero2'] = $favHero2;
	$final['favHero3'] = $favHero3;
	$final['favHero4'] = $favHero4;
	$final['favHero5'] = $favHero5;
	$final['favHero1_2'] = $favHero1_2;
	$final['favHero2_2'] = $favHero2_2;
	$final['favHero3_2'] = $favHero3_2;
	$final['favHero4_2'] = $favHero4_2;
	$final['favHero5_2'] = $favHero5_2;
	$final['favHero1Time'] = $favHero1Time;
	$final['favHero2Time'] = $favHero2Time;
	$final['favHero3Time'] = $favHero3Time;
	$final['favHero4Time'] = $favHero4Time;
	$final['favHero5Time'] = $favHero5Time;
	$final['xp2nextLevel'] = ($final['maxXP']-$final[$namePrefix.'exp']);
	$final['xpPercent'] = round((($final['maxXP']-$final[$namePrefix.'exp'])/$final['maxXP'])*100,2);
	$final['percentEM'] = $final[$namePrefix.'em_played'] != 0 ? round(($final[$namePrefix.'em_played']/$games_played)*100,2) : 0;
	//$final['k_d_a'] = $final['acc_deaths'] != 0 ? round(($final['acc_herokills']/$final['acc_deaths']),2)."/1/".round(($final['acc_hero_assists']/$final['acc_deaths'])) : $final['acc_herokills']."/0/".$final['acc_heroassists'];
	$final['k_d_a'] = $games_played != 0 ? round(($final[$namePrefix.'herokills']/$games_played),2)."/".round(($final[$namePrefix.'deaths']/$games_played),2)."/".round(($final[$namePrefix.'heroassists']/$games_played),2) : '0/0/0';
	$final['avgGameLength'] = $games_played != 0 ? round(($final[$namePrefix.'secs']/$games_played),2) : 0;
	$final['avgXP_min'] = $final[$namePrefix.'time_earning_exp'] != 0 ? round(($final[$namePrefix.'exp']/($final[$namePrefix.'time_earning_exp']/60)),2) : 0;
	$final['avgDenies'] = $games_played != 0 ? round(($final[$namePrefix.'denies']/$games_played),2) : 0;
	$final['avgCreepKills'] = $games_played != 0 ? round(($final[$namePrefix.'teamcreepkills']/$games_played),2) : 0;
	$final['avgNeutralKills'] = $games_played != 0 ? round(($final[$namePrefix.'neutralcreepkills']/$games_played),2) : 0;
	$final['avgActions_min'] = $final[$namePrefix.'secs'] != 0 ? round(($final[$namePrefix.'actions']/($final[$namePrefix.'secs']/60)),2) : 0;
	$final['avgWardsUsed'] = $games_played != 0 ? round(($final[$namePrefix.'wards']/$games_played),2) : 0;
        $final[$namePrefix.'secs'] += $final[$namePrefix_alt.'secs'];
	print serialize($final);


} else {
@mysql_close($ls);
print serialize(array(0 => "failure", 1 => 2));
exit;
}
@mysql_close($ls);

if(!$_REQUEST['nickname'] && !$_REQUEST['aId']){
	print serialize(array(0 => "failure", 1 => 2));
	exit;
}
//print $master_select;
?>
