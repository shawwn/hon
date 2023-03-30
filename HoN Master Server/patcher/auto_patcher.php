<?  
if(!empty($_POST['username']) AND !empty($_POST['password']) ){
    include('../inc/l1.php');

    $username=mysql_real_escape_string($_POST['username']);
    //$password=mysql_real_escape_string($_POST['password']);
    $password = $_POST['password'];

    $select = "SELECT * FROM hon_accounts.admin_account JOIN hon_accounts.admin_authorize USING (admin_id) WHERE username='{$username}'  AND upd_patch=1";
    if($result=mysql_query($select, $l1) )
    {	if(mysql_num_rows($result) > 0)
        {
	   $user = mysql_fetch_assoc($result);

	   if($user['password'] == md5(md5($password . $user['salt'])."|=essT0o>") ){

          if(isset($_POST['name']) )
          {
            foreach($_POST['name'] as $k => $v)
            {
              $name = mysql_real_escape_string($_POST['name'][$k]);
              $alpha = mysql_real_escape_string($_POST['alpha'][$k]);
              $beta = mysql_real_escape_string($_POST['beta'][$k]);
              $omega = mysql_real_escape_string($_POST['omega'][$k]);

              if(isset($_POST['hotfix'][$k]) )
              { $hotfix = mysql_real_escape_string($_POST['hotfix'][$k]); } else { $hotfix = 0; }



              $compat_alpha = mysql_real_escape_string($_POST['compat_alpha'][$k]);
              $compat_beta = mysql_real_escape_string($_POST['compat_beta'][$k]);
              $compat_omega = mysql_real_escape_string($_POST['compat_omega'][$k]);

              if(isset($_POST['compat_hotfix'][$k]) )
              { $compat_hotfix = mysql_real_escape_string($_POST['compat_hotfix'][$k]); } else { $compat_hotfix = 0; }

              $check_sum = mysql_real_escape_string($_POST['checksum'][$k]);
              $os = mysql_real_escape_string($_POST['os'][$k]);
              $arch = mysql_real_escape_string($_POST['arch'][$k]);
              $remove = mysql_real_escape_string($_POST['remove'][$k]);

              $where .= "('{$name}', '{$alpha}', '{$beta}', '{$omega}', '{$hotfix}', CURRENT_DATE, '{$check_sum}', '{$os}', '{$arch}', info, '{$remove}', url, url2, '{$compat_alpha}', '{$compat_beta}', '{$compat_omega}', '{$compat_hotfix}'), ";
            } $where = trim($where, ", ");

            $replace = "INSERT INTO hon_patches.version_check (name, alpha, beta, omega, hotfix, date, size, os, arch, info, remove, url, url2, compat_alpha, compat_beta, compat_omega, compat_hotfix) VALUES {$where} ON DUPLICATE KEY UPDATE name=VALUES(name), alpha=VALUES(alpha), beta=VALUES(beta), omega=VALUES(omega), hotfix=VALUES(hotfix), date=VALUES(date), size=VALUES(size), os=VALUES(os), arch=VALUES(arch), info=VALUES(info), remove=VALUES(remove)";
            if(!$res = mysql_query($replace, $l1) )
            {echo "error updating: {$name} {$insert}"; }

      	    $insert = "INSERT INTO hon_patches.version_store (name, alpha, beta, omega, hotfix, date, size, os, arch, info, remove) VALUES {$where}";
            if(!$res = mysql_query($insert, $l1) )
            {echo "error updating: {$name} {$insert}"; }
          }
	   } else {echo "wtf4"; }
       } else {echo "wtf3"; }
    } else {echo "wtf2"; }

    @mysql_close($l1);
    } else {echo "wtf"; }


?>
