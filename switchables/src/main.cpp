#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <PubSubClient.h>

#define XSTR(x) #x
#define STR(x) XSTR(x)

const char *ssid = STR(SSID);
const char *password = STR(PASS);
const char *mqttServer = STR(MQTT_SERVER);

int SWITCH_PIN = LED_BUILTIN;

WiFiClient espClient;
PubSubClient client(espClient);
long timeLastMsg = 0;
char msg[50];
int value = 0;

// Topics
// Publishments
const char *pub = "home/switches/bedroom/status";

// Subscriptions
const char *sub = "home/switches/bedroom/switch";

void heartBeat();

void setupWifi()
{
  delay(5);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void toggleSwitch()
{
  if (digitalRead(SWITCH_PIN) == HIGH)
  {
    digitalWrite(SWITCH_PIN, LOW);
  }
  else
  {
    digitalWrite(SWITCH_PIN, HIGH);
  }
  heartBeat();
}

void handleMqtt(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, sub) == 0)
  {
    toggleSwitch();
  }
}

void heartBeat()
{
  int currentState = digitalRead(SWITCH_PIN);
  snprintf(msg, 50, "%c", currentState == LOW ? '1' : '0');
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(pub, msg);
}

void setup()
{
  setupWifi();
  Serial.begin(115200);
  pinMode(SWITCH_PIN, OUTPUT);
  digitalWrite(SWITCH_PIN, HIGH);
  client.setServer(mqttServer, 1883);
  client.setCallback(handleMqtt);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      heartBeat();
      // ... and resubscribe
      client.subscribe(sub);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// the loop function runs over and over again forever
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  long currentTime = millis();
  if (currentTime - timeLastMsg > 60000)
  {
    timeLastMsg = currentTime;
    heartBeat();
  }
}
