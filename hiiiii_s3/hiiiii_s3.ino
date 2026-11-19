/*
 * ESP32-S3 "hiiiii" v1.2: updates by radio from the MacroPad (PadOTA.h), and over Wi-Fi from a browser.
 *  - From the pad: it shows up as "hiiiii" under Wireless controllers in the pad's setup page; pick the new .ino.bin there.
 *  - Hosts its own Wi-Fi called "Hiiiii" (no password). Join it: the hiiiii page opens by itself (or http://192.168.4.1).
 *  - Update: open http://192.168.4.1/update, pick the new .ino.bin (Sketch > Export Compiled Binary, board
 *    "ESP32S3 Dev Module") and tap Update. The board checks the file, installs it and restarts.
 *    A wrong or damaged file is refused and the old code keeps running.
 * Board: ESP32S3 Dev Module. No extra libraries needed.
 */
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Update.h>
#include "PadOTA.h"

constexpr const char* VERSION = "1.2";

WebServer server(80);
DNSServer dns;

const char PAGE[] = R"HTML(<!DOCTYPE html><html><head><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1"><title>hiiiii</title></head>
<body style="margin:0;height:100vh;display:flex;flex-direction:column;align-items:center;justify-content:center;background:#111;color:#fff;font-family:system-ui,sans-serif">
<div style="font:bold 18vw system-ui,sans-serif">hiiiii</div>
<a href="/update" style="color:#888;font-size:14px;margin-top:20px">update</a></body></html>)HTML";

const char UPDATE_PAGE[] = R"HTML(<!DOCTYPE html><html><head><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1"><title>Update</title></head>
<body style="font-family:system-ui,sans-serif;background:#111;color:#eee;padding:16px;max-width:520px;margin:auto">
<h2>Update</h2><p id=v style="color:#888"></p>
<input type=file id=f accept=".bin" style="font-size:16px"><br><br>
<button onclick="up()" style="font-size:16px;padding:10px 16px;border:0;border-radius:8px;background:#2d7ff9;color:#fff">Update</button>
<p id=s></p>
<p style="color:#888;font-size:13px">Use the file that ends in .ino.bin (Sketch &gt; Export Compiled Binary, board ESP32S3 Dev Module).</p>
<script>
fetch('/version').then(r=>r.text()).then(t=>v.textContent='Running v'+t);
function up(){const file=f.files[0];if(!file){alert('Choose a .bin file first.');return;}
 const fd=new FormData();fd.append('f',file,file.name);const x=new XMLHttpRequest();x.open('POST','/update');
 x.upload.onprogress=e=>{if(e.lengthComputable)s.textContent='Uploading '+Math.round(e.loaded*100/e.total)+' %';};
 x.onload=()=>{s.textContent=x.status==200?'Updated! Restarting... join the Wi-Fi again in a few seconds.':'Not updated: '+x.responseText;};
 x.onerror=()=>{s.textContent='Upload failed (connection lost). Try again.';};x.send(fd);}
</script></body></html>)HTML";

// a firmware file starts with 0xE9, has the chip id at byte 12 (9 = ESP32-S3) and its app description at byte 32
String updErr; bool updFirst = false;
void onUpload() {
  HTTPUpload& u = server.upload();
  if (u.status == UPLOAD_FILE_START) {
    updErr = ""; updFirst = true;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) updErr = String("No room for an update: ") + Update.errorString();
  } else if (u.status == UPLOAD_FILE_WRITE) {
    if (updErr.length()) return;
    if (updFirst) {
      updFirst = false;
      uint32_t magic = 0; if (u.currentSize >= 36) memcpy(&magic, u.buf + 32, 4);
      const uint16_t chip = u.currentSize >= 14 ? (u.buf[12] | (u.buf[13] << 8)) : 0xFFFF;
      if (u.currentSize < 36 || u.buf[0] != 0xE9) updErr = "This is not a firmware file (.ino.bin).";
      else if (chip != 9) updErr = "This file is for another chip. Pick ESP32S3 Dev Module and export again.";
      else if (magic != 0xABCD5432u) updErr = "This is not the app file. Use the one that ends in .ino.bin.";
      if (updErr.length()) { Update.abort(); return; }
    }
    if (Update.write(u.buf, u.currentSize) != u.currentSize) { updErr = String("Write failed: ") + Update.errorString(); Update.abort(); }
  } else if (u.status == UPLOAD_FILE_END) {
    if (!updErr.length() && !Update.end(true)) updErr = String("The file did not pass the check: ") + Update.errorString();
  } else if (u.status == UPLOAD_FILE_ABORTED) {
    Update.abort(); updErr = "Upload interrupted";
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Hiiiii", nullptr, 1);                      // open network, channel 1 (the pad's radio channel)
  dns.start(53, "*", WiFi.softAPIP());                    // every address leads here, so the page pops up
  server.on("/update", HTTP_GET, []() { server.send(200, "text/html", UPDATE_PAGE); });
  server.on("/update", HTTP_POST, []() {
    if (updErr.length()) { server.send(400, "text/plain", updErr); return; }
    server.send(200, "text/plain", "OK");
    delay(500);
    ESP.restart();
  }, onUpload);
  server.on("/version", HTTP_GET, []() { server.send(200, "text/plain", VERSION); });
  server.onNotFound([]() { server.send(200, "text/html", PAGE); });
  server.begin();
  padOtaBegin("hiiiii", VERSION);                         // the pad can send updates by radio
  Serial.printf("hiiiii v%s: join the Wi-Fi \"Hiiiii\", then open http://%s (update: /update)\n", VERSION, WiFi.softAPIP().toString().c_str());
}

void loop() {
  dns.processNextRequest();
  server.handleClient();
  padOtaLoop();
}
