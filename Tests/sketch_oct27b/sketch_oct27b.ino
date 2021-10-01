/*
  DHCP-based IP printer

  This sketch uses the DHCP extensions to the Ethernet library
  to get an IP address via DHCP and print the address obtained.
  using an Arduino Wiznet Ethernet shield.

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13

  created 12 April 2011
  modified 9 Apr 2012
  by Tom Igoe
  modified 02 Sept 2015
  by Arturo Guadalupi

*/

#include <SPI.h>
#include <Ethernet.h>
#include <SSLClient.h>
#include <PubSubClient.h>
#include "certificates.h" // This file must be regenerated
#include <ArduinoJson.h> //Download latest ArduinoJson 6 Version library only by Benoît Blanchon from Manage Libraries

#define THING_NAME "Mini_SSS3"
#define MQTT_PACKET_SIZE  1024

void MQTTPublish(const char *topic, char *payload);
void updateThing();

const char my_cert[] = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWjCCAkKgAwIBAgIVAO5D2K7k+NneF6pUELVsyZbcHbxcMA0GCSqGSIb3DQEB\n" \
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMDEwMjIwMjA3\n" \
"MDBaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCoxACteyh3hXb/y08Y\n" \
"KhrjZG/J9l1w9ozpOPBHWhTDLocOXeGt0LKVTvFnwacxFtY0OXRNhN/d5u8J1U81\n" \
"tla/E7x09e2lvK2ftTv0d79wOl4+e/Ljf3kYErun0Fy+LC464i/pGiQjNgReIou5\n" \
"LdyuGHgnYDLmcbb+lHq6M5NdhJfJDeqp9k6r2tD00Cuv6Sv990L6LEco33333Yub\n" \
"A3COfBlQvo4z6SUK9HjzGM4xx/PYRytznKul/Flzpa+tt5PmGAl3b2a0exHkbbmH\n" \
"AvzPy2IaCOrC3fv7oHEkmQjcCkN+IDDiPq7gCg+WLRNEcRlHP1OzwsVj1Sa8WejV\n" \
"DBlDAgMBAAGjYDBeMB8GA1UdIwQYMBaAFLWOb5PCfihTBhLQXCn5bhR3xOMHMB0G\n" \
"A1UdDgQWBBQQbJP45vsA21CoccegE+5Sa5yzSTAMBgNVHRMBAf8EAjAAMA4GA1Ud\n" \
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAVOmhhXpU1X7Iw+mYLqOlEGoD\n" \
"RSmnUv7Q8vrZZUqM+QZBZFFGbSgXqlc48WoCw+oFYPedFw8v+Xw3TnvBo4CRzrZW\n" \
"FRPlK1XZNlyYZb32L8eqF4GqY8bkn0im8GnFh44WUW9wAfmRub/1AxWBYvbjBTCQ\n" \
"EF632c8hfO2L5dHMtL2av38vd3nJS/1DRaldfbcR7LPIP/HVh6257Jyn1K4KNS6h\n" \
"/vxBBakuPlmmCap3bSjvn7d/ugp5KoKqYtG69OUdRjoZ5EodJxF9ZHwpLVDxXyEL\n" \
"Ch2DpnKgKgaAkjMDfHlAVA9chx0vDORhbBp4BDuK6O8qpHArxS6tL0pKrPt8VA==\n" \
"-----END CERTIFICATE-----\n";

const char my_key[] = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEogIBAAKCAQEAqMQArXsod4V2/8tPGCoa42RvyfZdcPaM6TjwR1oUwy6HDl3h\n" \
"rdCylU7xZ8GnMRbWNDl0TYTf3ebvCdVPNbZWvxO8dPXtpbytn7U79He/cDpePnvy\n" \
"4395GBK7p9BcviwuOuIv6RokIzYEXiKLuS3crhh4J2Ay5nG2/pR6ujOTXYSXyQ3q\n" \
"qfZOq9rQ9NArr+kr/fdC+ixHKN99992LmwNwjnwZUL6OM+klCvR48xjOMcfz2Ecr\n" \
"c5yrpfxZc6WvrbeT5hgJd29mtHsR5G25hwL8z8tiGgjqwt37+6BxJJkI3ApDfiAw\n" \
"4j6u4AoPli0TRHEZRz9Ts8LFY9UmvFno1QwZQwIDAQABAoIBAFSN1xUT5mNzn/pq\n" \
"n1cpCU2ybXvPjmNhc5oJ6ACntk1QC2R8Tuyp6j96TOLCGIU0vt9thcKs9wiTgmH3\n" \
"2Ha9w1p6hepHqOhnzsl8oOxL7sNIyx0dNsUagc443IIol77EHelsl7p/WWm2z5tm\n" \
"TYDa5HzpT5HS1c21XYzRUWfnHUc1rf7yt3xjPDcbfCxzFSm8r4qy3q4rco0f4dk3\n" \
"4s9d1SoqX/BDiA8F6xXmBsLVagLRYDTeUfTxTBWXrEeRndWGGwOrMyvt6np54UWU\n" \
"hZHYPELRuQeg2YqAF0YoGTUWhanH7RhC1WsAxyfKzqDueXvb9TiABkx2bz1ycGuY\n" \
"1ghGb1kCgYEA1qwD+uirpT3YkJW4wT7hAUu9K97/yaMtbSubLL/vixwLOcc3pLHj\n" \
"rb3Ry7KgTPPQdbCNuOk1TbSeQjhzQKYIYJXHHji/fhpom66oOdESaYDVJ11SVf0z\n" \
"iqdEm+mQubPfxkyJLQdPjz+yr7O/yCfj0dgcnPrcIH9oYWiopKY2bd0CgYEAyUGF\n" \
"akD2ZFV9rzM130Y83o1QlpJf1zOcKlCzk4r+h5GVfik4Yffb1OCFTk5y1oTdm6fL\n" \
"W4PuP0miX77BxzFBj6Byi/ZNnzuxJacEol0Re98ezc0IR6tS95GZCHfXBXQjixPy\n" \
"rIT5ucKFjt5bN74F4CZIW2u3lB9whaD2TskQAZ8CgYBmahZUA+CvqrvA7sz/1KG/\n" \
"3oRXg8IATosDhEQ5qy/669wnKnXPata2MtL+HTAAxmjzXNGgIBJACiVeblP9ybhB\n" \
"a/Nc5bHl1W+GpmK3gpkIIGCQsSLwxyVpxrcisuFK1xexIR5ojw5Txw3WaSjbsrwr\n" \
"hrDP65ZNIruHjUi3lFxvOQKBgB3e0q1tjORhZ/Eb+a/03c3uTXq7qOvo5hHFvpF0\n" \
"fMikS/rYpVytC/yxgjeYHyVFvRqAMgeYMkFEbYwmZN5qqh9mIfoyCV1L38QF4QM0\n" \
"Kwcmm4zSFJuGix7hhkQifVB651eaE1eVuPbgLzCAQiuo0XhuVijzeODMNcgvlr8k\n" \
"ynJjAoGAIkqC8fifUy7BWziJ5jAIhv1FdAuwPP2/hiReK3p0JEW7utfG5ulx43s7\n" \
"sdJeogZJroaAEVB7iPXz34h4MoHYemjLQZuogWA/trjDp9Kk4Lyc90ov89jSnJaw\n" \
"KjZ+P46mh0k1FO8NmyeFtoT5+z+OnPwUID4FAqqjVVAVQRHHRaE=\n" \
"-----END RSA PRIVATE KEY-----\n";

SSLClientParameters mTLS = SSLClientParameters::fromPEM(my_cert, sizeof my_cert, my_key, sizeof my_key);

const char* mqttServer = "aiz9usfggyt4s-ats.iot.us-east-1.amazonaws.com";
const char publishShadowUpdate[] = "$aws/things/" THING_NAME "/shadow/update";
char publishPayload[MQTT_PACKET_SIZE];
char *subscribeTopic[5] =
{
  "$aws/things/" THING_NAME "/shadow/update/accepted",
  "$aws/things/" THING_NAME "/shadow/update/rejected",
  "$aws/things/" THING_NAME "/shadow/update/delta",
  "$aws/things/" THING_NAME "/shadow/get/accepted",
  "$aws/things/" THING_NAME "/shadow/get/rejected"
};

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


EthernetClient ethClient;
SSLClient ethClientSSL(ethClient, TAs, (size_t)TAs_NUM, A5);
PubSubClient mqtt(mqttServer, 8883, callback, ethClientSSL);

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

void reconnect() 
{
  while (!mqtt.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect("arduinoClient")) 
    {
      Serial.println("connected");
      for (int i = 0; i < 5; i++) 
      {
//        Serial.println(subscribeTopic[i]);
        mqtt.subscribe(subscribeTopic[i]);
        ethClientSSL.flush();
      }
        Serial.println("Started updateThing ");
        updateThing();
        Serial.println("Done updateThing ");

    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(14);  // Most Arduino shields

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 ethClientSSL.setMutualAuthParams(mTLS);
  mqtt.setBufferSize(MQTT_PACKET_SIZE);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
    
}

void loop() {
  if (!mqtt.connected()) 
  {
//    Serial.println("Reconnect ");
    reconnect();
  }
  mqtt.loop();
//  Serial.println("in Loop");
}


void updateThing()
{
  strcpy(publishPayload, "{\"state\": {\"reported\": {\"powerState\":\"ON\"}}}");
// Serial.print("Payload: ");
// Serial.println(publishPayload);
  MQTTPublish(publishShadowUpdate, publishPayload);
//   Serial.print("END Published");

}

void MQTTPublish(const char *topic, char *payload)
{
  mqtt.publish(topic, payload);
  ethClientSSL.flush();
  Serial.printf("Published [%s] %s\r\n", topic, payload);  
}
