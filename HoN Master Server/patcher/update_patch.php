<?
session_start();

// redirect unauthorized logins
if(!$_SESSION['upd_patch']){
    header("Location: login.php");
    exit;
}

include('../inc/l1.php');

foreach($_POST as $k => $v)
{
    ${$k} = mysql_real_escape_string($v);
}

$update = "UPDATE hon_patches.version_check SET alpha='{$alpha}', beta='{$beta}', omega='{$omega}', hotfix='{$hotfix}', compat_alpha='{$compat_alpha}', compat_beta='{$compat_beta}', compat_omega='{$compat_omega}', compat_hotfix='{$compat_hotfix}', date=NOW(), size='{$size}', remove='{$remove}', url='{$url}', url2='{$url2}' WHERE os='{$os}' AND arch='{$arch}' AND name='{$name}' LIMIT 1";
if($result = mysql_query($update, $l1) )
{
    if(mysql_affected_rows($l1) == 1)
    {
        echo "<span style=\"color: #007c29\"> SUCCESS!</span><br />";
    } else {echo "<span style=\"color: #880e0e\">ERROR: ".mysql_affected_rows($l1)." records affected</span><br />"; }
    // form display
    echo "<form name=\"node_".$div."_fm\" id=\"node_".$div."_fm\" method=post>\n";
    echo "<input name=\"name_".$div."\" id=\"name_".$div."\" type=hidden value='{$_POST['name']}'>\n";
    echo "<input name=\"arch_".$div."\" id=\"arch_".$div."\" type=hidden value='{$_POST['arch']}'>\n";
    echo "<input name=\"os_".$div."\" id=\"os_".$div."\" type=hidden value='{$_POST['os']}'>\n";
    echo "Version: <input name=\"alpha_".$div."\" id=\"alpha_".$div."\" style=\"width: 20px;\" type=text value=\"{$_POST['alpha']}\">.<input name=\"beta_".$div."\" id=\"beta_".$div."\" style=\"width: 20px;\" type=text value=\"{$_POST['beta']}\">.<input name=\"omega_".$div."\" id=\"omega_".$div."\" style=\"width: 20px;\" type=text value=\"{$_POST['omega']}\">.<input name=\"hotfix_".$div."\" id=\"hotfix_".$div."\" style=\"width: 20px;\" type=text value=\"{$_POST['hotfix']}\"><br />\n";
    echo "Checksum: <input name=\"size_".$div."\" id=\"size_".$div."\" type=text value=\"{$_POST['size']}\"><br />\n";
    echo "Last Modified: ".date('Y-m-d')."<br />\n";
    echo "Compat Version: <input name=\"compat_alpha_{$div}\" id=\"compat_alpha_{$div}\" style=\"width: 20px;\" type=text value=\"{$_POST['compat_alpha']}\">.<input name=\"compat_beta_{$div}\" id=\"compat_beta_{$div}\" style=\"width: 20px;\" type=text value=\"{$_POST['compat_beta']}\">.<input name=\"compat_omega_{$div}\" id=\"compat_omega_{$div}\" style=\"width: 20px;\" type=text value=\"{$_POST['compat_omega']}\">.<input name=\"compat_hotfix_{$div}\" id=\"compat_hotfix_{$div}\" style=\"width: 20px;\" type=text value=\"{$_POST['compat_hotfix']}\"><br />\n";          
    echo "Tag for Removal: <input name=\"remove_".$div."\" id=\"remove_".$div."\" type=text value=\"{$_POST['remove']}\"><br />\n";
    echo "Primary URL: <input style=\"width:250px\" name=\"url_{$div}\" id=\"url_{$div}\" type=text value=\"{$_POST['url']}\"><br />\n";
    echo "Secondary URL: <input style=\"width:250px\" name=\"url2_{$div}\" id=\"url2_{$div}\" type=text value=\"{$_POST['url2']}\"><br />\n";
   echo "<input name=\"button_".$div."\" type=button value=\"Edit\" onclick=\"javascript:get('".$div."');\"><br />\n";
    echo "</form>\n";

}
@mysql_close($l1);


?>
