#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "aWOT.h"
#include "StaticFiles.h"
#include <ArduinoJson.h>
#include "Mini_SSS3_board_defs_rev_2.h"
#include <Microchip_PAC193x.h>
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
    0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x04};
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);
Application app;
uint8_t buff[2048];
char buff_c[2048];
DynamicJsonDocument doc(2048);
Microchip_PAC193x PAC;

bool parse_response(uint8_t *buffer)
{
  // Serial.print((char *)buffer);
  // Serial.println(" EOF");
  //Serial.println("parse_response");

  DeserializationError error = deserializeJson(doc, buffer);
  serializeJsonPretty(doc, Serial);

  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }

  return true;
}

void read_keySw(Request &req, Response &res)
{
  Serial.print("Got GET Request for LED returned: ");
  Serial.println(ignitionCtlState);
  res.print(ignitionCtlState);
}

void update_keySw(Request &req, Response &res)
{

  // JsonObject& config = jb.parseObject( &req);
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

void read_potentiometers(Request &req, Response &res)
{
  DynamicJsonDocument response(2048);
  char json[2048];
  Serial.print("Got GET Request for Potentiometers, returned: ");
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

  // serializeJson(response, json);
  serializeJsonPretty(response, json);
  Serial.println(json);
  res.print(json);
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

void read_pac1934(Request &req, Response &res){
  Serial.print("Got GET Request for PAC1934: ");
  DynamicJsonDocument response(2048);
  char json[2048];
  Serial.print("Got GET Request for Potentiometers, returned: ");
  PAC.UpdateVoltage();
  float temp = PAC.Voltage1;
  response["pot1"]["voltage"] = PAC.Voltage1/1000;
  response["pot1"]["current"] = -1;
  response["pot2"]["voltage"] = PAC.Voltage2/1000;
  response["pot2"]["current"] = -1;
  response["pot3"]["voltage"] = PAC.Voltage4/1000;
  response["pot3"]["current"] = -1;
  response["pot4"]["voltage"] = PAC.Voltage3/1000;
  response["pot4"]["current"] = -1;


  serializeJsonPretty(response, json);
  Serial.println(json);
  res.print(json);
}

void setup()
{
  setPinModes();
  Wire.begin();

  SPI.begin();
  Ethernet.init(14); // Most Arduino shields

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true)
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Ethernet cable is not connected.");
  }
  // print your local IP address:
  Serial.print("My IP address asdasd: ");
  Serial.println(Ethernet.localIP());
  app.get("/led", &read_keySw);
  app.put("/led", &update_keySw);
  app.post("/led", &update_keySw);
  app.get("/pots", &read_potentiometers);
  app.post("/pots", &update_potentiometers);
  app.get("/voltage", &read_pac1934);
  app.route(staticFiles());
  server.begin();
}

void loop()
{
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
  
}
