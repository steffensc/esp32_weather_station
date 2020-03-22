<?php

$LOG_ACCESS_TOKEN = ""; /* SPECIFY YOUR OWN CUSTOM TOKEN STRIN HERE!! Which is used in the microcontroller code ;) */
$ENVIRONMENT_LOG_FOLDER = "environment_sensordata_logs/";

$acessToken = $_POST['token'];
$sensorNo = $_POST['sensor'];

$temperature = $_POST['temperature'];
$humidity = $_POST['humidity'];
$pressure = $_POST['pressure'];

$date = date("d.m.Y");
$time = date("G:i");


if( !strcmp($LOG_ACCESS_TOKEN, $acessToken) ){
	$logfiles = scandir($ENVIRONMENT_LOG_FOLDER);
	$logfile = "sensor_" . $sensorNo . "_datalog.csv";

	$logfile_is_found = false;
	foreach($logfiles as $file){
		if( !strcmp($file, $logfile) ){
			$logfile_is_found = true;
			break;
		}
	}

	if($logfile_is_found){
		$csv_file_string = $ENVIRONMENT_LOG_FOLDER.$logfile;
		$fp = fopen($csv_file_string, 'a') or die("Unable to open CSV");

		fputcsv($fp, array($date, $time, $temperature, $humidity, $pressure));

		fclose($fp);
	}
	else{
		echo "Sensor Number is incorrect or Logfile does not exist";
	}
}
else{
	echo "Access Denied";
}




?>