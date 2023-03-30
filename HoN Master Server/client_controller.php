<?
## GARENA CLIENT CONTROLLER
define("_CURCLASS_", "ClientController");
define("_MAINTENANCE_", 0);

class ClientController{
    public $runFunc;
    public $method;
    public $vars;

    ## CONSTRUCT
    function ClientController($method){
        // Maintenance Mode
        if(_MAINTENANCE_){ $method="maintenance"; }

        $this->load_vars(); // load data

        if(class_exists(_CURCLASS_)) {
            $this->runFunc = (int) method_exists(_CURCLASS_, $method);
            $this->method = $method;
        }
    }

    // request data loader
    function load_vars(){
        foreach($_REQUEST AS $k => $v)
        { $this->vars[$k] = $v; }
    }

    ##
    ## CONSIDER THESE FUNCTIONS LIKE THE REQUESTER SWITCH
    ## USE FUNCTION DEFINITIONS FOR THE CASE INSTEAD
    ##

    // maintenance mode
    function maintenance(& $conn) {

    }

    // system login check
    function login_check(& $conn){
        $conn->login_check($this->vars['account_id'], $this->vars['password']);
    }

    // authentication
    function auth(& $conn) {
        $this->alpha_auth($conn);
    }
    function alpha_auth(& $conn){
        $conn->alpha_auth($this->vars['login'], $this->vars['password']);
    }

    // validation
    function validate_cookie(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->grab_chatserv_info();
            $conn->ret_array['account_type'] = $conn->cli_info['account_type'];
            $conn->ret_array['cookie'] = "TRUE";
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function chatclient_connect(& $conn){
        if( $conn->validate_cookie($this->vars['cookie']) ) {
            $conn->grab_chatserv_info();
            $conn->check_perms();
            $conn->ret_array['cookie'] = "TRUE";
            if (isset($this->vars['chat_mode_type'])) $conn->ret_array['chat_mode_type'] = $this->vars['chat_mode_type'];
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function logout(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->logout();
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function check_perms(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->check_perms();
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }

    function check_notification(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->check_notification($this->vars['key']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function private_notification(& $conn){
        $conn->private_notification($this->vars['account_id'], $this->vars['nickname'], $this->vars['ip'], $this->vars['port'], $this->vars['priv_key']);
    }

    function server_list(& $conn){
        if(@$conn->validate_cookie($this->vars['cookie']) ) {
            if(!isset($this->vars['region'])) $this->vars['region'] = "";
            if(!isset($this->vars['limit'])) $this->vars['limit'] = "";
            @$conn->get_server_list($this->vars['cookie'], $this->vars['gametype'], $this->vars['region'], $this->vars['limit']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }

    function buddy_list(& $conn){
        $conn->buddy_list($this->vars['account_id']);
    }
    function ignored_list(& $conn){
        $conn->ignored_list($this->vars['account_id']);
    }
    function banned_list(& $conn){
        $conn->banned_list($this->vars['account_id']);
    }
    // CHAT ROOMS
    function grab_rooms(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->grab_rooms($this->vars['account_id']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function add_room(& $conn){
        //if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->add_room($this->vars['account_id'], $this->vars['chatroom_name']);
        //} else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function remove_room(& $conn){
        //if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->remove_room($this->vars['account_id'], $this->vars['chatroom_name']);
        //} else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function clear_rooms(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->clear_rooms($this->vars['account_id']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }

    // CHAT LISTS
    function new_buddy(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->new_buddy($this->vars['account_id'], $this->vars['buddy_id']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function new_buddy2(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
          $conn->new_buddy2($this->vars['account_id'], $this->vars['buddynickname']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function approve_buddy2(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
          $conn->approve_buddy2($this->vars['account_id'], $this->vars['buddynickname']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function new_ignored(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->new_ignored($this->vars['account_id'], $this->vars['ignored_id']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function new_banned(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->new_banned($this->vars['account_id'], $this->vars['banned_id'], $this->vars['reason']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function remove_buddy(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->remove_buddy($this->vars['account_id'], $this->vars['buddy_id']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function remove_buddy2(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->remove_buddy2($this->vars['account_id'], $this->vars['buddy_id']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function remove_ignored(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->remove_ignored($this->vars['account_id'], $this->vars['ignored_id']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function remove_banned(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->remove_banned($this->vars['account_id'], $this->vars['banned_id']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }


    // clans
    function clan_list(& $conn){
        $conn->clan_rosterByClan_id($this->vars['clan_id']);
    }
    // clan admin
    function clan_nameCheck(& $conn){
        $conn->clan_nameCheck($this->vars['name'], $this->vars['tag']);
    }
    function create_clan(& $conn){
        if($conn->validate_cookie($this->vars['leader_ck']) ) {
            $conn->create_clan($this->vars['clan_name'], $this->vars['tag'], $conn->validate_user_cookie($this->vars['member1_ck']), $conn->validate_user_cookie($this->vars['member2_ck']), $conn->validate_user_cookie($this->vars['member3_ck']), $conn->validate_user_cookie($this->vars['member4_ck']));
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function add_member(& $conn){
        if($conn->validate_cookie($this->vars['member_ck']) ) {
            $conn->add_member( $conn->validate_user_cookie($this->vars['target_ck']), $this->vars['clan_id']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }
    function set_rank(& $conn){
        if($aid = $conn->validate_user_cookie($this->vars['member_ck']) ) {
            $conn->set_rank($aid, $this->vars['target_id'], $this->vars['clan_id'], $this->vars['rank']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }


    function nick2id(& $conn){
        $conn->ret_array = $conn->nick2id($this->vars['nickname']);
    }
    function id2nick(& $conn){
        $conn->ret_array = $conn->id2nick($this->vars['account_id']);
    }

    function grab_last_matches(& $conn){
        $conn->grab_last_matches($this->vars['account_id']);
    }

    function grab_match_stats(& $conn){
        $conn->grab_match_stats($this->vars['account_id'], $this->vars['match_id']);
    }
    function get_field_stats(& $conn){
        $aids = array();
        $fields = array();
        foreach($_REQUEST['account_id'] AS $k => $v)
        {   if(ereg('^[0-9]+$',$v) )
            { $aids[] = $v;  } // accept clean account_id
        }

        foreach($_REQUEST['field'] AS $k => $v)
        {    $fields[] = $v; }

        $conn->get_field_stats($aids, $fields);
    }
    function get_all_stats(& $conn){
        $aids = array();
        foreach($_REQUEST['account_id'] AS $k => $v)
        {   if(ereg('^[0-9]+$',$v) )
            { $aids[] = $v;  } // accept clean account_id
        }

        $conn->get_all_stats($aids);
    }
    function get_stats(& $conn){
        $aids = array();
        foreach($_REQUEST['account_id'] AS $k => $v)
        {   if(ereg('^[0-9]+$',$v) )
            { $aids[] = $v;  } // accept clean account_id
        }

        $conn->get_stats($aids);
    }
    function get_rank(& $conn){
        $aids = array();
        foreach($_REQUEST['account_id'] AS $k => $v)
        {   if(ereg('^[0-9]+$',$v) )
            { $aids[] = $v;  } // accept clean account_id
        }

        $conn->get_rank($aids);
    }

    function autocompleteNicks(& $conn){
        $conn->autocompleteNicks($this->vars['nickname']);
    }
    function autocompleteOnlineNicks(& $conn){
        $conn->autocompleteOnlineNicks($this->vars['nickname']);
    }

    function get_cgt(& $conn){
        $conn->get_cgt($this->vars['match_id'], $this->vars['nickname']);
    }
    function get_match_stats(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
        $mids = array();
        foreach($_REQUEST['match_id'] AS $k => $v)
        {   if(ereg('^[0-9]+$',$v) )
            { $mids[] = $v;  }  // accept clean match_id
        }
        if(sizeof($mids) ) {
            $conn->get_match_stats($mids);
        } } else { $conn->error['cookie']="Invalid Cookie"; }
    }

    function get_hero_usage(& $conn){
        $conn->get_hero_usage();
    }

    function get_last_login(& $conn){
        $conn->get_last_login($this->vars['account_id']);
    }

    function set_rating(& $conn){
        if($conn->validate_cookie($this->vars['cookie']) ) {
            $conn->set_rating($this->vars['type'], $this->vars['rate']);
        } else { $conn->error['cookie']="Invalid Cookie"; }
    }

    // BEGIN NEW NOTIFICATION STUFF
    // TODO: add cookie checks TO new_buddy2, approve_buddy2, remove_buddy2, and these notifications

    function test_create_notification(& $conn){
      //if($aid = $conn->validate_user_cookie($member_ck)) {
        $conn->create_notification($this->vars['type'], $this->vars['account_id'], $this->vars['params']);
      //} else { $conn->error['cookie']="Invalid Cookie"; }
    }

    function grab_notifications(& $conn){
        $conn->grab_notifications($this->vars['account_id']);
    }

    function delete_notification(& $conn){
      //if($aid = $conn->validate_user_cookie($member_ck)) {
        $conn->delete_notification($this->vars['account_id'], $this->vars['internal_id'], $this->vars['notify_id']);
      //} else { $conn->error['cookie']="Invalid Cookie"; }
    }

    function remove_all_notifications(& $conn){
      //if($aid = $conn->validate_user_cookie($member_ck)) {
        $conn->remove_all_notifications($this->vars['account_id']);
      //} else { $conn->error['cookie']="Invalid Cookie"; }
    }
    // END NEW NOTIFICATION STUFF

    function ip_capacity(& $conn){
        $conn->ip_capacity($this->vars['ip'], $this->vars['paid'], $this->vars['free']);
    }

}

?>