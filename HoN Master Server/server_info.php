<?
    header('Location: http://admin.heroesofnewerth.com/server_info.php');
	 exit;

    include('inc/l2.php');
    $chckips = array();
    $select = "SELECT * FROM hon_accounts.server_info WHERE enabled=1";
    if($result = mysql_query($select, $l2) )
    {
        while($row = mysql_fetch_assoc($result) )
        {
            $chckips[$row['ip']] = $row;
        }
    }

    $select = "SELECT *, IF(last_upd>DATE_SUB(NOW(), INTERVAL 5 MINUTE), 1, 0) AS online FROM hon_server.info ORDER BY location, ip, c_state";

    $result = mysql_query($select, $l2);
    while($row = mysql_fetch_assoc($result) )
    {   if(array_key_exists($row['ip'], $chckips) ){
        if($row['location'] != 'NUB' OR isset($_GET['nub']) )
        {

        // locations and ip summary
        $locs[$row['location']] +=1;
        $ips[$row['location']][$row['ip']]=$row;  //= (empty($ips[$row['ip']]))?1:$ips[$row['ip']]+1;

        // check if online
        if($row['online'] == 1)
        {   $onips[$row['ip']] = 1;

            // online summ
            $online[$row['location']] = (empty($online[$row['location']]))?1 : $online[$row['location']]+1;

            // online zhombie summ
            if($row['c_state'] == 0 OR $row['c_state'] == 1)
            { $zombies[$row['location']] = (empty($zombies[$row['location']]))?1:$zombies[$row['location']]+1; }
            // lobby
            if($row['c_state'] == 2)
            { $lobby[$row['location']] = (empty($lobby[$row['location']]))?1:$lobby[$row['location']]+1; }
            // ingame
            if($row['c_state'] == 3)
            { $ingame[$row['location']] = (empty($ingame[$row['location']]))?1:$ingame[$row['location']]+1; }

            $players[$row['location']] = (empty($players[$row['location']]))?$row['num_conn']:$players[$row['location']]+$row['num_conn'];

            $svr[]=$row;
        } else {
            $offline[$row['location']] = (empty($offline[$row['location']]))?1:$offline[$row['location']]+1;
            $offips[$row['ip']] = $row;
        }

        } }
    }
?>
    <table width=50% cellpadding=2 border=1>
<?  $headerchk = 1;
    foreach($chckips AS $k => $v)  //chckips
    {
        if(!array_key_exists($k, $onips))
        {   if($headerchk) { $headerchk = 0; ?>
            <th colspan=4 style="font-size: 14px; text-align:left"><b>Non Responsive Servers</b></th>
            <tr><td><b>Location</b></td><td><b>Name</b></td><td><b>Host</b></td><td><b>IP</b></td></tr>
        <?  } ?>
            <tr bgcolor="red">
                <td><?=$offips[$k]['location']?></td>
                <td><?=$offips[$k]['name']?></td>
                <td><?=$v['host']?></td>
                <td><?=$k?></td>
            </tr>
<?      }
    }  ?>
    </table><br />
    <table width=50% cellpadding=2 border=1>
        <th colspan=8 style="font-size: 14px; text-align:left"><b>Usage By Location</b></th>
        <tr>
            <td><b>Location</b></td>
            <td><b>IPs</b></td>
            <td><b>Online</b></td>
            <td><b>Available</b></td>
            <td><b>Lobby</b></td>
            <td><b>In Game</b></td>
            <td><b>Offline</b></td>
            <td><b>Players</b></td>
        </tr>
<?  $grandt = array('instances'=>0, 'online'=>0);
    foreach($locs AS $l => $v)
    {
        $total = array('instances' => 0, 'online' => 0, 'zombies' => 0, 'lobby' => 0, 'ingame' => 0, 'offline' => 0, 'players' => 0);



        $total['online'] += $online[$l];
        $grandt['online'] += $total['online'];

        $total['offline'] += $offline[$l];

        $total['instances'] += $total['online'] + $total['offline'];
        $grandt['instances'] += $total['instances'];

        $boxes[$l] = sizeof($ips[$l]);



        $total['zombies'] += $zombies[$l];
        $total['lobby'] += $lobby[$l];
        $total['ingame'] += $ingame[$l];
        $total['players'] += $players[$l];

        ?>
        <tr>
            <td><?=strtoupper($l)?></td>
            <td><?=number_format($boxes[$l])?></td>
            <td><?=$total['online']?> (<?=@number_format($total['online']/$total['instances']*100, 1)?>%)</td>
            <td><?=$total['zombies']?> (<?=@number_format($total['zombies']/$total['online']*100, 1)?>%)</td>
            <td><?=$total['lobby']?> (<?=@number_format($total['lobby']/$total['online']*100, 1)?>%)</td>
            <td><?=$total['ingame']?> (<?=@number_format($total['ingame']/$total['online']*100, 1)?>%)</td>
            <td><?=$total['offline']?> (<?=@number_format($total['offline']/$total['instances']*100, 1)?>%)</td>
            <td><?=$total['players']?></td>
        </tr>
<?  }  ?>
        <tr>
            <td><b>Total: </b></td>
            <td><b><?=@array_sum($boxes)?></b></td>
            <td><b><?=$o = array_sum($online)?> (<?=number_format($o/$grandt['instances']*100, 1)?>%)</b></td>
            <td><b><?=$z = array_sum($zombies)?> (<?=@number_format($z/$grandt['online']*100, 1)?>%)</b></td>
            <td><b><?=$b = array_sum($lobby)?> (<?=@number_format($b/$grandt['online']*100, 1)?>%)</b></td>
            <td><b><?=$g = array_sum($ingame)?> (<?=@number_format($g/$grandt['online']*100, 1)?>%)</b></td>
            <td><b><?=$f = array_sum($offline)?> (<?=@number_format($f/$grandt['instances']*100, 1)?>%)</b></td>
            <td><b><?=array_sum($players)?></b></td>
        </tr>
    </table>   <br />
    <table cellpadding=2 border=1>
        <th colspan=9 style="font-size: 16px; text-align:left"><b>All Online Sessions</b></th>
        <tr>
            <td><b>ID</b></td>
            <td><b>IP</b></td>
            <td><b>Port</b></td>
            <td><b>Connected</b></td>
            <td><b>Name</b></td>
            <td><b>Location</b></td>
            <td><b>CGT</b></td>
            <td><b>Status</b></td>
            <td><b>Last Update</b></td>
        </tr>
<?
    $status = array(0=>'Zhombie', 1=>'Active', 2=>'Setup', 3=>'In Game', 4=>'Undefined', 5=>'End Game', 10=>"disco");
    $color = array('USE'=>'#FA7699','USM'=>'#C78697','USW'=>'#9D8C91', 'GER'=>'#76FADF', 'UKL'=> '#87E887', 'NLD'=>'#ACC2AC', 'EU'=>'#B4DB83');
    foreach($svr AS $k => $v )
    {  ?>
      <tr bgcolor="<?=$color[$v['location']]?>">
        <td><?=$v['server_id']?>&nbsp;</td>
        <td><?=$v['ip']?>&nbsp;</td>
        <td><?=$v['port']?>&nbsp;</td>
        <td><?=$v['num_conn']?>&nbsp;</td>
        <td><?=$v['name']?>&nbsp;</td>
        <td><?=$v['location']?>&nbsp;</td>
        <td><?=$v['cgt']?>&nbsp;</td>
        <td><?=$status[$v['c_state']]?>&nbsp;</td>
        <td><?=$v['last_upd']?>&nbsp;</td>
      </tr>
<?  }  ?>
    </table>