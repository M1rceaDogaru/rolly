#include <ESP8266WebServer.h>

const char* ssid = "<your WIFi,s SSID>";
const char* pass = "<your WiFi's Password>";

const int echoPin = D5;
const int trigPin = D3;
long duration;
double distance;

const int okCode = 200;
const String htmlContentType = "text/html";
const String jsonContentType = "application/json";

String page = 
R"=====(
<!DOCTYPE html>
<html lang="en">
    <head>
        <title>Rolly</title>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css" integrity="sha384-Vkoo8x4CGsO3+Hhxv8T/Q5PaXtkKtu6ug5TOeNV6gBiFeWPGFN9MuhOf23Q9Ifjh" crossorigin="anonymous">
        <style>
            .dot {
                height: 80px;
                width: 80px;
                background-color: #bbb;
                border-radius: 50%;
                display: inline-block;
            }
            .outer-circle {
                height: 200px;
                width: 200px;
                border-radius: 50%;
                border: 5px solid #bbb;
            }
            .frame {
                width: 18rem;
                height: 18rem;
            }
        </style>
        <script src="https://code.jquery.com/jquery-3.4.1.min.js"></script>
        <script src="https://cdn.jsdelivr.net/npm/popper.js@1.16.0/dist/umd/popper.min.js"></script>
        <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.4.1/js/bootstrap.min.js"></script>
        <script>
            $(document).ready(function () {
                var errorDistance = 15.0;
                var emptyDistance = 10.0;
                var currentDistance = 6.0;
                var last5Distances = [];
                var status;
                var color;
                function calculateRollSize(value) {
                    xMax = 200;
                    xMin = 80;

                    yMax = emptyDistance - 4;
                    yMin = emptyDistance;

                    if (value > yMin) return xMin;
                    if (value < yMax) return xMax;

                    percent = (value - yMin) / (yMax - yMin);
                    output = percent * (xMax - xMin) + xMin;
                    return output;
                }
                setInterval(function () {
                    $.get('/getDistance', function(response) {
                        currentDistance = parseFloat(response);
                        if (currentDistance > errorDistance) return;
                        last5Distances.push(currentDistance);
                        if (last5Distances.length > 5) last5Distances.shift();

                        var sum = 0;
                        for(var i = 0; i < last5Distances.length; i++ ){
                            sum += last5Distances[i];
                        }
                        currentDistance = sum/last5Distances.length; 

                        if (currentDistance >= emptyDistance) {
                            status = "EMPTY!";
                            color = "#ff0000";
                        } else if (currentDistance > emptyDistance - 0.2) {
                            status = "CRITICALLY LOW!";
                            color = "#ff704d";
                        } else if (currentDistance > emptyDistance - 1) {
                            status = "Getting low";
                            color = "#ff9933";
                        } else if (currentDistance > emptyDistance - 2) {
                            status = "Halfway there";
                            color = "#ffff00";
                        } else {
                            status = "Plenty";
                            color = "#40ff00";
                        }
                        var rollSize = Math.round(calculateRollSize(currentDistance));
                        console.info(rollSize);
                        $(".outer-circle").css("border-color", color);
                        $(".outer-circle").animate({
                            height: rollSize,
                            width: rollSize
                        }, 300);
                        $(".dot").css("background-color", color);
                        $(".card-text").text("Status: " + status);
                    });                    
                }, 300);
            });
        </script>
    </head>
    <body>
        <div class="container">
            <h2>ROLLY</h2>
            <div class="card" style="width: 18rem;">
                <div class="card-img-top frame d-flex justify-content-center">
                    <div class="outer-circle d-flex justify-content-center align-self-center">
                        <div class="dot d-flex align-self-center"></div>
                    </div>                    
                </div>
                <div class="card-body">
                  <h5 class="card-title">Bathroom 1</h5>
                  <p class="card-text">Status: Awaiting data</p>
                  <a href="#" class="btn btn-primary btn-sm">Configure</a>
                </div>
            </div>
        </div>
    </body>
</html>
)=====";

ESP8266WebServer server(80);

void indexPage() 
{
  server.send(okCode, htmlContentType, page);
}

void getDistance() 
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration*0.034/2;
  
  char message[100];
  sprintf(message, "%.2f", distance);
  server.send(okCode, jsonContentType, message);
}

void setupServer()
{
  server.on("/", indexPage);
  server.on("/getDistance", getDistance);
  server.begin();

  Serial.println("HTTP server started");
}

void connectToWifi() {
  Serial.println("Connecting to Wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.println("Waiting for WiFi connection");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connection successful. IP: ");
  Serial.println(WiFi.localIP());
}

void setupDistanceSensor()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void setup() {
  setupDistanceSensor();
  
  Serial.begin(115200);
  delay(1000);

  connectToWifi();
  setupServer();
}

void loop() {
  server.handleClient();
}
