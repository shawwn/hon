<?
if(isset($_POST['login']) AND isset($_POST['password']) )
{   include('../inc/l2.php');
    $login = mysql_real_escape_string($_POST['login']);
    $password = $_POST['password']; //mysql_real_escape_string($_POST['password']);
    $query = "SELECT * FROM hon_accounts.admin_account JOIN hon_accounts.admin_authorize USING (admin_id) WHERE username = '{$login}'";
    if($result = mysql_query($query, $l2) )
    {   if(mysql_num_rows($result) > 0)
        { 
		$user = mysql_fetch_assoc($result);

		if($user['password'] == md5(md5($password . $user['salt'])."|=essT0o>") ){

		session_set_cookie_params(7776000, '/', 'masterserver.hon.s2games.com');
          session_start();
          if($user['upd_patch'])
          { // success
            foreach($user as $key => $val)
            {$_SESSION[$key]=$val; }
            header("Location: patch_admin.php");
            exit;
          } else { $error = "You do not have access permissions."; }  // fail
		} else { $error = "Bad login credentials.  Try again.1"; }
        } else { $error = "Bad login credentials.  Try again."; }
    } else { $error = "Could not connect. Try again."; }
    @mysql_close($l2);
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">

<head>
  <title>Patch Admin Login</title>
</head>

<body>
<form action='login.php' method=post>
<table>
<tr>
    <td style="font-weight:bold" colspan=2>Patch Admin Login</td>
</tr>
<? if(isset($error) ) { ?>
<tr>
    <td style="color: red" colspan=2><?=$error?></td>
</tr>
<? } ?>
<tr>
    <td>Username:</td> <td><input name=login type=text /></td>
</tr>
<tr>
    <td>Password:</td> <td><input name=password type=password /></td>
</tr>
<tr>
    <td colspan=2><input type=submit value="Submit" /></td>
</tr>
</table></form>


</body>

</html>
