<?
## GARENA SERVER CONTROLLER
define("_CURCLASS_", "ServerController");
define("_MAINTENANCE_", 0);

require_once("controller_class.php");

class ServerController extends Controller {
    # DECLARED PROPERTIES
    # public $runFunc;
    # public $method;
    # public $vars;

    ## CONSTRUCT
    function ServerController($method){
        // Maintenance Mode
        if(_MAINTENANCE_){ $method="maintenance"; }

        $this->load_vars(); // load data

        if(class_exists(_CURCLASS_)) {
            $this->runFunc = (int) method_exists(_CURCLASS_, $method);
            $this->method = $method;
        }
    }

    // maintenance mode - required
    function maintenance(& $conn) {

        echo "maintenance";

    }

    ##
    ## CONSIDER THESE FUNCTIONS LIKE THE REQUESTER SWITCH
    ## USE FUNCTION DEFINITIONS FOR THE CASE INSTEAD
    ##
    // standard authentication
    function auth(& $conn){
        if($conn->auth() )
        {  }
    }
    // replay authentication server update - DISABLE
    function replay_auth(& $conn){
        if($conn->auth() )
        {
          $conn->replay_auth();
        }
    }

    // create a new session
    function new_session(& $conn){
        if($conn->auth() )
        {
          $conn->new_session();
        }
    }

    // authenticate session
    function auth_session(& $conn){
        if(!$conn->auth_session() )
        { $conn->error['connect'] = "Could not authenticate. #2"; }
    }

    // accept game server key
    function accept_key(& $conn){
        if($conn->auth_session() )
        { $conn->accept_key();
        } else {$conn->error['connect'] = "Could not authenticate. #1"; }
    }

    // heartbeat
    function set_online(& $conn){
        if($conn->auth_session() )
        {   $_ok = 'OK';
            //update
            $conn->set_online($_ok);
        } else {$conn->error['connect'] = "Could not authenticate. #2"; }
    }

    // set ids online
    function set_online_ids(& $conn){
        if($conn->auth_session() )
        {
          $conn->set_online_ids();
        } else {$conn->error['connect'] = "Could not authenticate. #3"; }
    }

    // shut down
    function shutdown(& $conn){
        if($conn->auth_session() )
        {
          $conn->shutdown();
        } else {$conn->error['connect'] = "Could not authenticate. #4"; }
    }

    // new client connection
    function c_conn(& $conn){
        if($conn->auth_session() )
        { 	// maintenance request to kill new matches and connections
      	//$conn->error['connect']="Could not connect.";

      	$conn->c_conn();
        } else {$conn->error['connect'] = "Could not authenticate. #5"; }
    }

    // client disconnect
    function c_disco(& $conn){
        if($conn->auth_session() )
        {
          $conn->c_disco();
        } else {$conn->error['connect'] = "Could not authenticate. #6"; }
    }

    // start match
    function start_game(& $conn){
        if($conn->auth_session() )
        {
          $conn->start_game();
        } else {$conn->error['connect'] = "Could not authenticate. #7"; }
    }

    // update trial
    function update_trial(& $conn){
        if($conn->auth_session() )
        {
          $conn->update_trial();
        } else {$conn->error['connect'] = "Could not authenticate. #8"; }
    }

    // store CCU
    function chat_online(& $conn){
        $conn->chat_online();
    }

    // wtf is this?
    function whatismyip(& $conn){

        echo isset($_SERVER['HTTP_X_FORWARDED_FOR']) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : $_SERVER['REMOTE_ADDR'];
        die();

    }



}

?>