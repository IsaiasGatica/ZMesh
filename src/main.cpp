/*
  Nodo Colector / Nodo Sensor
*/

#include "painlessMesh.h"
#include <ArduinoJson.h>

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

// #define NodoSensor

#define RXD2 16
#define TXD2 17

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

void sendMessage(); // Prototype so PlatformIO doesn't complain

#ifdef NodoSensor
Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);
#endif

void receivedCallback(uint32_t from, String &msg)
{
  Serial.println(msg);

#ifdef NodoSensor
  StaticJsonDocument<200> doc;
  doc["Nodo"] = "4";
  doc["Valor"] = 55;
  serializeJson(doc, Serial2);
#else
  Serial2.println(msg);

#endif
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
  String Nodos = mesh.subConnectionJson();
  Serial.println(Nodos);
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
  mesh.setContainsRoot(true);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

#ifdef NodoSensor

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
#endif
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
}

#ifdef NodoSensor

void sendMessage()
{
  String msg;
  StaticJsonDocument<200> doc;
  doc["Nodo"] = "4";
  doc["Valor"] = 55;
  serializeJson(doc, Serial2);
  serializeJson(doc, msg);
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

#endif