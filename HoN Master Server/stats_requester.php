<?

//echo "<br />START :: ".memory_get_usage()." :: ".microtime(true)."<br />";
include("stats_request_class.php");

$stats = new Stats();

// purge matches less than 10 min or 2 players
if($stats->input['match_stats']['time_played'] > 600 AND sizeof($stats->input['player_stats']) > 1) {
// check that no errors are set
if(sizeof($stats->error) == 0) {
    // match cache
    //$stats->pre_ins();

    // check if ranked match

    // update match info
    $stats->set_match_summ();
    // set banned heroes
    $stats->set_banned_heroes();
    // player stats
    $stats->fmt_player_stats();
    // set stats
    $stats->set_match_stats();
    // set match history
    $stats->set_match_history();


    // match class switch
    switch($stats->match_info['class'])
    {
        case 2: // mm
        case 3: // league
        case 4: // tourney
        case 5: // something else
            if($stats->match_info['no_stats'] == 0){
                $stats->set_ranked_stats();
            }
        break;
        case 1:
        default:
            if($stats->match_info['no_stats'] == 0){
                $stats->set_player_stats();
            }
            // do this until levels are fixed in db
            $stats->set_levels();
        break;
    }

    // heroes
    $stats->set_match_hero_stats();
    if($stats->match_info['no_stats'] == 0){
        $stats->set_player_hero_stats();
    }
    // levels     // temporarily disabled
    //$stats->set_levels();

    // abilities
    $stats->fmt_ability_stamps();
    $stats->set_ability_stamps();

    // team stats
    $stats->set_team_summ();

    // item stamps
    $stats->set_item_stamps();

    // frag stamps
    $stats->set_frag_stamps();
    $stats->set_match_frags();
    
    // comments
    $stats->set_comments();
    // ratings
    $stats->set_ratings();

    //inventory
    $stats->set_final_inventory();
    // clans
    if( 0 ){}
    if($stats->match_info['no_stats'] == 0){
        $stats->addEventToClan();
    }

}  }

//output
$stats->to_server();
//echo "<br />END :: ".memory_get_usage()." :: ".microtime(true)."<br />";


?>