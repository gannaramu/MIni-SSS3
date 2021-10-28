#define ARDUINO_ETHERNET_SHIELD
#define BOARD_HAS_ECCX08
#define ARDUINOJSON_ENABLE_PROGMEM 0

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <aWOT.h>
#include <StaticFiles.h>
#include <ArduinoJson.h>
#include "Mini_SSS3_board_defs_rev_2.h"
#include <Microchip_PAC193x.h>
#include <FlexCAN_T4.h>
#include "CAN_Message_Threads.h"
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <arduino_secrets.h>
#include <TimeLib.h>
#include "Arduino_DebugUtils.h"

time_t RTCTime;

// extern void reloadCAN();
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
void publishMessage();
void publishPots();
IPAddress ip(192, 168, 1, 177);

EthernetServer server(80);
EthernetClient client;
BearSSLClient sslClient(client); // Used for SSL/TLS connection, integrates with ECC508
MqttClient mqttClient(sslClient);
const char broker[] = SECRET_BROKER;
const char *certificate = SECRET_CERTIFICATE;
unsigned long lastMillis = 0;

Application app;
uint8_t buff[2048];
char buff_c[2048];
DynamicJsonDocument doc(2048);

Microchip_PAC193x PAC;

CAN_message_t msg;
StaticJsonDocument<2048> can_dict;
// bool DEBUG = true;

// CAN Threads

// extern ThreadController can_thread_controller;

// extern CanThread* can_messages;

/* AWS IOT related Functions */
extern "C"
{
  // This must exist to keep the linker happy but is never called.
  int _gettimeofday(struct timeval *tv, void *tzvp)
  {
    Serial.println("_gettimeofday dummy");
    uint64_t t = Teensy3Clock.get();
    ;                                      // get uptime in nanoseconds
    tv->tv_sec = t / 1000000000;           // convert to seconds
    tv->tv_usec = (t % 1000000000) / 1000; // get remaining microseconds
    return 0;                              // return non-zero for error
  }                                        // end _gettimeofday()
}

unsigned long getTime()
{
  return Teensy3Clock.get();
}

bool parse_response(uint8_t *buffer)
{
  // Serial.print((char *)buffer);
  // Serial.println(" EOF");
  //Serial.println("parse_response");

  DeserializationError error = deserializeJson(doc, buffer);
  serializeJsonPretty(doc, Serial);

  if (error)
  {
    Debug.print(DBG_ERROR, "deserializeJson() failed: ");
    // Serial.println(error.f_str());
    return false;
  }

  return true;
}

void read_keySw(Request &req, Response &res)
{
  if (DEBUG)
    Serial.print("Got GET Request for LED returned: ");
  if (DEBUG)
    Serial.println(ignitionCtlState);
  res.print(ignitionCtlState);
}

void set_keySw(bool value)
{
  ignitionCtlState = value;
  redLEDstate = value;

  digitalWrite(redLEDpin, redLEDstate);           // Remove Later
  digitalWrite(ignitionCtlPin, ignitionCtlState); // Remove Later
}

void update_keySw(Request &req, Response &res)
{

  // JsonObject& config = jb.parseObject( &req);
  if (DEBUG)
    Serial.print("Got POST Request for LED: ");
  req.body(buff, sizeof(buff));
  if (!parse_response(buff))
  {
    res.print("Not a valid Json Format");
  }
  else
  {
    bool state = doc["ledOn"];
    set_keySw(state);

    // commandPrefix = "50";
    // if (ignitionCtlState)
    //   commandString = "1";
    // else
    //   commandString = "0";
    // fastSetSetting();
    mqttClient.beginMessage("$aws/things/mini-sss3-1/shadow/update");
    StaticJsonDocument<2048> update;
    update["state"]["reported"]["KeyOn"]["value"] = state;
    serializeJson(update, mqttClient);
    mqttClient.endMessage();
    return read_keySw(req, res);
  }
  // return read_keySw(req, res);
}

DynamicJsonDocument getStatus_Pots()
{
  DynamicJsonDocument response(2048);
  for (int i = 0; i < 4; i++)
  {
    response[String(i)]["wiper"]["value"] = SPIpotWiperSettings[i];
    response[String(i)]["sw"]["value"] = 0;
    response[String(i)]["sw"]["meta"] = "TBD";
  }

  return response;
}

DynamicJsonDocument getStatus_PWM()
{
  DynamicJsonDocument response(2048);
  for (int i = 0; i < numPWMs; i++)
  {
    response[String(i)]["duty"]["value"] = pwmValue[i];
    response[String(i)]["frequency"]["value"] = pwmFrequency[i];
    response[String(i)]["switch"]["value"] = 1;
  }
  return response;
}

DynamicJsonDocument getStatus_PAC()
{
  DynamicJsonDocument response(2048);
  PAC.UpdateVoltage();
  response["0"]["voltage"] = PAC.Voltage1 / 1000;
  response["0"]["current"] = -1;
  response["1"]["voltage"] = PAC.Voltage2 / 1000;
  response["1"]["current"] = -1;
  response["2"]["voltage"] = PAC.Voltage4 / 1000;
  response["2"]["current"] = -1;
  response["3"]["voltage"] = PAC.Voltage3 / 1000;
  response["3"]["current"] = -1;
  return response;
}

void read_potentiometers(Request &req, Response &res)
{
  char json[2048];
  Debug.print(DBG_INFO, "Got GET Request for Potentiometers, returned: ");
  serializeJsonPretty(getStatus_Pots(), json);
  Debug.print(DBG_DEBUG, json);
  res.print(json);
}

void read_pac1934(Request &req, Response &res)
{
  char json[2048];
  Debug.print(DBG_DEBUG, "Got GET Request for PAC1934: ");
  serializeJsonPretty(getStatus_PAC(), json);
  // Debug.print(DBG_DEBUG, json);
  res.print(json);
}

void read_PWM(Request &req, Response &res)
{
  char json[2048];
  Debug.print(DBG_DEBUG, "Got GET Request for PAC1934: ");
  serializeJsonPretty(getStatus_PWM(), json);
  Debug.print(DBG_DEBUG, json);
  res.print(json);
}

void read_CAN(Request &req, Response &res)
{
  Debug.print(DBG_DEBUG, "Got GET Request for Read CAN:");
  char json[2048];
  serializeJsonPretty(can_dict, json);
  // if (DEBUG) Serial.println(json);
  res.print(json);
}

uint8_t MCP41HVI2C_SetWiper(int pin, int potValue)
{
  digitalWrite(pin, LOW);
  SPI.transfer(0x00); //Write to wiper Register
  SPI.transfer(potValue);
  SPI.transfer(0x0C);                  //Read command
  uint8_t result = SPI.transfer(0xFF); //Read Wiper Register
  digitalWrite(pin, HIGH);
  return result;
}

uint8_t MCP41HVI2C_SetTerminals(uint8_t pin, uint8_t TCON_Value)
{
  digitalWrite(pin, LOW);
  SPI1.transfer(0x40); //Write to TCON Register
  SPI1.transfer(TCON_Value + 8);
  SPI1.transfer(0x4C);                 //Read Command
  uint8_t result = SPI.transfer(0xFF); //Read Terminal Connection (TCON) Register
  digitalWrite(pin, HIGH);

  return result & 0x07;
}

void update_Pots(int idx, int value)
{
  if (value > 256)
  {
    Debug.print(DBG_WARNING, "Value: %d is out of bound setting it to 256", value);
    value = 256;
  }
  else if (value < 0)
  {
    Debug.print(DBG_WARNING, "Value: %d is out of bound setting it to 0", value);
    value = 0;
  }

  if (idx > numSPIpots)
  {
    Debug.print(DBG_ERROR, "idx : %d is out of range of numSPIpots: %d", idx, numSPIpots);
    return;
  }
  Debug.print(DBG_INFO, "Entered update_Pots function with idx: %d, value: %d: ", idx, value);
  SPIpotWiperSettings[idx] = value;
  MCP41HVI2C_SetWiper(SPIpotCS[idx], value);

}

void on_POST_Pots(Request &req, Response &res)
{

  // JsonObject& config = jb.parseObject( &req);
  if (DEBUG)
    Serial.print("Got POST Request for Potentiometers: ");
  req.body(buff, sizeof(buff));
  if (!parse_response(buff))
  {
    res.print("Not a valid Json Format");
  }
  else
  {
    if (doc["0"])
    {
      update_Pots(0, doc["0"]["wiper"]["value"]);
    }
    if (doc["1"])
    {
      update_Pots(1, doc["1"]["wiper"]["value"]);
    }
    if (doc["2"])
    {
      update_Pots(2, doc["2"]["wiper"]["value"]);
    }
    if (doc["3"])
    {
      update_Pots(3, doc["3"]["wiper"]["value"]);
    }
    publishPots();

    return read_potentiometers(req, res);
  }
}

void on_POST_PWM(Request &req, Response &res)
{
}

void connectMQTT()
{
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883))
  {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("arduino/incoming");
  mqttClient.subscribe("update/PWM");
  mqttClient.subscribe("update/PAC");
  mqttClient.subscribe("update/Pots");
  mqttClient.subscribe("$aws/things/mini-sss3-1/shadow/get/accepted");
  mqttClient.subscribe("$aws/things/mini-sss3-1/shadow/update/accepted");

  mqttClient.beginMessage("$aws/things/mini-sss3-1/shadow/get");
  mqttClient.endMessage();
  publishMessage();
}

void onMessageReceived(int messageSize)
{
  // we received a message, print out the topic and contents
  Serial.println();
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  StaticJsonDocument<5000> onMessage;
  deserializeJson(onMessage, mqttClient);
  // serializeJsonPretty(onMessage, Serial);
  JsonObject root = onMessage.as<JsonObject>();

  for (JsonPair kv : root)
  {
    // Serial.println(kv.key().c_str());
    if (kv.key() == "state")
    {

      // serializeJsonPretty(kv.value(), Serial);
      JsonObject d_state = kv.value().as<JsonObject>();
      JsonObject r_state = d_state["reported"];
      for (JsonPair kw : r_state)
      {
        // Debug.print(DBG_DEBUG, "Entered 2nd for loop");
        // Serial.print(kw.key().c_str());
        // serializeJsonPretty(kw.value(), Serial);
        if (kw.key() == "Pots")
        {
          Debug.print(DBG_DEBUG, "Entered Pots");
          // serializeJsonPretty(kw.value(), Serial);
          JsonObject Pots = kw.value().as<JsonObject>();
          for (JsonPair kp : Pots)
          {
            JsonObject Pot = kp.value().as<JsonObject>();
            uint8_t val = Pot["wiper"]["value"];
            Debug.print(DBG_INFO, "got Wiper value: %d for pot%s", val, kp.key().c_str());
            update_Pots(atoi(kp.key().c_str()), val);
          }
        }
        else if (kw.key() == "PWM")
        {
        }
        else if (kw.key() == "KeyOn")
        {
          JsonObject KeyOn = kw.value().as<JsonObject>();
          bool val = KeyOn["value"];
          Debug.print(DBG_INFO, "AWS: got KeyOn value: %d ", val);
          set_keySw(val);
        }
      }
    }
    // Serial.println();
  }
}

void publishMessage()
{
  Debug.print(DBG_INFO, "Publishing message");

  mqttClient.beginMessage("$aws/things/mini-sss3-1/shadow/update");
  StaticJsonDocument<2048> update;
  update["state"]["reported"]["Pots"] = getStatus_Pots();
  serializeJson(update, mqttClient);
  mqttClient.endMessage();

  mqttClient.beginMessage("$aws/things/mini-sss3-1/shadow/update");
  update.clear();
  update["state"]["reported"]["PWM"] = getStatus_PWM();
  serializeJson(update, mqttClient);
  mqttClient.endMessage();

  mqttClient.beginMessage("$aws/things/mini-sss3-1/shadow/update");
  update.clear();
  update["state"]["reported"]["PAC"] = getStatus_PAC();
  serializeJson(update, mqttClient);
  mqttClient.endMessage();

  mqttClient.beginMessage("$aws/things/mini-sss3-1/shadow/update");
  update.clear();
  update["state"]["reported"]["KeyOn"]["value"] = ignitionCtlState;
  serializeJson(update, mqttClient);
  mqttClient.endMessage();
}

void publishPAC()
{
  mqttClient.beginMessage("$aws/things/mini-sss3-1/shadow/update");
  StaticJsonDocument<2048> update;
  update["state"]["reported"]["PAC"] = getStatus_PAC();
  serializeJson(update, mqttClient);
  mqttClient.endMessage();
}

void publishPots()
{
  mqttClient.beginMessage("$aws/things/mini-sss3-1/shadow/update");
  StaticJsonDocument<2048> update;
  update["state"]["reported"]["Pots"] = getStatus_Pots();
  serializeJson(update, mqttClient);
  mqttClient.endMessage();
}
void setup()
{
  setPinModes();
  Wire.begin();
  SPI.begin();
  Debug.setDebugLevel(DBG_INFO);

  // Get burned in MAC address
  byte mac[6];
  for (uint8_t by = 0; by < 2; by++)
    mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
  for (uint8_t by = 0; by < 4; by++)
    mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
  Ethernet.init(14); // Most Arduino shields

  Can1.begin();
  Can1.setBaudRate(250000);
  Can2.begin();
  Can2.setBaudRate(250000);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // start the Ethernet connection and the server:
  // Ethernet.begin(mac, ip); for server
  Ethernet.begin(mac);
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Debug.print(DBG_DEBUG, "Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true)
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF)
  {
    Debug.print(DBG_DEBUG, "Ethernet cable is not connected.");
  }

  Debug.print(DBG_INFO, "MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println();
  Debug.print(DBG_INFO, "My IP address: ");
  Serial.println(Ethernet.localIP());
  ArduinoBearSSL.onGetTime(getTime);
  sslClient.setEccSlot(0, certificate);
  mqttClient.onMessage(onMessageReceived);

  app.get("/led", &read_keySw);
  app.put("/led", &update_keySw);
  app.post("/led", &update_keySw);
  app.get("/pots", &read_potentiometers);
  app.post("/pots", &on_POST_Pots);
  app.get("/voltage", &read_pac1934);
  app.get("/can", &read_CAN);
  app.get("/cangen", &read_CAN_Gen);
  app.post("/cangen", &update_CAN_Gen);
  app.route(staticFiles());
  server.begin();
  ignitionCtlState = true;
  reloadCAN();
  delay(100);
  stopCAN();
}

void loop()
{
  EthernetClient client = server.available();

  if (!mqttClient.connected())
  {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  // poll for new MQTT messages and send keep alive
  mqttClient.poll();

  // publish a message roughly every 5 seconds.
  if (millis() - lastMillis > 5000)
  {
    lastMillis = millis();
    publishPAC();
    // publishMessage();
  }

  if (client.connected())
  {
    app.process(&client);
  }
  if (Can1.read(msg))
  {
    digitalWrite(greenLEDpin, HIGH);
    // Serial.print("CAN1 ");
    // Serial.print("MB: "); Serial.print(msg.mb);
    char CAN_ID[32];
    itoa(msg.id, CAN_ID, 16);
    // Serial.print("  ID: 0x"); Serial.println(outputString);
    bool hasID = can_dict.containsKey(CAN_ID); // true
    if (hasID)
    {
      int count = can_dict[CAN_ID]["count"];
      can_dict[CAN_ID]["count"] = count + 1;
      can_dict[CAN_ID]["LEN"] = msg.len;
      can_dict[CAN_ID]["ID"] = CAN_ID;
      for (uint8_t i = 0; i < 8; i++)
      {
        char CAN_data[4];
        // itoa( msg.buf[i], CAN_data, 16);
        sprintf(CAN_data, "%02X", msg.buf[i]);
        can_dict[CAN_ID]["DATA"][i] = CAN_data;
      }
    }
    else
    {
      can_dict[CAN_ID]["count"] = 1;
    }
    digitalWrite(greenLEDpin, LOW);
  }
  // else if ( can2.read(msg) ) {
  //   Serial.print("CAN2 ");
  //   Serial.print("MB: "); Serial.print(msg.mb);
  //   Serial.print("  ID: 0x"); Serial.print(msg.id, HEX );
  //   Serial.print("  EXT: "); Serial.print(msg.flags.extended );
  //   Serial.print("  LEN: "); Serial.print(msg.len);
  //   Serial.print(" DATA: ");
  //   for ( uint8_t i = 0; i < 8; i++ ) {
  //     Serial.print(msg.buf[i]); Serial.print(" ");
  //   }
  //   Serial.print("  TS: "); Serial.println(msg.timestamp);
  // }
}
