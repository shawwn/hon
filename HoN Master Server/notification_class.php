<?
    // NOTIFICATIONS CLASS

class Notification {
    var $create_time;
    var $expire_time;
    var $type;
    var $notify_id;

    //construct
    function Notification($type){
        $this->type = $type;
        $expiry = 0;
        switch($type)
        {
            case "TEST_1H":
                $expiry = 3600;  // 1 hour
            break;
            case "TEST_1D":
                $expiry = 86400; // 1 day
            break;
            default:
              $expiry = 3600;  // 1 hour
            break;            

        } $this->create_time = mktime();  $this->expire_time = $this->create_time+$expiry;
    }

    // notification base
    function base_notify($params, &$link){
        $type = mysql_real_escape_string($this->type);
        $params = mysql_real_escape_string($params, $link);
        $create_time = mysql_real_escape_string($this->create_time);
        $expire_time = mysql_real_escape_string($this->expire_time);

        $insert = "INSERT INTO hon_chat.notify (type, create_time, expire_time, data) VALUES ('{$type}', '{$create_time}', '{$expire_time}', '{$params}')";
        if($result = mysql_query($insert, $link) )
        {   $this->notify_id=mysql_insert_id($link);
        } else { $this->notify_id=0; }
    }

    function accounts_notify($aids, &$link){
        if( $this->notify_id )
        {   $id = mysql_real_escape_string($this->notify_id);
            // setup notifications for all linked account_ids
            $insert = "INSERT INTO hon_chat.notify_account (notify_id, account_id) VALUES ";
            if(sizeof($aids) > 0){
            foreach($aids AS $k => $v)
            {   if(ereg('^[1-9][0-9]+$', $v) )
                {
                    $this->remove_excess_notifications($v);
                    $v = mysql_real_escape_string($v);
                    $insert .= "('{$id}', '{$v}'), ";
                }
            } $insert = trim($insert, ", ");

            if($result = mysql_query($insert, $link) )
            {   return true;
            } else { return false; }
            } else { return false; }
        } else { return false; }

    }
    
    
    function remove_excess_notifications($account_id)
    {
      include('inc/l1.php');
      
      // see if any notifications need to be cleared out
      $sql = "SELECT COUNT(account_id) AS Count FROM hon_chat.notify_account WHERE account_id = $account_id";
      $result = mysql_query($sql, $l1);
      if ($row = mysql_fetch_assoc($result))      
      {
        if ($row['Count'] > 40)
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

}
