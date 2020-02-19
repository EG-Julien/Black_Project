<?php

namespace App\Controllers;

use Psr\Http\Message\RequestInterfarce;
use Psr\Http\Message\ResponseInterface;

class Controller {
    private $container;
    public static $DB;

    function __construct($container) {
        $this->container = $container;
    }

    public function render($response, $name, $params = []) {
        $this->container->view->render($response, $name, $params);
    }

    static function setDB($DB) {
        self::$DB = $DB;
    }

    static function getDB() {
        return self::$DB;
    }
}