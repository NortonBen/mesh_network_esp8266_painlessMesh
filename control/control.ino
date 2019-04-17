//************************************************************
// this is a simple example that uses the painlessMesh library
// 
// This example shows how to build a mesh with named nodes
//
//************************************************************
#include "namedMesh.h"

#define   MESH_SSID       "node_home"
#define   MESH_PASSWORD   "nodehome1"
#define   MESH_PORT       5555

Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;

int ledPin = 0;
int plugPin = 14;
int touchPin = 16;

boolean touchState = LOW;
boolean touchLastState = HIGH;
boolean ledState = LOW;

boolean plugState = LOW;

String nodeName = "Node_Control_1"; // Name needs to be unique

void btnTouch();
void updateAndSendState();

Task taskTouch( TASK_SECOND * 0.1 , TASK_FOREVER, &btnTouch );

void btnTouch() {
    touchState = digitalRead(touchPin);
    if(touchState == HIGH && touchLastState == LOW) {
      if (ledState == HIGH){
        ledState = LOW;
      } else {
        ledState = HIGH;
      }
      updateAndSendState();
    }
    touchLastState = touchState;
    
}

void updateAndSendState() {

  digitalWrite(ledPin, ledState);
  digitalWrite(plugPin, plugState);

  DynamicJsonDocument doc(1024);
  doc["name"] = nodeName;
  doc["led"] = ledState;
  doc["plug"] = plugState;
  doc["type"] = "state";

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
  DynamicJsonDocument doc(1024);

  deserializeJson(doc, msg);
  JsonObject obj = doc.as<JsonObject>();

  if(obj[String("name")] == nodeName) {
     ledState = obj["led"].as<boolean>();
     plugState = obj["plug"].as<boolean>();
     digitalWrite(ledPin, ledState);
     digitalWrite(plugPin, plugState);
     updateAndSendState();
  }
}


void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(plugPin, OUTPUT);  
  pinMode(touchPin, INPUT);

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](String &from, String &msg) { 
    Serial.printf("Received message by name from: %s ", from.c_str());
  });

  mesh.onReceive(&receivedCallback);

  userScheduler.addTask(taskTouch);
  taskTouch.enable();
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}
