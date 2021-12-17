#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>

#define LED_PIN1 2
#define LED_PIN2 0
#define NUM_LEDS1 36
#define NUM_LEDS2 0
#define LED_TYPE WS2812B

#define Day 0
#define Hour 1
#define Minute 2
#define Second 3

const char SSID[] = "Klaffenboeck_Wifi";
const char PWD[] = "31862025034211431218";
const long utcOffsetInSeconds = 3600;

IPAddress local_IP(192, 168, 188, 130);
IPAddress gateway(192, 168, 188, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 188, 103);


ESP8266WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

StaticJsonDocument<384> jsonDocument;
char buffer[384];

Adafruit_NeoPixel Strip_1(NUM_LEDS1, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel Strip_2(NUM_LEDS2, LED_PIN2, NEO_GRB + NEO_KHZ800);

bool manualyTurnedOn = false;

struct LedStrip
{
    byte red = 0;
    byte green = 0;
    byte blue = 0;
    byte mode = 0;
    byte speed = 0;
    bool state = false;
} Strip_1_data, Strip_2_data;

struct DevideData
{
    IPAddress ipAddress;
    byte sysTime[4] = {0, 0, 0, 0};
    byte timeStart[4] = {0, 0, 0, 0};
    byte timeStop[4] = {0, 0, 0, 0};
} Device;

void connectToWiFi()
{
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS))
    {
        Serial.println("STA Failed to configure");
    }

    Serial.print("Connecting to ");
    Serial.println(SSID);

    WiFi.begin(SSID, PWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void add_json_object(char *tag, byte value)
{
    jsonDocument[tag] = value;
}
void add_json_object_Array(char *tag, byte sice, byte *value)
{
    JsonArray arr = jsonDocument.createNestedArray(tag);

    for (byte i = 0; i < sice; i++)
    {
        arr.add(value[i]);
    }
}

void setStrip1()
{
    String body = server.arg("plain");
    deserializeJson(jsonDocument, body);
    // Get RGB components
    if (jsonDocument.containsKey("red"))
    {
        Strip_1_data.red = jsonDocument["red"];
    }
    if (jsonDocument.containsKey("green"))
    {
        Strip_1_data.green = jsonDocument["green"];
    }
    if (jsonDocument.containsKey("blue"))
    {
        Strip_1_data.blue = jsonDocument["blue"];
    }
    if (jsonDocument.containsKey("mode"))
    {
        Strip_1_data.mode = jsonDocument["mode"];
    }
    if (jsonDocument.containsKey("speed"))
    {
        Strip_1_data.speed = jsonDocument["speed"];
    }
    if (jsonDocument.containsKey("state"))
    {
        Strip_1_data.state = jsonDocument["state"];
        if (Strip_1_data.state)
        {
            manualyTurnedOn = true;
        }
    }
    // Respond to the client
    server.send(200, "application/json", "{}");
}
void setStrip2()
{
    String body = server.arg("plain");
    deserializeJson(jsonDocument, body);
    // Get RGB components
    if (jsonDocument.containsKey("red"))
    {
        Strip_2_data.red = jsonDocument["red"];
    }
    if (jsonDocument.containsKey("green"))
    {
        Strip_2_data.green = jsonDocument["green"];
    }
    if (jsonDocument.containsKey("blue"))
    {
        Strip_2_data.blue = jsonDocument["blue"];
    }
    if (jsonDocument.containsKey("mode"))
    {
        Strip_2_data.mode = jsonDocument["mode"];
    }
    if (jsonDocument.containsKey("speed"))
    {
        Strip_2_data.speed = jsonDocument["speed"];
    }
    if (jsonDocument.containsKey("state"))
    {
        Strip_2_data.state = jsonDocument["state"];
        if (Strip_2_data.state)
        {
            manualyTurnedOn = true;
        }
    }
    // Respond to the client
    server.send(200, "application/json", "{}");
}
void setDevideData()
{
    String body = server.arg("plain");
    deserializeJson(jsonDocument, body);

    Device.timeStart[Hour] = jsonDocument["TimeStart"][0];
    Device.timeStart[Minute] = jsonDocument["TimeStart"][1];
    Device.timeStart[Second] = jsonDocument["TimeStart"][2];
    Device.timeStop[Hour] = jsonDocument["TimeStop"][0];
    Device.timeStop[Minute] = jsonDocument["TimeStop"][1];
    Device.timeStop[Second] = jsonDocument["TimeStop"][2];
    // Respond to the client
    server.send(200, "application/json", "{}");
}

void getStrip1()
{
    jsonDocument.clear();
    add_json_object("red", Strip_1_data.red);
    add_json_object("green", Strip_1_data.green);
    add_json_object("blue", Strip_1_data.blue);
    add_json_object("mode", Strip_1_data.mode);
    add_json_object("speed", Strip_1_data.speed);
    add_json_object("state", Strip_1_data.state);
    serializeJson(jsonDocument, buffer);
    server.send(200, "application/json", buffer);
}
void getStrip2()
{
    jsonDocument.clear();
    add_json_object("red", Strip_2_data.red);
    add_json_object("green", Strip_2_data.green);
    add_json_object("blue", Strip_2_data.blue);
    add_json_object("mode", Strip_2_data.mode);
    add_json_object("speed", Strip_2_data.speed);
    add_json_object("state", Strip_2_data.state);
    serializeJson(jsonDocument, buffer);
    server.send(200, "application/json", buffer);
}
void getDevideData()
{
    jsonDocument.clear();
    jsonDocument["IP"] = WiFi.localIP();
    jsonDocument["WifiSSID"] = SSID;
    jsonDocument["WifiPWD"] = PWD;

    add_json_object_Array("SysTime", 4, Device.sysTime);
    add_json_object_Array("TimeStart", 4, Device.timeStart);
    add_json_object_Array("TimeStop", 4, Device.timeStop);
    serializeJson(jsonDocument, buffer);
    server.send(200, "application/json", buffer);
}

void setup_routing()
{
    server.on("/Strip1", HTTP_POST, setStrip1);
    server.on("/Strip2", HTTP_POST, setStrip2);
    server.on("/DevideData", HTTP_POST, setDevideData);
    server.on("/Strip1", HTTP_GET, getStrip1);
    server.on("/Strip2", HTTP_GET, getStrip2);
    server.on("/DevideData", HTTP_GET, getDevideData);
    server.begin();
}

void handleLEDs()
{
    if (Strip_1_data.state)
    {
        Strip_1.fill(Strip_1.Color(Strip_1_data.red, Strip_1_data.green, Strip_1_data.blue));
        Strip_1.show();
    }
    else
    {
        Strip_1.fill(Strip_1.Color(0, 0, 0));
        Strip_1.show();
    }

    if (Strip_2_data.state)
    {
        Strip_2.fill(Strip_2.Color(Strip_2_data.red, Strip_2_data.green, Strip_2_data.blue));
        Strip_2.show();
    }
    else
    {
        Strip_2.fill(Strip_2.Color(0, 0, 0));
        Strip_2.show();
    }
}

void handleAutomaticTurnOnOf()
{
    bool OnEnable = (Device.sysTime[Hour] >= Device.timeStart[Hour]) && (Device.sysTime[Minute] >= Device.timeStart[Minute]) && (Device.sysTime[Second] >= Device.timeStart[Second]);
    bool OffEnable = (Device.sysTime[Hour] >= Device.timeStop[Hour]) && (Device.sysTime[Minute] >= Device.timeStop[Minute]) && (Device.sysTime[Second] >= Device.timeStop[Second]);

    if (OnEnable && !OffEnable)
    {
        Strip_1_data.state = true;
        Strip_2_data.state = true;
        manualyTurnedOn = false;
    }

    if (OffEnable && !manualyTurnedOn)
    {
        Strip_1_data.state = false;
        Strip_2_data.state = false;
    }
}

void loadEEprom()
{
    // TODO Load wifi data from eeprom
    // TODO Load automatic start and stop time
}
void writeEEprom()
{
    // TODO Write wifi data to eeprom
    // TODO Write automatic start and stop time
}

void setup()
{
    Serial.begin(9600);
    connectToWiFi();
    setup_routing();

    ArduinoOTA.begin();

    Strip_1.begin();
    Strip_2.begin();
    Strip_1.fill(Strip_1.Color(0, 0, 0));
    Strip_1.show();
    Strip_2.fill(Strip_2.Color(0, 0, 0));
    Strip_2.show();
}

void loop()
{
    server.handleClient();
    ArduinoOTA.handle();
    timeClient.update();
    // handleAutomaticTurnOnOf();
    handleLEDs();
    Device.sysTime[Day] = timeClient.getDay();
    Device.sysTime[Hour] = timeClient.getHours();
    Device.sysTime[Minute] = timeClient.getMinutes();
    Device.sysTime[Second] = timeClient.getSeconds();
}