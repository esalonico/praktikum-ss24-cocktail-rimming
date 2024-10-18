<?php
$esp32_ip = '131.159.6.138:8080';  // Local IP address of ESP32 on the Cocktail_Mixer network

$url = "http://$esp32_ip/weight";

// Build the shell command to run curl
$curl_command = "curl -s $url";  // -s for silent mode

// Execute the curl command using shell_exec()
$response = shell_exec($curl_command);

// Check if the response is empty or false (in case of errors)
if ($response === null || $response === false) {
    echo json_encode([
        'error' => 'Unable to connect to ESP32 or no response from the device.'
    ]);
} else {
    // Output the ESP32's response in JSON format
    header('Content-Type: application/json');
    echo $response;
}
?>