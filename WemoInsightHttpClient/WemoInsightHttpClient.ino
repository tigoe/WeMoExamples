/*
  WeMo Insight HTTP Client
  Reads parameters from a Wemo Insight.
  Details of the Insight's parameters are not easy to find on Belkin's site,
  but see https://www.eclipse.org/smarthome/documentation/features/bindings/wemo/readme.html

  Based on WemoHttpClient from Making Things Talk, 3rd edition
  https://github.com/tigoe/MakingThingsTalk2/tree/master/3rd_edition/chapter9/WemoHttpClient

  Circuit:
  * None, but requires a Belkin Wemo Insight

  created 1 Feb 2019
  by Tom Igoe
*/

// include required libraries and config files
#include <SPI.h>
#include <WiFi101.h>          // use this for the MKR 1000
//#include <WiFiNINA.h>       // use this for the MKR 1010
//#include <ESP8266WiFi.h>    // use this for ESP8266 modules
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

WiFiClient netSocket;               // network socket to device
const char wemo[] = "192.168.0.14"; // device address; fill in yours
const int port = 49153;                   // port number
String route = "/upnp/control/insight1";  // API route
String soap;                        // string for the SOAP request

void setup() {
  Serial.begin(9600);               // initialize serial communication
  // while you're not connected to a WiFi AP,
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(SECRET_SSID);
    WiFi.begin(SECRET_SSID, SECRET_PASS);     // try to connect
    delay(2000);
  }

  // When you're connected, print out the device's network status:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // set up the SOAP request string. This formatting is just
  // for readability of the code:
  soap = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
  soap += "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"";
  soap += "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">";
  soap += "<s:Body>";
  soap += "<u:GetInsightParams xmlns:u=\"urn:Belkin:service:insight:1\"></u:GetInsightParams>";
  soap += "</s:Body></s:Envelope>";
}

void loop() {
  HttpClient http(netSocket, wemo, port); // make an HTTP client
  http.connectionKeepAlive();             // keep the connection alive
  http.beginRequest();                    // start assembling the request
  http.post(route);                       // set the route
  // add the headers:
  http.sendHeader("Content-type", "text/xml; charset=utf-8");
  String soapAction = "\"urn:Belkin:service:insight:1#GetInsightParams\"";
  http.sendHeader("SOAPACTION", soapAction);
  http.sendHeader("Connection: keep-alive");
  http.sendHeader("Content-Length", soap.length());
  http.endRequest();                      // end the request
  http.println(soap);                     // add the body
  Serial.println("request sent");

  while (http.connected()) {       // while connected to the server,
    if (http.available()) {        // if there is a response from the server,
      // read until you find <InsightParams>:
      if (http.find("<InsightParams>")) {
        // see https://www.eclipse.org/smarthome/documentation/features/bindings/wemo/readme.html
        // for a listing of the parameters.

        // to see the unparsed string, uncomment this instead of parseResults
        //Serial.println(http.readString());
        parseResults(http);
      }
    }
  }
  Serial.println();             // end of the response
  delay(5000);                  // wait 5 seconds
}

void parseResults(HttpClient response) {
  // to see the result parsed, use these:
  int onOff = response.parseInt();
  long lastChangedAt =  response.parseInt();
  int lastOnFor =  response.parseInt();
  int onToday =  response.parseInt() ;
  int onTotal =  response.parseInt();
  int timespan =  response.parseInt();
  int averagePower =  response.parseInt();
  int currentPower =  response.parseInt();
  int energyToday =  response.parseInt();
  long energyTotal =  response.parseInt();  // the only one that's a float
  int standbyLimit =  response.parseInt();

  // make a string to print the results:
  String results = "on/Off State: " + String(onOff);
  results += "\nlastChangedAt: " + String(lastChangedAt);
  results += "\nlastOnFor: " + String(lastOnFor);
  results += " sec\nonToday: " + String(onToday);
  results += " sec\nonTotal: " + String(onTotal);
  results += " sec\ntimespan: " + String(timespan);
  results += " sec\naveragePower: " + String(averagePower);
  results += " W\ncurrentPower: " + String(currentPower);
  results += " mW\nenergyToday: " + String(energyToday);
  results += " Wh\nenergyTotal: " + String(energyTotal);
  results += " Wh\nstandbyLimit: " + String(standbyLimit);
  results += " mW\n";
  Serial.println(results);
}
