<? exit;  
?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">

<head>
  <title>Hello!</title>
<script language=javascript>
var http_request = false;

function postRequest() {
    //alert(document.getElementById('nickname'+val).value);
     //var url = "http://masterserver.hon.s2games.com/stats_requester.php";
var url = "http://masterserver.hon.s2games.com/stats_requester_xxx.php";
     //var parameters = "f=new_session&login=S2KingKtulu&pass=9b2844fc82a25a15305f1ec08814dfb4&port=2121&name=My+Server&desc=hehe";
     //var parameters = "f=new_session&login=S2Maliken&pass=025ce905d8ce80da55fca175ff21a9a8&port=2121&name=My+Server&desc=hehe";
     //var parameters = "f=new_session&login=S2Ryan&pass=bbf8d5822a87f5d05f95780ced643c19&port=2121&name=My+Server&desc=hehe";
     //var parameters = "f=new_session&login=S2Jesse&pass=cb4d020c1974ec925c1b109ea56da485&port=2121&name=My+Server&desc=hehe";
     //var parameters = "f=new_session&login=S2Kyle&pass=889d6cb83920a5d32c3c8e369efeaeba&port=2121&name=My+Server&desc=hehe";
     //var parameters = "f=new_session&login=S2Toolz&pass=02a6859d4def9a40ee64c11d7b44e27c&port=2121&name=My+Server&desc=hehe";

     //var parameters = "f=shutdown&session=5495feea37359e8eac91c34b55d2c1a7";
//var parameters = "session=e30ac709e3710a8765ab5db2f3e51e34&match_stats[match_id]=14&match_stats[map]=tmap&match_stats[map_version]=1.00.01&match_stats[time_played]=534&match_stats[file_size]=1024&match_stats[file_name]=14.s2z&match_stats[c_state]=5&match_stats[version]=1.0.0"
//+"&player_stats[28][Ebulus][clan_id]=1&player_stats[28][Ebulus][team]=1&player_stats[28][Ebulus][wins]=1&player_stats[28][Ebulus][losses]=0&player_stats[28][Ebulus][herokills]=16&player_stats[28][Ebulus][herodmg]=14332&player_stats[28][Ebulus][herokillsgold]=7233&player_stats[28][Ebulus][heroassists]=55&player_stats[28][Ebulus][deaths]=8&player_stats[28][Ebulus][goldlost2death]=3570&player_stats[28][Ebulus][secs_dead]=120&player_stats[28][Ebulus][teamcreepkills]=36&player_stats[28][Ebulus][teamcreepdmg]=2400&player_stats[28][Ebulus][teamcreepgold]=1600&player_stats[28][Ebulus][neutralcreepkills]=7&player_stats[28][Ebulus][neutralcreepdmg]=230&player_stats[28][Ebulus][neutralcreepgold]=1200&player_stats[28][Ebulus][bdmg]=7000&player_stats[28][Ebulus][razed]=4&player_stats[28][Ebulus][bgold]=4300&player_stats[28][Ebulus][denies]=3&player_stats[28][Ebulus][exp_denied]=3650&player_stats[28][Ebulus][gold]=12343&player_stats[28][Ebulus][gold_spent]=5000&player_stats[28][Ebulus][exp]=8236&player_stats[28][Ebulus][actions]=760&player_stats[28][Ebulus][secs]=600"
//+"&player_stats[28][Manabreaker][clan_id]=1&player_stats[28][Manabreaker][team]=1&player_stats[28][Manabreaker][wins]=1&player_stats[28][Manabreaker][losses]=0&player_stats[28][Manabreaker][herokills]=16&player_stats[28][Manabreaker][herodmg]=14332&player_stats[28][Manabreaker][herokillsgold]=7233&player_stats[28][Manabreaker][heroassists]=55&player_stats[28][Manabreaker][deaths]=8&player_stats[28][Manabreaker][goldlost2death]=3570&player_stats[28][Manabreaker][secs_dead]=120&player_stats[28][Manabreaker][teamcreepkills]=36&player_stats[28][Manabreaker][teamcreepdmg]=2400&player_stats[28][Manabreaker][teamcreepgold]=1600&player_stats[28][Manabreaker][neutralcreepkills]=7&player_stats[28][Manabreaker][neutralcreepdmg]=230&player_stats[28][Manabreaker][neutralcreepgold]=1200&player_stats[28][Manabreaker][bdmg]=7000&player_stats[28][Manabreaker][razed]=4&player_stats[28][Manabreaker][bgold]=4300&player_stats[28][Manabreaker][denies]=3&player_stats[28][Manabreaker][exp_denied]=3650&player_stats[28][Manabreaker][gold]=12343&player_stats[28][Manabreaker][gold_spent]=5000&player_stats[28][Manabreaker][exp]=8236&player_stats[28][Manabreaker][actions]=760&player_stats[28][Manabreaker][secs]=600"
//+"&player_stats[7][Ebulus][clan_id]=2&player_stats[7][Ebulus][team]=1&player_stats[7][Ebulus][wins]=0&player_stats[7][Ebulus][losses]=1&player_stats[7][Ebulus][herokills]=7&player_stats[7][Ebulus][herodmg]=55332&player_stats[7][Ebulus][herokillsgold]=5533&player_stats[7][Ebulus][heroassists]=33&player_stats[7][Ebulus][deaths]=6&player_stats[7][Ebulus][goldlost2death]=3270&player_stats[7][Ebulus][secs_dead]=300&player_stats[7][Ebulus][teamcreepkills]=3&player_stats[7][Ebulus][teamcreepdmg]=3200&player_stats[7][Ebulus][teamcreepgold]=2300&player_stats[7][Ebulus][neutralcreepkills]=6&player_stats[7][Ebulus][neutralcreepdmg]=120&player_stats[7][Ebulus][neutralcreepgold]=2200&player_stats[7][Ebulus][bdmg]=4300&player_stats[7][Ebulus][razed]=2&player_stats[7][Ebulus][bgold]=2300&player_stats[7][Ebulus][denies]=7&player_stats[7][Ebulus][exp_denied]=3680&player_stats[7][Ebulus][gold]=43343&player_stats[7][Ebulus][gold_spent]=3400&player_stats[7][Ebulus][exp]=4536&player_stats[7][Ebulus][actions]=890&player_stats[7][Ebulus][secs]=500"
//+"&player_stats[5][Ebulus][clan_id]=&player_stats[5][Ebulus][team]=1&player_stats[5][Ebulus][wins]=1&player_stats[5][Ebulus][losses]=0&player_stats[5][Ebulus][herokills]=16&player_stats[5][Ebulus][herodmg]=14332&player_stats[5][Ebulus][herokillsgold]=7233&player_stats[5][Ebulus][heroassists]=55&player_stats[5][Ebulus][deaths]=8&player_stats[5][Ebulus][goldlost2death]=3570&player_stats[5][Ebulus][secs_dead]=120&player_stats[5][Ebulus][teamcreepkills]=36&player_stats[5][Ebulus][teamcreepdmg]=2400&player_stats[5][Ebulus][teamcreepgold]=1600&player_stats[5][Ebulus][neutralcreepkills]=7&player_stats[5][Ebulus][neutralcreepdmg]=230&player_stats[5][Ebulus][neutralcreepgold]=1200&player_stats[5][Ebulus][bdmg]=7000&player_stats[5][Ebulus][razed]=4&player_stats[5][Ebulus][bgold]=4300&player_stats[5][Ebulus][denies]=3&player_stats[5][Ebulus][exp_denied]=3650&player_stats[5][Ebulus][gold]=12343&player_stats[5][Ebulus][gold_spent]=5000&player_stats[5][Ebulus][exp]=8236&player_stats[5][Ebulus][actions]=760&player_stats[5][Ebulus][secs]=600"
//+"&player_stats[5][Manabreaker][clan_id]=&player_stats[5][Manabreaker][team]=1&player_stats[5][Manabreaker][wins]=1&player_stats[5][Manabreaker][losses]=0&player_stats[5][Manabreaker][herokills]=16&player_stats[5][Manabreaker][herodmg]=14332&player_stats[5][Manabreaker][herokillsgold]=7233&player_stats[5][Manabreaker][heroassists]=55&player_stats[5][Manabreaker][deaths]=8&player_stats[5][Manabreaker][goldlost2death]=3570&player_stats[5][Manabreaker][secs_dead]=120&player_stats[5][Manabreaker][teamcreepkills]=36&player_stats[5][Manabreaker][teamcreepdmg]=2400&player_stats[5][Manabreaker][teamcreepgold]=1600&player_stats[5][Manabreaker][neutralcreepkills]=7&player_stats[5][Manabreaker][neutralcreepdmg]=230&player_stats[5][Manabreaker][neutralcreepgold]=1200&player_stats[5][Manabreaker][bdmg]=7000&player_stats[5][Manabreaker][razed]=4&player_stats[5][Manabreaker][bgold]=4300&player_stats[5][Manabreaker][denies]=3&player_stats[5][Manabreaker][exp_denied]=3650&player_stats[5][Manabreaker][gold]=12343&player_stats[5][Manabreaker][gold_spent]=5000&player_stats[5][Manabreaker][exp]=8236&player_stats[5][Manabreaker][actions]=760&player_stats[5][Manabreaker][secs]=600"
//+"&player_stats[6][Ebulus][clan_id]=&player_stats[6][Ebulus][team]=1&player_stats[6][Ebulus][wins]=0&player_stats[6][Ebulus][losses]=1&player_stats[6][Ebulus][herokills]=7&player_stats[6][Ebulus][herodmg]=55332&player_stats[6][Ebulus][herokillsgold]=5533&player_stats[6][Ebulus][heroassists]=33&player_stats[6][Ebulus][deaths]=6&player_stats[6][Ebulus][goldlost2death]=3270&player_stats[6][Ebulus][secs_dead]=300&player_stats[6][Ebulus][teamcreepkills]=3&player_stats[6][Ebulus][teamcreepdmg]=3200&player_stats[6][Ebulus][teamcreepgold]=2300&player_stats[6][Ebulus][neutralcreepkills]=6&player_stats[6][Ebulus][neutralcreepdmg]=120&player_stats[6][Ebulus][neutralcreepgold]=2200&player_stats[6][Ebulus][bdmg]=4300&player_stats[6][Ebulus][razed]=2&player_stats[6][Ebulus][bgold]=2300&player_stats[6][Ebulus][denies]=7&player_stats[6][Ebulus][exp_denied]=3680&player_stats[6][Ebulus][gold]=43343&player_stats[6][Ebulus][gold_spent]=3400&player_stats[6][Ebulus][exp]=4536&player_stats[6][Ebulus][actions]=890&player_stats[6][Ebulus][secs]=500"
//+"&player_stats[4][Ebulus][clan_id]=&player_stats[4][Ebulus][team]=1&player_stats[4][Ebulus][wins]=1&player_stats[4][Ebulus][losses]=0&player_stats[4][Ebulus][herokills]=16&player_stats[4][Ebulus][herodmg]=14332&player_stats[4][Ebulus][herokillsgold]=7233&player_stats[4][Ebulus][heroassists]=55&player_stats[4][Ebulus][deaths]=8&player_stats[4][Ebulus][goldlost2death]=3570&player_stats[4][Ebulus][secs_dead]=120&player_stats[4][Ebulus][teamcreepkills]=36&player_stats[4][Ebulus][teamcreepdmg]=2400&player_stats[4][Ebulus][teamcreepgold]=1600&player_stats[4][Ebulus][neutralcreepkills]=7&player_stats[4][Ebulus][neutralcreepdmg]=230&player_stats[4][Ebulus][neutralcreepgold]=1200&player_stats[4][Ebulus][bdmg]=7000&player_stats[4][Ebulus][razed]=4&player_stats[4][Ebulus][bgold]=4300&player_stats[4][Ebulus][denies]=3&player_stats[4][Ebulus][exp_denied]=3650&player_stats[4][Ebulus][gold]=12343&player_stats[4][Ebulus][gold_spent]=5000&player_stats[4][Ebulus][exp]=8236&player_stats[4][Ebulus][actions]=760&player_stats[4][Ebulus][secs]=600"
//+"&player_stats[4][Manabreaker][clan_id]=&player_stats[4][Manabreaker][team]=1&player_stats[4][Manabreaker][wins]=1&player_stats[4][Manabreaker][losses]=0&player_stats[4][Manabreaker][herokills]=16&player_stats[4][Manabreaker][herodmg]=14332&player_stats[4][Manabreaker][herokillsgold]=7233&player_stats[4][Manabreaker][heroassists]=55&player_stats[4][Manabreaker][deaths]=8&player_stats[4][Manabreaker][goldlost2death]=3570&player_stats[4][Manabreaker][secs_dead]=120&player_stats[4][Manabreaker][teamcreepkills]=36&player_stats[4][Manabreaker][teamcreepdmg]=2400&player_stats[4][Manabreaker][teamcreepgold]=1600&player_stats[4][Manabreaker][neutralcreepkills]=7&player_stats[4][Manabreaker][neutralcreepdmg]=230&player_stats[4][Manabreaker][neutralcreepgold]=1200&player_stats[4][Manabreaker][bdmg]=7000&player_stats[4][Manabreaker][razed]=4&player_stats[4][Manabreaker][bgold]=4300&player_stats[4][Manabreaker][denies]=3&player_stats[4][Manabreaker][exp_denied]=3650&player_stats[4][Manabreaker][gold]=12343&player_stats[4][Manabreaker][gold_spent]=5000&player_stats[4][Manabreaker][exp]=8236&player_stats[4][Manabreaker][actions]=760&player_stats[4][Manabreaker][secs]=600"
//+"&player_stats[8][Ebulus][clan_id]=&player_stats[8][Ebulus][team]=2&player_stats[8][Ebulus][wins]=0&player_stats[8][Ebulus][losses]=1&player_stats[8][Ebulus][herokills]=7&player_stats[8][Ebulus][herodmg]=55332&player_stats[8][Ebulus][herokillsgold]=5533&player_stats[8][Ebulus][heroassists]=33&player_stats[8][Ebulus][deaths]=6&player_stats[8][Ebulus][goldlost2death]=3270&player_stats[8][Ebulus][secs_dead]=300&player_stats[8][Ebulus][teamcreepkills]=3&player_stats[8][Ebulus][teamcreepdmg]=3200&player_stats[8][Ebulus][teamcreepgold]=2300&player_stats[8][Ebulus][neutralcreepkills]=6&player_stats[8][Ebulus][neutralcreepdmg]=120&player_stats[8][Ebulus][neutralcreepgold]=2200&player_stats[8][Ebulus][bdmg]=4300&player_stats[8][Ebulus][razed]=2&player_stats[8][Ebulus][bgold]=2300&player_stats[8][Ebulus][denies]=7&player_stats[8][Ebulus][exp_denied]=3680&player_stats[8][Ebulus][gold]=43343&player_stats[8][Ebulus][gold_spent]=3400&player_stats[8][Ebulus][exp]=4536&player_stats[8][Ebulus][actions]=890&player_stats[8][Ebulus][secs]=500"
//+"&player_stats[3][Ebulus][clan_id]=&player_stats[3][Manabreaker][team]=2&player_stats[3][Ebulus][wins]=1&player_stats[3][Ebulus][losses]=0&player_stats[3][Ebulus][herokills]=16&player_stats[3][Ebulus][herodmg]=14332&player_stats[3][Ebulus][herokillsgold]=7233&player_stats[3][Ebulus][heroassists]=55&player_stats[3][Ebulus][deaths]=8&player_stats[3][Ebulus][goldlost2death]=3570&player_stats[3][Ebulus][secs_dead]=120&player_stats[3][Ebulus][teamcreepkills]=36&player_stats[3][Ebulus][teamcreepdmg]=2400&player_stats[3][Ebulus][teamcreepgold]=1600&player_stats[3][Ebulus][neutralcreepkills]=7&player_stats[3][Ebulus][neutralcreepdmg]=230&player_stats[3][Ebulus][neutralcreepgold]=1200&player_stats[3][Ebulus][bdmg]=7000&player_stats[3][Ebulus][razed]=4&player_stats[3][Ebulus][bgold]=4300&player_stats[3][Ebulus][denies]=3&player_stats[3][Ebulus][exp_denied]=3650&player_stats[3][Ebulus][gold]=12343&player_stats[3][Ebulus][gold_spent]=5000&player_stats[3][Ebulus][exp]=8236&player_stats[3][Ebulus][actions]=760&player_stats[3][Ebulus][secs]=600"
//+"&player_stats[3][Manabreaker][clan_id]=&player_stats[3][Manabreaker][team]=2&player_stats[3][Manabreaker][wins]=1&player_stats[3][Manabreaker][losses]=0&player_stats[3][Manabreaker][herokills]=16&player_stats[3][Manabreaker][herodmg]=14332&player_stats[3][Manabreaker][herokillsgold]=7233&player_stats[3][Manabreaker][heroassists]=55&player_stats[3][Manabreaker][deaths]=8&player_stats[3][Manabreaker][goldlost2death]=3570&player_stats[3][Manabreaker][secs_dead]=120&player_stats[3][Manabreaker][teamcreepkills]=36&player_stats[3][Manabreaker][teamcreepdmg]=2400&player_stats[3][Manabreaker][teamcreepgold]=1600&player_stats[3][Manabreaker][neutralcreepkills]=7&player_stats[3][Manabreaker][neutralcreepdmg]=230&player_stats[3][Manabreaker][neutralcreepgold]=1200&player_stats[3][Manabreaker][bdmg]=7000&player_stats[3][Manabreaker][razed]=4&player_stats[3][Manabreaker][bgold]=4300&player_stats[3][Manabreaker][denies]=3&player_stats[3][Manabreaker][exp_denied]=3650&player_stats[3][Manabreaker][gold]=12343&player_stats[3][Manabreaker][gold_spent]=5000&player_stats[3][Manabreaker][exp]=8236&player_stats[3][Manabreaker][actions]=760&player_stats[3][Manabreaker][secs]=600"
//+"&player_stats[9][Ebulus][clan_id]=&player_stats[9][Ebulus][team]=2&player_stats[9][Ebulus][wins]=0&player_stats[9][Ebulus][losses]=1&player_stats[9][Ebulus][herokills]=7&player_stats[9][Ebulus][herodmg]=55332&player_stats[9][Ebulus][herokillsgold]=5533&player_stats[9][Ebulus][heroassists]=33&player_stats[9][Ebulus][deaths]=6&player_stats[9][Ebulus][goldlost2death]=3270&player_stats[9][Ebulus][secs_dead]=300&player_stats[9][Ebulus][teamcreepkills]=3&player_stats[9][Ebulus][teamcreepdmg]=3200&player_stats[9][Ebulus][teamcreepgold]=2300&player_stats[9][Ebulus][neutralcreepkills]=6&player_stats[9][Ebulus][neutralcreepdmg]=120&player_stats[9][Ebulus][neutralcreepgold]=2200&player_stats[9][Ebulus][bdmg]=4300&player_stats[9][Ebulus][razed]=2&player_stats[9][Ebulus][bgold]=2300&player_stats[9][Ebulus][denies]=7&player_stats[9][Ebulus][exp_denied]=3680&player_stats[9][Ebulus][gold]=43343&player_stats[9][Ebulus][gold_spent]=3400&player_stats[9][Ebulus][exp]=4536&player_stats[9][Ebulus][actions]=890&player_stats[9][Ebulus][secs]=500"
//+"&player_stats[2][Ebulus][clan_id]=&player_stats[2][Ebulus][team]=2&player_stats[2][Ebulus][wins]=1&player_stats[2][Ebulus][losses]=0&player_stats[2][Ebulus][herokills]=16&player_stats[2][Ebulus][herodmg]=14332&player_stats[2][Ebulus][herokillsgold]=7233&player_stats[2][Ebulus][heroassists]=55&player_stats[2][Ebulus][deaths]=8&player_stats[2][Ebulus][goldlost2death]=3570&player_stats[2][Ebulus][secs_dead]=120&player_stats[2][Ebulus][teamcreepkills]=36&player_stats[2][Ebulus][teamcreepdmg]=2400&player_stats[2][Ebulus][teamcreepgold]=1600&player_stats[2][Ebulus][neutralcreepkills]=7&player_stats[2][Ebulus][neutralcreepdmg]=230&player_stats[2][Ebulus][neutralcreepgold]=1200&player_stats[2][Ebulus][bdmg]=7000&player_stats[2][Ebulus][razed]=4&player_stats[2][Ebulus][bgold]=4300&player_stats[2][Ebulus][denies]=3&player_stats[2][Ebulus][exp_denied]=3650&player_stats[2][Ebulus][gold]=12343&player_stats[2][Ebulus][gold_spent]=5000&player_stats[2][Ebulus][exp]=8236&player_stats[2][Ebulus][actions]=760&player_stats[2][Ebulus][secs]=600"
//+"&player_stats[2][Manabreaker][clan_id]=&player_stats[2][Manabreaker][team]=2&player_stats[2][Manabreaker][wins]=1&player_stats[2][Manabreaker][losses]=0&player_stats[2][Manabreaker][herokills]=16&player_stats[2][Manabreaker][herodmg]=14332&player_stats[2][Manabreaker][herokillsgold]=7233&player_stats[2][Manabreaker][heroassists]=55&player_stats[2][Manabreaker][deaths]=8&player_stats[2][Manabreaker][goldlost2death]=3570&player_stats[2][Manabreaker][secs_dead]=120&player_stats[2][Manabreaker][teamcreepkills]=36&player_stats[2][Manabreaker][teamcreepdmg]=2400&player_stats[2][Manabreaker][teamcreepgold]=1600&player_stats[2][Manabreaker][neutralcreepkills]=7&player_stats[2][Manabreaker][neutralcreepdmg]=230&player_stats[2][Manabreaker][neutralcreepgold]=1200&player_stats[2][Manabreaker][bdmg]=7000&player_stats[2][Manabreaker][razed]=4&player_stats[2][Manabreaker][bgold]=4300&player_stats[2][Manabreaker][denies]=3&player_stats[2][Manabreaker][exp_denied]=3650&player_stats[2][Manabreaker][gold]=12343&player_stats[2][Manabreaker][gold_spent]=5000&player_stats[2][Manabreaker][exp]=8236&player_stats[2][Manabreaker][actions]=760&player_stats[2][Manabreaker][secs]=600"
//+"&player_stats[10][Ebulus][clan_id]=&player_stats[10][Ebulus][team]=2&player_stats[10][Ebulus][wins]=0&player_stats[10][Ebulus][losses]=1&player_stats[10][Ebulus][herokills]=7&player_stats[10][Ebulus][herodmg]=55332&player_stats[10][Ebulus][herokillsgold]=5533&player_stats[10][Ebulus][heroassists]=33&player_stats[10][Ebulus][deaths]=6&player_stats[10][Ebulus][goldlost2death]=3270&player_stats[10][Ebulus][secs_dead]=300&player_stats[10][Ebulus][teamcreepkills]=3&player_stats[10][Ebulus][teamcreepdmg]=3200&player_stats[10][Ebulus][teamcreepgold]=2300&player_stats[10][Ebulus][neutralcreepkills]=6&player_stats[10][Ebulus][neutralcreepdmg]=120&player_stats[10][Ebulus][neutralcreepgold]=2200&player_stats[10][Ebulus][bdmg]=4300&player_stats[10][Ebulus][razed]=2&player_stats[10][Ebulus][bgold]=2300&player_stats[10][Ebulus][denies]=7&player_stats[10][Ebulus][exp_denied]=3680&player_stats[10][Ebulus][gold]=43343&player_stats[10][Ebulus][gold_spent]=3400&player_stats[10][Ebulus][exp]=4536&player_stats[10][Ebulus][actions]=890&player_stats[10][Ebulus][secs]=500";
//     parameters = parameters + "&abilities[28][1][hero_id]=Manabreaker&abilities[28][1][sk_slot]=1&abilities[28][1][secs]=300&abilities[28][2][hero_id]=Manabreaker&abilities[28][2][sk_slot]=1&abilities[28][2][secs]=600&abilities[28][3][hero_id]=Manabreaker&abilities[28][3][sk_slot]=2&abilities[28][3][secs]=700&abilities[28][4][hero_id]=Manabreaker&abilities[28][4][sk_slot]=2&abilities[28][4][secs]=800";
 //    parameters = parameters + "&abilities[7][1][hero_id]=Manabreaker&abilities[7][1][sk_slot]=1&abilities[7][1][secs]=300&abilities[7][2][hero_id]=Manabreaker&abilities[7][2][sk_slot]=1&abilities[7][2][secs]=600&abilities[7][3][hero_id]=Manabreaker&abilities[7][3][sk_slot]=2&abilities[7][3][secs]=700&abilities[7][4][hero_id]=Manabreaker&abilities[7][4][sk_slot]=2&abilities[7][4][secs]=800";
  //   parameters = parameters + "&comments[28]=hoogityhooblah+yourmomma+<br>hi<p>heh<b>bold</b><u>underlined</u><i>italic</i><br><a>cocacola</a>";
   //  parameters = parameters + "&ratings[28]=10&ratings[2]=1&ratings[5]=0";
var parameters = "session=efc4d96e587db7de26faae5f8527e3d8&match_stats%5bmatch_id%5d=523649&match_stats%5bmap%5d=test&match_stats%5bmap_version%5d=0.0.0&match_stats%5btime_played%5d=589&match_stats%5bfile_size%5d=355650&match_stats%5bfile_name%5d=%2f20090730%2fM523649.honreplay&match_stats%5bc_state%5d=5&match_stats%5bversion%5d=0.1.30.0&player_stats%5b71234333%5d%5bHero_PollywogPriest%5d%5bclan_id%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bclan_id%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bteam%5d=1&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bposition%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bwins%5d=1&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5blosses%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bdiscos%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bconcedes%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bkicked%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bpub_skill%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bconcedevotes%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bherokills%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bherodmg%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bherokillsgold%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bheroassists%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bheroexp%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bdeaths%5d=1&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bbuybacks%5d=1&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bgoldlost2death%5d=240&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bsecs_dead%5d=30&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bteamcreepkills%5d=49&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bteamcreepdmg%5d=19814&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bteamcreepgold%5d=1959&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bteamcreepexp%5d=4159&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bneutralcreepkills%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bneutralcreepdmg%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bneutralcreepgold%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bneutralcreepexp%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bbdmg%5d=5259&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5brazed%5d=3&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bbdmgexp%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bbgold%5d=1999&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bdenies%5d=5&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bexp_denied%5d=247&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bgold%5d=3958&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bgold_spent%5d=7725&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bexp%5d=4159&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bactions%5d=461&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bsecs%5d=589&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5blevel%5d=8&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bconsumables%5d=0&player_stats%5b7%5d%5bHero_PollywogPriest%5d%5bwards%5d=0&inventory%5b7%5d%5b0%5d=Item_Weapon1&inventory%5b7%5d%5b1%5d=Item_Punchdagger&inventory%5b7%5d%5b2%5d=Item_Steamstaff&inventory%5b7%5d%5b3%5d=Item_Punchdagger&inventory%5b7%5d%5b4%5d=Item_Steamstaff&inventory%5b7%5d%5b5%5d=Item_AcolytesStaff&items%5b0%5d%5baccount_id%5d=7&items%5b0%5d%5bcli_name%5d=Item_Steamstaff&items%5b0%5d%5bsecs%5d=0&items%5b0%5d%5baction%5d=1&items%5b1%5d%5baccount_id%5d=7&items%5b1%5d%5bcli_name%5d=Item_Punchdagger&items%5b1%5d%5bsecs%5d=0&items%5b1%5d%5baction%5d=1&items%5b2%5d%5baccount_id%5d=7&items%5b2%5d%5bcli_name%5d=Item_Steamstaff&items%5b2%5d%5bsecs%5d=0&items%5b2%5d%5baction%5d=1&items%5b3%5d%5baccount_id%5d=7&items%5b3%5d%5bcli_name%5d=Item_Punchdagger&items%5b3%5d%5bsecs%5d=0&items%5b3%5d%5baction%5d=1&items%5b4%5d%5baccount_id%5d=7&items%5b4%5d%5bcli_name%5d=Item_Steamstaff&items%5b4%5d%5bsecs%5d=0&items%5b4%5d%5baction%5d=1&items%5b5%5d%5baccount_id%5d=7&items%5b5%5d%5bcli_name%5d=Item_AcolytesStaff&items%5b5%5d%5bsecs%5d=492&items%5b5%5d%5baction%5d=1&items%5b6%5d%5baccount_id%5d=7&items%5b6%5d%5bcli_name%5d=Item_ApprenticesRobe&items%5b6%5d%5bsecs%5d=584&items%5b6%5d%5baction%5d=1&items%5b7%5d%5baccount_id%5d=7&items%5b7%5d%5bcli_name%5d=Item_Scarab&items%5b7%5d%5bsecs%5d=586&items%5b7%5d%5baction%5d=1&items%5b8%5d%5baccount_id%5d=7&items%5b8%5d%5bcli_name%5d=Item_Weapon1&items%5b8%5d%5bsecs%5d=588&items%5b8%5d%5baction%5d=2&abilities%5b7%5d%5b0%5d%5bhero_cli_name%5d=Hero_PollywogPriest&abilities%5b7%5d%5b0%5d%5bability_cli_name%5d=Ability_PollywogPriest1&abilities%5b7%5d%5b0%5d%5bsecs%5d=0&abilities%5b7%5d%5b0%5d%5bslot%5d=0&abilities%5b7%5d%5b1%5d%5bhero_cli_name%5d=Hero_PollywogPriest&abilities%5b7%5d%5b1%5d%5bability_cli_name%5d=Ability_AttributeBoost&abilities%5b7%5d%5b1%5d%5bsecs%5d=23&abilities%5b7%5d%5b1%5d%5bslot%5d=4&abilities%5b7%5d%5b2%5d%5bhero_cli_name%5d=Hero_PollywogPriest&abilities%5b7%5d%5b2%5d%5bability_cli_name%5d=Ability_PollywogPriest1&abilities%5b7%5d%5b2%5d%5bsecs%5d=78&abilities%5b7%5d%5b2%5d%5bslot%5d=0&abilities%5b7%5d%5b3%5d%5bhero_cli_name%5d=Hero_PollywogPriest&abilities%5b7%5d%5b3%5d%5bability_cli_name%5d=Ability_AttributeBoost&abilities%5b7%5d%5b3%5d%5bsecs%5d=114&abilities%5b7%5d%5b3%5d%5bslot%5d=4&abilities%5b7%5d%5b4%5d%5bhero_cli_name%5d=Hero_PollywogPriest&abilities%5b7%5d%5b4%5d%5bability_cli_name%5d=Ability_PollywogPriest1&abilities%5b7%5d%5b4%5d%5bsecs%5d=196&abilities%5b7%5d%5b4%5d%5bslot%5d=0&abilities%5b7%5d%5b5%5d%5bhero_cli_name%5d=Hero_PollywogPriest&abilities%5b7%5d%5b5%5d%5bability_cli_name%5d=Ability_PollywogPriest4&abilities%5b7%5d%5b5%5d%5bsecs%5d=258&abilities%5b7%5d%5b5%5d%5bslot%5d=3&abilities%5b7%5d%5b6%5d%5bhero_cli_name%5d=Hero_PollywogPriest&abilities%5b7%5d%5b6%5d%5bability_cli_name%5d=Ability_PollywogPriest1&abilities%5b7%5d%5b6%5d%5bsecs%5d=346&abilities%5b7%5d%5b6%5d%5bslot%5d=0&abilities%5b7%5d%5b7%5d%5bhero_cli_name%5d=Hero_PollywogPriest&abilities%5b7%5d%5b7%5d%5bability_cli_name%5d=Ability_AttributeBoost&abilities%5b7%5d%5b7%5d%5bsecs%5d=433&abilities%5b7%5d%5b7%5d%5bslot%5d=4&frags%5b0%5d%5bkiller_id%5d=-1&frags%5b0%5d%5btarget_id%5d=7&frags%5b0%5d%5bsecs%5d=545";
     //var parameters = "f=set_online&session=4398b682bea722f481db2df93b29da16&num_conn=0&c_state=2&cgt=00:00:00&new=12345&max_players=3&location=USA&min_lvl=0&max_lvl=5&karma=3";
     //var parameters = "f=new_session&login=S2Seacow&pass=18241332807d0abd05c6e01609440458&port=2121&name=My+Server&desc=hehe";

     //var parameters = "f=accept_key&session=4a64745d3d11594d930232e9887cff7f&acc_key=c2e45cbb97c5132c59d21cdc5203a6dc";
      //var parameters = "f=c_conn&session=4398b682bea722f481db2df93b29da16&cookie=45b4765aba9869d9421031ca0fd2e0bd&ip=64.136.239.226";
      http_request = false;
      if (window.XMLHttpRequest) { // Mozilla, Safari,...
         http_request = new XMLHttpRequest();
         if (http_request.overrideMimeType) {
         	// set type accordingly to anticipated content type
            //http_request.overrideMimeType('text/xml');
            http_request.overrideMimeType('text/html');
         }
      } else if (window.ActiveXObject) { // IE
         try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
         } catch (e) {
            try {
               http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
         }
      }

      if (!http_request) {
         alert('Cannot create XMLHTTP instance');
         return false;
      }

      http_request.onreadystatechange = function () {
        if (http_request.readyState == 4) {
         if (http_request.status == 200) {
            document.getElementById('output').innerHTML = http_request.responseText;

         } else { alert('There was a problem with the request.'); }
        }
      };

      http_request.open('POST', url, true);
      http_request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      http_request.setRequestHeader("Content-length", parameters.length);
      http_request.setRequestHeader("Connection", "close");
      http_request.send(parameters);

}
</script>

</head>

<body>

<div id=output name=output>Nothing...</div>
<span onclick="postRequest()">SUBMIT&nbsp;REQUEST</span>
</body>

</html>