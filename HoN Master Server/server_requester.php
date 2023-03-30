<?
## PRELOADER AND CONTROLLER INSTANTIATOR

## SERVER REQUEST ##
include_once("server_request_class.php");
## SERVER CONTROLLER ##
include_once("server_controller.php");

## instantiate controller
$Controller = new ServerController($_REQUEST['f']);

## instantiate model object
$conn = new ServerRequest();

$conn->input = $Controller->vars;
// validate request method
if($Controller->runFunc)
{
    // controller method for this request
    $Controller->{$Controller->method}($conn);
}

// view
$conn->to_server();

?>