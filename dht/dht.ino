//************************************************************
// this is a simple example that uses the painlessMesh library
// 
// This example shows how to build a mesh with named nodes
//
//************************************************************
#include "namedMesh.h"
#include <DHT.h>


#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

#define   MESH_SSID       "node_home"
#define   MESH_PASSWORD   "nodehome1"
#define   MESH_PORT       5555

Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;

String nodeName = "Node_DHT_1"; // Name needs to be unique

float temp = 0;
float humi = 0;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 10 , TASK_FOREVER, &sendMessage );

void sendMessage() {

  temp = dht.readTemperature();
  humi = dht.readHumidity();

  DynamicJsonDocument doc(1024);
  doc["name"] = nodeName;
  doc["temp"] = temp;
  doc["humi"] = humi;

  String output;
  serializeJson(doc, output);
  String to = "Node_GATEWAY";
  Serial.printf("send %s \n", output.c_str());

  //mesh.sendBroadcast(output);
  mesh.sendSingle(to, output);
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  
  Serial.printf("(%u) , %s\n", from, msg.c_str());
}


void setup() {
  Serial.begin(115200);

  dht.begin();

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](String &from, String &msg) { 
    Serial.printf("Received message by name from: %s ", from.c_str());
  });

  mesh.onReceive(&receivedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}
