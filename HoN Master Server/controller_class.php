<?
## ABSTRACT CONTROLLER CLASS ##
abstract class Controller {
    public $runFunc;
    public $method;
    public $vars;

    # FORCE EXTENDING CLASS TO DEFINE METHODS
    // maintenance mode
    abstract function maintenance(& $conn);

    # COMMON METHODS
    // request data loader
    function load_vars(){
        foreach($_REQUEST AS $k => $v)
        { $this->vars[$k] = $v; }
    }

}
?>