#include <WiFi.h>
#include <time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <ESP32Time.h>

#define PIR_SENSOR_PIN 19
int motion = 0;

const char* ssid = "IIIT-Guest";
const char* password = "I%GR#*S@!";

#define CSE_IP      "dev-onem2m.iiit.ac.in"
#define IP      "10.2.137.98"

#define OM2M_ORGIN    "Tue_20_12_22:Tue_20_12_22"
#define CSE_PORT    443
#define HTTPS     false
#define OM2M_MN     "/~/in-cse/in-name/"
#define OM2M_AE     "AE-AQ"
#define OM2M_NODE_ID   "AQ-KH00-01"
#define OM2M_DATA_CONT  "AQ-KH00-00/Data"
#define OM2M_DATA_LBL   "[\"AE-AQ\", \"V4.0.0\", \"AQ-KH00-00\", \"AQ-V4.0.0\"]"

WiFiClient client;
HTTPClient http;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.google.com");
ESP32Time rtc(0);

#define MIN_VALID_TIME 1672531200
#define MAX_VALID_TIME 2082844799

static uint64_t ntp_epoch_time = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIR_SENSOR_PIN, INPUT);
  connectToWiFi();
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());    
  Serial.print("Mac Address :");
  Serial.println(WiFi.macAddress());
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.println(rssi);
}

void sync_time() {
  static bool first_time = true;
  int num_tries = 5;  // Number of attempts to get valid NTP time

  // Check if the ESP32 is connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    if (first_time == true) {
      // Initialize NTPClient only once
      timeClient.begin();
      timeClient.setTimeOffset(0);
      timeClient.update();
      first_time = false;
    }

    while (num_tries > 0) {
      
      // Get the obtained NTP time
      uint64_t ntp_time = timeClient.getEpochTime();
      // Serial.println(ntp_time);
      // Validate the obtained NTP time
      if (ntp_time != 0 && ntp_time >= MIN_VALID_TIME && ntp_time <= MAX_VALID_TIME) {
        // Set the RTC time using the NTP time
        rtc.setTime(ntp_time);
        ntp_epoch_time = rtc.getEpoch();
        Serial.println(ntp_epoch_time);
        if(ntp_epoch_time < 1672000000){
          sync_time();
        }
        // Print the universal time and epoch time
          Serial.print("Universal time: " + rtc.getDateTime(true) + "\t");
          Serial.println(ntp_epoch_time, DEC); // For UTC, take timeoffset as 0
          // Serial.println("");
        return;  // Exit the function after successful synchronization
      } else {
        // Invalid NTP time, reduce the number of remaining attempts
        Serial.println("Invalid NTP time, retrying...");
        num_tries--;
        delay(1000);  
        Serial.println("All attempts failed, performing restarting time function ...");
        ESP.restart();   
      }
    }
  }
}

void postToOneM2M() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  String data = "[" + String(ntp_epoch_time) + " , " + String(motion) + "]";
  
  String server = "http://" + String(CSE_IP) + ":" + String(CSE_PORT) + String(OM2M_MN);

  http.begin(server + OM2M_AE + "/" + OM2M_DATA_CONT + "/");

  http.addHeader("X-M2M-Origin", OM2M_ORGIN); 
  http.addHeader("Content-Type", "application/json;ty=4");
  http.addHeader("Content-Length", String(data.length()));

  String req_data = "{\"m2m:cin\": {"
                    "\"con\": \"" + data + "\","
                    "\"lbl\": " + OM2M_DATA_LBL + ","
                    "\"cnf\": \"text\""
                    "}}";

  Serial.println(req_data);
  int code = http.POST(req_data);
  http.end();
  Serial.println("OneM2M HTTP Response Code: " + String(code));
}

void postToPythonServer() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  String data = "{\"motion\": " + String(motion) + ", \"timestamp\": " + String(ntp_epoch_time) + ", \"node_id\": \"" + String(OM2M_NODE_ID) + "\"}";


  String server = "http://" + String(IP) + ":8080/receive_motion";  // Replace with your server IP

  http.begin(server);

  http.addHeader("Content-Type", "application/json");

  int code = http.POST(data);
  http.end();
  Serial.println("Python Server HTTP Response Code: " + String(code));
}


void loop() {
  int pirValue = digitalRead(PIR_SENSOR_PIN);
    if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    connectToWiFi();
  }
  
  if (pirValue == HIGH) {
    Serial.println("Motion detected!");
    motion = pirValue;
    Serial.print("Motion value stored: ");
    Serial.println(motion); 
  } else {
    Serial.println("No motion detected!");
    motion = 0; 
    Serial.print("Motion value stored: ");
    Serial.println(motion); 
  }
  sync_time();
  postToOneM2M();
  postToPythonServer();
  delay(5000);
}
