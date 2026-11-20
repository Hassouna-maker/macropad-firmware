/*
 * Wireless joystick board v3.2 (Waveshare ESP32-C6-Zero) -> ESP-NOW -> MacroPad main board
 * Needs main board (ESP32-S3) code v4.7 or newer (v4.9 for the stopwatch, v5.3 for updates over the air).
 *
 * NEW in v3.2: 118 more effects copied from WLED (https://github.com/wled/WLED, EUPL-1.2, see LICENSE in the repository).
 *  They sit between "song progress" and "off" in the effect list (names start with W:). Music reactive ones are left out.
 *  Needs the pad code v6.4 or newer to show their names on the pad's OLED and in its setup page.
 *
 * NEW in v3.1: first update sent through the pad (a test that the update path works). The version shows in the
 *  pad's setup page and on its OLED after the update. Nothing else changes.
 *
 * NEW in v3.0: updates over the air. Upload the stick file (.ino.bin) in the pad's setup page; the pad sends it
 *  the next time the stick is on and left still. The strip fills up purple while it comes in, then the stick
 *  restarts with the new code. If the transfer is cut, the old code keeps running.
 *  Upload this version by USB once. Keep a Partition Scheme with room for updates (the default 4MB one is fine).
 *
 * NEW in v2.9: scroll button fix. A click turned on precision mode (blue) instead of scroll mode when the
 *  stick doesn't rest exactly at its saved centre. Now only a real movement after pressing counts, and the
 *  onboard LED only turns blue once you actually move the stick.
 *
 * NEW in v2.8: the pad's stopwatch on the strip: cyan bar that fills once a minute (blinks when paused).
 *
 * NEW in v2.7:
 *  - The strip shows the pad's news by itself, then goes back to your effect:
 *      timer running = countdown bar (green -> red), setting the timer = amber breathing, time's up = red flashing,
 *      pad battery 10 % = red flashes, Bluetooth connects / mode changes = 1 s blue (iPad) or green (PC).
 *  - New effect "song progress": fills up as the song plays (colour per song), dims when paused.
 *  - Night mode from the pad: the strip dims at night too.
 *  - Precision mode: hold the scroll button and move the stick = slow cursor for small, exact moves
 *    (onboard LED blue). A quick click still turns scroll mode on / off, and holding 3 s without moving
 *    the stick still recalibrates.
 *
 * NEW in v2.6: instant start. No 3 s wait at power-on: the radio and LEDs start right away.
 *
 * NEW in v2.5: the stick tells the pad its effect, so the pad's OLED shows it under STRIP FX / MATRIX FX.
 *
 * NEW in v2.4:
 *  - Direct link: the stick finds your pad by itself (no MAC addresses to type). The pad answers the
 *    stick, the stick remembers it and from then on sends only to your pad. The radio confirms every
 *    message and repeats it if it got lost.
 *    Forget the pad (to pair with another one): hold the scroll button while you plug the stick in
 *    and keep holding until the strip flashes blue (2 s).
 *  - The pad's setup page sets the stick: LED brightness, effect, LED count (1-256), power limit,
 *    colour, speed and sleep time (0 = never sleep). Sent over the wireless link.
 *    The effect button on the stick still works; the pad's setting is used again after you save the setup page.
 *  - Recalibrate the centre: hold the scroll button 3 s (hands off the stick) -> the strip flashes green.
 *  - Stick button gestures (set in the pad's setup page):
 *      click = the click you chose, double click, long press (default right click),
 *      press and move = drag, double tap on the scroll button = any action you choose.
 *  - Smoother mouse: 100 updates a second while the stick moves (was 50).
 *  - New effects: joystick, tilt color, push meter, warp, click burst, lava lamp, ocean, twin comets.
 *
 * v2.3: the LED strip flashes amber in the last 10 s before sleep.
 * v2.2: transistor on GPIO15, 5 s wake-up signal.
 * v2.1: for 5 s after starting / waking, packets carry a "just woke up" flag so a sleeping pad wakes too.
 * v2.0: deep sleep when not used, dim after 1 min, strip power cut in sleep (2N2222A).
 *
 *  - Stick = mouse (or scroll when scroll mode is ON).
 *  - Scroll button (GPIO5): click = scroll mode ON / OFF (onboard LED white while ON), hold + move = precision,
 *    hold 3 s without moving the stick = recalibrate.
 *  - Effect button (GPIO4): press = next, hold 0.7-3 s and release = previous, hold 3 s = off / on.
 *  - Wake from sleep: press any button.
 *
 * LED power switch wiring (2N2222A, low side):
 *   5V ----------------------- strip +5V
 *   strip GND ---------------- 2N2222A collector,  emitter -> GND
 *   GPIO15 -- 220R ----------- 2N2222A base,       base -- 10k -- GND (keeps the strip off in sleep)
 *   GPIO3 -- 330R ------------ strip DIN
 * More than ~30 LEDs: use a logic-level MOSFET instead of the 2N2222A and a big enough 5 V supply.
 * Set PIN_LED_PWR to -1 if you don't fit the transistor.
 */
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <Preferences.h>
#include <FastLED.h>
#include <Update.h>
#include "WledCore.h"      // WLED effects (EUPL-1.2, see LICENSE): strip effects copied from WLED v16.0.1
#include "WledFx1D.h"
#include "WledUndef.h"

constexpr const char* FW_VERSION = "3.2";              // sent to the pad (shown in its setup page)

// ---- joystick
constexpr uint8_t PIN_X      = 0;    // stick X (analog)
constexpr uint8_t PIN_Y      = 1;    // stick Y (analog)
constexpr uint8_t PIN_BTN    = 2;    // stick button to GND                 (also wakes from sleep)
constexpr uint8_t PIN_SCROLL = 5;    // scroll button to GND                (also wakes from sleep)
// ---- LEDs
constexpr uint8_t  PIN_LED     = 3;  // WS2812B data
constexpr uint8_t  PIN_FXBTN   = 4;  // effect button to GND                (also wakes from sleep)
constexpr uint8_t  PIN_ONBOARD = 8;  // Waveshare ESP32-C6-Zero onboard RGB LED
constexpr int8_t   PIN_LED_PWR = 15; // 2N2222A base (via 220R): HIGH = strip powered. -1 = no switch fitted
constexpr uint16_t MAX_LEDS    = 256;// most LEDs the pad can set

// ---- defaults, used until the pad sends its settings (then the pad's setup page decides)
constexpr uint16_t DEF_LEDS   = 12;  // LEDs on the strip
constexpr uint8_t  DEF_BRIGHT = 60;  // max brightness (0-255)
constexpr uint16_t DEF_MA     = 250; // LED current limit in mA
constexpr uint8_t  DEF_HUE    = 160; // colour for effects that use one (0-255)
constexpr uint8_t  DEF_SPEED  = 128; // effect speed (0-255)
constexpr uint8_t  DEF_SLEEP  = 5;   // minutes without use before deep sleep (0 = never)

// ---- power saving
constexpr uint32_t DIM_AFTER_MS   = 60UL * 1000UL;        // dim LEDs after 1 min without use
constexpr uint8_t  DIM_BRIGHT     = 15;                   // brightness while dimmed
constexpr uint32_t WARN_MS        = 10000;                // strip flashes amber this long before sleep
constexpr int      MOVE_THRESHOLD = 200;                  // stick push (from centre) that counts as use
constexpr uint32_t CPU_MHZ        = 80;                   // 80 MHz is plenty and saves current

// ---- stick button gestures
constexpr uint32_t LONG_MS = 500;    // hold this long = long press
constexpr uint32_t DBL_MS  = 300;    // second press within this = double click
constexpr int      DRAG_T  = 450;    // press and push the stick this far = drag

// If FastLED gives a "pin invalid" / PARLIO error, change these two lines only.
#define ADD_STRIP()    FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, nLeds)
#define ADD_ONBOARD()  FastLED.addLeds<WS2812B, PIN_ONBOARD, GRB>(onboard, 1)

// ---- wireless packets (must match the main board)
// stick -> pad. btn: bit0 stick button down, bit1 scroll mode ON, bit2 just woke up,
//               bit3 hold the mouse button (drag / hold), bit4 looking for a pad
// ev / evSeq: last gesture (1 click, 2 double click, 3 long press, 4 scroll button double tap); evSeq counts up
// fx: the stick's effect (255 = strip off), shown on the pad's OLED
struct __attribute__((packed)) JoyPkt2 { uint8_t magic; int16_t x, y; uint8_t btn, ev, evSeq; uint16_t cfgId; uint8_t fx; };
// pad -> stick: settings from the pad's setup page. gest: bit0 double click set, bit1 long press set,
// bit2 scroll double tap set. fx 255 = keep the stick's own effect.
struct __attribute__((packed)) PadPkt { uint8_t magic; uint16_t cfgId; uint8_t bright, fx; uint16_t count, maxMa;
                                        uint8_t sleepMin, hue, speed, gest; };
// pad -> stick, 5 times a second: what the pad is doing. mode 0 = not connected, 1 = iPad, 2 = PC.
// timer 0 idle, 1 setting, 2 running, 3 time's up (tLeft / tTotal in ms), 4 stopwatch running, 5 stopwatch paused
// (tLeft = stopwatch time in ms). bat -1 = unknown.
// song 0 = nothing, 1 = playing, 2 = paused (sFrac 0-65535 = how far). night = brightness scale (255 = day).
struct __attribute__((packed)) PadInfo { uint8_t magic, mode, timer; uint32_t tLeft, tTotal; int8_t bat;
                                         uint8_t song; uint16_t sFrac; uint8_t sHue, night; };
enum { EV_NONE, EV_TAP, EV_DBL, EV_LONG, EV_SDBL };
static const uint8_t BCAST[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static int cx = 2048, cy = 2048; static float rng = 1800;
static bool ready = false;
static bool scrollMode = false;

// settings (from the pad, saved in flash)
static uint16_t nLeds = DEF_LEDS;
static uint8_t  maxBright = DEF_BRIGHT, HUE = DEF_HUE, SPEED = DEF_SPEED, gest = 0;
static uint16_t maxMa = DEF_MA;
static uint32_t sleepAfterMs = DEF_SLEEP * 60000UL;     // 0 = never sleep
static uint16_t cfgId = 0;                               // which pad settings were applied last

// the pad we talk to
static uint8_t padMac[6];
static bool    padKnown = false;

// kept in RTC memory across deep sleep (lost on power-off)
RTC_DATA_ATTR static bool rtcValid = false;
RTC_DATA_ATTR static bool rtcScroll = false;
RTC_DATA_ATTR static bool rtcStripOff = false;

static uint32_t lastUse = 0;
static uint32_t helloUntil = 0;         // send the "just woke up" flag until this time
static uint8_t  curBright = DEF_BRIGHT;
static void markUsed() { lastUse = millis(); }

// gesture state sent to the pad
static bool     stickDown = false, holdBtn = false, forceSend = false, precise = false;
static PadInfo  pinfo = {};                          // latest news from the pad
static volatile uint32_t pinfoAt = 0;
static bool infoFresh() { return pinfoAt && millis() - pinfoAt < 2000; }
static uint8_t  evCode = EV_NONE, evSeq = 0, burst = 0;
static void sendEvent(uint8_t e) { evCode = e; evSeq++; forceSend = true; burst = 3; }

// stick position for the effects (-1..1) and the last click (click burst effect)
static float    jx = 0, jy = 0;
static uint32_t clickAt[4] = {0}; static uint8_t clickHue[4]; static uint8_t clickIdx = 0;

// ============================ Effects ============================
enum { E_SOLID, E_BLINK, E_BREATHE, E_WIPE, E_WIPERANDOM, E_RANDOMCOLORS, E_SWEEP, E_DYNAMIC, E_COLORLOOP, E_RAINBOW,
       E_SCAN, E_DUALSCAN, E_FADE, E_THEATER, E_THEATERRAINBOW, E_RUNNING, E_SAW, E_TWINKLE, E_DISSOLVE, E_SPARKLE,
       E_FLASHSPARKLE, E_HYPERSPARKLE, E_STROBE, E_STROBEBOW, E_BLINKRAINBOW, E_CHASE, E_CHASERAINBOW, E_CHASEFLASH,
       E_COMET, E_FIREWORKS, E_RAIN, E_FIRE2012, E_FIREFLICKER, E_CANDLE, E_COLORTWINKLE, E_METEOR, E_JUGGLE, E_BPM,
       E_PRIDE, E_PALETTE, E_LAKE, E_RIPPLE, E_PLASMA, E_LIGHTNING, E_GRADIENT, E_HEARTBEAT, E_NOISE, E_AURORA,
       E_ANDROID, E_BOUNCING, E_SINELON, E_POPCORN, E_LARSON, E_POLICE, E_TRIWIPE, E_CONFETTI,
       E_JOY, E_TILT, E_METER, E_WARP, E_CLICKBURST, E_LAVA, E_OCEAN, E_TWINCOMETS, E_SONG,
       E_WLED, E_OFF = E_WLED + wfx::WFX1D_N };   // E_WLED.. = effects copied from WLED

static const char* const FX_NAMES[] = {
  "solid","blink","breathe","wipe","wipe random","random colors","sweep","dynamic","colorloop","rainbow",
  "scan","dual scan","fade","theater chase","theater rainbow","running lights","saw","twinkle","dissolve","sparkle",
  "flash sparkle","hyper sparkle","strobe","strobe rainbow","blink rainbow","chase","chase rainbow","chase flash",
  "comet","fireworks","rain","fire 2012","fire flicker","candle","colortwinkle","meteor","juggle","bpm",
  "pride","palette","lake","ripple","plasma","lightning","gradient","heartbeat","noise","aurora",
  "android","bouncing balls","sinelon","popcorn","larson scanner","police","tri wipe","confetti",
  "joystick","tilt color","push meter","warp","click burst","lava lamp","ocean","twin comets","song progress"};   // then the WLED effects (names start with W:), then "off"
constexpr uint8_t N_FX = E_OFF + 1;
static_assert(sizeof(FX_NAMES) / sizeof(FX_NAMES[0]) == E_WLED, "effect names and enum do not match");
static char wledName[wfx::WFX1D_N][24];                     // "W:" + WLED's own name
static struct WledNameInit { WledNameInit() { for (uint8_t i = 0; i < wfx::WFX1D_N; i++) snprintf(wledName[i], sizeof wledName[i], "W:%s", wfx::WFX1D[i].name); } } wledNameInit;
static const char* fxName(uint8_t i) { return i < E_WLED ? FX_NAMES[i] : i < E_OFF ? wledName[i - E_WLED] : "off"; }
static inline bool isWledFx(uint8_t fx) { return fx >= E_WLED && fx < E_OFF; }

static CRGB leds[MAX_LEDS];
static CRGB onboard[1];
static CLEDController* stripCtl = nullptr;
static uint8_t curFx = 0;
static bool stripOff = false;
static Preferences prefs;
static uint32_t T = 0, fdt = 0;          // effect clock and last frame time (both scaled by SPEED)

// effect state
static uint8_t heat[MAX_LEDS];
static bool    dis[MAX_LEDS];
static bool    disTarget = true;
static int16_t dpos[6];
static float   bpos[3], bvel[3];
static bool    binit = false;
static bool    kon[5];
static float   kpos[5], kvel[5];
static uint8_t khue[5];
static float   wpos[8];
static uint16_t rc = 0; static uint8_t rh = 0;
static uint32_t rt0 = 0;

static void resetFx() {
  fill_solid(leds, MAX_LEDS, CRGB(0, 0, 0));
  memset(heat, 0, sizeof heat);
  memset(dis, 0, sizeof dis);
  disTarget = true;
  for (auto& d : dpos) d = -1;
  binit = false;
  for (auto& k : kon) k = false;
  for (uint8_t i = 0; i < 8; i++) wpos[i] = random16(1000) / 1000.0f;
  rt0 = 0;
  if (isWledFx(curFx)) wfx::wfxSelect(0, wfx::WFX1D[curFx - E_WLED]); else wfx::gSegs[0].resetRuntime();
}

// beat helper: returns lo..hi, 'bpm' cycles per minute, driven by the effect clock
static uint8_t tb(uint32_t t, uint8_t bpm, uint8_t lo, uint8_t hi) {
  uint8_t ph = (uint8_t)(((uint64_t)t * bpm) / 234);
  return lo + (uint8_t)(((uint16_t)sin8(ph) * (uint8_t)(hi - lo)) >> 8);
}

// soft dot at a fractional position (spread over 2 LEDs)
static void dot(float p, CRGB c) {
  const uint16_t n = nLeds;
  if (p < 0 || p > n - 1) return;
  uint16_t i0 = (uint16_t)p; float fr = p - i0;
  CRGB a = c; a.nscale8((uint8_t)(255 * (1.0f - fr)));
  leds[i0] += a;
  if (i0 + 1 < n) { CRGB b = c; b.nscale8((uint8_t)(255 * fr)); leds[i0 + 1] += b; }
}

static void runFx(uint8_t fx) {
  const uint16_t n = nLeds;
  const uint32_t t = T;
  const CRGB col = CHSV(HUE, 255, 255);
  const CRGB blk(0, 0, 0);
  const CRGB wht(255, 255, 255);

  switch (fx) {
    case E_SOLID: fill_solid(leds, n, col); break;
    case E_BLINK: fill_solid(leds, n, ((t / 500) & 1) ? col : blk); break;
    case E_BREATHE: fill_solid(leds, n, CHSV(HUE, 255, tb(t, 12, 20, 255))); break;

    case E_WIPE: {
      uint32_t p = (t / 60) % (2 * n);
      for (uint16_t i = 0; i < n; i++) { bool on = p < n ? (i <= p) : (i > p - n); leds[i] = on ? col : blk; }
    } break;

    case E_WIPERANDOM: {
      uint32_t step = t / 60; uint32_t p = step % (2 * n);
      CRGB c = CHSV((uint8_t)((step / (2 * n)) * 91), 255, 255);
      for (uint16_t i = 0; i < n; i++) { bool on = p < n ? (i <= p) : (i > p - n); leds[i] = on ? c : blk; }
    } break;

    case E_RANDOMCOLORS: {
      static uint32_t last = 0; static CRGB ca = CRGB(0, 0, 0), cb = CRGB(0, 0, 0);
      if (t - last >= 1500) { last = t; ca = cb; cb = CHSV(random8(), 255, 255); }
      uint32_t el = t - last; uint8_t am = el >= 600 ? 255 : (uint8_t)(el * 255 / 600);
      fill_solid(leds, n, blend(ca, cb, am));
    } break;

    case E_SWEEP: {
      uint16_t pos = (triwave8(t / 6) * n) >> 8;
      for (uint16_t i = 0; i < n; i++) leds[i] = (i <= pos) ? col : blk;
    } break;

    case E_DYNAMIC: {
      static uint8_t dh[MAX_LEDS]; static uint32_t last = 0;
      if (t - last >= 150) { last = t; for (uint8_t k = 0; k < 4; k++) dh[random16(n)] = random8(); }
      for (uint16_t i = 0; i < n; i++) nblend(leds[i], CRGB(CHSV(dh[i], 255, 255)), 40);
    } break;

    case E_COLORLOOP: fill_solid(leds, n, CHSV((uint8_t)(t / 40), 255, 255)); break;
    case E_RAINBOW:   fill_rainbow(leds, n, (uint8_t)(t / 50), max(1, 255 / n)); break;

    case E_SCAN: {
      fill_solid(leds, n, blk);
      leds[(triwave8(t / 5) * n) >> 8] = col;
    } break;
    case E_DUALSCAN: {
      fill_solid(leds, n, blk);
      uint16_t pos = (triwave8(t / 5) * n) >> 8;
      leds[pos] = col; leds[n - 1 - pos] = CHSV(HUE + 96, 255, 255);
    } break;

    case E_FADE: fill_solid(leds, n, blend(col, CRGB(CHSV(HUE + 128, 255, 255)), tb(t, 15, 0, 255))); break;

    case E_THEATER: {
      uint8_t q = (t / 90) % 3;
      for (uint16_t i = 0; i < n; i++) leds[i] = ((i + q) % 3 == 0) ? col : blk;
    } break;
    case E_THEATERRAINBOW: {
      uint8_t q = (t / 90) % 3;
      for (uint16_t i = 0; i < n; i++) {
        CRGB c = CHSV((uint8_t)(t / 20 + i * 255 / n), 255, 255);
        leds[i] = ((i + q) % 3 == 0) ? c : blk;
      }
    } break;

    case E_RUNNING:
      for (uint16_t i = 0; i < n; i++) leds[i] = CHSV(HUE, 255, sin8((uint8_t)(i * 32 - (t >> 2))));
      break;
    case E_SAW:
      for (uint16_t i = 0; i < n; i++) leds[i] = CHSV(HUE, 255, (uint8_t)(i * 8 - t / 4));
      break;

    case E_TWINKLE:
      fadeToBlackBy(leds, n, 40);
      if (random8() < 60) leds[random16(n)] = col;
      break;

    case E_DISSOLVE: {
      static uint32_t last = 0;
      if (t - last >= 50) {
        last = t;
        uint16_t k = random16(n);
        for (uint16_t j = 0; j < n; j++) { uint16_t idx = (k + j) % n; if (dis[idx] != disTarget) { dis[idx] = disTarget; break; } }
        bool all = true;
        for (uint16_t i = 0; i < n; i++) if (dis[i] != disTarget) all = false;
        if (all) disTarget = !disTarget;
      }
      for (uint16_t i = 0; i < n; i++) leds[i] = dis[i] ? col : blk;
    } break;

    case E_SPARKLE:
      fill_solid(leds, n, blk);
      leds[random16(n)] = col;
      break;
    case E_FLASHSPARKLE:
      fill_solid(leds, n, CHSV(HUE, 255, 60));
      if (random8() < 40) leds[random16(n)] = wht;
      break;
    case E_HYPERSPARKLE:
      fill_solid(leds, n, CHSV(HUE, 255, 60));
      for (uint8_t k = 0; k < 4; k++) if (random8() < 120) leds[random16(n)] = wht;
      break;

    case E_STROBE: fill_solid(leds, n, (t % 100) < 20 ? col : blk); break;
    case E_STROBEBOW: {
      CRGB c = CHSV((uint8_t)((t / 100) * 37), 255, 255);
      fill_solid(leds, n, (t % 100) < 20 ? c : blk);
    } break;
    case E_BLINKRAINBOW: {
      CRGB c = CHSV((uint8_t)((t / 700) * 43), 255, 255);
      fill_solid(leds, n, ((t / 350) & 1) ? c : blk);
    } break;

    case E_CHASE: {
      uint16_t pos = (t / 50) % n; CRGB bg = CHSV(HUE + 128, 255, 40);
      for (uint16_t i = 0; i < n; i++) leds[i] = (((i + n - pos) % n) < 6) ? col : bg;
    } break;
    case E_CHASERAINBOW: {
      uint16_t pos = (t / 50) % n; CRGB c = CHSV((uint8_t)(t / 25), 255, 255);
      for (uint16_t i = 0; i < n; i++) leds[i] = (((i + n - pos) % n) < 6) ? c : blk;
    } break;
    case E_CHASEFLASH: {
      uint16_t pos = (t / 45) % n; bool fl = (t % 1000) < 100;
      for (uint16_t i = 0; i < n; i++) leds[i] = fl ? wht : ((((i + n - pos) % n) < 4) ? col : blk);
    } break;

    case E_COMET:
      fadeToBlackBy(leds, n, 60);
      leds[(t / 35) % n] = col;
      break;

    case E_FIREWORKS:
      fadeToBlackBy(leds, n, 45);
      if (random8() < 22) {
        uint16_t p = random16(n); CRGB c = CHSV(random8(), 255, 255);
        leds[p] = c;
        CRGB d = c; d.nscale8(110);
        if (p > 0) leds[p - 1] += d;
        if (p < n - 1) leds[p + 1] += d;
      }
      break;

    case E_RAIN: {
      static uint32_t last = 0;
      fadeToBlackBy(leds, n, 80);
      if (t - last >= 45) {
        last = t;
        for (auto& d : dpos) {
          if (d >= 0) d--;
          else if (random8() < 20) d = n - 1;
        }
      }
      for (auto& d : dpos) if (d >= 0 && d < (int)n) leds[d] = CHSV(HUE, 100, 255);
    } break;

    case E_FIRE2012: {
      static uint32_t last = 0;
      if (t - last >= 25) {
        last = t;
        uint16_t cl = 550 / n + 2; if (cl > 255) cl = 255;
        for (uint16_t i = 0; i < n; i++) heat[i] = qsub8(heat[i], random8(0, cl));
        for (int k = n - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
        if (random8() < 120) { uint8_t y = random8(n < 7 ? n : 7); heat[y] = qadd8(heat[y], random8(160, 255)); }
      }
      for (uint16_t i = 0; i < n; i++) leds[i] = HeatColor(heat[i]);
    } break;

    case E_FIREFLICKER: {
      static uint32_t last = 0; static CRGB tg[MAX_LEDS];
      if (t - last >= 60) { last = t; for (uint16_t i = 0; i < n; i++) tg[i] = HeatColor(random8(100, 190)); }
      for (uint16_t i = 0; i < n; i++) nblend(leds[i], tg[i], 70);
    } break;

    case E_CANDLE: {
      static uint32_t last = 0; static int lvl = 150;
      if (t - last >= 40) {
        last = t; lvl += (int)random8(21) - 10;
        if (lvl < 90) lvl = 90;
        if (lvl > 255) lvl = 255;
      }
      fill_solid(leds, n, CHSV(25, 240, (uint8_t)lvl));
    } break;

    case E_COLORTWINKLE:
      fadeToBlackBy(leds, n, 25);
      if (random8() < 40) leds[random16(n)] = CHSV(random8(), 255, 255);
      break;

    case E_METEOR: {
      for (uint16_t i = 0; i < n; i++) if (random8() < 90) leds[i].nscale8(190);
      uint32_t head = (t / 45) % (n + 6);
      for (int j = 0; j < 4; j++) {
        int p = (int)head - j;
        if (p >= 0 && p < (int)n) leds[p] = CHSV(HUE, j ? 255 : 140, 255 - j * 50);
      }
    } break;

    case E_JUGGLE: {
      fadeToBlackBy(leds, n, 20);
      uint8_t dh = 0;
      for (uint8_t i = 0; i < 8; i++) { leds[tb(t, i + 7, 0, n - 1)] |= CHSV(dh, 255, 255); dh += 32; }
    } break;

    case E_BPM: {
      uint8_t beat = tb(t, 62, 64, 255), g = (uint8_t)(t / 40);
      for (uint16_t i = 0; i < n; i++) leds[i] = ColorFromPalette(PartyColors_p, g + i * 2, (uint8_t)(beat - g + i * 10));
    } break;

    case E_PRIDE:
      for (uint16_t i = 0; i < n; i++)
        leds[i] = CHSV((uint8_t)(t / 30 + i * 8 + (sin8((uint8_t)(i * 10 + t / 25)) >> 2)), 255,
                       110 + (sin8((uint8_t)(i * 20 - t / 12)) >> 1));
      break;

    case E_PALETTE:
      for (uint16_t i = 0; i < n; i++)
        leds[i] = ColorFromPalette(RainbowColors_p, (uint8_t)(inoise8(i * 30, t >> 3) + (t >> 6)), 255);
      break;

    case E_LAKE:
      for (uint16_t i = 0; i < n; i++)
        leds[i] = CHSV(130 + (sin8((uint8_t)(i * 9 + t / 12)) >> 3), 255, 90 + (sin8((uint8_t)(i * 14 - t / 8)) >> 1));
      break;

    case E_RIPPLE: {
      int r = (int)((t - rt0) / 70);
      if (rt0 == 0 || r > (int)n) { rt0 = t; rc = random16(n); rh = random8(); r = 0; }
      fill_solid(leds, n, blk);
      for (uint16_t i = 0; i < n; i++) {
        int d = abs((int)i - (int)rc);
        if (d == r)          leds[i] = CHSV(rh, 255, 255);
        else if (d == r - 1) leds[i] = CHSV(rh, 255, 120);
        else if (d == r - 2) leds[i] = CHSV(rh, 255, 50);
      }
    } break;

    case E_PLASMA:
      for (uint16_t i = 0; i < n; i++) {
        uint8_t v = (sin8((uint8_t)(i * 14 + t / 6)) + sin8((uint8_t)(i * 9 - t / 9)) + sin8((uint8_t)(i * 5 + t / 14))) / 3;
        leds[i] = ColorFromPalette(PartyColors_p, (uint8_t)(v + t / 60), 255);
      }
      break;

    case E_LIGHTNING: {
      static uint32_t nextAt = 0, until = 0; static uint16_t lp = 0, ll = 3;
      if (t >= nextAt) { nextAt = t + random16(400, 2500); until = t + random8(40, 120); lp = random16(n); ll = random16(3, max<uint16_t>(4, n / 2)); }
      if (t < until) {
        uint8_t v = random8(120, 255);
        for (uint16_t i = 0; i < n; i++) leds[i] = (i >= lp && i < lp + ll) ? CRGB(v, v, v) : blk;
      } else fadeToBlackBy(leds, n, 80);
    } break;

    case E_GRADIENT: {
      uint8_t base = HUE + (sin8((uint8_t)(t / 25)) >> 2);
      for (uint16_t i = 0; i < n; i++) leds[i] = CHSV((uint8_t)(base + i * 60 / n), 255, 255);
    } break;

    case E_HEARTBEAT: {
      uint16_t ph = t % 1300; uint8_t v = 0;
      if (ph < 200) v = triwave8((ph * 255) / 200);
      else if (ph >= 320 && ph < 520) v = scale8(triwave8(((ph - 320) * 255) / 200), 190);
      fill_solid(leds, n, CHSV(HUE, 255, v));
    } break;

    case E_NOISE:
      for (uint16_t i = 0; i < n; i++) leds[i] = CHSV((uint8_t)(HUE + (inoise8(i * 45, t >> 2) >> 1)), 255, 255);
      break;

    case E_AURORA:
      for (uint16_t i = 0; i < n; i++) {
        uint8_t a = inoise8(i * 50, t >> 2), b = inoise8(i * 30 + 5000, t >> 3);
        leds[i] = CHSV(HUE + (b >> 2), 255, qadd8(a, 40));
      }
      break;

    case E_ANDROID: {
      fill_solid(leds, n, blk);
      uint16_t head = (t / 55) % n, len = 4 + tb(t, 20, 0, 12);
      for (uint16_t j = 0; j < len && j < n; j++) leds[(head + j) % n] = col;
    } break;

    case E_BOUNCING: {
      const float g = 0.0004f, v0 = 0.157f;
      if (!binit) { binit = true; for (uint8_t k = 0; k < 3; k++) { bpos[k] = 0; bvel[k] = v0 * (0.55f + 0.2f * k); } }
      fill_solid(leds, n, blk);
      for (uint8_t k = 0; k < 3; k++) {
        bvel[k] -= g * fdt; bpos[k] += bvel[k] * fdt;
        if (bpos[k] < 0) { bpos[k] = 0; bvel[k] = -bvel[k] * 0.88f; if (bvel[k] < 0.04f) bvel[k] = v0 * (0.7f + 0.15f * k); }
        int p = (int)(bpos[k] * n / 12.0f); if (p > (int)n - 1) p = n - 1;
        leds[p] = CHSV(HUE + k * 50, 255, 255);
      }
    } break;

    case E_SINELON:
      fadeToBlackBy(leds, n, 20);
      leds[tb(t, 25, 0, n - 1)] += col;
      break;

    case E_POPCORN: {
      fadeToBlackBy(leds, n, 70);
      for (uint8_t k = 0; k < 5; k++) {
        if (!kon[k]) {
          if (random8() < 4) { kon[k] = true; kpos[k] = 0; kvel[k] = 0.07f + random8() / 3500.0f; khue[k] = random8(); }
        } else {
          kvel[k] -= 0.0003f * fdt; kpos[k] += kvel[k] * fdt;
          if (kpos[k] < 0) kon[k] = false;
          else { int p = (int)(kpos[k] * n / 12.0f); if (p > (int)n - 1) p = n - 1; leds[p] = CHSV(khue[k], 255, 255); }
        }
      }
    } break;

    case E_LARSON:
      fadeToBlackBy(leds, n, 60);
      leds[(triwave8(t / 4) * n) >> 8] = col;
      break;

    case E_POLICE: {
      uint8_t ph = (t / 110) % 6;
      for (uint16_t i = 0; i < n; i++) {
        if ((ph == 0 || ph == 2) && i < n / 2) leds[i] = CRGB(255, 0, 0);
        else if ((ph == 3 || ph == 5) && i >= n / 2) leds[i] = CRGB(0, 0, 255);
        else leds[i] = blk;
      }
    } break;

    case E_TRIWIPE: {
      uint32_t step = t / 60; uint32_t seg = step / n; uint16_t k = step % n;
      CRGB cur = CHSV(HUE + (seg % 3) * 85, 255, 255), prev = CHSV(HUE + ((seg + 2) % 3) * 85, 255, 255);
      for (uint16_t i = 0; i < n; i++) leds[i] = (i <= k) ? cur : prev;
    } break;

    case E_CONFETTI:
      fadeToBlackBy(leds, n, 12);
      if (random8() < 90) leds[random16(n)] += CHSV((uint8_t)(t / 50) + random8(64), 200, 255);
      break;

    // ---- stick effects
    case E_JOY: {                                       // a light that follows the stick left / right (up / down = colour)
      fadeToBlackBy(leds, n, 60);
      dot((jx + 1.0f) * 0.5f * (n - 1), CHSV(HUE + (int8_t)(jy * 60.0f), 230, 255));
    } break;

    case E_TILT: {                                      // push direction = colour, push distance = brightness
      float d = sqrtf(jx * jx + jy * jy); if (d > 1) d = 1;
      if (d < 0.15f) fill_solid(leds, n, CHSV(HUE, 255, tb(t, 10, 30, 90)));        // centred: slow breathing
      else {
        uint8_t h = (uint8_t)((atan2f(jy, jx) + PI) * (255.0f / (2 * PI)));
        CRGB c = CHSV(h, 255, (uint8_t)(70 + 185 * d));
        for (uint16_t i = 0; i < n; i++) nblend(leds[i], c, 60);
      }
    } break;

    case E_METER: {                                     // bar grows with how far the stick is pushed (green -> red)
      float d = sqrtf(jx * jx + jy * jy); if (d > 1) d = 1;
      static float lv = 0, peak = 0; static uint32_t peakAt = 0;
      lv += (d - lv) * 0.3f;
      if (lv >= peak) { peak = lv; peakAt = millis(); }
      else if (millis() - peakAt > 600) peak = max(lv, peak - 0.02f);
      float fl = lv * n;
      for (uint16_t i = 0; i < n; i++) {
        CRGB c = CHSV((uint8_t)(96 - 96 * i / max<uint16_t>(1, n - 1)), 255, 255);
        if (i + 1 <= fl) leds[i] = c;
        else if (i < fl) { c.nscale8((uint8_t)((fl - i) * 255)); leds[i] = c; }
        else leds[i] = CHSV(HUE, 255, 12);
      }
      uint16_t pk = (uint16_t)(peak * (n - 1) + 0.5f);
      if (peak > 0.05f) leds[pk] = wht;
    } break;

    case E_WARP: {                                      // stars fly along the strip; push the stick to go faster / reverse
      fadeToBlackBy(leds, n, 90);
      float sp = 0.0006f + fabsf(jx) * 0.006f;           // strip lengths per ms
      float dir = jx < -0.1f ? -1.0f : 1.0f;
      for (uint8_t k = 0; k < 8; k++) {
        wpos[k] += dir * sp * (0.6f + 0.1f * k) * fdt;
        if (wpos[k] > 1) wpos[k] -= 1;
        if (wpos[k] < 0) wpos[k] += 1;
        dot(wpos[k] * (n - 1), CHSV(HUE + k * 12, k & 1 ? 120 : 30, 255));
      }
    } break;

    case E_CLICKBURST: {                                // every stick click sends a burst of colour out from the middle
      fill_solid(leds, n, CHSV(HUE, 255, 10));
      const uint32_t now = millis(); const float mid = (n - 1) * 0.5f;
      for (uint8_t k = 0; k < 4; k++) {
        if (!clickAt[k]) continue;
        uint32_t age = now - clickAt[k];
        if (age > 900) { clickAt[k] = 0; continue; }
        float r = age / 900.0f * (mid + 2);
        uint8_t v = 255 - (uint8_t)(age * 255 / 900);
        dot(mid - r, CHSV(clickHue[k], 220, v));
        dot(mid + r, CHSV(clickHue[k], 220, v));
      }
    } break;

    case E_LAVA:
      for (uint16_t i = 0; i < n; i++) leds[i] = ColorFromPalette(LavaColors_p, inoise8(i * 40, t >> 3), 255);
      break;

    case E_OCEAN:
      for (uint16_t i = 0; i < n; i++) leds[i] = ColorFromPalette(OceanColors_p, inoise8(i * 35 + (t >> 4), t >> 3), 255);
      break;

    case E_TWINCOMETS: {                                // two comets from both ends cross in the middle
      fadeToBlackBy(leds, n, 50);
      uint16_t p = (t / 40) % n;
      leds[p] += col;
      leds[n - 1 - p] += CHSV(HUE + 128, 255, 255);
    } break;

    case E_SONG: {                                      // fills up as the song plays (from the pad / iPad)
      if (!infoFresh() || !pinfo.song) { fill_solid(leds, n, CHSV(HUE, 200, tb(t, 8, 15, 60))); break; }   // nothing playing: soft glow
      const bool pl = pinfo.song == 1; const uint8_t sh = pinfo.sHue;
      const uint8_t v = pl ? 255 : tb(t, 15, 50, 110);  // paused: dim and breathing
      const float f = pinfo.sFrac / 65535.0f * n;
      for (uint16_t i = 0; i < n; i++) {
        if (i + 1 <= f)  leds[i] = CHSV(sh + i * 3, 230, v);
        else if (i < f)  leds[i] = CHSV(sh, 150, scale8(v, (uint8_t)((f - i) * 255)));
        else             leds[i] = CHSV(sh, 230, 6);
      }
      if (pl) { uint16_t hd = (uint16_t)f; if (hd > n - 1) hd = n - 1; leds[hd] += CHSV(sh, 60, tb(t, 40, 40, 160)); }   // the head glows
    } break;

    default:
      if (isWledFx(fx)) wfx::wfxFrame(0, wfx::WFX1D[fx - E_WLED], leds, n, 0, 0, nullptr, SPEED, HUE, millis());
      else fill_solid(leds, n, blk);
      break;
  }
}

// ============================ Pad news on the strip (over the effect) ============================
static void padOverlay(uint32_t now) {
  if (!infoFresh()) return;
  const uint16_t n = nLeds;
  static uint8_t lastMode = 0; static uint32_t modeAt = 0;
  if (pinfo.mode != lastMode) { if (pinfo.mode) modeAt = now ? now : 1; lastMode = pinfo.mode; }
  static int8_t lastBat = -1; static uint32_t lowAt = 0;
  const int8_t b = pinfo.bat;
  if (b >= 0 && b <= 10 && (lastBat < 0 || lastBat > 10)) lowAt = now ? now : 1;
  lastBat = b;

  if (pinfo.timer == 3) {                                // time's up: red flashing
    fill_solid(leds, n, ((now / 250) & 1) ? CRGB(255, 0, 0) : CRGB(0, 0, 0));
    return;
  }
  if (pinfo.timer == 2) {                                // running: countdown bar, green -> red
    const uint32_t el = now - pinfoAt;
    const uint32_t left = pinfo.tLeft > el ? pinfo.tLeft - el : 0;
    float fr = pinfo.tTotal ? (float)left / pinfo.tTotal : 0; if (fr > 1) fr = 1;
    const float f = fr * n; const uint8_t h = (uint8_t)(fr * 96);
    for (uint16_t i = 0; i < n; i++) {
      if (i + 1 <= f)  leds[i] = CHSV(h, 255, 255);
      else if (i < f)  leds[i] = CHSV(h, 255, (uint8_t)((f - i) * 255));
      else             leds[i] = CHSV(h, 255, 8);
    }
  } else if (pinfo.timer == 4 || pinfo.timer == 5) {     // stopwatch: cyan bar that fills once a minute
    const bool run = pinfo.timer == 4;
    const uint32_t el = pinfo.tLeft + (run ? now - pinfoAt : 0);
    const float f = (el % 60000) / 60000.0f * n;
    const uint8_t v = run || ((now / 400) & 1) ? 255 : 70;   // blinks when paused
    for (uint16_t i = 0; i < n; i++) {
      if (i + 1 <= f)  leds[i] = CHSV(135, 255, scale8(v, 150));
      else if (i < f)  leds[i] = CHSV(135, 120, v);      // the head is brighter
      else             leds[i] = CHSV(135, 255, 8);
    }
  } else if (pinfo.timer == 1) {                         // setting the timer: amber breathing
    fill_solid(leds, n, CHSV(25, 255, tb(T, 30, 40, 220)));
  }
  if (b >= 0 && b <= 10 && ((lowAt && now - lowAt < 3000) || (now % 30000) < 900))   // pad battery low
    fill_solid(leds, n, ((now / 150) & 1) ? CRGB(255, 0, 0) : CRGB(0, 0, 0));
  if (modeAt && now - modeAt < 1000) {                   // connected / mode changed: blue = iPad, green = PC
    CRGB c = pinfo.mode == 2 ? CRGB(0, 255, 0) : CRGB(0, 80, 255);
    c.nscale8(255 - (uint8_t)((now - modeAt) * 255 / 1000));
    fill_solid(leds, n, c);
  }
}

// ============================ LED helpers ============================
static void flashStrip(CRGB c, uint8_t times) {   // short blocking flash (calibrate / pair / forget)
  for (uint8_t k = 0; k < times; k++) {
    fill_solid(leds, nLeds, c); FastLED.setBrightness(max<uint8_t>(maxBright, 40)); FastLED.show(); delay(150);
    fill_solid(leds, nLeds, CRGB(0, 0, 0)); FastLED.show(); delay(150);
  }
  FastLED.setBrightness(curBright);
  resetFx();
}

static void setLedCount(uint16_t n) {
  n = constrain(n, 1, MAX_LEDS);
  if (n == nLeds) return;
  fill_solid(leds, MAX_LEDS, CRGB(0, 0, 0));
  if (stripCtl && n < nLeds) FastLED.show();         // blank the LEDs that are being dropped
  nLeds = n;
  if (stripCtl) stripCtl->setLeds(leds, nLeds);
  resetFx();
}

// ============================ Effect button ============================
// Quick press = next effect. Hold 0.7-3 s and release = previous effect.
// Keep holding for 3 s = strip off (or back on). While off, a quick press turns it back on.
static bool     fxDirty = false;
static uint32_t fxChangedMs = 0;

static void changeFx(int d) {
  curFx = (uint8_t)((curFx + N_FX + d) % N_FX);
  resetFx();
  fxDirty = true; fxChangedMs = millis();
  Serial.printf("Effect %d/%d: %s\n", curFx + 1, N_FX, fxName(curFx));
}

static void fxButtonTick() {
  static bool stable = HIGH, lastRaw = HIGH, longDone = false;
  static uint32_t lastChange = 0, downAt = 0;
  uint32_t now = millis();
  bool raw = digitalRead(PIN_FXBTN);
  if (raw != lastRaw) { lastRaw = raw; lastChange = now; }
  if (now - lastChange > 30 && raw != stable) {
    stable = raw;
    markUsed();
    if (raw == LOW) { downAt = now; longDone = false; }
    else if (!longDone) {                                  // released
      if (stripOff) { stripOff = false; resetFx(); Serial.println("Strip on"); }
      else changeFx((now - downAt) < 700 ? +1 : -1);
    }
  }
  if (stable == LOW && !longDone && now - downAt >= 3000) {   // 3 s hold
    longDone = true;
    stripOff = !stripOff;
    if (!stripOff) resetFx();
    Serial.println(stripOff ? "Strip off" : "Strip on");
  }
}

// ============================ Stick centre ============================
static void setRange() {
  int m = min(min(cx, 4095 - cx), min(cy, 4095 - cy)) - 60;
  rng = m > 300 ? m : 300;
}
static void measureCentre() {
  long sx = 0, sy = 0;
  for (int i = 0; i < 32; i++) { sx += analogRead(PIN_X); sy += analogRead(PIN_Y); delay(2); }
  cx = sx / 32; cy = sy / 32;
  setRange();
}
static void recalibrate() {                         // hold the scroll button 3 s, hands off the stick
  measureCentre();
  prefs.putInt("cx", cx); prefs.putInt("cy", cy);
  Serial.printf("Centre recalibrated: %d, %d\n", cx, cy);
  flashStrip(CRGB(0, 255, 0), 3);
}

// ============================ Scroll button ============================
// Click = scroll mode on / off. Double tap = the pad's action (if set). Hold 3 s = recalibrate the centre.
static void toggleScroll() {
  scrollMode = !scrollMode; forceSend = true;
  Serial.println(scrollMode ? "Scroll mode ON" : "Scroll mode OFF");
}
// Hold + move the stick = precision mode (slow cursor) while held.
static void scrollButtonTick(int16_t x, int16_t y) {
  static bool stable = HIGH, lastRaw = HIGH, longDone = false, pending = false, movedWhile = false;
  static int16_t x0 = 0, y0 = 0;                           // where the stick was when the button went down
  static uint32_t lastChange = 0, downAt = 0, pendingAt = 0;
  const uint32_t now = millis();
  const bool hasDbl = gest & 4;
  bool raw = digitalRead(PIN_SCROLL);
  if (raw != lastRaw) { lastRaw = raw; lastChange = now; }
  if (now - lastChange > 30 && raw != stable) {
    stable = raw;
    markUsed();
    if (stable == LOW) {
      downAt = now; longDone = false; movedWhile = false; x0 = x; y0 = y;
      if (pending) { pending = false; longDone = true; sendEvent(EV_SDBL); }   // second tap = double tap
    } else if (!longDone && !movedWhile && now - downAt < 700) {
      if (hasDbl) { pending = true; pendingAt = now; }     // wait: maybe a double tap
      else toggleScroll();
    }
  }
  // moved since pressing (not just resting a bit off centre) = precision mode while held
  if (stable == LOW && (abs(x - x0) > 300 || abs(y - y0) > 300)) movedWhile = true;
  precise = stable == LOW && movedWhile;
  if (stable == LOW && !longDone && !movedWhile && now - downAt >= 3000) { longDone = true; recalibrate(); }
  if (pending && stable == HIGH && now - pendingAt >= DBL_MS) { pending = false; toggleScroll(); }
}

// ============================ Stick button gestures ============================
// No gestures set on the pad: the button is passed straight through (hold = hold).
// Gestures set: click, double click, long press, press and move = drag (button held while moving).
static void stickButtonTick(int16_t x, int16_t y) {
  static bool stable = HIGH, lastRaw = HIGH, decided = false, pending = false;
  static uint32_t lastChange = 0, downAt = 0, pendingAt = 0;
  const uint32_t now = millis();
  const bool hasDbl = gest & 1, hasLong = gest & 2, plain = !(gest & 3);
  bool raw = digitalRead(PIN_BTN);
  if (raw != lastRaw) { lastRaw = raw; lastChange = now; }
  if (now - lastChange > 15 && raw != stable) {
    stable = raw;
    markUsed(); forceSend = true; burst = 3;
    if (stable == LOW) {                                   // pressed
      downAt = now; decided = false;
      clickAt[clickIdx] = now ? now : 1; clickHue[clickIdx] = random8(); clickIdx = (clickIdx + 1) & 3;
      if (plain) { holdBtn = true; decided = true; }
      else if (pending) { pending = false; decided = true; sendEvent(EV_DBL); }
    } else {                                               // released
      if (holdBtn) holdBtn = false;
      else if (!decided) { if (hasDbl) { pending = true; pendingAt = now; } else sendEvent(EV_TAP); }
    }
  }
  if (stable == LOW && !decided) {
    if (abs(x) > DRAG_T || abs(y) > DRAG_T) { decided = true; pending = false; holdBtn = true; forceSend = true; }   // drag
    else if (now - downAt >= LONG_MS) {
      decided = true; pending = false;
      if (hasLong) sendEvent(EV_LONG); else { holdBtn = true; forceSend = true; }
    }
  }
  if (pending && stable == HIGH && now - pendingAt >= DBL_MS) { pending = false; sendEvent(EV_TAP); }
  stickDown = stable == LOW;
}

// ============================ Wireless link ============================
// ---- update over the air from the pad (pad code v5.3+)
// pad -> stick: magic 0xC0, op 1 hello, 2 begin (a = size, b = crc32), 3 data (a = offset, then the bytes), 4 end, 5 cancel
// stick -> pad: magic 0xC1, op (same), status (0 ok, 1 no room, 2 write error, 3 data changed, 4 check failed, 5 not started),
//               next = bytes received so far, ver = this code's version
struct __attribute__((packed)) OtaAck { uint8_t magic, op, status; uint32_t next; char ver[12]; };
static volatile bool otaRxReady = false;                 // one message from the pad waiting for loop()
static uint8_t       otaRx[250];
static volatile int  otaRxLen = 0;
static bool     otaActive = false;
static uint32_t otaSize = 0, otaGot = 0, otaCrc = 0, otaCrcWant = 0, otaLastAt = 0;
static uint32_t crc32Add(uint32_t c, const uint8_t* p, size_t n) {   // same CRC-32 as the pad
  c = ~c;
  while (n--) { c ^= *p++; for (uint8_t k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u))); }
  return ~c;
}

static volatile bool padMsg = false;
static PadPkt   rxPkt;
static uint8_t  rxMac[6];

static void onRecv(const esp_now_recv_info_t* info, const uint8_t* d, int len) {
  if (info && info->src_addr && len >= 2 && len <= (int)sizeof(otaRx) && d[0] == 0xC0) {   // update from the pad
    if (!padKnown || memcmp(info->src_addr, padMac, 6) != 0 || otaRxReady) return;       // only our own pad
    memcpy(otaRx, d, len); otaRxLen = len; otaRxReady = true;                            // handled in loop()
    return;
  }
  if (info && info->src_addr && len == (int)sizeof(PadInfo) && d[0] == 0xB8) {        // pad news
    if (padKnown && memcmp(info->src_addr, padMac, 6) == 0) { memcpy(&pinfo, d, sizeof pinfo); pinfoAt = millis(); }
    return;
  }
  if (!info || !info->src_addr || len != (int)sizeof(PadPkt) || padMsg) return;
  PadPkt p; memcpy(&p, d, sizeof p);
  if (p.magic != 0xB7) return;
  if (padKnown && memcmp(info->src_addr, padMac, 6) != 0) return;     // not our pad
  memcpy(rxMac, info->src_addr, 6); rxPkt = p; padMsg = true;          // handled in loop()
}

static void addPeer(const uint8_t* mac) {
  if (esp_now_is_peer_exist(mac)) return;
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = 0; peer.ifidx = WIFI_IF_STA; peer.encrypt = false;
  esp_now_add_peer(&peer);
}

static void applySettings(const PadPkt& p) {
  maxBright = p.bright;
  if (p.fx < N_FX && p.fx != curFx) { curFx = p.fx; resetFx(); }
  setLedCount(p.count);
  maxMa = constrain(p.maxMa, 50, 5000);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, maxMa);
  sleepAfterMs = (uint32_t)min<uint8_t>(p.sleepMin, 120) * 60000UL;
  HUE = p.hue; SPEED = p.speed; gest = p.gest;
  cfgId = p.cfgId;
  prefs.putUChar("br", maxBright); prefs.putUChar("fx", curFx); prefs.putUShort("n", nLeds);
  prefs.putUShort("ma", maxMa);    prefs.putUChar("sl", p.sleepMin); prefs.putUChar("hue", HUE);
  prefs.putUChar("sp", SPEED);     prefs.putUChar("gest", gest);   prefs.putUShort("cfg", cfgId);
  Serial.printf("Settings from the pad: %u LEDs, brightness %u, effect %s, sleep %u min\n",
                nLeds, maxBright, fxName(curFx), p.sleepMin);
}

static void padTick() {
  if (!padMsg) return;
  PadPkt p = rxPkt;
  if (!padKnown) {                                         // first pad that answers: remember it
    memcpy(padMac, rxMac, 6); padKnown = true;
    prefs.putBytes("pad", padMac, 6);
    addPeer(padMac);
    Serial.printf("Paired with pad %02X:%02X:%02X:%02X:%02X:%02X\n", padMac[0], padMac[1], padMac[2], padMac[3], padMac[4], padMac[5]);
    flashStrip(CRGB(0, 200, 255), 2);
  }
  if (p.cfgId != cfgId) applySettings(p);
  padMsg = false;
}

static void sendPkt(int16_t x, int16_t y, uint8_t flags) {
  JoyPkt2 p; p.magic = 0xA6; p.x = x; p.y = y; p.btn = flags; p.ev = evCode; p.evSeq = evSeq; p.cfgId = cfgId;
  p.fx = stripOff ? 255 : curFx;
  esp_now_send(padKnown ? padMac : BCAST, (uint8_t*)&p, sizeof p);   // direct to our pad once we know it
}

// ---- update: answer the pad, write what comes in, check it at the end and restart
static void otaReply(uint8_t op, uint8_t st) {
  OtaAck a = {}; a.magic = 0xC1; a.op = op; a.status = st; a.next = otaGot;
  strlcpy(a.ver, FW_VERSION, sizeof a.ver);
  esp_now_send(padMac, (const uint8_t*)&a, sizeof a);
}
static void otaStop(const char* why) {
  if (otaActive) Update.abort();
  otaActive = false; otaGot = 0;
  Serial.printf("Update stopped: %s\n", why);
  resetFx(); markUsed();
}
static void otaTick() {
  if (otaActive && millis() - otaLastAt > 15000) otaStop("the pad stopped sending");   // old code keeps running
  if (!otaRxReady) return;
  uint8_t b[sizeof otaRx]; const int n = otaRxLen;
  memcpy(b, otaRx, n); otaRxReady = false;
  switch (b[1]) {
    case 1: otaReply(1, 0); break;                                                       // hello: version
    case 2: {                                                                            // begin
      if (n < 10) return;
      uint32_t size, crc; memcpy(&size, b + 2, 4); memcpy(&crc, b + 6, 4);
      if (otaActive) Update.abort();
      otaGot = 0; otaCrc = 0; otaSize = size; otaCrcWant = crc; otaLastAt = millis();
      otaActive = Update.begin(size);
      if (!otaActive) { Serial.printf("Update: no room for %lu bytes (%s)\n", (unsigned long)size, Update.errorString()); otaReply(2, 1); break; }
      Serial.printf("Update: receiving %lu bytes...\n", (unsigned long)size);
      holdBtn = false; precise = false;
      otaReply(2, 0);
    } break;
    case 3: {                                                                            // data
      if (!otaActive) { otaReply(3, 5); break; }
      if (n < 7) return;
      uint32_t off; memcpy(&off, b + 2, 4);
      const uint32_t dl = n - 6;
      otaLastAt = millis();
      if (off == otaGot && otaGot + dl <= otaSize) {                                     // the next piece (repeats are just confirmed)
        if (Update.write(b + 6, dl) != dl) { otaReply(3, 2); otaStop("write error"); break; }
        otaCrc = crc32Add(otaCrc, b + 6, dl); otaGot += dl;
      }
      otaReply(3, 0);
    } break;
    case 4: {                                                                            // end: check and restart
      if (!otaActive) { otaReply(4, 5); break; }
      if (otaGot != otaSize || otaCrc != otaCrcWant) { otaReply(4, 3); otaStop("data changed on the way"); break; }
      if (!Update.end(true)) { otaActive = false; otaReply(4, 4); Serial.printf("Update: check failed (%s)\n", Update.errorString()); resetFx(); markUsed(); break; }
      otaReply(4, 0);
      Serial.println("Update: done, restarting");
      delay(100);
      flashStrip(CRGB(0, 255, 80), 2);
      ESP.restart();
    } break;
    case 5: if (otaActive) otaStop("cancelled by the pad"); break;
    default: break;
  }
}
static void otaDraw() {                                   // strip fills up purple, onboard LED blinks purple
  static uint32_t last = 0;
  if (millis() - last < 60) return;
  last = millis();
  const float f = otaSize ? (float)otaGot * nLeds / otaSize : 0;
  for (uint16_t i = 0; i < nLeds; i++)
    leds[i] = (i + 1 <= f) ? CRGB(140, 0, 255) : (i < f) ? CRGB(60, 0, 110) : CRGB(6, 0, 12);
  onboard[0] = ((millis() / 300) & 1) ? CRGB(140, 0, 255) : CRGB(0, 0, 0);
  FastLED.setBrightness(max<uint8_t>(30, maxBright));
  FastLED.show();
}

// ============================ Helpers ============================
static int readAxis(uint8_t pin) {                 // 4-sample average = less jitter, fewer packets
  int s = 0;
  for (uint8_t i = 0; i < 4; i++) s += analogRead(pin);
  return s >> 2;
}

static bool anyButtonDown() {
  return digitalRead(PIN_BTN) == LOW || digitalRead(PIN_SCROLL) == LOW || digitalRead(PIN_FXBTN) == LOW;
}

// ============================ Deep sleep ============================
static void goToSleep() {
  Serial.println("Not used for a while -> deep sleep (press any button to wake)");
  if (fxDirty) { prefs.putUChar("fx", curFx); fxDirty = false; }
  rtcValid = true; rtcScroll = scrollMode; rtcStripOff = stripOff;

  if (ready) {                                     // tell the pad: stick centred, button released
    holdBtn = false;
    for (uint8_t i = 0; i < 3; i++) { sendPkt(0, 0, 0); delay(5); }
    esp_now_deinit();
  }
  WiFi.mode(WIFI_OFF);

  FastLED.clear(true);                             // strip + onboard LED black
  delay(5);

  // 1) data line low first, so the strip can't power itself through DIN once its ground is cut
  rtc_gpio_init((gpio_num_t)PIN_LED);
  rtc_gpio_set_direction((gpio_num_t)PIN_LED, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_dis((gpio_num_t)PIN_LED);
  rtc_gpio_pulldown_en((gpio_num_t)PIN_LED);

  // 2) then switch the strip off and keep the transistor base pulled low during sleep
  if (PIN_LED_PWR >= 0) digitalWrite(PIN_LED_PWR, LOW);
  if (PIN_LED_PWR >= 0 && rtc_gpio_is_valid_gpio((gpio_num_t)PIN_LED_PWR)) {   // GPIO 0-7 only; others rely on the 10k
    rtc_gpio_init((gpio_num_t)PIN_LED_PWR);
    rtc_gpio_set_direction((gpio_num_t)PIN_LED_PWR, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_dis((gpio_num_t)PIN_LED_PWR);
    rtc_gpio_pulldown_en((gpio_num_t)PIN_LED_PWR);
  }

  // wake on any of the three buttons (all are LP GPIOs 0-7 on the C6)
  const uint64_t mask = (1ULL << PIN_BTN) | (1ULL << PIN_SCROLL) | (1ULL << PIN_FXBTN);
  esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ANY_LOW);
  const gpio_num_t wakePins[] = {(gpio_num_t)PIN_BTN, (gpio_num_t)PIN_SCROLL, (gpio_num_t)PIN_FXBTN};
  for (gpio_num_t g : wakePins) { rtc_gpio_pullup_en(g); rtc_gpio_pulldown_dis(g); }
#if SOC_PM_SUPPORT_RTC_PERIPH_PD
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#endif
  Serial.flush();
  esp_deep_sleep_start();
}

// ============================ setup / loop ============================
void setup() {
  setCpuFrequencyMhz(CPU_MHZ);
  Serial.begin(115200);

  const bool fromSleep = (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1);
  if (fromSleep) {                                 // pins were LP IO during sleep -> back to normal GPIO
    const gpio_num_t lp[] = {(gpio_num_t)PIN_BTN, (gpio_num_t)PIN_SCROLL, (gpio_num_t)PIN_FXBTN, (gpio_num_t)PIN_LED};
    for (gpio_num_t g : lp) rtc_gpio_deinit(g);
    if (PIN_LED_PWR >= 0 && rtc_gpio_is_valid_gpio((gpio_num_t)PIN_LED_PWR)) rtc_gpio_deinit((gpio_num_t)PIN_LED_PWR);
  }

  analogReadResolution(12);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_SCROLL, INPUT_PULLUP);
  pinMode(PIN_FXBTN, INPUT_PULLUP);

  // Scroll button held at power-on (and still held after 2 s) = forget the pad. No wait otherwise.
  bool forgetPad = false;
  if (!fromSleep && digitalRead(PIN_SCROLL) == LOW) {
    uint32_t t0 = millis(); forgetPad = true;
    while (millis() - t0 < 2000) { if (digitalRead(PIN_SCROLL) == HIGH) { forgetPad = false; break; } delay(10); }
  }

  prefs.begin("joy", false);
  if (forgetPad) { prefs.remove("pad"); prefs.remove("cfg"); Serial.println("Pad forgotten"); }
  padKnown = prefs.getBytes("pad", padMac, 6) == 6;
  uint8_t s = prefs.getUChar("fx", 0);
  if (s < N_FX) curFx = s;
  nLeds     = constrain(prefs.getUShort("n", DEF_LEDS), 1, MAX_LEDS);
  maxBright = prefs.getUChar("br", DEF_BRIGHT);
  maxMa     = prefs.getUShort("ma", DEF_MA);
  sleepAfterMs = (uint32_t)prefs.getUChar("sl", DEF_SLEEP) * 60000UL;
  HUE   = prefs.getUChar("hue", DEF_HUE);
  SPEED = prefs.getUChar("sp", DEF_SPEED);
  gest  = prefs.getUChar("gest", 0);
  cfgId = prefs.getUShort("cfg", 0);
  curBright = maxBright;
  if (rtcValid) { scrollMode = rtcScroll; stripOff = rtcStripOff; }

  // strip power on: data line low first, then switch the 2N2222A on and let the LEDs start up
  if (PIN_LED_PWR >= 0) {
    pinMode(PIN_LED, OUTPUT); digitalWrite(PIN_LED, LOW);
    pinMode(PIN_LED_PWR, OUTPUT); digitalWrite(PIN_LED_PWR, HIGH);
    delay(10);
  }

  // ignore the press that woke us: wait until all buttons are released (max 5 s)
  if (fromSleep) {
    uint32_t t0 = millis();
    while (anyButtonDown() && millis() - t0 < 5000) delay(10);
    Serial.println("Woke up from sleep");
  }

  stripCtl = &ADD_STRIP();
  ADD_ONBOARD();
  FastLED.setBrightness(maxBright);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, maxMa);
  resetFx();
  if (forgetPad) flashStrip(CRGB(0, 0, 255), 3);

  // stick centre: the saved one (from "hold scroll 3 s"), or measured now
  if (prefs.isKey("cx") && prefs.isKey("cy")) { cx = prefs.getInt("cx", 2048); cy = prefs.getInt("cy", 2048); setRange(); }
  else measureCentre();

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);  // must match the main board
  esp_wifi_set_max_tx_power(40);                   // about 10 dBm: lower current peaks, plenty for a stick next to the pad
  if (esp_now_init() == ESP_OK) {
    addPeer(BCAST);
    if (padKnown) addPeer(padMac);
    esp_now_register_recv_cb(onRecv);
    ready = true;
  } else {
    Serial.println("ESP-NOW init failed");
  }
  Serial.printf("Effect %d/%d: %s  |  %s\n", curFx + 1, N_FX, fxName(curFx), padKnown ? "pad known" : "looking for a pad");
  markUsed();
  helloUntil = millis() + 5000;
}

void loop() {
  static uint32_t lastJoy = 0, lastLed = 0;
  static uint8_t lastFlags = 0, lastFx = 0; static int16_t lx = 0, ly = 0;

  otaTick();
  if (otaActive) { otaDraw(); markUsed(); delay(1); return; }   // receiving an update: nothing else until it is done

  const int16_t x = readAxis(PIN_X) - cx, y = readAxis(PIN_Y) - cy;
  jx = constrain(x / rng, -1.0f, 1.0f); jy = constrain(y / rng, -1.0f, 1.0f);

  fxButtonTick();
  scrollButtonTick(x, y);
  stickButtonTick(x, y);
  padTick();

  if (stickDown || abs(x) > MOVE_THRESHOLD || abs(y) > MOVE_THRESHOLD) markUsed();

  if (ready) {
    uint8_t flags = (stickDown ? 1 : 0) | (scrollMode ? 2 : 0) | (millis() < helloUntil ? 4 : 0)
                  | (holdBtn ? 8 : 0) | (padKnown ? 0 : 16) | (precise ? 32 : 0);
    uint32_t t = millis();
    const uint8_t fxNow = stripOff ? 255 : curFx;
    bool moved = abs(x - lx) > 20 || abs(y - ly) > 20 || flags != lastFlags || fxNow != lastFx;   // effect changed: tell the pad now
    // 100 updates a second while moving, right away on a click, a keep-alive every 100 ms when idle.
    // After a click the next 3 packets go out quickly too (extra safety on top of the radio's own repeats).
    if (((forceSend || moved || burst) && t - lastJoy >= 10) || t - lastJoy >= 100) {
      if (burst && !forceSend && !moved) burst--;
      lastJoy = t; lastFlags = flags; lastFx = fxNow; lx = x; ly = y; forceSend = false;
      sendPkt(x, y, flags);
    }
  }

  uint32_t now = millis();
  uint32_t idle = now - lastUse;
  if (sleepAfterMs && idle >= sleepAfterMs) goToSleep();        // never returns (0 = never sleep)

  if (now - lastLed >= 20) {
    uint32_t dt = now - lastLed; if (dt > 100) dt = 100;
    lastLed = now;
    fdt = (dt * (SPEED + 16u)) >> 7;
    T += fdt;

    // dim after DIM_AFTER_MS idle: fade down slowly, come back quickly
    const bool warnSleep = sleepAfterMs && idle > sleepAfterMs - WARN_MS;
    uint8_t dimTo = min(DIM_BRIGHT, maxBright);
    uint8_t target = (warnSleep || idle <= DIM_AFTER_MS) ? maxBright : dimTo;   // full brightness for the warning
    if (curBright < target)      curBright = (uint8_t)min<int>(target, curBright + 10);
    else if (curBright > target) curBright = (uint8_t)max<int>(target, curBright - 2);
    const uint8_t night = infoFresh() ? pinfo.night : 255;               // the pad's night mode dims us too
    FastLED.setBrightness(max<uint8_t>(1, scale8(curBright, night)));

    if (stripOff) fill_solid(leds, nLeds, CRGB(0, 0, 0));
    else if (warnSleep) fill_solid(leds, nLeds, ((now / 250) & 1) ? CRGB(255, 80, 0) : CRGB(0, 0, 0));   // about to sleep: amber flash
    else { runFx(curFx); padOverlay(now); }                              // the pad's timer / battery / mode over the effect

    onboard[0] = precise ? CRGB(0, 0, 255) : scrollMode ? CRGB(255, 255, 255) : CRGB(0, 0, 0);   // blue = precision, white = scroll mode
    FastLED.show();
  }

  if (fxDirty && millis() - fxChangedMs > 2500) {  // remember the chosen effect after you stop pressing
    fxDirty = false;
    prefs.putUChar("fx", curFx);
  }
  delay(1);
}
