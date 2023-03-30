<?
error_reporting(E_ALL);
ini_set('display_errors', '1');
ini_set ( "memory_limit", "20M");
print "THIS IS SPARTA";
session_start();

// redirect unauthorized logins
if(!$_SESSION['upd_patch']){
    header("Location: login.php");
    exit;
}

set_time_limit(0);

include('../inc/l1.php');

global $arch, $os, $detail;

// search strings
if(isset($_POST['arch']) )
{$arch = mysql_real_escape_string($_POST['arch']); } else {$arch = $_POST['arch'] = 'i686'; }
if(isset($_POST['os']) )
{$os = mysql_real_escape_string($_POST['os']); } else {$os = $_POST['os'] = 'wac'; }
if(strlen($_POST['name']) > 0 )
{$name = " AND name like '".mysql_real_escape_string($_POST['name'])."' "; } else {$name = $_POST['name'] = ''; }

// tree functions
function explodeTree($array, $delimiter = '_', $baseval = false)
{
    $splitRE = '/' . preg_quote($delimiter, '/') . '/';
    $returnArr = array();
    foreach ($array as $key => $val) {
        // Get parent parts and the current leaf
        $parts = preg_split($splitRE, $key, -1, PREG_SPLIT_NO_EMPTY);
        $leafPart = array_pop($parts);

        // Build parent structure (might be slow for really deep and large structures)
        $parentArr = &$returnArr;
        foreach ($parts as $part) {
            if (!isset($parentArr[$part])) {
                $parentArr[$part] = array();
            } elseif (!is_array($parentArr[$part])) {
                $parentArr[$part] = $baseval ? array('__base_val' => $parentArr[$part]) : array();
            }
            $parentArr = &$parentArr[$part];
        }

        // Add the final part to the structure
        if (empty($parentArr[$leafPart])) {
            $parentArr[$leafPart] = $val;
        } elseif ($baseval && is_array($parentArr[$leafPart])) {
            $parentArr[$leafPart]['__base_val'] = $val;
        }
    }
    return $returnArr;
}



function plotTree($arr, $indent=0, $mother_run=true){
    global $detail;
    static $div = 0;

    if($mother_run){
        // the beginning of plotTree. We're at rootlevel
        echo "Start...<br />";
    }


    foreach($arr as $k=>$v){
        $div++;
        $disp = "block";
        // skip the baseval thingy. Not a real node.
        if($k == "__base_val") continue;
        // determine the real value of this node.
        $show_val = ( is_array($v) ? $v["__base_val"] : $v );

        // show the indents
        //echo str_repeat("&nbsp;&nbsp;&nbsp;&nbsp;", $indent);
        if($indent == 0){
            // this is a root node. no parents
            if(array_key_exists($show_val, $detail) )
            {   echo "<b>O: <a href=\"javascript:switchIt('node_{$div}')\">".$k."</b></a><br />\n";
                $disp = "none";
            } else {
                echo "<b><a href=\"javascript:switchIt('node_{$div}')\">/".$k."</b></a><br />\n";
            }
        } elseif(is_array($v)){
            // this is a normal node. parents and children
            echo "<b>+: <a href=\"javascript:switchIt('node_{$div}')\">/".$k."</b></a><br />\n";
        } else{
            // this is a leaf node. no children
            echo "<b>-: <a href=\"javascript:switchIt('node_{$div}')\">".$k."</b></a><br />\n";
            $disp = "none";
        }

        // show the actual node
        echo "<div id=\"node_{$div}\" style='display: {$disp}; margin-left: 15px'>\n";
        if(array_key_exists($show_val, $detail) )
        {   echo "<form name=\"node_{$div}_fm\" id=\"node_{$div}_fm\" method=post>\n";
            echo "<input name=\"name_{$div}\" id=\"name_{$div}\" type=hidden value='{$detail[$show_val]['name']}'>\n";
            echo "<input name=\"arch_{$div}\" id=\"arch_{$div}\" type=hidden value='{$detail[$show_val]['arch']}'>\n";
            echo "<input name=\"os_{$div}\" id=\"os_{$div}\" type=hidden value='{$detail[$show_val]['os']}'>\n";
            echo "Version: <input name=\"alpha_{$div}\" id=\"alpha_{$div}\" style=\"width: 20px;\" type=text value=\"{$detail[$show_val]['alpha']}\">.<input name=\"beta_{$div}\" id=\"beta_{$div}\" style=\"width: 20px;\" type=text value=\"{$detail[$show_val]['beta']}\">.<input name=\"omega_{$div}\" id=\"omega_{$div}\" style=\"width: 20px;\" type=text value=\"{$detail[$show_val]['omega']}\">.<input name=\"hotfix_{$div}\" id=\"hotfix_{$div}\" style=\"width: 20px;\" type=text value=\"{$detail[$show_val]['hotfix']}\"><br />\n";
            echo "Checksum: <input name=\"size_{$div}\" id=\"size_{$div}\" type=text value=\"{$detail[$show_val]['size']}\"><br />\n";
            echo "Last Modified: {$detail[$show_val]['date']}<br />\n";
            echo "Compat Version: <input name=\"compat_alpha_{$div}\" id=\"compat_alpha_{$div}\" style=\"width: 20px;\" type=text value=\"{$detail[$show_val]['compat_alpha']}\">.<input name=\"compat_beta_{$div}\" id=\"compat_beta_{$div}\" style=\"width: 20px;\" type=text value=\"{$detail[$show_val]['compat_beta']}\">.<input name=\"compat_omega_{$div}\" id=\"compat_omega_{$div}\" style=\"width: 20px;\" type=text value=\"{$detail[$show_val]['compat_omega']}\">.<input name=\"compat_hotfix_{$div}\" id=\"compat_hotfix_{$div}\" style=\"width: 20px;\" type=text value=\"{$detail[$show_val]['compat_hotfix']}\"><br />\n";          
            echo "Tag for Removal (0|1): <input name=\"remove_{$div}\" id=\"remove_{$div}\" type=text value=\"{$detail[$show_val]['remove']}\"><br />\n";

            echo "Primary URL: <input style=\"width:250px\" name=\"url_{$div}\" id=\"url_{$div}\" type=text value=\"{$detail[$show_val]['url']}\"><br />\n";
            echo "Secondary URL: <input style=\"width:250px\" name=\"url2_{$div}\" id=\"url2_{$div}\" type=text value=\"{$detail[$show_val]['url2']}\"><br />\n";

            echo "<input name=\"button_{$div}\" type=button value=\"Edit\" onclick=\"javascript:get('{$div}');\"><br />\n";
            echo "</form>\n";
        }

        if(is_array($v)){

            // this is what makes it recursive, rerun for childs
            plotTree($v, ($indent+1), false);
        }

        echo "</div>\n";

    }

    if($mother_run){
        echo "End.<br />";
    }
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
?>
<html>
<head>
<title>Savage 2 - A Tortured Soul</title>

<script language="JavaScript">
   var http_request = false;
   function makePOSTRequest(parameters, num) {
      http_request = false;
      if (window.XMLHttpRequest) { // Mozilla, Safari,...
         http_request = new XMLHttpRequest();
         if (http_request.overrideMimeType) {
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

      http_request.onreadystatechange = function(){
        var div = "node_" + num;
        if (http_request.readyState == 4) {
         if (http_request.status == 200) {
            result = http_request.responseText;
            document.getElementById(div).innerHTML = result;
         } else {
            alert('There was a problem with the request.');
         }
        }
      }

      http_request.open('POST', 'update_patch.php', true);
      http_request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      http_request.setRequestHeader("Content-length", parameters.length);
      http_request.setRequestHeader("Connection", "close");
      http_request.send(parameters);
   }

   function get(num) {
      var name = "name_" + num;
      var alpha = "alpha_" + num;
      var beta = "beta_" + num;
      var omega = "omega_" + num;
      var hotfix = "hotfix_" + num;
      var size = "size_" + num;
      var arch = "arch_" + num;
      var os = "os_" + num;
      var remove = "remove_" + num;
      var url = "url_" + num;
      var url2 = "url2_" + num;

      var compat_alpha = "compat_alpha_" + num;
      var compat_beta = "compat_beta_" + num;
      var compat_omega = "compat_omega_" + num;
      var compat_hotfix = "compat_hotfix_" + num;

      var div = num;
      var poststr = "name=" + escape(encodeURI( document.getElementById(name).value )) +
                    "&alpha=" + escape(encodeURI( document.getElementById(alpha).value )) +
                    "&beta=" + escape(encodeURI( document.getElementById(beta).value )) +
                    "&omega=" + escape(encodeURI( document.getElementById(omega).value )) +
                    "&hotfix=" + escape(encodeURI( document.getElementById(hotfix).value )) +
                    "&size=" + escape(encodeURI( document.getElementById(size).value )) +
                    "&arch=" + escape(encodeURI( document.getElementById(arch).value )) +
                    "&os=" + escape(encodeURI( document.getElementById(os).value )) +
                    "&remove=" + escape(encodeURI( document.getElementById(remove).value )) +
                    "&url=" + escape(encodeURI( document.getElementById(url).value )) +
                    "&url2=" + escape(encodeURI( document.getElementById(url2).value )) +
                    "&compat_alpha=" + escape(encodeURI( document.getElementById(compat_alpha).value )) +
                    "&compat_beta=" + escape(encodeURI( document.getElementById(compat_beta).value )) +
                    "&compat_omega=" + escape(encodeURI( document.getElementById(compat_omega).value )) +
                    "&compat_hotfix=" + escape(encodeURI( document.getElementById(compat_hotfix).value )) +
                    "&div=" + escape(encodeURI(div));

      //document.getElementById(div).innerHTML = poststr;
      makePOSTRequest(poststr, num);
   }

    function showIt(div){
         if (document.getElementById) {  // for Netscape
           myDiv = document.getElementById(div);
              myDiv.style.display = 'block';
         }

         if (document.all) {     // for IE
           eval("myDiv = "+document.getElementById(div)+";");
              myDiv.style.display = 'block';
         }
    }
    function hideIt(div){
         if (document.getElementById) {  // for Netscape
           myDiv = document.getElementById(div);
              myDiv.style.display = 'none';
         }

         if (document.all) {     // for IE
           eval("myDiv = "+document.getElementById(div)+";");
              myDiv.style.display = 'none';
         }
    }
    function switchIt(div){
        if (document.getElementById) {
            myDiv = document.getElementById(div);
            if(myDiv.style.display == 'block') {
                hideIt(div);
            } else {
                showIt(div);
            }
        }

        if (document.all) {
            eval("myDiv = "+document.getElementById(div)+";");
            if(myDiv.style.display == 'block') {
                hideIt(div);
            } else {
                showIt(div);
            }
        }
    }
</script>
</head>

<body>

<?
$array = array();
$detail= array();

$select = "SELECT * FROM hon_patches.version_check WHERE os='{$os}' AND arch='{$arch}' {$name} ORDER BY name";
if($result = mysql_query($select, $l1) )
{

    while($row = mysql_fetch_assoc($result) )
    {
	
        $array[$row['name']] = $row['name'];
        $detail[$row['name']] = $row;

    }

}

@mysql_close($l1);

?>
<table>
    <tr><form name=fm1 action="<?=$_SERVER['SCRIPT_NAME']?>" method=post>
        <td>OS:</td><td><input name="os" type=text value="<?=$_POST['os']?>"/>*</td>
        <td>Arch:</td><td><input name="arch" type=text value="<?=$_POST['arch']?>"/>*</td>
        <td><a href=logout.php>-Logout-</a></td>
    </tr>
    <tr>
        <td>Name/Path:</td><td><input name="name" type=text value="<?=$_POST['name']?>"/></td>
        <td colspan=2>Use % as a wildcard. IE. "/game/beast/%"</td>
        <td>&nbsp;</td>
    </tr>
    <tr>
        <td colspan=5><input name=sub1 type=submit value="Submit" /></td>
    </tr>
    </form>
</table>

<?



$tree = explodeTree($array, '/', true);





plotTree($tree);




?>
</body>
