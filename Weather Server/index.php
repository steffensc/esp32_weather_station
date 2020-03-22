<?php
    setlocale(LC_TIME, "de_DE");

	$darkmode = $_COOKIE["darkmode"];

    $chart_hours = $_GET["chart_h"];
    if($chart_hours == NULL){
        $chart_hours = 4;
    }

    $ENVIRONMENT_LOG_FOLDER = "environment_sensordata_logs/";

    $logfiles = scandir($ENVIRONMENT_LOG_FOLDER);

    $recent_environment_data = array();
    $chart_environment_data = array();
    $chart_environment_data_avg = array();

    $currentday_environment_data = array();


    foreach($logfiles as $file){
        if( strpos($file, "sensor_") !== false ){
            $rows = file($ENVIRONMENT_LOG_FOLDER.$file);
            $first_row = $rows[0];
            $last_row = array_pop($rows);

            $name = $first_row;
            $data = str_getcsv($last_row);

            array_push($recent_environment_data, array($data[1], $data[2], $data[3], $data[4], $name) );


            $first_time = strtotime($data[0]." ".$data[1]);

            $hist_temperatures = array();
            $hist_humidities = array();
            $hist_pressures = array();

            for($i=count($rows)-1; $i > 0; $i--) { 
                $current_row = str_getcsv($rows[$i]);

                $time = strtotime($current_row[0]." ".$current_row[1]);
                if( ($first_time - $time) > 60*60*$chart_hours){
                    break;
                }
                else{
                    array_push($hist_temperatures, $current_row[2]);
                    array_push($hist_humidities, $current_row[3]);
                    array_push($hist_pressures, $current_row[4]);
                }
            }

            $hist_temperatures = array_reverse($hist_temperatures);
            $hist_humidities = array_reverse($hist_humidities);
            $hist_pressures = array_reverse($hist_pressures);

            array_push($chart_environment_data, array($hist_temperatures, $hist_humidities, $hist_pressures));

            // Calculate current Day Avgs, Min and Maxes
            $last_day_time = strtotime("today");
            $hist_currentday_temperatures = array();
            $hist_currentday_humidities = array();
            $hist_currentday_pressures = array();
            for($i=count($rows)-1; $i > 0; $i--) { 
                $current_row = str_getcsv($rows[$i]);

                $time = strtotime($current_row[0]." ".$current_row[1]);
 
                if( $time < $last_day_time){
                    break;
                }
                else{
                    array_push($hist_currentday_temperatures, $current_row[2]);
                    array_push($hist_currentday_humidities, $current_row[3]);
                    array_push($hist_currentday_pressures, $current_row[4]);
                }
            }

            if(count($hist_currentday_temperatures) > 0 && count($hist_currentday_humidities) > 0 && count($hist_currentday_pressures) > 0){
               
                $day_avrgs = array( (array_sum($hist_currentday_temperatures) / count($hist_currentday_temperatures)), (array_sum($hist_currentday_humidities) / count($hist_currentday_humidities)), (array_sum($hist_currentday_pressures) / count($hist_currentday_pressures)) );
                $day_minis = array(min($hist_currentday_temperatures), min($hist_currentday_humidities), min($hist_currentday_pressures));
                $day_maxes = array(max($hist_currentday_temperatures), max($hist_currentday_humidities), max($hist_currentday_pressures));
                array_push($currentday_environment_data, array($day_avrgs, $day_minis, $day_maxes));
            }
            else{
                array_push($currentday_environment_data, array(array("-", "-", "-"), array("-", "-", "-"), array("-", "-", "-")));
            }

        }
    }



    //Load calculated historic Day Avgs, Min and Maxes
    $ENVIRONMENT_HISTS_FOLDER = "environment_day_hists/";

    $day_hists = array();

    $MAX_DAYS = 4;
    
    foreach(scandir($ENVIRONMENT_HISTS_FOLDER) as $file){
        if( strpos($file, "sensor_") !== false ){
            $rows = file($ENVIRONMENT_HISTS_FOLDER.$file);

            $sensor_hists = array();
            $daycounter = 0;
            for($i=count($rows)-1; $i >= 0; $i--) { 
                $current_row = str_getcsv($rows[$i]);

                $day_headline =  strftime("%a", strtotime($current_row[0]));

                array_push($sensor_hists, array(array($current_row[1], $current_row[2], $current_row[3]), array($current_row[4], $current_row[5], $current_row[6]), array($current_row[7], $current_row[8], $current_row[9]), $day_headline  ));

                $daycounter++;

                if($daycounter >= $MAX_DAYS){
                    break;
                }
            }
            array_push($day_hists, array_reverse($sensor_hists));
        }
    }

?>

<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<meta http-equiv="cache-control" content="max-age=5">
	<meta name="robots" content="no-index, no-follow">

    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">

    <link rel="stylesheet" type="text/css" href="web_assets/style.css">
    <link rel="icon" href="web_assets/favicon.png" type="image/x-icon" />

    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-title" content="Weather PWA">
    <link rel="apple-touch-icon" href="web_assets/apple_icon.png">

    <meta name="description" content="Raumtemperatur, Luftfeuchtigkeit und Luftduck">

	<title>Raumklima</title>

	<?php if ($darkmode == "true"): ?>
        <meta name="apple-mobile-web-app-status-bar-style" content="black">
		<meta name="theme-color" content="#383735">	
		<link rel="manifest" href="web_assets/manifest_dark.json">
	<?php else: ?>
        <meta name="apple-mobile-web-app-status-bar-style" content="white">
		<meta name="theme-color" content="#ffffff">
		<link rel="manifest" href="web_assets/manifest.json">
	<?php endif; ?>

	<link href="https://fonts.googleapis.com/css?family=Red+Hat+Text:400,500|Open+Sans:400|Lato:400&display=swap" rel="stylesheet">
	
    <link rel="stylesheet" href="web_assets/font-awesome-4.7.0/css/font-awesome.min.css">

    <script type="text/javascript" src="web_assets/jquery-3.1.1.min.js"></script>
    
	<link rel="stylesheet" href="//cdn.jsdelivr.net/chartist.js/latest/chartist.min.css">
    <script src="//cdn.jsdelivr.net/chartist.js/latest/chartist.min.js"></script>
    <script src="web_assets/chartist-plugin-pointlabels.js"></script>

    <link rel="stylesheet" href="https://unpkg.com/swiper/css/swiper.min.css">
    <script src="https://unpkg.com/swiper/js/swiper.min.js"></script>


    <script type="text/javascript">
        var basic_options = {
            fullWidth: true,
            chartPadding: {
                top: 25,
                right: 0,
                bottom: 0,
                left: 0
            }, 
            axisX: {
                showGrid: false,
                showLabel: true,
            },
            axisY: {
                offset: 0,
                showGrid: false,
                showLabel: false,
            },
            plugins: [
                Chartist.plugins.ctPointLabels({
                    textAnchor: 'middle'
                })
            ]
        };
        function get_temperature_data(data_array){
            var data = {
                series: [{
                    className: 'temperature',
                    name: 'temperature',
                    data: data_array
                }]
            };
            return data;
        }
        function get_temperature_options(min, max){
            var options = Object.assign(
                {
                    series: {
                        'temperature': {
                            lineSmooth: Chartist.Interpolation.monotoneCubic(),
                            showLine: true,
                        }
                    },
                    high: max,
                    low: min,
                },
                basic_options
            );
            return options;
        }
        function get_humidity_data(data_array){
            var data = {
                series: [{
                    className: 'humidity',
                    name: 'humidity',
                    data: data_array
                }]
            };
            return data;
        }
        function get_humidity_options(min, max){
            var options = Object.assign(
                {
                    series: {
                        'humidity': {
                            lineSmooth: Chartist.Interpolation.monotoneCubic(),
                            showArea: true,
                        }
                    },
                    high: max,
                    low: min,
                },
                basic_options
            );
            return options;
        }
    </script>


</head>

<body class="<?php if ($darkmode == 'true'){ echo 'darkmode'; }?>">
<div id="content">	

	<a id="darkmode_button" class="controlbutton corner shadow clickarea">
		<i class="fa fa-lightbulb-o" aria-hidden="true"></i>
    </a>
    
    <!-- Slider main container -->
    <div class="swiper-container">
        <!-- Additional required wrapper -->
        <div class="swiper-wrapper">

            <?php foreach($recent_environment_data as $idx=>$recent_data): ?>
                <!-- Slide -->
                <div class="swiper-slide">

                    <div class="mainview wrapper verticalcenter center">
                        <h3><?php echo $recent_data[4]; ?></h3>
                        <p><?php echo $recent_data[0]; ?></p>
                        <h1><?php echo $recent_data[1]; ?><span>°</span></h1>
                        
                        <div class="center">
                            <h2 style="text-align: right; margin-right: 30px;"><?php echo $recent_data[2]; ?>%</h2>
                            <h2 style="text-align: left"><?php echo round($recent_data[3]); ?> hPa</h2>
                        </div>

                        <div class="spacer-medium desktop-only"></div>
                        <div class="spacer-small mobile-only"></div>

                        <div class="chart container center">
                            <div class="ct-chart-plot <?php echo "chart-hours-".$chart_hours; ?>" id="humidity-chart-<?php echo $idx;?>"></div>
                            <div class="ct-chart-plot <?php echo "chart-hours-".$chart_hours; ?> second" id="temperature-chart-<?php echo $idx;?>"></div>
                        </div>

                        <div class="spacer-small desktop-only"></div>

                        <div class="day container center">
                            <table width="100%">
                                <tr>
                                    <?php if(count($day_hists) > 0): ?>
                                    <?php foreach($day_hists[$idx] as $day_idx=>$day): ?>
                                        <td>
                                            <?php echo $day[3];?>
                                        </td>
                                    <?php endforeach; ?>
                                    <?php endif; ?>
                                    <td>
                                        Heute
                                    </td>
                                </tr>
                                <tr class="temps">
                                    <?php if(count($day_hists) > 0): ?>
                                    <?php foreach($day_hists[$idx] as $day_idx=>$day): ?>
                                        <td>
                                            <p><?php echo $day[0][0];?>°</p>
                                            <span><?php echo $day[0][1];?></span>
                                            <span><?php echo $day[0][2];?></span>
                                        </td>
                                    <?php endforeach; ?>
                                    <?php endif; ?>
                                  
                                    <td>
                                        <p><?php echo round($currentday_environment_data[$idx][0][0], 2);?>°</p>
                                        <span><?php echo $currentday_environment_data[$idx][1][0];?></span>
                                        <span><?php echo $currentday_environment_data[$idx][2][0];?></span>
                                    </td>
                                </tr>
                                <tr class="humis">
                                    <?php if(count($day_hists) > 0): ?>
                                    <?php foreach($day_hists[$idx] as $day_idx=>$day): ?>
                                        <td>
                                            <p><?php echo round($day[1][0]);?> hPa</p>
                                            <span><?php echo round($day[1][1]);?></span>
                                            <span><?php echo round($day[1][2]);?></span>
                                        </td>
                                    <?php endforeach; ?>
                                    <?php endif; ?>
                                    <td>
                                        <p><?php echo round($currentday_environment_data[$idx][0][2]);?> hPa</p>
                                        <span><?php echo round($currentday_environment_data[$idx][1][2]);?></span>
                                        <span><?php echo round($currentday_environment_data[$idx][2][2]);?></span>
                                    </td>
                                </tr>
                            </table>
                        </div>

                        <div class="spacer-small desktop-only"></div>

                        <div class="chart-selection">
                            <ul>
                                <li><a class="button clickarea corner <?php if($chart_hours == 4){echo "active";}?>" href="?chart_h=4">4h</a></li>
                                <li><a class="button clickarea corner <?php if($chart_hours == 12){echo "active";}?>" href="?chart_h=12">12h</a></li>
                                <li><a class="button clickarea corner <?php if($chart_hours == 24){echo "active";}?>" href="?chart_h=24">24h</a></li>
                            </ul>
                        </div>
                    </div>

                    <script type="text/javascript">
                        /*HUMIDITY*/
                        h_data = <?php echo json_encode($chart_environment_data[$idx][1]); ?>;
                        new Chartist.Line('#humidity-chart-<?php echo $idx?>', get_humidity_data(h_data), get_humidity_options(0, 100));
                                
                        /*TEMPERATURE*/
                        <?php 
                            $max_temp = max($chart_environment_data[$idx][0]);
                            $min_temp = min($chart_environment_data[$idx][0]);

                            $min_display_range = 2.5; //Fixed minmal Scalerange which won't be undershot
                            if($max_temp - $min_temp < $min_display_range){
                                $margin_append = ($min_display_range - ($max_temp - $min_temp)) / 2.0;
                                $min_temp = $min_temp - $margin_append;
                                $max_temp = $max_temp + $margin_append;
                            }
                        ?>
                        t_data = <?php echo json_encode($chart_environment_data[$idx][0]); ?>;
                        new Chartist.Line('#temperature-chart-<?php echo $idx;?>', get_temperature_data(t_data), get_temperature_options(<?php echo $min_temp . ", " . $max_temp; ?>));
                    </script>

                </div>
            <?php endforeach; ?>

        </div>

        <!-- Navigation buttons -->
        <div class="swiper-button-prev desktop-only"></div>
        <div class="swiper-button-next desktop-only"></div>
    </div>

</div>

<script>
function getCookie(cname) {
  var name = cname + "=";
  var decodedCookie = decodeURIComponent(document.cookie);
  var ca = decodedCookie.split(';');
  for(var i = 0; i <ca.length; i++) {
    var c = ca[i];
    while (c.charAt(0) == ' ') {
      c = c.substring(1);
    }
    if (c.indexOf(name) == 0) {
      return c.substring(name.length, c.length);
    }
  }
  return "";
}


$(document).ready(function() {
	var sel = getCookie('darkmode'); // get the cookie
	sel = sel=="true"; // convert to boolean - null is false, "false" is false

	$("#darkmode_button").click(function(){
    	sel = !sel; // toggle
		$("body").toggleClass( "darkmode", sel );
		document.cookie = "darkmode=" + sel  + "; path=/";
    });
    
    //initialize swiper when document ready
    var mySwiper = new Swiper ('.swiper-container', {
        navigation: {
            nextEl: '.swiper-button-next',
            prevEl: '.swiper-button-prev',
        },
            initialSlide: <?php if(is_null($_COOKIE["swipeSlide"])){echo 0; }else{ echo $_COOKIE["swipeSlide"];} ?>
        
    });

    mySwiper.on('slideChange', function () {
        document.cookie = "swipeSlide=" + mySwiper.realIndex  + "; path=/";
    });
});
</script>

</body>
</html>