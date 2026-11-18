/*
 * ESP32-S3: hosts its own Wi-Fi called "Hiiiii" and shows a page that says hiiiii.
 * Join the Wi-Fi (no password); the page opens by itself, or go to http://192.168.4.1
 * Board: ESP32S3 Dev Module. No extra libraries needed.
 */
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

WebServer server(80);
DNSServer dns;

const char PAGE[] = R"(<!DOCTYPE html><html><head><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1"><title>hiiiii</title></head>
<body style="margin:0;height:100vh;display:flex;align-items:center;justify-content:center;background:#111;color:#fff;font:bold 18vw system-ui,sans-serif">hiiiii</body></html>)";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Hiiiii");                                  // open network, no password
  dns.start(53, "*", WiFi.softAPIP());                    // every address leads here, so the page pops up
  server.onNotFound([]() { server.send(200, "text/html", PAGE); });
  server.begin();
  Serial.printf("Join the Wi-Fi \"Hiiiii\", then open http://%s\n", WiFi.softAPIP().toString().c_str());
}

void loop() {
  dns.processNextRequest();
  server.handleClient();
}
