<?php
class psrLadder{
	function setVars(&$array){
		$this->arr = $array;
	}
	function setGridVars($page, $type, $cols, $sort, $dir, $sort_num, $fieldList = NULL, $clan_id = NULL, $clan_tag = NULL, $excludes = NULL){
		// get the grid vars for admin display
		$page ? $this->page = $page : $this->page = 1;
		$type ? $this->type = $type : $this->type = 'default';
		$cols ? $this->cols = $cols : $this->cols = '5';
		$sort ? $this->sort = $sort : $this->sort = 'name';
		$dir ? $this->dir = $dir : $this->dir = 'DESC';
		$sort_num ? $this->sort_num = $sort_num : $this->sort_num = 20;
		$clan_id ? $this->clan_id = $clan_id : $this->clan_id = NULL;
		$clan_tag ? $this->clan_tag = $clan_tag : $this->clan_tag = NULL;
		$fieldList ? $this->fieldList = $fieldList : $this->fieldList = NULL;
		count($excludes) > 0 ? $this->excludes = $excludes : $this->excludes = NULL;
	}
	
	public function displayLadder($q = NULL, $account_id = NULL, $nickname = NULL){
		include('../inc/ls.php');
		include("clientFieldArray.php");
		if($account_id){
			$account_id = mysql_real_escape_string($account_id);
		}
		if($nickname){
			$nickname = mysql_real_escape_string($nickname);
		}
		$account = $account_id;
		$nickname = $_SESSION['nickname'] ? $_SESSION['nickname'] : $nickname;
	
		if ($this->page){
			$lim1 = (($this->page * $this->sort_num) - $this->sort_num);
			$lim2 = $this->sort_num;
		} else {
			$lim1 = 0;
			$lim2 = $this->sort_num;
		}
		$lim2 > 25 ? $lim2 = 25 : $lim2 = $this->sort_num;

		$this->dir == "ASC" ? $dirLink = "DESC" : $dirLink = "ASC";

		if($_GET['type']){
			$dirLink = $dirLink."/".$_GET['type'];
		}
		$q ? $qLink2 = "/".$q : $qLink2 = ''; // gamesPlayed, k:d:a, avg psr +17
		
			if($q){
				$qLink = " WHERE ps.acc_nickname = '".mysql_real_escape_string($q)."' ";
			}
			$explode = explode("&",$this->fieldList);
			//print_r($explode);
			foreach($explode as $v){
				$result = substr($v,-1);
				if($result == '1'){
					$va = substr($v,0,-2);
					//print "va = ".$va;
					if(array_key_exists($va,$fields_select)){
						$selecter = $selecter.", ".$fields_select[$va];
						if(!$set){
							if($this->sort == "acc_".$va){
								$order = "acc_".$va;
								//$displayOrder = "acc_".$va;
								//$order = $va;
								$displayOrder = $va;
								$set = true;
							} else {
								$order = "ps.rank";
								$displayOrder = "acc_pub_skill";
							}
						}
					} else if(array_key_exists("acc_".$va,$fields_display)) {
						$selecter = $selecter.", ps.acc_".$va;
						if(!$set){
							if($this->sort == "acc_".$va){
								//$order = "ps.acc_".$va;
								//$displayOrder = "acc_".$va;
								$order = "ps.acc_".$va;
								$displayOrder = $va;
								$set = true;
							} else {
								$order = "ps.rank";
								$displayOrder = "acc_pub_skill";
							}
						}
					}
				}
			}
			
			$exc = '';
			$excC = '';
			if($this->excludes && is_array($this->excludes)){
				if(!$q){
					$exc = " WHERE ";
					$excC = " WHERE ";
				} else {
					$exc = " AND ";
					$excC = " AND ";
				}
				$i=1;
				$dispExc = " - <strong>Excluding</strong> ";
				foreach($this->excludes as $k => $v){
					switch($k){
						case "emExclude":
							//$exc = $exc."((ps.acc_em_played / ps.acc_games_played)*100) < '".mysql_real_escape_string($v)."' ";
							//$excC = $excC."((acc_em_played / acc_games_played)*100) < '".mysql_real_escape_string($v)."' ";
							$exc = $exc."ps.acc_em_percent < '".mysql_real_escape_string($v)."' ";
							$excC = $excC."acc_em_percent < '".mysql_real_escape_string($v)."' ";
							$dispExc = $dispExc."<a href=\"javascript:rExc('emExclude');\" style=\"color:red;\">EM % > ".$v."</a> ";
						break;
						case "gpExclude":
							$exc = $exc." ps.acc_games_played > '".mysql_real_escape_string($v)."' ";
							$excC = $excC." acc_games_played > '".mysql_real_escape_string($v)."' ";
							$dispExc = $dispExc."<a href=\"javascript:rExc('gpExclude');\" style=\"color:red;\">GP < ".$v."</a> ";
						break;
					}
					if(count($this->excludes) > 1 && $i < count($this->excludes)){
						$exc = $exc."AND ";
						$excC = $excC."AND ";
						$dispExc = $dispExc."and ";
					}
					$i++;
				}
			}
			
			if($this->sort == "k"){
				$selecter = $selecter.", round((ps.acc_herokills/ps.acc_deaths),2) AS kills";
				$order = "kills";
			}
			if($this->sort == "a"){
				$selecter = $selecter.", round((ps.acc_heroassists/ps.acc_deaths),2) AS assists";
				$order = "assists";
			}
			//$select = "SELECT ps.acc_clan_tag AS tag, ps.acc_nickname AS nickname, ps.account_id, ps.rank, ps.acc_pub_skill".$selecter." FROM hon_stats.psr_ladder_stats AS ps ".$qLink.$exc;
			$select = "SELECT ps.acc_clan_tag AS tag, ps.acc_nickname AS nickname, ps.account_id, ps.rank, ps.acc_pub_skill".$selecter." FROM hon_stats.psr_ladder_stats AS ps ".$qLink;
			if($q){
				$select = $select."LIMIT ".mysql_real_escape_string($lim1).", ".mysql_real_escape_string($lim2);
			} else {
				$select = $select."ORDER BY ".$order." ".mysql_real_escape_string($this->dir)." LIMIT ".mysql_real_escape_string($lim1).", ".mysql_real_escape_string($lim2);
			}
			//print $select;
			if($qLink){
				$select_without = "SELECT COUNT(ps.account_id) AS totalRows FROM hon_stats.psr_ladder_stats AS ps ".$qLink.$exc;
			} else if($excC){
				$select_without = "SELECT COUNT(account_id) AS totalRows FROM hon_stats.psr_ladder_stats".$excC;
			} else {
				$select_without = "SELECT COUNT(account_id) AS totalRows FROM hon_stats.psr_ladder_stats";
			}

		if($result = mysql_query($select,$ls)){
			$i = 2;
			$color1 = "#112b42";
			$color2 = "#0a1030";
			$doWithout = mysql_query($select_without,$ls);
			$found_rows = mysql_fetch_assoc($doWithout);

			$selfer = array();
			$totalRows = $found_rows['totalRows'];
			$totalPages = ceil($totalRows/15);

			if($account){
				$self = "SELECT ps.acc_nickname AS nickname, ps.rank, ps.acc_pub_skill".$selecter." FROM hon_stats.psr_ladder_stats AS ps 
				WHERE ps.account_id = '".mysql_real_escape_string($account)."'
				LIMIT 1";
				//print $self;
				$doSelf = mysql_query($self,$ls);
				if(mysql_num_rows($doSelf) == 1){
					$sRow = mysql_fetch_assoc($doSelf);
					
					//////////////////
					// Rank for self
					//
					
					
					//$sRow['rank'] > 999 ? $selfer[] = "XXX" : $selfer[] = $sRow['rank'];
					$selfer[] = $sRow['rank'];
						$displaySelf = '';
						if($sRow['tag'] != ''){
							//print "[".$sRow['tag']."]";
							$displaySelf .= "[".$sRow['tag']."]";
						}
						$displaySelf .= substr($sRow['nickname'],0,22);
						$selfer[] = $displaySelf;
					//////////////////
					// tag + name
					//
					
					$selfer[] = $sRow['acc_pub_skill'];
					
					//////////////////
					// Load Fields for Self
					//
					foreach($explode as $var){
						$match = substr($var,-1);
						$var = substr($var,0,-2);
						if($match == 1){

							is_numeric($sRow['acc_'.$var]) ? intval($sRow['acc_'.$var]) == $sRow['acc_'.$var] ? $selfer[] = number_format($sRow['acc_'.$var]) : $selfer[] = number_format($sRow['acc_'.$var],2,".",",") : $selfer[] = $sRow['acc_'.$var];

						}
					}
					$selfer[] = $account_id;

				}
			}
			//////////////////
			// Labels
			//
			$labels = array();
			$labels['rank'] = "Rank";
			$labels["Player"] = "Player";
			$labels["PSR"] = "Player Skill Rating";
			$widths[] = 4;
			$widths[] = 13;
			$widths[] = 5;
			foreach($explode as $var){
				// loop through added labels
				$match = substr($var,-1);
				$var = substr($var,0,-2);
				if($match == 1){
					$words = explode(" ",$fields_display['acc_'.$var]);
					$abbrev = '';
					foreach($words as $v){
						// appreviate labels
						$abbrev = $abbrev.strtoupper(substr($v,0,1));
					}
					if($abbrev == "KDA"){
						//$labels[$abbrev] = "K / D / A";
						$labels[] = 'acc_k_d_a';
						$widths[] = 12;
					} else {
						//$labels[$abbrev] = $fields_display['acc_'.$var];
						$labels[] = 'acc_'.$var;
						$widths[] = $fields['acc_'.$var];
					}
				}
			}
			//print $teamSorter;
			// loop through data
			while($row = mysql_fetch_assoc($result)){
				
				//$row['rank'] > 999 ? $display[$i][] = "XXX" : $display[$i][] = $row['rank'];
				//$row['rank'] > 999 ? $display[$i][] = $row['rank'] : $display[$i][] = $row['rank'];
				$display[$i][] = $row['rank'];
				
					$showName = '';
					if($row['tag'] != ''){
						$showName .= "[".$row['tag']."]";
					}
					$showName .= substr($row['nickname'],0,22);
				
				$display[$i][] = $showName;
				$display[$i][] = $row['acc_pub_skill'];
				foreach($explode as $var){
					$match = substr($var,-1);
					$var = substr($var,0,-2);
					if($match == 1){
						is_numeric($row['acc_'.$var]) ? intval($row['acc_'.$var]) == $row['acc_'.$var] ? $display[$i][] = number_format($row['acc_'.$var]) : $display[$i][] = number_format($row['acc_'.$var],2,".",",") : $display[$i][] = $row['acc_'.$var];
					}
				}
				$display[$i][] = $row['account_id'];
				$i++;
			}
			//print_r($selfer);
			//print_r($labels);
			//print_r($display);
			
			$fieldContents1 = '';
			$fieldContents2 = '';
			$fieldContents3 = '';
			$fieldContents4 = '';
			$fieldContents5 = '';
			$fieldContents6 = '';
			$fieldContents7 = '';
			$fieldContents8 = '';
			$fieldContents9 = '';
			$fieldContents10 = '';
			$fieldContents11 = '';
			$fieldContents12 = '';
			$fieldContents13 = '';
			$fieldContents14 = '';
			$fieldContents15 = '';
			
			$fieldNames = '';
			
			foreach($labels as $v){
				$fieldNames .= str_pad($v,20);
			}
			//$display = array_merge($display,$selfer);
			$display[] = $selfer;
			foreach($display as $k => $v){
				$i=1;
				foreach($v as $d){
					$vn = "fieldContents".$i;
					${$vn} .= str_pad($d,20);
					$i++;
				}
			}
			$fieldwidths = '';
			$fieldoffset = '';
			$offsets = 0;
			$totalW = 0;
			foreach($widths as $w){
				$fieldwidths .= str_pad($w,2,'0', STR_PAD_LEFT);
				$fieldoffsets .= str_pad(strval($offsets),2,'0', STR_PAD_LEFT);
				$offsets = $offsets+$w;
				if(($totalW + $w) > 100){
					break;
				}
			}
			
			$final=array();
			for($f=count($labels),$i=1;$i<=$f;$i++){
				$varN = "fieldContents".$i;
				$final["fieldcontents".$i] = ${$varN};
			}
			//$final = array("fieldcontents1" => $fieldContents1, "fieldcontents2" => $fieldContents2, "fieldcontents3" => $fieldContents3, "fieldcontents4" => $fieldContents4, "fieldcontents5" => $fieldContents5, "fieldcontents6" => $fieldContents6, "fieldcontents7" => $fieldContents7, "fieldcontents8" => $fieldContents8, "fieldcontents9" => $fieldContents9, "fieldcontents10" => $fieldContents10, "success" => "YES", "fieldnames" => $fieldNames, "fieldwidths" => $fieldwidths, "fieldoffsets" => $fieldoffsets, "totalpages" => $totalPages);
			
			$final['success'] = "YES";
			$final['fieldnames'] = $fieldNames;
			$final['fieldwidths'] = $fieldwidths;
			$final['fieldoffsets'] = $fieldoffsets;
			$final['totalpages'] = $totalPages;
			
			print serialize($final);
			//print_r($final);
			//print_r($final);
			@mysql_close($ls);
		} else {
			@mysql_close($ls);
			print serialize(array(0 => "failure", 1 => 2));
			exit;
		}

	}
	
}
	
?>