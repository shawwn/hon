<?
    ini_set ( "memory_limit", "20M");
    set_time_limit(0);

    include('../inc/l2.php');
	//if($_POST['os'] == 'win-beta')
	//{$_POST['os'] = 'win';}
    $arch = isset($_POST['arch'])?mysql_real_escape_string($_POST['arch']):"";
    $os = isset($_POST['os'])?mysql_real_escape_string($_POST['os']):"";
    $file = array();
    // version checking
    if(isset($_POST['version']) )
    {
        $version = explode('.', $_POST['version']);
        $version[0] = mysql_real_escape_string($version[0]);
        $version[1] = mysql_real_escape_string($version[1]);
        $version[2] = mysql_real_escape_string($version[2]);
        if(isset($version[3]) )
        { $version[3] = mysql_real_escape_string($version[3]); } else { $version[3] = 0; }

        $select = "SELECT * FROM hon_patches.version_check WHERE (os='{$os}' OR os='all') AND (arch='{$arch}' OR arch='shared') AND (alpha > {$version[0]} OR (alpha >= {$version[0]} AND beta > {$version[1]} ) OR (alpha >= {$version[0]} AND beta >= {$version[1]} AND omega > {$version[2]} ) OR (alpha >= {$version[0]} AND beta >= {$version[1]} AND omega >= {$version[2]} AND hotfix > {$version[3]} )) ORDER BY name ASC";
        if($result = mysql_query($select, $l2) )
        {   $count = 0;
            while($row = mysql_fetch_assoc($result) )
            {
             $file[$count]["name"] = $row['name'];
             $file[$count]["version"] = $row['alpha'].".".$row['beta'].".".$row['omega'].".".$row['hotfix'];
             if($row['hotfix'] == 0)
             {$file[$count]["version"] = $row['alpha'].".".$row['beta'].".".$row['omega'];
             } else {$file[$count]["version"] = $row['alpha'].".".$row['beta'].".".$row['omega'].".".$row['hotfix'];  }
             // compat versions
             $file[$count]["compat_version"] = $row['compat_alpha'].".".$row['compat_beta'].".".$row['compat_omega'].".".$row['compat_hotfix'];
             if($row['compat_hotfix'] == 0)
             {$file[$count]["compat_version"] = $row['compat_alpha'].".".$row['compat_beta'].".".$row['compat_omega'];
             } else {$file[$count]["compat_version"] = $row['compat_alpha'].".".$row['compat_beta'].".".$row['compat_omega'].".".$row['compat_hotfix'];  }

             $file[$count]["os"] = $row['os'];
             $file[$count]["arch"] = $row['arch'];
		$file[$count]["url"] = $row['url'];
             $file[$count]["url2"] = $row['url2'];
             $count++;
            }

        } else {$file['error'] = "Connection error. Try again."; }

        $select_vs = "SELECT * FROM hon_patches.version_check WHERE os='{$os}' AND arch='{$arch}' ORDER BY alpha DESC, beta DESC, omega DESC, hotfix DESC LIMIT 1";
        if($result = mysql_query($select_vs, $l2) )
        {   $row = mysql_fetch_assoc($result);
            $file['version']=$row['alpha'].".".$row['beta'].".".$row['omega'].".".$row['hotfix'];
        }
    }

    // get latest version
    if(isset($_POST['latest']) )
    {
        $select = "SELECT alpha, beta, omega, hotfix, url, url2 FROM hon_patches.version_check WHERE os='{$os}' AND arch='{$arch}' ORDER BY alpha DESC, beta DESC, omega DESC LIMIT 1";
        if($result = mysql_query($select, $l2) )
        {   if(mysql_num_rows($result) > 0)
            {   $row = mysql_fetch_assoc($result);
                $file['latest'] = $row['alpha']. "." .$row['beta']. "." . $row['omega'].".".$row['hotfix'];
            }
        } else {$file['error'] = "Connection error. Try again."; }
    }

    // checksum check
    if(isset($_POST['checksum']) )
    {
        foreach($_POST['file'] as $k => $v)
        {   $v = mysql_real_escape_string($v);
            if(ereg('%', $v) )
            {   $where .= "name like '{$v}' OR ";
            } else {
                $where .= "name = '{$v}' OR ";
            }
        }
        $where = trim($where, "OR ");
        $select = "SELECT name, size FROM hon_patches.version_check WHERE (os='{$os}' OR os='all') AND (arch='{$arch}' OR arch='shared') AND ({$where})";
        if($result = mysql_query($select, $l2) )
        {   $count = 0;
            while($row = mysql_fetch_assoc($result) )
            {
             $file[$row['name']] = $row['size'];
             $count++;
            }
        } else {$file['error'] = "Connection error. Try again."; }
    }

    // full version checking
    if(isset($_POST['fullversion']) )
    {
        foreach($_POST['file'] as $k => $v)
        {   $k = mysql_real_escape_string($k);
            $v = mysql_real_escape_string($v);
            $where .= "!(name = '{$k}' and size='{$v}' ) AND ";
        }

        $where = trim($where, 'AND ');
        $select = "SELECT * FROM hon_patches.version_check WHERE (os='{$os}' OR os='all') AND (arch='{$arch}' OR arch='shared') AND ({$where})";
        if($result = mysql_query($select, $l2) )
        {   $count = 0;
            while($row = mysql_fetch_assoc($result) )
            {
             $file[$count]["name"] = $row['name'];
             $file[$count]["version"] = $row['alpha'].".".$row['beta'].".".$row['omega'].".".$row['hotfix'];
             $file[$count]["os"] = $row['os'];
             $file[$count]["arch"] = $row['arch'];
             $count++;
            }
        } else {$file['error'] = "Connection error. Try again."; }

        $select_vs = "SELECT * FROM hon_patches.version_check WHERE os='{$os}' AND arch='{$arch}' ORDER BY alpha DESC, beta DESC, omega DESC, hotfix DESC LIMIT 1";
        if($result = mysql_query($select_vs, $l2) )
        {   $row = mysql_fetch_assoc($result);
            $file['version']=$row['alpha'].".".$row['beta'].".".$row['omega'].".".$row['hotfix'];
        }
    }
    @mysql_close($l2);
    // echo result
    echo serialize($file);
?>