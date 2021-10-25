#define ARDUINO_ETHERNET_SHIELD
#define BOARD_HAS_ECCX08
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
}


void onMessageReceived(int messageSize)
{
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available())
  {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();
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
    // #TODO: implement printf_debug
    if (DEBUG)
      Serial.print(F("deserializeJson() failed: "));
    if (DEBUG)
      Serial.println(error.f_str());
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
    ignitionCtlState = doc["ledOn"];
    redLEDstate = doc["ledOn"];
    digitalWrite(redLEDpin, redLEDstate);           // Remove Later
    digitalWrite(ignitionCtlPin, ignitionCtlState); // Remove Later

    // commandPrefix = "50";
    // if (ignitionCtlState)
    //   commandString = "1";
    // else
    //   commandString = "0";
    // fastSetSetting();
    return read_keySw(req, res);
  }
  // return read_keySw(req, res);
}

DynamicJsonDocument getStatus_pots()
{
  DynamicJsonDocument response(2048);
  response["pot1"]["wiper"]["value"] = SPIpotWiperSettings[0];
  response["pot1"]["sw"]["value"] = 0;
  response["pot1"]["sw"]["meta"] = "TBD";

  response["pot2"]["wiper"]["value"] = SPIpotWiperSettings[1];
  response["pot2"]["sw"]["value"] = 0;
  response["pot2"]["sw"]["meta"] = "TBD";

  response["pot3"]["wiper"]["value"] = SPIpotWiperSettings[2];
  response["pot3"]["sw"]["value"] = 0;
  response["pot3"]["sw"]["meta"] = "TBD";

  response["pot4"]["wiper"]["value"] = SPIpotWiperSettings[3];
  response["pot4"]["sw"]["value"] = 0;
  response["pot4"]["sw"]["meta"] = "TBD";
  return response;
}
void read_potentiometers(Request &req, Response &res)
{
  // DynamicJsonDocument response(2048);
  char json[2048];
  Debug.print(DBG_INFO,"Got GET Request for Potentiometers, returned: ");
  serializeJsonPretty(getStatus_pots(), json);
  Debug.print(DBG_DEBUG,json);
  res.print(json);
}

void publishMessage()
{
  Serial.println("Publishing message");

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage("arduino/outgoing");
  // mqttClient.print("Hello from Mini SSS3 ");
  // mqttClient.print(millis());

  serializeJson(getStatus_pots(), mqttClient);
  mqttClient.endMessage();
}


int digitalPotWrite(int value, int CS)
{
  byte address = 0x00;
  digitalWrite(CS, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(CS, HIGH);
}

void update_potentiometers(Request &req, Response &res)
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
    if (doc["pot1"])
    {
      SPIpotWiperSettings[0] = doc["pot1"]["wiper"]["value"];
      digitalPotWrite(SPIpotWiperSettings[0], CS_U1);
    }
    if (doc["pot2"])
    {
      SPIpotWiperSettings[1] = doc["pot2"]["wiper"]["value"];
      digitalPotWrite(SPIpotWiperSettings[1], CS_U2);
    }
    if (doc["pot3"])
    {
      SPIpotWiperSettings[2] = doc["pot3"]["wiper"]["value"];
      digitalPotWrite(SPIpotWiperSettings[2], CS_U3);
    }
    if (doc["pot4"])
    {
      SPIpotWiperSettings[3] = doc["pot4"]["wiper"]["value"];
      digitalPotWrite(SPIpotWiperSettings[3], CS_U4);
    }
    return read_potentiometers(req, res);
  }
}

void read_pac1934(Request &req, Response &res)
{
  if (DEBUG)
    Serial.print("Got GET Request for PAC1934: ");
  DynamicJsonDocument response(2048);
  char json[2048];
  if (DEBUG)
    Serial.print("Got GET Request for Potentiometers, returned: ");
  PAC.UpdateVoltage();
  float temp = PAC.Voltage1;
  response["pot1"]["voltage"] = PAC.Voltage1 / 1000;
  response["pot1"]["current"] = -1;
  response["pot2"]["voltage"] = PAC.Voltage2 / 1000;
  response["pot2"]["current"] = -1;
  response["pot3"]["voltage"] = PAC.Voltage4 / 1000;
  response["pot3"]["current"] = -1;
  response["pot4"]["voltage"] = PAC.Voltage3 / 1000;
  response["pot4"]["current"] = -1;

  serializeJsonPretty(response, json);
  if (DEBUG)
    Serial.println(json);
  res.print(json);
}

void read_CAN(Request &req, Response &res)
{
  if (DEBUG)
    Serial.print("Got GET Request for Read CAN: ");
  char json[2048];
  serializeJsonPretty(can_dict, json);
  // if (DEBUG) Serial.println(json);
  res.print(json);
}

void setup()
{
  setPinModes();
  Wire.begin();
  SPI.begin();

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

   Debug.print(DBG_INFO,"MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println();
  Debug.print(DBG_INFO,"My IP address: ");
  Serial.println(Ethernet.localIP());
  ArduinoBearSSL.onGetTime(getTime);
  sslClient.setEccSlot(0, certificate);
  mqttClient.onMessage(onMessageReceived);

  app.get("/led", &read_keySw);
  app.put("/led", &update_keySw);
  app.post("/led", &update_keySw);
  app.get("/pots", &read_potentiometers);
  app.post("/pots", &update_potentiometers);
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

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  // poll for new MQTT messages and send keep alive
  mqttClient.poll();

  // publish a message roughly every 5 seconds.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();

    publishMessage();
  }

  EthernetClient client = server.available();
  // SSLClient client(base_client, TAs, (size_t)TAs_NUM, rand_pin);

  // WriteLoggingStream loggingClient(client, Serial);
  // loggingClient.println("GET / HTTP/1.1");
  // loggingClient.println("User-Agent: Arduino");

  // deserializeJson(doc, loggingStream);
  // loggingStream.write("test");
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
