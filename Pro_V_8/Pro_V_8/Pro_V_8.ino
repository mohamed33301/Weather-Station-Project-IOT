// Import required libraries
#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <Wire.h>
#include <AsyncTCP.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_Sensor.h>
#include "ESP32_MailClient.h"
#include "ESPAsyncWebServer.h"

// Replace with your network credentials
const char* ssid = "Ragab";
const char* password = "12345678";

Adafruit_BMP085 bmp;   // BMP 180
#define DHTPIN 15      // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11  // DHT 11
DHT dht(DHTPIN, DHTTYPE);
File myFile;           // Make Object For SD
const int CS = 5;      // For SD

//// To send Emails using Gmail on port 465 (SSL) :
#define emailSenderAccount "Ragab12mo@gmail.com"
#define emailSenderPassword "qitz uipq bdxm crql"
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
#define emailSubject "[ALERT] ESP32 Temperature"

// Default Recipient Email Address :
String inputMessage = "elsakam869@gmail.com";
String enableEmailChecked = "checked";
String inputMessage2 = "true";

// Default Threshold Temperature Value
String inputMessage3 = "25.0";
String lastTemperature;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  // float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // float h = dht.readDHTHumidity(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    Serial.println(h);
    return String(h);
  }
}

String readBMPPressure() {
  // Read temperature as Pascal (the default)
  float p = bmp.readPressure();
  //float P = dht.readPressure(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(p)) {
    Serial.println("Failed to read from Pressure sensor!");
    return "--";
  } else {
    Serial.println(p);
    return String(p);
  }
}

// Flag to track if the header line has been printed to the file
bool headerPrinted = false;

void WriteFile(const char* path, const char* temperature, const char* pressure, const char* humidity) {
  File file = SD.open(path, FILE_APPEND);
  if (file) {
    // If the file is empty or the header hasn't been printed, write the header
    if (file.size() == 0 || !headerPrinted) {
      file.println("Temperature   Pressure   Humidity");
      Serial.println("Temperature  Pressure  Humidity"); // Print to serial monitor
      headerPrinted = true; // Set the flag to true after printing the header
    }

    // Construct the line to write with temperature, pressure, and humidity data in columns
    String lineToWrite = String(temperature) + "        " + String(pressure) + "   " + String(humidity);
    file.println(lineToWrite);
    file.close();
    Serial.printf("Data written to %s\n", path);
  } else {
    Serial.println("Error opening file for writing");
  }
}

//* Web Page *//
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
        integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css"
        integrity="sha512-DTOQO9RWCH3ppGqcWaEA1BIZOC6xxalwEsw9c2QQeAIftl+Vegovlnee1c9QX4TctnWMn13TZye+giMm8e2LwA=="
        crossorigin="anonymous" referrerpolicy="no-referrer" />
    <style>
        html {
            font-family: Arial;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }

        body {
            background-color: #0b131e;
        }

        .Arrow {
            transform: rotateZ(-90deg);
            color: white;
            font-size: 25px;
        }

        h2 {
            color: #c3c3c3;
            font-style: italic;
            font-size: 25px;
            display: inline-block;
            margin-left: 25px;
            margin-right: 25px;
            padding: 15px;
        }

        .umbrella {
            color: #41b4e4;
            font-size: 45px;
            transform: rotateZ(10deg);
        }

        .icon-size {
            font-size: 40px;
        }

        .Paragraph {
            font-size: 3.0rem;
            background-color: #202b3b;
            margin-left: auto;
            margin-right: auto;
            padding: 15px;
            border-radius: 20px;
            margin: 15px 0px;
        }

        .value-color {
            color: white;
            font-size: 45px;
        }

        .units {
            font-size: 1.2rem;
            color: #c3c3c3;
            font-style: italic;
        }

        .dht-labels {
            font-size: 1.5rem;
            vertical-align: middle;
            padding-bottom: 15px;
            color: #c3c3c3;
            font-style: italic;
        }

        .INside {
            width: 400px;
            padding: 5px;
            color: #41b4e4;
            font-size: 17px;
            background-color: transparent;
            border: none;
            text-align: center;
            border-bottom: 3px solid #202b3b;
        }

        .As_Button {
            background-color: #41b4e4;
            width: 410px;
            height: 30px;
            padding: 10px;
            color: white;
            font-size: 17px;
            line-height: 5px;
            margin: 25px 0px;
            border-radius: 7px;
        }
    </style>
</head>

<body>
    <i class="fa-solid fa-angle-up Arrow"></i>
    <h2>Weather Station</h2>
    <i class="fa-solid fa-umbrella umbrella"></i>
    <div style="height: 250px;">
        <form action="/get">
            <input class="INside" type="email" name="email_input" value="%EMAIL_INPUT%" required><br>
            <input style="display: none;" type="checkbox" name="enable_email_input" value="true" checked
                %ENABLE_EMAIL%><br>
            <input class="INside" type="number" step="0.1" value="%THRESHOLD%" name="threshold_input" value="%THRESHOLD%"
                required><br>
            <input class="As_Button" type="submit" value="Submit">
        </form>
    </div>
    <p class="Paragraph">
        <i class="fa-solid fa-gauge-high icon-size" style="color: #113568;"></i>
        <span class="dht-labels">Pressure</span>
        <span id="pressure" class="value-color">%PRESSURE%</span>
        <sup class="units">pa</sup>
    </p>
        <p class="Paragraph">
        <i class="fas fa-thermometer-half icon-size" style="color:#cc0048;"></i>
        <span class="dht-labels">Temperature</span>
        <span id="temperature" class="value-color">%TEMPERATURE%</span>
        <sup class="units">&deg;C</sup>
    </p>
    <p class="Paragraph">
        <i class="fas fa-tint icon-size" style="color:#4fadc1;"></i>
        <span class="dht-labels">Humidity</span>
        <span id="humidity" class="value-color">%HUMIDITY%</span>
        <sup class="units">%</sup>
    </p>
</body>
<script>
    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("temperature").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/temperature", true);
        xhttp.send();
    }, 10000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("humidity").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/humidity", true);
        xhttp.send();
    }, 10000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("pressure").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/pressure", true);
        xhttp.send();
    }, 10000);
</script>

</html>)rawliteral";
//* Web Page *//

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

// Replaces placeholder with DHT & BMP values
String processor(const String& var) {
  if (var == "TEMPERATURE") {
    return readDHTTemperature();
  } else if (var == "HUMIDITY") {
    return readDHTHumidity();
  } else if (var == "PRESSURE") {
    return readBMPPressure();
  } else if (var == "EMAIL_INPUT") {
    return inputMessage;
  } else if (var == "ENABLE_EMAIL") {
    return enableEmailChecked;
  } else if (var == "THRESHOLD") {
    return inputMessage3;
  }
  return String();
}

// Flag variable to keep track if email notification was sent or not
bool emailSent = false;

const char* PARAM_INPUT_1 = "email_input";
const char* PARAM_INPUT_2 = "enable_email_input";
const char* PARAM_INPUT_3 = "threshold_input";

// Define global variables to store last recorded values
float lastTemperature_MS = 0.0;
float lastPressure = 0.0;
float lastHumidity = 0.0;

// Interval between sensor readings. Learn more about timers: https://RandomNerdTutorials.com/esp32-pir-motion-sensor-interrupts-timers/
unsigned long previousMillis = 0;
const long interval = 5000;

// The Email Sending data object contains config and data to send
SMTPData smtpData;

bool sendEmailNotification(String emailMessage);  // Declaration of sendEmailNotification function
void sendCallback(SendStatus msg);                // Declaration of sendCallback function

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  while (!Serial) { ; }  // wait for the serial port to connect. Needed for the native USB port only
  Serial.println("Initializing SD card...");
  
  if (!SD.begin(CS)) {
    Serial.println("Initialization failed!");
    return;
  }
  Serial.println("Initialization done.");

  // Start Connection With Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  dht.begin();
  bmp.begin();
  Wire.begin();

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());


// Write initial sensor readings to the file
  String temperatureMessage = readDHTTemperature();
  String pressureMessage = readBMPPressure();
  String humidityMessage = readDHTHumidity();

  WriteFile("/WeatherData.txt", temperatureMessage.c_str(), pressureMessage.c_str(), humidityMessage.c_str());

  // Store initial readings as last recorded values
  lastTemperature_MS = temperatureMessage.toFloat();
  lastPressure = pressureMessage.toFloat();
  lastHumidity = humidityMessage.toFloat();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", readBMPPressure().c_str());
  });

  //New
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    // GET email_input value on <ESP_IP>/get?email_input=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      // GET enable_email_input value on <ESP_IP>/get?enable_email_input=<inputMessage2>
      if (request->hasParam(PARAM_INPUT_2)) {
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        enableEmailChecked = "checked";
      } else {
        inputMessage2 = "false";
        enableEmailChecked = "";
      }
      // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage3>
      if (request->hasParam(PARAM_INPUT_3)) {
        inputMessage3 = request->getParam(PARAM_INPUT_3)->value();
      }
    } else {
      inputMessage = "No message sent";
    }

    Serial.println(inputMessage);
    Serial.println(inputMessage2);
    Serial.println(inputMessage3);

    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });
  // New

  // Start server
  server.onNotFound(notFound);
  server.begin();
}


void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    lastTemperature = readDHTTemperature();
    float temperature = lastTemperature.toFloat();
    // Check if temperature is above threshold and if it needs to send the Email alert
    if (temperature > inputMessage3.toFloat() && inputMessage2 == "true" && !emailSent) {
      String emailMessage = String("Temperature above threshold. Current temperature: ") + String(temperature) + String("C");
      if (sendEmailNotification(emailMessage))
       {
        Serial.println(emailMessage);
        emailSent = true;
      } else {
        Serial.println("Email failed to send");
      }
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if ((temperature < inputMessage3.toFloat()) && inputMessage2 == "true" && emailSent) {
      String emailMessage = String("Temperature below threshold. Current temperature: ") + String(temperature) + String(" C");
      if (sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        emailSent = false;
      } else {
        Serial.println("Email failed to send");
      }
    }
  }

 // Read sensor values
  String temperatureMessage = readDHTTemperature();
  String pressureMessage = readBMPPressure();
  String humidityMessage = readDHTHumidity();

  // Convert sensor readings to floats for comparison
  float currentTemperature = temperatureMessage.toFloat();
  float currentPressure = pressureMessage.toFloat();
  float currentHumidity = humidityMessage.toFloat();

  // Check if any sensor reading has changed
  if (currentTemperature != lastTemperature_MS || currentPressure != lastPressure || currentHumidity != lastHumidity) {
    // Update last recorded values
    lastTemperature_MS = currentTemperature;
    lastPressure = currentPressure;
    lastHumidity = currentHumidity;

    // Write updated sensor data to the file
    WriteFile("/WeatherData.txt", temperatureMessage.c_str(), pressureMessage.c_str(), humidityMessage.c_str());
  }

}

bool sendEmailNotification(String emailMessage) {
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be
  // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
  //smtpData.setSTARTTLS(true);

  // Set the sender name and Email
  smtpData.setSender("ESP32", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(inputMessage);

  smtpData.setSendCallback(sendCallback);

  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }
  // Clear all data from Email object to free memory
  smtpData.empty();
  return true;
}

// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }
}