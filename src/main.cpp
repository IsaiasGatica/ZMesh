/*
  Nodo Colector / Nodo Sensor
*/

#include "painlessMesh.h"
#include <ArduinoJson.h>

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

#define NodoSensor
// #define NodoColector

#define RXD2 16
#define TXD2 17

std::list<uint32_t> nodes;

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

void sendMessage(); // Prototype so PlatformIO doesn't complain

#ifdef NodoSensor
Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);
#endif

void receivedCallback(uint32_t from, String &msg)
{
  Serial.println(msg);

#ifdef NodoColector
  Serial2.println(msg);
#endif
}

void newConnectionCallback(uint32_t nodeId)
{
#ifdef NodoColector

  // String msg;
  // StaticJsonDocument<200> doc;
  // doc["Tipo"] = "Alerta";
  // doc["Mensaje"] = "Nueva conexión";
  // doc["Valor"] = nodeId;
  // serializeJson(doc, msg);
  // Serial2.println(msg);

#endif
}

void changedConnectionCallback()
{
#ifdef NodoColector
  String msg;
  StaticJsonDocument<100> doc;
  doc["Tipo"] = "Alerta";
  doc["Mensaje"] = "Cambio en la topología";
  JsonArray data = doc.createNestedArray("Valor");

  nodes = mesh.getNodeList();
  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end())
  {

    data.add(*node);
    node++;
  }

  serializeJson(doc, Serial2);

#endif
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  // Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup()
{
  Serial.begin(115200);
#ifdef NodoColector
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
#endif

  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
  mesh.setContainsRoot(true);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);

#ifdef NodoColector
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
#endif

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
  doc["Tipo"] = "Nodo";
  doc["Mensaje"] = "4";
  doc["Valor"] = random(100);
  serializeJson(doc, msg);
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 5, TASK_SECOND * 10));
}

#endif