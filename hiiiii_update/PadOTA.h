#pragma once
/*
 * PadOTA.h v1.0: lets the MacroPad send this board new code by radio (ESP-NOW on Wi-Fi channel 1).
 * Works on ESP32, ESP32-S2, S3, C3 and C6. Put this file next to your .ino (it shows up as a tab in the Arduino IDE).
 *
 *   #include "PadOTA.h"
 *   setup():  padOtaBegin("my board", "1.0");    // name (max 15 letters) and version (max 11) shown in the pad's setup page
 *   loop():   padOtaLoop();                      // call it often (no long delay() in loop)
 *
 * - The board says hello to the pad every 2 s. The pad lists it under "Wireless controllers" in its setup page,
 *   where you pick a new .ino.bin for it. The pad sends it the next time it hears the board.
 * - The board takes updates only from the first pad that sends it one (it remembers that pad).
 *   padOtaForgetPad() clears that (e.g. on a button held at start-up).
 * - A cut or damaged transfer changes nothing: the old code keeps running.
 * - Wi-Fi: without Wi-Fi in your sketch, PadOTA starts it on channel 1. With your own access point, use channel 1:
 *   WiFi.softAP("name", "password", 1). If the board joins a router, ESP-NOW follows the router's channel, so the pad
 *   can only reach it if that is channel 1.
 * - If your sketch uses ESP-NOW itself: padOtaBegin("name", "1.0", false) and, in your receive callback, call
 *   padOtaHandle(info, data, len) first (it returns true when the packet was for PadOTA).
 */
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Update.h>
#include <Preferences.h>

namespace padota {
struct __attribute__((packed)) Hello { uint8_t magic; uint16_t chip; char name[16]; char ver[12]; };   // 0xD1
struct __attribute__((packed)) Ack   { uint8_t magic, op, status; uint32_t next; char ver[12]; };       // 0xD2
static const uint8_t BCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static char     name_[16] = "", ver_[12] = "";
static uint8_t  pad_[6];
static bool     padKnown = false, active = false;
static volatile bool rxReady = false;
static volatile int  rxLen = 0;
static uint8_t  rx[250], rxMac[6], fromMac[6];
static uint32_t size_ = 0, got = 0, crc = 0, crcWant = 0, lastAt = 0, lastHello = 0;
static wifi_interface_t ifx = WIFI_IF_STA;

inline uint16_t chipId() {
#if CONFIG_IDF_TARGET_ESP32S3
  return 9;
#elif CONFIG_IDF_TARGET_ESP32C3
  return 5;
#elif CONFIG_IDF_TARGET_ESP32C6
  return 13;
#elif CONFIG_IDF_TARGET_ESP32S2
  return 2;
#elif CONFIG_IDF_TARGET_ESP32
  return 0;
#else
  return 0xFFFF;
#endif
}
inline uint32_t crc32Add(uint32_t c, const uint8_t* p, size_t n) {   // same CRC-32 as the pad
  c = ~c;
  while (n--) { c ^= *p++; for (uint8_t k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u))); }
  return ~c;
}
inline void addPeer(const uint8_t* m) {
  if (esp_now_is_peer_exist(m)) return;
  esp_now_peer_info_t p = {};
  memcpy(p.peer_addr, m, 6); p.channel = 0; p.ifidx = ifx; p.encrypt = false;
  esp_now_add_peer(&p);
}
inline void reply(uint8_t op, uint8_t st) {
  Ack a = {}; a.magic = 0xD2; a.op = op; a.status = st; a.next = got;
  strlcpy(a.ver, ver_, sizeof a.ver);
  addPeer(fromMac);
  esp_now_send(fromMac, (const uint8_t*)&a, sizeof a);
}
inline void stop(const char* why) {
  if (active) Update.abort();
  active = false; got = 0;
  Serial.printf("PadOTA: update stopped (%s)\n", why);
}
inline void onRecv(const esp_now_recv_info_t* info, const uint8_t* d, int len);
}  // namespace padota

// Give PadOTA a received ESP-NOW packet; true = it was for PadOTA (only needed with your own receive callback).
inline bool padOtaHandle(const esp_now_recv_info_t* info, const uint8_t* d, int len) {
  using namespace padota;
  if (!info || !info->src_addr || len < 2 || len > (int)sizeof rx || d[0] != 0xD0) return false;
  if (padKnown && memcmp(info->src_addr, pad_, 6) != 0) return true;    // another pad: ignore
  if (rxReady) return true;                                             // still busy with the last one (the pad repeats)
  memcpy(rx, d, len); memcpy(rxMac, info->src_addr, 6); rxLen = len; rxReady = true;
  return true;
}
inline void padota::onRecv(const esp_now_recv_info_t* info, const uint8_t* d, int len) { padOtaHandle(info, d, len); }

inline void padOtaForgetPad() {
  Preferences p; p.begin("padota", false); p.remove("pad"); p.end();
  padota::padKnown = false;
}

inline void padOtaBegin(const char* name, const char* version, bool ownCallback = true) {
  using namespace padota;
  strlcpy(name_, name, sizeof name_); strlcpy(ver_, version, sizeof ver_);
  if (WiFi.getMode() == WIFI_OFF) WiFi.mode(WIFI_STA);
  ifx = (WiFi.getMode() & WIFI_MODE_STA) ? WIFI_IF_STA : WIFI_IF_AP;
  if (ifx == WIFI_IF_STA && WiFi.status() != WL_CONNECTED) esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);   // the pad's channel
  esp_now_init();                                                       // fine if the sketch started it already
  if (ownCallback) esp_now_register_recv_cb(onRecv);
  Preferences p; p.begin("padota", true); padKnown = p.getBytes("pad", pad_, 6) == 6; p.end();
  addPeer(BCAST);
  if (padKnown) addPeer(pad_);
  Serial.printf("PadOTA: \"%s\" v%s ready (%s)\n", name_, ver_, padKnown ? "pad known" : "waiting for a pad");
}

inline bool padOtaBusy() { return padota::active; }                     // true while an update is coming in

inline void padOtaLoop() {
  using namespace padota;
  const uint32_t now = millis();
  if (!active && now - lastHello >= 2000) {                             // hello: name, chip, version
    lastHello = now;
    Hello h = {}; h.magic = 0xD1; h.chip = chipId();
    strlcpy(h.name, name_, sizeof h.name); strlcpy(h.ver, ver_, sizeof h.ver);
    esp_now_send(BCAST, (const uint8_t*)&h, sizeof h);
  }
  if (active && now - lastAt > 15000) stop("the pad stopped sending");
  if (!rxReady) return;
  uint8_t b[sizeof rx]; const int n = rxLen;
  memcpy(b, rx, n); memcpy(fromMac, rxMac, 6); rxReady = false;
  switch (b[1]) {
    case 1: reply(1, 0); break;                                         // hello from the pad: answer with the version
    case 2: {                                                           // begin
      if (n < 10) return;
      if (!padKnown) {                                                  // first update: remember this pad
        memcpy(pad_, fromMac, 6); padKnown = true; addPeer(pad_);
        Preferences p; p.begin("padota", false); p.putBytes("pad", pad_, 6); p.end();
      }
      memcpy(&size_, b + 2, 4); memcpy(&crcWant, b + 6, 4);
      if (active) Update.abort();
      got = 0; crc = 0; lastAt = now;
      active = Update.begin(size_);
      if (!active) { Serial.printf("PadOTA: no room for %lu bytes\n", (unsigned long)size_); reply(2, 1); break; }
      Serial.printf("PadOTA: receiving %lu bytes...\n", (unsigned long)size_);
      reply(2, 0);
    } break;
    case 3: {                                                           // data
      if (!active) { reply(3, 5); break; }
      if (n < 7) return;
      uint32_t off; memcpy(&off, b + 2, 4);
      const uint32_t dl = n - 6;
      lastAt = now;
      if (off == got && got + dl <= size_) {                            // the next piece (repeats are just confirmed)
        if (Update.write(b + 6, dl) != dl) { reply(3, 2); stop("write error"); break; }
        crc = crc32Add(crc, b + 6, dl); got += dl;
      }
      reply(3, 0);
    } break;
    case 4: {                                                           // end: check, then restart with the new code
      if (!active) { reply(4, 5); break; }
      if (got != size_ || crc != crcWant) { reply(4, 3); stop("data changed on the way"); break; }
      if (!Update.end(true)) { active = false; reply(4, 4); Serial.println("PadOTA: the new code did not pass the check"); break; }
      reply(4, 0);
      Serial.println("PadOTA: done, restarting");
      delay(300);
      ESP.restart();
    } break;
    case 5: if (active) stop("cancelled by the pad"); break;
    default: break;
  }
}
