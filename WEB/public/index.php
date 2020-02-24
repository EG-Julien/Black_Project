<?php
@session_start();

date_default_timezone_set('Europe/Paris');

require '/home/pi/Automation/Black_Project/WEB/vendor/autoload.php';
require '/home/pi/Automation/Black_Project/WEB/config.php';

$app = new \Slim\App(
    [
        'settings' => [
            'displayErrorDetails' => true
        ]
    ]
);

require '/home/pi/Automation/Black_Project/WEB/app/container.php';

/**try {
    $DB = new PDO('mysql:dbname=' . $dbname . ';host=' . $dbhost . ';charset=utf8', "$dbuser", "$dbpassword");
    $DB->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $DB->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_OBJ);
} catch (Exception $e) {
    //die($e);
}

\App\Controllers\Controller::setDB($DB);

**/

$app->get('/', \App\Controllers\HomeCtrl::class . ':Home');
$app->get('/set/{room}/{stuff}/{state}', function ($request, $response, $args) {

    $room  = $args['room'];
    $stuff = $args['stuff'];
    $state = $args['state'];

    if ($room == "salon" && $stuff == "table") {
        if ($state == "on")
            $state = 100;
        if ($state == "off")
            $state = 0;
        $url = "http://192.168.33.157/set?power=$state";
        return $response->write(file_get_contents($url));
    }
});

$app->run();