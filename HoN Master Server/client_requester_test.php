<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">

<head>
  <title>Hello!</title>
<script language=javascript>
var http_request = false;

function postRequest() {

     var url = "client_requester.php";
     //var parameters = "f=alpha_auth&login=S2Seacow&password=18241332807d0abd05c6e01609440458";
     //var parameters = "f=alpha_auth&login=S2KingKtulu&password=9b2844fc82a25a15305f1ec08814dfb4";
     //var parameters = "f=server_list&cookie=d6e5e8b0f01f4f22fca0a8466bd8ac31&gametype=1-1111111&location=USA";//&max_players=10&karma=100&min_level=1&max_level=99";
     //var parameters = "f=server_list&cookie=fe8ee740753f76aeead3f05b57ffdbc8&gametype=10";
	//var parameters = "f=validate_cookie&cookie=dcba1ec341b16e1ecb2dbc2ba273ab04";
    //var parameters = "f=new_buddy&cookie=82e5620145f34085bb112058af9cbce1&account_id=28&buddy_id=7";
   // var parameters = "f=logout&cookie=dcba1ec341b16e1ecb2dbc2ba273ab04";


	//var parameters = "f=auth&login=WitheR&password=3dc71ee50383b498c91508bb24f5eafa";
	//var parameters = "f=auth&login=Mortician&password=c969b336246b9de94b0694eeb3268c90";
	//var parameters = "f=auth&login=Negative_guy&password=fffe07356b892d7ab664be167e070951";
    //var parameters = "f=grab_last_matches&account_id=28";
    //var parameters = "f=grab_match_stats&account_id=28&match_id=29768";
    //var parameters = "f=get_field_stats&account_id[]=28&field[]=level&field[]=acc_esxp";
	//var parameters = "f=nick2id&nickname[]=S2KingKtulu";
var parameters = "f=add_room&account_id=28&chatroom_name=test10&cookie=d6e5e8b0f01f4f22fca0a8466bd8ac31";
      http_request = false;
      if (window.XMLHttpRequest) { // Mozilla, Safari,...
         http_request = new XMLHttpRequest();
         if (http_request.overrideMimeType) {
         	// set type accordingly to anticipated content type
            //http_request.overrideMimeType('text/xml');
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

      http_request.onreadystatechange = function () {
        if (http_request.readyState == 4) {
         if (http_request.status == 200) {
            document.getElementById('output').innerHTML = http_request.responseText;
         } else { alert('There was a problem with the request.'); }
        }
      };
      http_request.open('POST', url, true);
      http_request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      http_request.setRequestHeader("Content-length", parameters.length);
      http_request.setRequestHeader("Connection", "close");
      http_request.send(parameters);

}
</script>

</head>

<body>

<div id=output name=output>Nothing...</div>
<span onclick="postRequest()">SUBMIT&nbsp;REQUEST</span>
</body>

</html>