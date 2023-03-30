<?php

error_reporting(E_ALL);
ini_set('display_errors', '1');

class ClientRequest {
  var $error;
  var $ret_array;
  var $cli_info;
  
  //construct
  function ClientRequest()
  {
    $this->ret_array = array();
    $this->error = array();
  }
  
  //output function
  function output()
  {
    if(count($this->error) > 0)
    {   
      $false = array(false);
      $return = array_merge($false, $this->error);
      echo serialize($return);
    } 
    else 
    {
      //format output
      $true = array(true);
      $return = array_merge($true, $this->ret_array);
      
      $data = serialize($return);
      echo $data;
    }
  }
}

$conn = new ClientRequest();

foreach($_REQUEST as $key => $val)
{
  ${$key} = $val;
}


// Returns a pipe "|" delimited string of the upgrade names for a given account_id, 
// note that upgrades are tied to the super_id not just the account_id
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
    $super_id = $row['super_id'];
                                    
    // query all the upgrade names available for this super_id
    $sql = "SELECT hon_micro.products.name FROM hon_micro.upgrades JOIN hon_micro.products ON hon_micro.upgrades.product_id = hon_micro.products.product_id WHERE hon_micro.upgrades.super_id = $super_id;";
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

switch($_REQUEST['f'])
{

  case "get_upgrades":
  {   
    include('inc/l2.php');
    
    $upgrades = getUpgrades($account_id);
    $conn->ret_array['upgrades'][] = $upgrades;
    
    @mysql_close($l2);
  }
  break;
  
  case "c_conn":
  {
    include('inc/l1.php');

    $select = "SELECT account_id FROM hon_accounts.account WHERE hon_accounts.account.account_id = $account_id LIMIT 1";
    if($result = mysql_query($select, $l1))
    {
      if(mysql_num_rows($result))
      {
        $row = mysql_fetch_assoc($result);

        $select = "SELECT player_rank.account_id, clan_id, tag, level, acc_games_played, acc_pub_skill, acc_pub_count, rnk_amm_solo_rating, rnk_amm_solo_count, rnk_amm_solo_conf, rnk_amm_team_rating, rnk_amm_team_count, rnk_amm_team_conf, rnk_discos, rnk_games_played, acc_wins, acc_losses, acc_discos, IFNULL(acc_discos/(acc_wins+acc_losses),0) AS leaverpct FROM hon_stats.player_rank JOIN hon_stats.player_stats USING (account_id) JOIN hon_stats.ranked_stats USING (account_id)LEFT JOIN hon_clans.roster USING (account_id) LEFT JOIN hon_clans.clan_info USING (clan_id) WHERE account_id = '{$row['account_id']}' LIMIT 1";
        if($result = mysql_query($select, $l1))
        {
          $row = mysql_fetch_assoc($result);
          $conn->ret_array['infos'][$row['account_id']] = $row;
        }
        
        $upgrades = getUpgrades($account_id);
        if (!empty($upgrades))
          $conn->ret_array['infos']['upgrades'] = $upgrades;

        $conn->ret_array['c_conn'] = "OK";
      }
      else 
      { 
        $conn->error['cookie'] = "Invalid cookie."; 
      }
    } 
    else 
    { 
      $conn->error['connect'] = "Could not connect."; 
    }

    @mysql_close($l1);
  }    
  break;
  
	default:
		echo "pwned";
	break;
}

print_r($conn);

//$conn->output();

?>
