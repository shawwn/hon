<?

## PRELOADER AND CONTROLLER INSTANTIATOR

## CLIENT REQUEST ##
include_once("client_request_class.php");
## CLIENT CONTROLLER ##
include_once("client_controller.php");

## instantiate controller
$Controller = new ClientController($_REQUEST['f']);


## instantiate model object
$conn = new ClientRequest();

// validate request method
if($Controller->runFunc)
{
    // controller method for this request
    $Controller->{$Controller->method}($conn);
}

// view
$conn->output();


?>