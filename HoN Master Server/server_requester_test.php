<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">

<head>
  <title>Hello!</title>
<script language=javascript>
var http_request = false;

function postRequest() {
    //alert(document.getElementById('nickname'+val).value);
     var url = "http://sl1.hon.s2games.com/server_requester.php";
	//var url = "http://sl1.hon.s2games.com/patcher/auto_patcher.php";
	//var parameters = "f=auth&login=S2KingKtulu&pass=ef2d5b26ca3a83de1113a7ec80fa99b8";
var parameters = "f=replay_auth&login=S2KingKtulu&pass=ef2d5b26ca3a83de1113a7ec80fa99b8";

     //var parameters = "f=new_session&login=md5testserv&pass=9b2844fc82a25a15305f1ec08814dfb4";
     //var parameters = "f=new_session&login=S2Maliken&pass=025ce905d8ce80da55fca175ff21a9a8&port=2121&name=My+Server&desc=hehe";
     //var parameters = "f=new_session&login=S2Ryan&pass=bbf8d5822a87f5d05f95780ced643c19&port=2121&name=My+Server&desc=hehe";
     //var parameters = "f=new_session&login=S2Jesse&pass=cb4d020c1974ec925c1b109ea56da485&port=2121&name=My+Server&desc=hehe";
     //var parameters = "f=new_session&login=S2Kyle&pass=889d6cb83920a5d32c3c8e369efeaeba&port=2121&name=My+Server&desc=hehe";
     //var parameters = "f=new_session&login=S2Toolz&pass=02a6859d4def9a40ee64c11d7b44e27c&port=2121&name=My+Server&desc=hehe";

     //var parameters = "f=shutdown&session=5495feea37359e8eac91c34b55d2c1a7";
     //var parameters = "f=set_online&session=15c7be38761512b1626db7f9aea11aff&num_conn=0&c_state=2&cgt=00:00:00";
     //var parameters = "f=set_online&session=3104639a490278399eecb3f341e4fcdc&num_conn=0&c_state=2&cgt=00:00:00&new=210-1111111&mname=Match+Name+For+Reals&max_players=3&location=USA&min_lvl=0&max_lvl=5&karma=3";
     //var parameters = "f=new_session&login=S2Seacow&pass=18241332807d0abd05c6e01609440458&port=2121&name=My+Server&desc=hehe";

     //var parameters = "f=accept_key&session=4a64745d3d11594d930232e9887cff7f&acc_key=c2e45cbb97c5132c59d21cdc5203a6dc";
      //var parameters = "f=c_conn&session=4398b682bea722f481db2df93b29da16&cookie=45b4765aba9869d9421031ca0fd2e0bd&ip=64.136.239.226";
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

         } else { alert('There was a problem with the request.'+http_request.status); }
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