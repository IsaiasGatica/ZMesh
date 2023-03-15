
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

#define STATION_SSID "GWIFI"
#define STATION_PASSWORD "independenciaGatica"

#define HOSTNAME "MQTT_Bridge"

const char *mqtt_broker = "broker.emqx.io";
const char *topic = "esp32/message";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Prototypes
void receivedCallback(const uint32_t &from, const String &msg);
void MQTTcallback(char *topic, byte *payload, unsigned int length);

painlessMesh mesh;

void setup()
{

  Serial.begin(115200);
  WiFi.begin(STATION_SSID, STATION_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network!");

  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(MQTTcallback);
  while (!mqttClient.connected())
  {
    String client_id = String(WiFi.macAddress());

    if (mqttClient.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Connected to Public MQTT Broker");
    }
    else
    {
      Serial.print("Failed to connect with MQTT Broker");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }

  mqttClient.publish(topic, "Hello! I am ESP32");
  mqttClient.subscribe(topic);

  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&receivedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
}

void loop()
{

  mqttClient.loop();
  mesh.update();
}

void receivedCallback(const uint32_t &from, const String &msg)
{
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
}

void MQTTcallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("************");
}
