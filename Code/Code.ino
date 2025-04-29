// ###########################################################################################################################################
// #
// # Code for the "Ethernet switchable 8xUSB-port outlets" with WT32-ETH01 project: https://www.printables.com/model/1277752-ethernet-switchable-8xusb-port-outlets
// #
// # Code by AWSW on https://github.com/AWSW-de > https://github.com/AWSW-de/Ethernet-switchable-8xUSB-port-outlets
// #
// # Released under licenses:
// # GNU General Public License v3.0: https://github.com/AWSW-de/Ethernet-switchable-8xUSB-port-outlets#GPL-3.0-1-ov-file
// #
// # Important: NonCommercial — You may not use the material for commercial purposes !
// #
// # Parts of this code are based on a tutorial which source I could not find anymore. Props to the creators! =)
// #
// ###########################################################################################################################################


// ###########################################################################################################################################
// # Version: V1.0.0
// ###########################################################################################################################################


// ###########################################################################################################################################
// # Includes:
// #
// # You will need to add the following libraries to your Arduino IDE to use the project:
// # - AsyncWebServer_WT32_ETH01     https://github.com/khoih-prog/AsyncWebServer_WT32_ETH01/archive/refs/heads/main.zip
// # - AsyncTCP                      https://github.com/me-no-dev/AsyncTCP/archive/master.zip
// #
// # HowTo upload the code to the board: https://www.youtube.com/watch?v=W6hmV76SRHs
// #
// ###########################################################################################################################################
#include <Arduino.h>
#include <AsyncTCP.h>
#include <AsyncWebServer_WT32_ETH01.h>


// ###########################################################################################################################################
// # Static IP-Address settings: CHANGE TO YOUR ENVIRONMENT SETTINGS:
// ###########################################################################################################################################
String DirectIP = "192.168.178.249";  // IP-address used for the direct links section
IPAddress myIP(192, 168, 178, 249);   // IP-ADDRESS of your device
IPAddress mySN(255, 255, 255, 0);     // SUBNET MASK of your device
IPAddress myGW(192, 168, 178, 1);     // GATEWAY address of your router
IPAddress myDNS(192, 168, 178, 1);    // DNS address of your router


// ###########################################################################################################################################
// # Relay settings:
// ###########################################################################################################################################
#define RELAY_NO true                                          // Set to true to define relay as Normally Open (NO)
#define NUM_RELAYS 8                                           // Set number of relays
int relayGPIOs[NUM_RELAYS] = { 32, 33, 5, 17, 14, 15, 2, 4 };  // Assign each GPIO pins to a relays


// ###########################################################################################################################################
// # Code settings:
// ###########################################################################################################################################
AsyncWebServer server(80);            // Internal web server and port
#define _ASYNC_WEBSERVER_LOGLEVEL_ 2  // Web server log level
const char* PARAM_INPUT_1 = "relay";  // Relay input phrase
const char* PARAM_INPUT_2 = "state";  // Relasy status phrase


// ###########################################################################################################################################
// # Create the internal web page:                                                             !!! CHANGE THE URLS SHOWN ON THE PAGE BELOW !!!
// ###########################################################################################################################################
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 500px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #EE4B2B; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #20E326}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>Ethernet switchable 8xUSB-port outlets</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}</script>
<br><br><br>
</body>
</html>
)rawliteral";


// ###########################################################################################################################################
// # Replaces placeholder with button section in your web page:
// ###########################################################################################################################################
String processor(const String& var) {
  //Serial.println(var);
  if (var == "BUTTONPLACEHOLDER") {
    String buttons = "";
    for (int i = 1; i <= NUM_RELAYS; i++) {
      String relayStateValue = relayState(i);
      buttons += "<h4>Relay #" + String(i) + " - GPIO " + relayGPIOs[i - 1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" " + relayStateValue + "><span class=\"slider\"></span></label><br>";
      buttons += "<b>Relay #" + String(i) + " - ON:</b> <br><a href='http://" + String(DirectIP) + "/update?relay=" + String(i) + "&state=1' target='_blank'>http://" + String(DirectIP) + "/update?relay=" + String(i) + "&state=1</a><br>";
      buttons += "<b>Relay #" + String(i) + " - OFF:</b><br><a href='http://" + String(DirectIP) + "/update?relay=" + String(i) + "&state=0' target='_blank'>http://" + String(DirectIP) + "/update?relay=" + String(i) + "&state=0</a><br><br>";
    }
    return buttons;
  }
  return String();
}


// ###########################################################################################################################################
// # Relay status:
// ###########################################################################################################################################
String relayState(int numRelay) {
  if (RELAY_NO) {
    if (digitalRead(relayGPIOs[numRelay - 1])) {
      return "";
    } else {
      return "checked";
    }
  } else {
    if (digitalRead(relayGPIOs[numRelay - 1])) {
      return "checked";
    } else {
      return "";
    }
  }
  return "";
}


// ###########################################################################################################################################
// # Startup function:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000)
    ;
  delay(200);


  // To be called before ETH.begin()
  WT32_ETH01_onEvent();
  // bool begin(uint8_t phy_addr=ETH_PHY_ADDR, int power=ETH_PHY_POWER, int mdc=ETH_PHY_MDC, int mdio=ETH_PHY_MDIO,
  //           eth_phy_type_t type=ETH_PHY_TYPE, eth_clock_mode_t clk_mode=ETH_CLK_MODE);
  // ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);
  // Static IP, leave without this line to get IP via DHCP
  // bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  ETH.config(myIP, myGW, mySN, myDNS);
  WT32_ETH01_waitForConnect();

  // Serial print the MAC address:
  Serial.print("ETH MAC: ");
  Serial.println(ETH.macAddress());

  // Set all relays to off when the program starts - if set to Normally Open (NO), the relay is off when you set the relay to HIGH
  for (int i = 1; i <= NUM_RELAYS; i++) {
    pinMode(relayGPIOs[i - 1], OUTPUT);
    if (RELAY_NO) {
      digitalWrite(relayGPIOs[i - 1], HIGH);
    } else {
      digitalWrite(relayGPIOs[i - 1], LOW);
    }
  }


  // Root web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });


  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      if (RELAY_NO) {
        Serial.print("NO ");
        digitalWrite(relayGPIOs[inputMessage.toInt() - 1], !inputMessage2.toInt());
      } else {
        Serial.print("NC ");
        digitalWrite(relayGPIOs[inputMessage.toInt() - 1], inputMessage2.toInt());
      }
    } else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage + inputMessage2);
    request->send(200, "text/plain", "OK");
  });


  // Start server
  server.begin();

  Serial.print(F("HTTP EthernetWebServer URL is available @ http://"));
  Serial.println(ETH.localIP());
}


// ###########################################################################################################################################
// # Runtime function:
// ###########################################################################################################################################
void loop() {
  // not used in this project
}