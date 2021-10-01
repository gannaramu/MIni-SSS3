#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "aWOT.h"
#include "StaticFiles.h"
#include <ArduinoJson.h>
#include "Mini_SSS3_board_defs_rev_2.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
    0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x04};
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);
Application app;
uint8_t buff[2048];
char buff_c[2048];
  StaticJsonDocument<200> doc;

bool parse_response(uint8_t *buffer)
{
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

void setup()
{

  setPinModes();
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
  app.route(staticFiles());
}

void loop()
{
 EthernetClient client  = server.available();
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
