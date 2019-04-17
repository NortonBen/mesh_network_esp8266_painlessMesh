//************************************************************
// this is a simple example that uses the painlessMesh library
// 
// This example shows how to build a mesh with named nodes
//
//************************************************************
#include "namedMesh.h"

#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>

const char* STATION_SSID     = "DungBat";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* STATION_PASSWORD = "dungcobat";     // The password of the Wi-Fi network#define   STATION_SSID     "mySSID"


#define   MESH_SSID       "node_home"
#define   MESH_PASSWORD   "nodehome1"
#define   MESH_PORT       5555

Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;

String nodeName = "Node_GATEWAY"; // Name needs to be unique

// User stub
void updateState() ; // Prototype so PlatformIO doesn't complain

Task taskUpdateState( TASK_SECOND * 4 , TASK_FOREVER, &updateState );

void updateState() {   
  if (WiFi.status() == WL_CONNECTED) {
    //http.begin(WiFi.gatewayIP().toString()+"/doan/getdata.php");  //Specify request destination

     StaticJsonDocument<1024> doc;
   
    WiFiClient client;

    HTTPClient http;

    if (http.begin(client, "http://192.168.1.75/doan/getdata.php")) {  // HTTP
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          //Serial.println(payload);
          deserializeJson(doc, payload);

          for(int i = 0; i< doc.size(); i++) {

             String output;
            serializeJson(doc[i], output);
             Serial.println(output);
            mesh.sendBroadcast(output);
          }
          
          
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

// 
//
//  String output;
//  serializeJson(doc, output);
//  String to = "Node_GATEWAY";
//  Serial.printf("send %s \n", output.c_str());
//
//  mesh.sendBroadcast(output);
//  mesh.sendSingle(to, output);

  taskUpdateState.setInterval(TASK_SECOND * 5);
}

// Needed for painless library
void receivedCallback(String &from, String &msg) {
  Serial.printf("Received message by name from: %s  %s", from.c_str(), msg.c_str());

  if (WiFi.status() == WL_CONNECTED) {

        //http.begin(WiFi.gatewayIP().toString()+"/doan/update.php");
 
    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if ( http.begin("http://192.168.1.75/doan/update.php")) {  // HTTP

      Serial.print("[HTTP] POST...\n");
      http.addHeader("Content-Type", "application/json");
      http.POST(msg);
      http.writeToStream(&Serial);
      http.end();
      
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
}


void setup() {
  Serial.begin(115200);


  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA);
  mesh.setName(nodeName); // This needs to be an unique name! 
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This and all other mesh should ideally now the mesh contains a root
  mesh.setContainsRoot(true);


  mesh.onReceive(&receivedCallback);
  userScheduler.addTask(taskUpdateState);
  taskUpdateState.enable();
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}
