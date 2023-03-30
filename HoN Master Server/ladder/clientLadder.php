<?php
include('../inc/l1.php');
$defaultFieldList = "games_played=1&wins=1&losses=1&concedes=0&concedevotes=0&buybacks=0&discos=0&kicked=0&pub_count=0&avg_score=0&herokills=0&herodmg=0&heroexp=0&herokillsgold=0&heroassists=0&deaths=0&goldlost2death=0&secs_dead=0&teamcreepkills=0&teamcreepdmg=0&teamcreepexp=0&teamcreepgold=0&neutralcreepkills=0&neutralcreepdmg=0&neutralcreepexp=0&neutralcreepgold=0&bdmg=0&bdmgexp=0&razed=0&bgold=0&denies=0&exp_denied=0&gold=0&gold_spent=0&exp=0&secs=0&consumables=0&wards=0&em_played=1&expPerMin=1&k_d_a=1&wardsPerGame=0&consumablesPerGa=0&avgGameLength=0&expPerGame=0&goldPerGame=0&goldPerMin=1&spentPerMin=0&deniesPerMin=0&deniesPerGame=1&buildingsPerGame=0&actionsPerMin=0";

foreach($_GET as $k => $v){
	$_POST[$k] = $v;
}
if($_POST['debug']){
	print_r($_POST);
	@mysql_close($l1);
	exit;
}
if($_POST['aId']){
	
	$getFieldList = "SELECT fieldList, sortString, dirString, emE, gpE FROM hon_server.ladderFields WHERE account_id = '".mysql_real_escape_string($_POST['aId'])."' LIMIT 1";
	if($doGetFieldList = mysql_query($getFieldList,$l1)){
		if(mysql_num_rows($doGetFieldList) > 0){
			$fRow = mysql_fetch_assoc($doGetFieldList);
			
			if($_POST['s']){
				$update = "UPDATE hon_server.ladderFields SET sortString = '".mysql_real_escape_string($_POST['s'])."' WHERE account_id = '".mysql_real_escape_string($_POST['aId'])."' LIMIT 1";
				if(!$doUpdate = mysql_query($update,$l1)){
					@mysql_close($l1);
					print serialize(array(0 => "failure", 1 => 1));
					exit;
				}
			} else if($fRow['sortString']){
				$_POST['s'] = $fRow['sortString'];
			}
			
			if($_POST['d']){
				$update = "UPDATE hon_server.ladderFields SET dirString = '".mysql_real_escape_string($_POST['d'])."' WHERE account_id = '".mysql_real_escape_string($_POST['aId'])."' LIMIT 1";
				if(!$doUpdate = mysql_query($update,$l1)){
					@mysql_close($l1);
					print serialize(array(0 => "failure", 1 => 1));
					exit;
				}
			} else if($fRow['dirString']){
				$_POST['d'] = $fRow['dirString'];
			}
			
			if($_POST['emE']){
				$update = "UPDATE hon_server.ladderFields SET emE = '".mysql_real_escape_string($_POST['emE'])."' WHERE account_id = '".mysql_real_escape_string($_POST['aId'])."' LIMIT 1";
				if(!$doUpdate = mysql_query($update,$l1)){
					@mysql_close($l1);
					print serialize(array(0 => "failure", 1 => 1));
					exit;
				}
			} else if($fRow['emE']){
				$_POST['emE'] = $fRow['emE'];
			}
			
			if($_POST['gpE']){
				$update = "UPDATE hon_server.ladderFields SET dirString = '".mysql_real_escape_string($_POST['gpE'])."' WHERE account_id = '".mysql_real_escape_string($_POST['aId'])."' LIMIT 1";
				if(!$doUpdate = mysql_query($update,$l1)){
					@mysql_close($l1);
					print serialize(array(0 => "failure", 1 => 1));
					exit;
				}
			} else if($fRow['gpE']){
				$_POST['gpE'] = $fRow['gpE'];
			}
			
			if($_POST['a']){
				
				$explode = explode("&",$fRow['fieldList']);
				
				switch($_POST['a']){
					case "r": //4
						$key = array_search((substr($_POST['f'],4)."=1"),$explode);
						$explode[$key] = str_replace("=1","=0",$explode[$key]);
					break;
					case "a":
						$key = array_search((substr($_POST['f'],4)."=0"),$explode);
						$explode[$key] = str_replace("=0","=1",$explode[$key]);
					break;
					case "x":
						$fieldList = $defaultFieldList;
					break;
					default:
				}

				if(!$fieldList){
					$fieldList = implode('&',$explode);
				}
				$update = "UPDATE hon_server.ladderFields SET fieldList = '".mysql_real_escape_string($fieldList)."' WHERE account_id = '".mysql_real_escape_string($_POST['aId'])."' LIMIT 1";
				if(!$doUpdate = mysql_query($update,$l1)){
					@mysql_close($l1);
					print serialize(array(0 => "failure", 1 => 1));
					exit;
				}
			} else {
				$fieldList = $fRow['fieldList'];
				//print $fieldList;
			}
		} else {
			$fieldList = $defaultFieldList;
			$insert = "INSERT INTO hon_server.ladderFields (account_id, fieldList) VALUES ('".mysql_real_escape_string($_POST['aId'])."', '".mysql_real_escape_string($fieldList)."')";
			if(!$doInsert = mysql_query($insert,$l1)){
				@mysql_close($l1);
				print serialize(array(0 => "failure", 1 => 1));
				exit;
			}
		}
	} else {
		@mysql_close($l1);
		print serialize(array(0 => "failure", 1 => 1));
		exit;
		$fieldList = $defaultFieldList;
	}
	
}
@mysql_close($l1);
include('clientLadderClass.php');
/*
a(action) = 	r(remove)
				a(add)
				x(reset)
f = field name
p(page) = int
s(sort) = field name
d = asc/dir
aId = account_id
emE = em exclude % (expecting int)
gpE = games played exclude # (expecting int)
*/

if($_POST['emE'] || $_POST['gpE']){
	$excludes = array();
}
if($_POST['emE']){
	$excludes['emExclude'] = $_POST['emE'];
}
if($_POST['gpE']){
	$excludes['gpExclude'] = $_POST['gpE'];
}



$_POST['type'] ? $type = $_POST['type'] : $type = "name";
$cols = "default";
$_POST['s'] ? $sort = $_POST['s'] : $sort = "default";
$_POST['d'] ? $dir = $_POST['d'] : $dir = "ASC";
$perPage ? $sort_num = $perPage : $sort_num = 15;
$_POST['p'] ? $page = $_POST['p'] : $page = 1;
$client = new psrLadder();
$testVars = $client->setGridVars($page, $type, $cols, $sort, $dir, $sort_num, $fieldList, NULL, NULL, $excludes);
$displayGrid = $client->displayLadder($_POST['q'], $_POST['aId']);	

?>