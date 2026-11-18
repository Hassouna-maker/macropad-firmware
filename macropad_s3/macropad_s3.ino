struct Action;   // forward declarations (stop the Arduino IDE auto-prototype errors)
struct ActPair;
struct Macro;
struct Step;
struct Alias;
struct ArForm;
struct TextLine;

/*
 * ESP32-S3 Wireless Macro Pad v5.5  (Arduino IDE / arduino-esp32 core 3.x)
 *
 * NEW in v5.8: the flasher's waiting screen shows what the USB host sees (to find out why a board isn't found).
 *  Built with USB CDC On Boot off: the pad's serial messages now come out of its COM port.
 *
 * NEW in v5.7: the USB flasher also finds a board that was plugged in before it started (and checks every 0.5 s).
 *
 * NEW in v5.6: the USB flasher also does ESP32-S3, ESP32-C3 and ESP32-C6 boards (on their UART port or on their
 *  own USB port), and merged files for those chips.
 *
 * NEW in v5.5: USB flasher for classic ESP32 boards.
 *  - Setup page, card "Flash a board on the USB port": save the board's file (.ino.bin, or .merged.bin for a
 *    blank board), then "Start flashing". The pad restarts as a USB flasher: plug the board into the pad's
 *    USB-C port marked USB (the S3's own port, not UART / COM). The OLED shows the progress; at the end the
 *    board restarts with its new code and the pad goes back to Bluetooth mode by itself.
 *  - Works with the usual USB chips on ESP32 boards: CP2102 / CP2104, CH340 / CH341 and CH9102 / CH343.
 *  - The board needs 5 V from that port: bridge the USB-OTG pads of the S3 board if it has them, or use an
 *    OTG adapter / Y-cable that brings in power.
 *  - Needs Tools > USB Mode "Hardware CDC and JTAG" (the default) on the pad.
 *
 * NEW in v5.4: small fixes.
 *  - Timer: pausing and going on keeps the progress bar where it was (it used to jump back to full).
 *  - Wireless stick: a one-off electrical spike can no longer make full speed / speed boost hard to reach.
 *    The furthest push it learned slowly settles back when the stick isn't pushed that far.
 *  - Setup page: the games list names all 7 games; comments say 5 s for the page shown after a press.
 *
 * NEW in v5.3: firmware updates from the setup page (no USB cable).
 *  - "Firmware update" card: one area for the main board file, one for the stick file.
 *    In the Arduino IDE: Sketch > Export Compiled Binary, then pick the file that ends in .ino.bin.
 *  - Main board file: checked (it must be an ESP32-S3 app), installed, then the pad restarts.
 *  - Stick file: checked (it must be an ESP32-C6 app) and kept on the pad. The next time the stick is on and
 *    left still for 2 s, the pad sends it over the wireless link (about a minute). The OLED and the stick's
 *    strip show the progress. Needs stick code v3.0 or newer (upload v3.0 by USB once).
 *  - Partition scheme must stay on "16M Flash (3MB APP/9.9MB FATFS)": the stick file is kept in the FATFS part.
 *
 * NEW in v5.2: "lock" (media:lock) now turns the iPad screen off. It sends the keyboard Power key, which iPadOS
 *  answers (the old Terminal Lock key was ignored). In PC mode "lock" still sends Win+L.
 *  The Bluetooth report map changed: on the iPad, forget "MacroPad" in Bluetooth settings and pair again.
 *
 * NEW in v5.1: speed boost (and the cursor speed) now work the same in all 4 directions.
 *  Most sticks can't reach the end of the range on one side (the ADC tops out before 3.3 V), so right / down
 *  never counted as "all the way". The pad now learns how far the stick really goes in each direction.
 *
 * NEW in v5.0 (wireless stick):
 *  - Speed boost: push the stick all the way and the cursor speeds up (up to 2x-5x, set in the card).
 *  - Faster clicks: a click is sent the moment you let go of the stick button. "doubleclick" as the
 *    stick's double click action is no longer needed (two quick clicks already double-click), so the stick
 *    no longer waits to see if a second click comes.
 *
 * NEW in v4.9: stopwatch on the timer button
 *  - Triple click the timer button = switch between timer and stopwatch.
 *  - Stopwatch: tap = start / pause / go on, double tap = back to 0:00. Shown on the matrix (timer effect),
 *    the OLED and the stick's strip (with stick code v2.8). The timer works as before.
 *
 * NEW in v4.8:
 *  - Setup page on your home Wi-Fi: in config mode the pad also joins the home Wi-Fi set in the Clock card.
 *    Open http://macropad.local (or the address on the OLED) from any device on that Wi-Fi.
 *    The pad's own "MacroPad-Setup" Wi-Fi still works too.
 *  - Small dot in the top right corner of the OLED while the wireless stick is connected.
 *
 * NEW in v4.7 (with wireless stick code v2.7):
 *  - The pad sends its news to the stick 5 times a second: timer, battery, iPad / PC mode, song progress
 *    and night mode. The stick's strip shows them by itself.
 *  - New strip effect "song progress": fills up as the song plays (colour per song), dims when paused.
 *  - Night mode (Clock card): the strip, matrix and stick dim between two hours you choose. Needs the clock's Wi-Fi.
 *  - Hold to repeat (each button): holding the button keeps repeating its action (e.g. volume up).
 *  - Precision mode from the stick (hold its scroll button and move = slow cursor).
 *
 * NEW in v4.6: instant start
 *  - No 3 s wait at power-on: Bluetooth and the wireless stick link start right away.
 *  - The clock gets the time in the background (the OLED says "GETTING TIME"), so the Wi-Fi no longer
 *    holds up Bluetooth. The stick link pauses for those few seconds and comes back by itself.
 *
 * NEW in v4.5 (with wireless stick code v2.5):
 *  - The OLED effect page also shows STICK FX (the wireless stick's effect), and pops up when you change
 *    the effect on the stick. Shown while the stick is on.
 *
 * NEW in v4.4 (with wireless stick code v2.4):
 *  - Direct link: the pad answers the stick, so the stick learns the pad by itself and sends only to it
 *    (with delivery confirmation). Older sticks still work.
 *  - Stick settings in the Wireless joystick card: LED brightness, effect, LED count, power limit, colour,
 *    speed and sleep time (0 = never). Sent to the stick after you save.
 *  - Stick button gestures: double click and long press (default: double click / right click), press and
 *    move = drag, and an action for a double tap on the stick's scroll button.
 *  - Smoother mouse: the stick is read up to 100 times a second (the speed setting feels the same).
 *
 * NEW in v4.3:
 *  - LED strip: up to 256 LEDs (was 32). Set "LED power limit" in the setup page to what your power can give.
 *
 * NEW in v4.2:
 *  - "Game speed" and "Text speed" in the Matrix card of the setup page (1 = slowest, 10 = fastest,
 *    3 = default, about half the old speed). Text speed is also used for game scores.
 *
 * NEW in v4.1:
 *  - Setup page: actions are checked while you type (e.g. "volupp" -> did you mean volup?), with a
 *    warning before saving. Backup / restore of the whole setup. The Wi-Fi password is never sent
 *    back to the page (leave it empty to keep the saved one).
 *  - 3 more matrix games: "tetris", "flappy", "invaders".
 *  - Timer: fading progress bar on every matrix size (each LED is a slice of the time and dims as it runs out).
 *
 * NEW in v4.0: battery
 *  - Set "Battery pin" in the setup page (ADC1: GPIO 1-10). Wiring: battery + -- 100k -- pin -- 100k -- GND.
 *  - Battery % in the bottom right corner of the clock screen, and in the iPad's Batteries widget.
 *  - 10 %: the strip and matrix flash red (3 s, then a short flash every 30 s).
 *  - 5 % for 30 s: "BATTERY EMPTY" on the OLED, then a sleep that only a restart (reset / power off-on) ends.
 *  - Battery saver (setup page, on by default): below 50 % the pad sleeps sooner (3 min at 50 % down to
 *    1 min at 10 %) and the LEDs get dimmer (down to 30 % brightness at 10 %).
 *
 * NEW in v3.9:
 *  - Matrix effects: "text" (scrolling text from the setup page, {time} and {song} work in it),
 *    "day color" (colour follows the time of day), "timer", "joystick" (a light that follows the stick)
 *    and 4 games played with the joystick: "snake", "pong", "breakout", "dodge".
 *    While a game is on the matrix the joystick plays the game instead of moving the mouse.
 *  - Timer button ("+ Timer button" in the setup page): tap = +1 min, hold = +5 s every 50 ms,
 *    double tap = -1 min. 3 s after the last press it starts. Press while it runs = back to setting.
 *    The matrix shows the timer by itself; when time is up the matrix and strip flash red until you press it.
 *    The pad doesn't go to sleep while a timer is set or running.
 *  - Strip effects: "key ripple", "encoder rainbow", "joystick", "typing heat", "day color".
 *
 * NEW in v3.8:
 *  - Media page: song progress bar with the time played and the song length (iPad / iPhone).
 *
 * NEW in v3.7:
 *  - Macros: several actions in one field, separated by commas, with pauses:
 *        selectall, copy, wait:200, appswitch, paste
 *  - Plain names for common actions (copy, paste, undo, home, play, volup, click...). Each one does
 *    the right thing in iPad mode and in PC mode by itself (see ALIASES below).
 *  - Keys are sent by a background task, so buttons and encoders keep being read while a key,
 *    a macro or a text is being sent.
 *  - Encoders are counted by the S3's hardware pulse counter (up to 4 encoders; more use software),
 *    so fast turns don't lose steps.
 *  - The wireless stick is paired: the pad remembers the first stick it hears and ignores all others.
 *    "Forget stick" in the setup page lets you pair another one.
 *
 * NEW in v3.6:
 *  - iPad / PC mode, switched automatically: if the connected device shares Apple's media info it is
 *    an iPad (or iPhone), otherwise a PC. Can also be fixed in the setup page, or switched with the
 *    action profile:toggle (profile:ipad / profile:pc). The OLED shows the mode when it changes and on
 *    the Status page. Write actions for the iPad: in PC mode gui (Cmd) becomes Ctrl automatically.
 *    For a completely different PC action write both:  key:gui+h | key:gui+d   (iPad | PC).
 *  - Long press and double tap actions on every button and encoder push-switch (optional).
 *    A button without them still acts the moment you press it.
 *
 * NEW in v3.5:
 *  - Media page shows only the song name (bigger), and long names scroll back and forth.
 *  - Clock: waits up to 30 s for the Wi-Fi and 15 s for the time server (was 15 s / 6 s).
 *  - Sleep after 6 minutes. In the last 10 s the strip and the matrix flash amber as a warning.
 *
 * NEW in v3.4:
 *  - Media page shows Arabic song titles and artists: letters are joined into their proper shapes,
 *    written right-to-left, and long names scroll smoothly (12 px Arabic + Latin font).
 *
 * NEW in v3.3:
 *  - OLED "Media" page: song title, artist, play / pause and a volume slider, read from an iPad or
 *    iPhone over Bluetooth (Apple Media Service). Shown for 3 s after a media key (volume, play,
 *    next...), when the song changes, or when the volume is changed on the iPad itself.
 *    Can also be picked as the page for every press. Windows / Android do not share this
 *    information: there the page shows which media key you pressed.
 *    Arabic titles are shown properly from v3.4.
 *
 * NEW in v3.2:
 *  - The pad only joins your Wi-Fi for the time when the OLED main screen is "Clock".
 *  - It does not reconnect on every mode change / wake: only when the time is unknown or more than
 *    an hour old. After a failed try it waits 15 min before trying again.
 *  - If Wi-Fi is not reached within 15 s, the OLED shows "UNABLE TO CONNECT" instead of the clock.
 *
 * NEW in v3.1:
 *  - OLED main screen "Movies": 4 short movies played one after another (MOON TRIP, NIGHT RUN,
 *    FISH TALE, SNOW DAY). Each starts with a typed title card and ends with a typed THE END.
 *  - "All animations" now changes every 8 s.
 *
 * NEW in v3.0:
 *  - OLED main screen choice in the web page: clock, a page, one of 13 animations, or all animations
 *    in turn (20 s each). Animations: stripes, rain, starfield, cube, waves, bounce, life, snow, tunnel,
 *    eyes, and three short looping scenes: rocket, night drive, fish tank.
 *  - New action oled:main = switch to the next main screen (clock -> animations -> cycle -> page).
 *
 * NEW in v2.9:
 *  - Two LED power switches: GPIO12 = strip, GPIO13 = matrix. Power is cut during deep sleep
 *    (black LEDs still draw ~1 mA each). Data lines are held low first and kept low during sleep.
 *    GPIO12 and 13 are reserved for this now (set a pin to -1 to free it).
 *
 * NEW in v2.8:
 *  - The OLED shows a full-screen clock. When you press a button, turn an encoder, click the joystick
 *    or change an effect, it shows the page chosen in the web page ("Page shown after a press")
 *    for 3 s, then goes back to the clock. Without Wi-Fi time it shows that page all the time.
 *
 * NEW in v2.7:
 *  - Bigger clock on the OLED.
 *  - The wireless joystick can wake the pad: while asleep the pad listens for the stick for ~0.4 s
 *    every 3 s (only when a wireless joystick is set up). Needs joystick sketch v2.1 or newer.
 *
 * NEW in v2.6:
 *  - Clock on the OLED: at power-on (and at most once an hour after waking up) the pad joins your
 *    home Wi-Fi for a few seconds, gets the time from the internet, then turns Wi-Fi off again.
 *    The time is shown in a bar at the top of every OLED page. Set Wi-Fi, timezone and 12/24 h in the web page.
 *  - Fn (control) button ("+ Fn button" in the web page). Hold it and press a button / turn an encoder
 *    to run that button's "with Fn held" action. Empty = the normal action runs. OLED shows FN while held.
 *  - Sleep after 10 minutes (6 minutes from v3.5).
 *
 *  - Own BLE HID layer (keyboard + media/iPad keys + MOUSE with vertical AND horizontal scroll) on NimBLE-Arduino 2.x.
 *  - Joystick -> mouse (or scroll): wired, or wireless over ESP-NOW ("+ Wireless joystick").
 *    Wireless stick: click the scroll button on the stick board to turn scroll mode on/off.
 *    Scroll mode ON: stick scrolls up/down/left/right (further push = faster). OFF: stick moves the cursor.
 *  - WS2812B strip (max 256 LEDs since v4.3) with 26 effects, WS2812B matrix (max 16x16) with 29 effects.
 *  - Built-in effect buttons: GPIO39 = strip, GPIO40 = matrix.
 *      quick press = next effect, hold 0.7-3 s and release = previous, hold 3 s = turn off / on.
 *  - 3 s wait at power-on so a booster/battery can settle before the radio starts.
 *  - OLED settings in the web UI, preset list for every action field.
 *
 * NEW in v2.5 (same as the wireless stick):
 *  - LEDs (strip + matrix) dim after 1 minute without use, back to full brightness on any use.
 *  - Deep sleep after 5 minutes without use. The OLED shows a 10 s countdown before sleeping.
 *    Wake up = press the mode button (GPIO21) or any button / encoder push-switch / joystick button
 *    wired to GPIO 1-18. (GPIO39/40 effect buttons and encoder turning CANNOT wake the board.)
 *    The press that wakes the board is ignored. Bluetooth reconnects by itself in 1-3 s.
 *  - Moving the wired or wireless joystick counts as use.
 *  - LED power switches (PIN_STRIP_PWR / PIN_MAT_PWR) cut LED power during sleep.
 *  - No 3 s start-up wait when waking from sleep. Strip/matrix on/off and effects are kept.
 *  - No sleep in config mode.
 *
 * Normal mode : BLE HID. Config mode : Wi-Fi AP + web page (http://192.168.4.1). Mode button toggles.
 *
 * NOTE: the HID report map changed in v2.4 (horizontal scroll). On the iPad/PC, forget "MacroPad" in
 *       Bluetooth settings and pair again, otherwise the old descriptor is cached.
 *
 * Libraries   : ArduinoJson (v7), FastLED (>=3.9), NimBLE-Arduino (2.x), U8g2
 * Board       : "ESP32S3 Dev Module", Flash 16MB, PSRAM "OPI PSRAM", USB CDC On Boot: Enabled.
 * Wiring      : mode button GPIO21 | strip fx button GPIO39 | matrix fx button GPIO40 (buttons to GND)
 *               strip data GPIO38 | matrix data GPIO47 | OLED SDA 41 / SCL 42
 *
 * LED power switches (low side, same idea as the stick board). HIGH = LEDs powered.
 *   Strip  : strip GND -> 2N2222A collector, emitter -> GND, GPIO12 -> 220R -> base, base -> 10k -> GND,
 *            GPIO38 -> 330R -> strip DIN.  A 2N2222A is fine for the strip (32 LEDs, max ~500 mA).
 *   Matrix : matrix GND -> logic-level N-MOSFET drain (IRLZ44N, AO3400...), source -> GND,
 *            GPIO13 -> 100R -> gate, gate -> 10k -> GND, GPIO47 -> 330R -> matrix DIN.
 *            Do NOT use a 2N2222A for the matrix: a 16x16 matrix can draw several amps.
 *
 * Action strings:
 *   key:ctrl+shift+a   key:gui+h   key:f5
 *   media:playpause next prev stop mute volup voldown ff rew home osk search brightup brightdown lock back
 *   mouse:left right middle double scrollup scrolldown
 *   led:next led:prev led:toggle mat:next mat:prev mat:toggle oled:next oled:toggle oled:main
 *   text:Hello world
 *   profile:toggle profile:ipad profile:pc
 *   Any action can be split for iPad | PC:  key:gui+h | key:gui+d
 */

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <esp_system.h>
#include <esp_sleep.h>
#include <time.h>
#include <esp_sntp.h>
#include <driver/rtc_io.h>
#include <driver/gpio.h>
#include <driver/pulse_cnt.h>
#include <vector>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <Update.h>
#include <FFat.h>
#include <MD5Builder.h>
#include <usb/usb_host.h>
#include <freertos/stream_buffer.h>

#define USE_OLED 1
#define OLED_HW_I2C 1      // 1 = hardware I2C (recommended), 0 = software I2C (old behaviour)
#if USE_OLED
  #include <U8g2lib.h>
  #include <Wire.h>
#endif

// ============================ Fixed hardware pins ============================
constexpr uint8_t  PIN_MODE_BTN  = 21;   // to GND, internal pull-up
constexpr uint8_t  PIN_STRIP_BTN = 39;   // strip effect button (to GND)
constexpr uint8_t  PIN_MAT_BTN   = 40;   // matrix effect button (to GND)
constexpr uint8_t  PIN_STRIP     = 38;   // WS2812B strip data
constexpr uint8_t  PIN_MATRIX    = 47;   // WS2812B matrix data
constexpr int8_t   PIN_STRIP_PWR = 12;   // strip power switch (2N2222A base), HIGH = strip on. -1 = none
constexpr int8_t   PIN_MAT_PWR   = 13;   // matrix power switch (MOSFET gate), HIGH = matrix on. -1 = none
                                         // these pins are blocked in the web UI
constexpr int      PIN_OLED_SDA  = 41;
constexpr int      PIN_OLED_SCL  = 42;
constexpr uint16_t MAX_STRIP     = 256;
constexpr uint8_t  MAX_W         = 16;
constexpr uint8_t  MAX_H         = 16;
constexpr uint16_t MAX_MAT       = MAX_W * MAX_H;
constexpr const char* AP_SSID    = "MacroPad-Setup";
constexpr const char* AP_PASS    = "macropad123";   // min 8 chars
constexpr uint8_t  MAX_COMPONENTS = 24;
constexpr size_t   MAX_JSON_BYTES = 3800;           // NVS string limit is ~4000 bytes

// ---- power saving
constexpr uint32_t SLEEP_AFTER_MS = 6UL * 60UL * 1000UL;  // deep sleep after 6 min without use
constexpr uint32_t PEEK_EVERY_MS  = 3000;                 // asleep + wireless stick: listen for it every 3 s
constexpr uint32_t CLOCK_RESYNC_S = 3600;                 // get the time again at most once an hour (on boot / wake)
constexpr uint32_t DIM_AFTER_MS   = 60UL * 1000UL;        // dim LEDs after 1 min without use
constexpr uint8_t  DIM_LEVEL      = 38;                   // dimmed brightness, 0-255 of normal (38 = 15 %)
constexpr uint32_t WARN_MS        = 10000;                // OLED countdown before sleep

// If FastLED gives a "pin invalid" / PARLIO error, change only these two lines (see notes in chat).
#define ADD_STRIP()   FastLED.addLeds<WS2812B, PIN_STRIP, GRB>(sOut, sCfg.count)
#define ADD_MATRIX()  FastLED.addLeds<WS2812B, PIN_MATRIX, GRB>(mOut, mCfg.w * mCfg.h)

// Pins the web UI may assign. Excluded: 0,3,45,46 (strapping), 19,20 (USB), 26-32 (flash/PSRAM),
// 33-37 (octal PSRAM), 41,42 (OLED), 21 (mode btn), 38 (strip), 39/40 (effect btns), 47 (matrix),
// 43,44 (UART), 48 (RGB). PIN_STRIP_PWR / PIN_MAT_PWR are blocked too when they are set.
static const uint8_t ALLOWED_PINS[] = {1,2,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};

static const char DEFAULT_JSON[] =
  "{\"name\":\"MacroPad\",\"power\":500,\"pmode\":0,\"pswap\":1,"
  "\"led\":{\"count\":8,\"bright\":60,\"hue\":160,\"fx\":\"breathe\",\"speed\":128},"
  "\"matrix\":{\"w\":0,\"h\":0,\"bright\":40,\"hue\":96,\"fx\":\"plasma\",\"speed\":128,\"serp\":1,\"vert\":0,\"flipX\":0,\"flipY\":0},"
  "\"oled\":{\"on\":1,\"bright\":128,\"rot\":0,\"sleep\":0,\"page\":0,\"home\":\"clock\",\"text\":\"MacroPad\"},"
  "\"clock\":{\"on\":1,\"h24\":1,\"ssid\":\"\",\"pass\":\"\",\"tz\":\"EET-2EEST,M3.5.0/0,M10.5.0/0\"},"
  "\"components\":["
  "{\"type\":\"button\",\"pin\":4,\"action\":\"key:ctrl+c\"},"
  "{\"type\":\"button\",\"pin\":5,\"action\":\"key:ctrl+v\"},"
  "{\"type\":\"button\",\"pin\":6,\"action\":\"media:playpause\"},"
  "{\"type\":\"button\",\"pin\":7,\"action\":\"media:mute\"},"
  "{\"type\":\"encoder\",\"a\":8,\"b\":9,\"div\":4,\"cw\":\"media:volup\",\"ccw\":\"media:voldown\","
  "\"sw\":10,\"swAction\":\"media:mute\"}]}";

// ============================ Effect names ============================
static const char* const STRIP_FX[] = {"breathe","static","rainbow","colorloop","scanner","meteor","twinkle",
                                       "confetti","fire","juggle","bpm","theater","running","aurora","wipe",
                                       "comet","sparkle","fireworks","chase","pride","lake","heartbeat",
                                       "police","candle","gradient","key ripple","encoder rainbow","joystick",
                                       "typing heat","day color","song progress","off"};
enum { SF_BREATHE, SF_STATIC, SF_RAINBOW, SF_COLORLOOP, SF_SCANNER, SF_METEOR, SF_TWINKLE, SF_CONFETTI,
       SF_FIRE, SF_JUGGLE, SF_BPM, SF_THEATER, SF_RUNNING, SF_AURORA, SF_WIPE,
       SF_COMET, SF_SPARKLE, SF_FIREWORKS, SF_CHASE, SF_PRIDE, SF_LAKE, SF_HEARTBEAT,
       SF_POLICE, SF_CANDLE, SF_GRADIENT, SF_RIPPLE, SF_ENCRAIN, SF_JOYLIGHT, SF_HEAT, SF_DAY, SF_SONG, SF_OFF };
constexpr uint8_t N_STRIP_FX = sizeof(STRIP_FX) / sizeof(STRIP_FX[0]);
static_assert(N_STRIP_FX == SF_OFF + 1, "strip effect names and enum do not match");

static const char* const MAT_FX[] = {"plasma","psfire","fire","rain","waves","lava","ripples","life","stars",
                                     "lissajous","rainbow","swirl","tunnel","radar","noise","ocean","checker",
                                     "bounce","fireworks","starfield","equalizer","heart","sinewave","orbits",
                                     "aurora","metaballs","fireflies","static","text","day color","timer",
                                     "joystick","snake","pong","breakout","dodge","tetris","flappy","invaders","off"};
enum { MF_PLASMA, MF_PSFIRE, MF_FIRE, MF_RAIN, MF_WAVES, MF_LAVA, MF_RIPPLES, MF_LIFE, MF_STARS,
       MF_LISSAJOUS, MF_RAINBOW, MF_SWIRL, MF_TUNNEL, MF_RADAR, MF_NOISE, MF_OCEAN, MF_CHECKER,
       MF_BOUNCE, MF_FIREWORKS, MF_STARFIELD, MF_EQ, MF_HEART, MF_SINEWAVE, MF_ORBITS,
       MF_AURORA, MF_METABALLS, MF_FIREFLIES, MF_STATIC, MF_TEXT, MF_DAY, MF_TIMER, MF_JOY,
       MF_SNAKE, MF_PONG, MF_BREAKOUT, MF_DODGE, MF_TETRIS, MF_FLAPPY, MF_INVADERS, MF_OFF };
static bool isGameFx(uint8_t fx) { return fx >= MF_SNAKE && fx <= MF_INVADERS; }
constexpr uint8_t N_MAT_FX = sizeof(MAT_FX) / sizeof(MAT_FX[0]);
static_assert(N_MAT_FX == MF_OFF + 1, "matrix effect names and enum do not match");

// OLED main screen: clock, page, cycle through animations, or one animation
static const char* const OANIM_NAMES[] = {"stripes","rain","starfield","cube","waves","bounce","life","snow",
                                          "tunnel","eyes","rocket","night drive","fish tank"};
enum { OA_STRIPES, OA_RAIN, OA_STARS, OA_CUBE, OA_WAVES, OA_BOUNCE, OA_LIFE, OA_SNOW, OA_TUNNEL, OA_EYES,
       OA_ROCKET, OA_DRIVE, OA_FISH };
constexpr uint8_t N_OANIM = sizeof(OANIM_NAMES) / sizeof(OANIM_NAMES[0]);
static_assert(N_OANIM == OA_FISH + 1, "OLED animation names and enum do not match");
constexpr int8_t HOME_CLOCK = -1, HOME_PAGE = -2, HOME_CYCLE = -3, HOME_MOVIES = -4;
constexpr uint32_t CYCLE_MS = 8000;                      // "all animations": each one runs this long
static volatile int8_t oHome = HOME_CLOCK;
static int8_t parseHome(const String& h) {
  if (h.equalsIgnoreCase("page"))  return HOME_PAGE;
  if (h.equalsIgnoreCase("cycle")) return HOME_CYCLE;
  if (h.equalsIgnoreCase("movies")) return HOME_MOVIES;
  for (uint8_t i = 0; i < N_OANIM; i++) if (h.equalsIgnoreCase(OANIM_NAMES[i])) return (int8_t)i;
  return HOME_CLOCK;
}

// ============================ Globals ============================
static Preferences prefs;
static WebServer   server(80);
static DNSServer   dns;
static bool        cfgMode = false;
static String      deviceName = "MacroPad";
static uint16_t    powerMA = 500;

struct StripCfg { uint16_t count = 8; uint8_t bright = 60, hue = 160, speed = 128; } sCfg;
static char matText[65] = "MacroPad";                   // matrix "text" effect ({time} and {song} are filled in)
static uint8_t matGameSpeed = 3, matTextSpeed = 3;        // setup page, 1 (slowest) .. 10 (fastest)
static float gameSpeedF() { return 0.15f * matGameSpeed; }   // 3 -> 0.45 x the original speed, 10 -> 1.5 x
static uint32_t gms(uint32_t ms) { return (uint32_t)(ms / gameSpeedF()); }   // a game interval, adjusted to the game speed
static int textScroll(uint32_t now, int w, int W) {        // x position of scrolling text (2.5 pixels / s per speed step)
  const uint32_t pos = (uint32_t)((uint64_t)now * matTextSpeed * 5 / 2000);
  return W - (int)(pos % (uint32_t)(w + W));
}

// inputs for the new LED effects and games
static volatile float    gStickX = 0, gStickY = 0;       // joystick -1..1 (right / down = positive)
static volatile bool     gStickBtn = false;               // joystick button (in games)
static volatile int32_t  encSpin = 0;                     // encoder steps (encoder rainbow)
static volatile uint16_t keyPresses = 0;                  // every action (key ripple, typing heat)

// timer (timer button + matrix "timer" effect)
enum : uint8_t { T_IDLE, T_SET, T_RUN, T_ALARM };
static volatile uint8_t  timerState = T_IDLE;
static volatile int32_t  timerSetMs = 0;                  // time being set (or left, when paused)
static volatile uint32_t timerEndAt = 0, timerTotalMs = 1, timerAlarmAt = 0, timerTouchAt = 0, timerPopupUntil = 0;
static int16_t           timerPrevFx = -1;                // matrix effect to go back to after the timer
static int32_t timerRemaining() {
  if (timerState == T_RUN) { int32_t r = (int32_t)(timerEndAt - millis()); return r > 0 ? r : 0; }
  return timerSetMs;
}
// stopwatch (timer button after a triple click)
enum : uint8_t { SW_IDLE, SW_RUN, SW_PAUSE };
static volatile bool     swMode = false;                  // the timer button works as a stopwatch
static volatile uint8_t  swState = SW_IDLE;
static volatile uint32_t swStartAt = 0, swAccum = 0;      // elapsed = swAccum (+ time since swStartAt while running)
static uint32_t swElapsed() { return swState == SW_RUN ? swAccum + (millis() - swStartAt) : swAccum; }
constexpr const char* FW_VERSION = "5.8";
static volatile bool sotaBusy = false;                    // sending a firmware file to the stick right now
static bool sleepAllowed() { return timerState == T_IDLE && swState == SW_IDLE && !sotaBusy; }   // never sleep with a timer / stopwatch / stick update going

// battery (measured through a 100k + 100k divider on an ADC1 pin)
static int8_t            batPin = -1;                     // -1 = no battery measured
static bool              batSaver = true;                 // below 50 %: sleep sooner, dimmer LEDs
static volatile int8_t   batPct = -1;                     // 0-100, -1 = unknown / no battery
static volatile bool     batDead = false;                 // 5 %: showing "battery empty" before the endless sleep
static volatile uint32_t batLowAt = 0;                    // when it dropped to 10 % (first red flashing)
static uint32_t sleepAfterMs() {                          // 6 min, or 3 min at 50 % down to 1 min at 10 % with battery saver
  const int p = batPct;
  if (!batSaver || p < 0 || p >= 50) return SLEEP_AFTER_MS;
  if (p <= 10) return 60000UL;
  return 60000UL + (uint32_t)(p - 10) * 120000UL / 40;
}
static uint8_t batBrightScale() {                         // 255 = full; down to 30 % at 10 % battery
  const int p = batPct;
  if (!batSaver || p < 0 || p >= 50) return 255;
  if (p <= 10) return 77;
  return (uint8_t)(77 + (p - 10) * (255 - 77) / 40);
}

struct MatCfg   { uint8_t w = 0, h = 0, bright = 40, hue = 96, speed = 128;
                  bool serp = true, vert = false, flipX = false, flipY = false; } mCfg;
struct OledCfg  { bool on = true; uint8_t bright = 128; bool rot = false; uint16_t sleepS = 0; uint8_t page = 0;
                  char text[37] = "MacroPad"; } oCfg;   // main screen is kept in oHome
static bool matOn = false;

// runtime state shared between loop() and the LED/OLED task
static volatile uint8_t  curSFx = 0, curMFx = 0, oledPage = 0, flashLevel = 0;
static volatile bool     oledOn = true, fxDirty = false, forceWhite = false, stripOff = false, matOff = false;
static volatile uint32_t fxChangedMs = 0, fxPopupUntil = 0, lastInput = 0;
static volatile uint32_t pagePopupUntil = 0;             // OLED shows the chosen page instead of the clock until then
static volatile bool     popupMedia = false;              // the popup is for a media key / song / volume change
static volatile uint16_t lastMediaKey = 0;                // last media key sent (bit mask, see parseMedia)

// iPad / PC mode
static volatile uint8_t  profile = 0;                     // 0 = iPad, 1 = PC
static volatile uint8_t  nightNow = 255;                  // night mode brightness scale right now (255 = day)
// config mode on the home Wi-Fi: 0 = not set, 1 = joining, 2 = on it, 3 = failed
static volatile uint8_t homeState = 0;
static char     homeIp[16] = "";
static uint32_t homeSince = 0;
static uint8_t           profileMode = 0;                 // setup page: 0 = automatic, 1 = always iPad, 2 = always PC
static bool              pcSwapCmd = true;                // in PC mode gui (Cmd) becomes Ctrl
static volatile bool     profileDirty = false;            // save the mode (done in loop)
static volatile uint32_t profilePopupUntil = 0;           // OLED shows "iPad mode" / "PC mode" until then
static void setProfile(uint8_t p, bool show) {
  profile = p ? 1 : 0;
  profileDirty = true;
  if (show) profilePopupUntil = millis() + 2000;
}
constexpr uint32_t POPUP_MS = 5000;                      // how long a press shows that page
static char lastLabel[21] = "";

// Fn (control) button: while it is held, buttons / encoders use their "with Fn held" action
static volatile uint8_t fnDown = 0;

// clock: time from the internet over the home Wi-Fi (default timezone = Beirut)
struct ClockCfg { bool on = true, h24 = true; bool night = false; uint8_t nfrom = 22, nto = 7, nlvl = 25;   // night mode: hours, brightness %
                  char ssid[33] = ""; char pass[65] = "";
                  char tz[48] = "EET-2EEST,M3.5.0/0,M10.5.0/0"; } cCfg;

// sleep
static volatile bool goSleep = false, sleepAck = false;   // loop() asks the LED task to black out, task confirms
static uint64_t wakeMask = 0;                             // pins that can wake the board (RTC GPIO 0-21)
RTC_DATA_ATTR static bool rtcValid = false;               // kept across deep sleep, lost on power-off
RTC_DATA_ATTR static bool rtcStripOff = false, rtcMatOff = false;
RTC_DATA_ATTR static uint64_t rtcWakeMask = 0;            // wake pins, kept so a quick "listen" wake can sleep again
RTC_DATA_ATTR static bool rtcRemote = false;              // a wireless joystick is configured
// clock sync state: RTC_NOINIT survives deep sleep AND restarts (mode button), unlike RTC_DATA_ATTR
enum : uint8_t { CK_NEVER = 0, CK_OK, CK_NOWIFI, CK_NOSERVER };
constexpr uint32_t CK_MAGIC = 0xC10C4B1D;
RTC_NOINIT_ATTR static uint32_t ckMagic;
RTC_NOINIT_ATTR static time_t ckLastOk, ckLastFail;      // last good sync / last failed try
RTC_NOINIT_ATTR static uint8_t ckState;                   // result of the last try (shown on the OLED)
constexpr uint32_t CLOCK_WIFI_TIMEOUT_MS = 30000;         // give up joining the Wi-Fi after 30 s
constexpr uint32_t CLOCK_NTP_TIMEOUT_MS  = 15000;         // give up waiting for the time server after 15 s
constexpr uint32_t CLOCK_RETRY_FAIL_S   = 900;            // after a failed try, wait 15 min before trying again
static bool timeValid() { return time(nullptr) > 1700000000; }   // true once the clock has been set

static void triggerFlash() { flashLevel = 255; }
static void markUsed() { lastInput = millis(); }
static void noteInput(const char* label) {
  lastInput = millis();
  keyPresses = keyPresses + 1;                            // key ripple / typing heat
  pagePopupUntil = millis() + POPUP_MS;                   // a press: show the chosen page for 5 s
  popupMedia = false;                                     // fire() sets it again for media keys
  if (label && label[0]) { strncpy(lastLabel, label, 20); lastLabel[20] = 0; }
}
// matrix switches to the timer while it is used, and back afterwards
static void timerShow() {
  if (matOn && curMFx != MF_TIMER) { timerPrevFx = curMFx; curMFx = MF_TIMER; }
}
static void timerHide() {                                  // matrix back to its effect
  if (timerPrevFx >= 0 && curMFx == MF_TIMER) curMFx = (uint8_t)timerPrevFx;
  timerPrevFx = -1;
}
static bool timerResume = false;                          // paused while running: going on keeps the full length
static void timerStop() {
  timerState = T_IDLE; timerSetMs = 0; timerPopupUntil = 0; timerResume = false;
  timerHide();
}
// called from loop(): starts the timer 3 s after the last press, rings when time is up
static void timerTick() {
  const uint32_t now = millis();
  if (timerState == T_SET && now - timerTouchAt >= 3000) {
    if (timerSetMs > 0) {
      timerTotalMs = (timerResume && (uint32_t)timerSetMs <= timerTotalMs) ? timerTotalMs : (uint32_t)timerSetMs;   // after a pause: same bar
      timerResume = false;
      timerState = T_RUN; timerEndAt = now + timerSetMs;
      timerPopupUntil = now + 2000;
    } else timerStop();
  } else if (timerState == T_RUN && timerRemaining() == 0) {
    timerState = T_ALARM; timerAlarmAt = now; timerShow(); markUsed();
  } else if (timerState == T_ALARM && now - timerAlarmAt > 60000) {
    timerStop();                                           // stop ringing after 1 minute
  }
}

// ms since last use (never negative, even if the other core just updated lastInput)
static int32_t idleMs() {
  int32_t d = (int32_t)(millis() - lastInput);
  return d < 0 ? 0 : d;
}

// ============================ BLE HID: keyboard + media + mouse ============================
// Report IDs: 1 keyboard, 2 media/consumer, 3 mouse (buttons, X, Y, wheel, horizontal pan)
static const uint8_t HID_MAP[] = {
  // ---- Keyboard (ID 1)
  0x05,0x01, 0x09,0x06, 0xA1,0x01, 0x85,0x01,
  0x05,0x07, 0x19,0xE0, 0x29,0xE7, 0x15,0x00, 0x25,0x01, 0x75,0x01, 0x95,0x08, 0x81,0x02,
  0x95,0x01, 0x75,0x08, 0x81,0x01,
  0x95,0x05, 0x75,0x01, 0x05,0x08, 0x19,0x01, 0x29,0x05, 0x91,0x02,
  0x95,0x01, 0x75,0x03, 0x91,0x01,
  0x95,0x06, 0x75,0x08, 0x15,0x00, 0x25,0x65, 0x05,0x07, 0x19,0x00, 0x29,0x65, 0x81,0x00,
  0xC0,
  // ---- Media / consumer (ID 2): 16 one-bit usages
  0x05,0x0C, 0x09,0x01, 0xA1,0x01, 0x85,0x02,
  0x15,0x00, 0x25,0x01, 0x75,0x01, 0x95,0x10,
  0x09,0xB5, 0x09,0xB6, 0x09,0xB7, 0x09,0xCD, 0x09,0xE2, 0x09,0xE9, 0x09,0xEA,   // next prev stop play mute vol+ vol-
  0x0A,0x23,0x02, 0x0A,0xAE,0x01, 0x0A,0x21,0x02,                                // AC Home, on-screen keyboard, AC Search
  0x09,0x6F, 0x09,0x70, 0x09,0xB3, 0x09,0xB4,                                    // brightness+ brightness- ff rew
  0x09,0x30, 0x0A,0x24,0x02,                                                     // power (iPad: screen off / lock), AC Back
  0x81,0x02, 0xC0,
  // ---- Mouse (ID 3)
  0x05,0x01, 0x09,0x02, 0xA1,0x01, 0x85,0x03, 0x09,0x01, 0xA1,0x00,
  0x05,0x09, 0x19,0x01, 0x29,0x05, 0x15,0x00, 0x25,0x01, 0x95,0x05, 0x75,0x01, 0x81,0x02,
  0x95,0x01, 0x75,0x03, 0x81,0x03,
  0x05,0x01, 0x09,0x30, 0x09,0x31, 0x09,0x38, 0x15,0x81, 0x25,0x7F, 0x75,0x08, 0x95,0x03, 0x81,0x06,
  0x05,0x0C, 0x0A,0x38,0x02, 0x95,0x01, 0x81,0x06,                               // horizontal scroll (AC Pan)
  0xC0, 0xC0
};

static bool asciiToHid(char c, uint8_t& code, bool& shift) {
  shift = false;
  if (c >= 'a' && c <= 'z') { code = 0x04 + (c - 'a'); return true; }
  if (c >= 'A' && c <= 'Z') { code = 0x04 + (c - 'A'); shift = true; return true; }
  if (c >= '1' && c <= '9') { code = 0x1E + (c - '1'); return true; }
  if (c == '0') { code = 0x27; return true; }
  static const char sh[] = "!@#$%^&*()";
  for (uint8_t i = 0; i < 10; i++) if (c == sh[i]) { code = (i == 9) ? 0x27 : 0x1E + i; shift = true; return true; }
  switch (c) {
    case '\n': code = 0x28; return true;
    case '\t': code = 0x2B; return true;
    case ' ':  code = 0x2C; return true;
    case '-':  code = 0x2D; return true;
    case '_':  code = 0x2D; shift = true; return true;
    case '=':  code = 0x2E; return true;
    case '+':  code = 0x2E; shift = true; return true;
    case '[':  code = 0x2F; return true;
    case '{':  code = 0x2F; shift = true; return true;
    case ']':  code = 0x30; return true;
    case '}':  code = 0x30; shift = true; return true;
    case '\\': code = 0x31; return true;
    case '|':  code = 0x31; shift = true; return true;
    case ';':  code = 0x33; return true;
    case ':':  code = 0x33; shift = true; return true;
    case '\'': code = 0x34; return true;
    case '"':  code = 0x34; shift = true; return true;
    case '`':  code = 0x35; return true;
    case '~':  code = 0x35; shift = true; return true;
    case ',':  code = 0x36; return true;
    case '<':  code = 0x36; shift = true; return true;
    case '.':  code = 0x37; return true;
    case '>':  code = 0x37; shift = true; return true;
    case '/':  code = 0x38; return true;
    case '?':  code = 0x38; shift = true; return true;
  }
  return false;
}

// ============================ Apple Media Service (iPad / iPhone: what is playing + volume) ============================
// iOS shares the current song and the volume with any bonded Bluetooth accessory through AMS.
// Once the iPad has paired and encrypted the link, a one-shot task looks for the service, subscribes to it
// and asks for the track (artist, title) and the player (play state, volume). Windows / Android don't have it.
static const char* const AMS_SVC = "89D3502B-0F36-433A-8EF4-C502AD55F8DC";
static const char* const AMS_EU  = "2F7CABCE-808D-411F-9A0C-BB92BA96C102";   // "entity update" characteristic
struct MediaInfo { char title[129]; char artist[129]; int8_t vol; int8_t state; bool have;
                  float elapsed, rate, dur; uint32_t infoAt; };   // position (s) at infoAt (ms), speed, song length (s)   // UTF-8   // vol 0-100 or -1, state 0 paused 1 playing
static MediaInfo media = {"", "", -1, -1, false};
static portMUX_TYPE mediaMux = portMUX_INITIALIZER_UNLOCKED;
static NimBLEServer* bleServer = nullptr;
static volatile uint16_t amsConn = 0xFFFF;
static volatile bool     amsBusy = false;
static volatile uint32_t amsQuietUntil = 0;              // the first values after subscribing don't pop up the page

// copy UTF-8 text without cutting a letter in half
static void copyUtf8(const uint8_t* src, size_t n, char* out, size_t outSz) {
  if (n > outSz - 1) {
    n = outSz - 1;
    while (n > 0 && (src[n] & 0xC0) == 0x80) n--;        // don't end in the middle of a letter
  }
  for (size_t i = 0; i < n; i++) out[i] = (src[i] < 0x20) ? ' ' : (char)src[i];
  out[n] = 0;
}

static void clearMedia() {
  portENTER_CRITICAL(&mediaMux);
  media.title[0] = 0; media.artist[0] = 0; media.vol = -1; media.state = -1; media.have = false;
  media.elapsed = 0; media.rate = 0; media.dur = 0; media.infoAt = 0;
  portEXIT_CRITICAL(&mediaMux);
}

// notification: [entity, attribute, flags, value...]  entity 0 = player, 2 = track
static void amsNotify(NimBLERemoteCharacteristic*, uint8_t* d, size_t len, bool) {
  if (len < 3) return;
  const uint8_t ent = d[0], attr = d[1];
  char buf[129]; copyUtf8(d + 3, len - 3, buf, sizeof buf);
  int num = -1;
  float rate = 0, elapsed = 0, dur = 0;
  if (ent == 0 && attr == 1) {                                                  // "state,rate,elapsed"
    char* q = buf; num = (int)strtol(q, &q, 10);
    if (*q == ',') { rate = strtof(q + 1, &q); if (*q == ',') elapsed = strtof(q + 1, &q); }
  }
  if (ent == 2 && attr == 3) dur = atof(buf);                                   // song length in seconds
  const uint32_t nowMs = millis();
  if (ent == 0 && attr == 2) { float f = atof(buf); num = (int)(f * 100.0f + 0.5f); if (num < 0) num = 0; if (num > 100) num = 100; }
  bool show = false;
  portENTER_CRITICAL(&mediaMux);
  if (ent == 2 && attr == 2) {
    if (strcmp(media.title, buf) != 0) {
      strcpy(media.title, buf); show = buf[0] != 0;
      if (nowMs - media.infoAt > 1000) { media.elapsed = 0; media.infoAt = nowMs; }   // new song starts at 0:00
    }
    media.have = true;
  }
  else if (ent == 2 && attr == 3) { media.dur = dur; }
  else if (ent == 2 && attr == 0) { strcpy(media.artist, buf); media.have = true; }
  else if (ent == 0 && attr == 1) { media.state = (int8_t)num; media.rate = rate; media.elapsed = elapsed; media.infoAt = nowMs; }
  else if (ent == 0 && attr == 2) { if (num != media.vol) { show = true; media.vol = (int8_t)num; } }
  portEXIT_CRITICAL(&mediaMux);
  if (show && (int32_t)(millis() - amsQuietUntil) > 0) {   // song or volume changed: show the Media page for 5 s
    lastMediaKey = 0;                                     // not a key press: show the play state instead
    popupMedia = true;
    pagePopupUntil = millis() + POPUP_MS;
  }
}

static void amsTask(void*) {
  vTaskDelay(pdMS_TO_TICKS(1500));                        // let the iPad finish setting up the keyboard first
  const uint16_t ch = amsConn;
  NimBLEClient* c = (bleServer && ch != 0xFFFF) ? bleServer->getClient(ch) : nullptr;
  const bool listed = c && !c->getServices(true).empty();  // could we read the device's services at all?
  NimBLERemoteService* svc = listed ? c->getService(NimBLEUUID(AMS_SVC)) : nullptr;
  if (listed && profileMode == 0) setProfile(svc ? 0 : 1, true);   // automatic: Apple media info = iPad, none = PC
  NimBLERemoteCharacteristic* eu = svc ? svc->getCharacteristic(NimBLEUUID(AMS_EU)) : nullptr;
  if (eu && eu->canNotify() && eu->subscribe(true, amsNotify)) {
    amsQuietUntil = millis() + 2500;
    const uint8_t track[]  = {2, 2, 3};                   // track: title, duration
    const uint8_t player[] = {0, 1, 2};                   // player: play state, volume
    eu->writeValue(track, sizeof track, true);
    eu->writeValue(player, sizeof player, true);
    Serial.println("Media info: connected (iPad / iPhone)");
  } else {
    Serial.println("Media info: not available on this device (only iPad / iPhone share it)");
  }
  amsBusy = false;
  vTaskDelete(nullptr);
}

class BleHid : public NimBLEServerCallbacks {
public:
  void begin(const char* name) {
    NimBLEDevice::init(name);
    NimBLEDevice::setSecurityAuth(true, false, true);         // bonding, no MITM (device has no display/keypad)
    NimBLEServer* srv = NimBLEDevice::createServer();
    bleServer = srv;
    srv->setCallbacks(this);
    hid = new NimBLEHIDDevice(srv);
    inKb    = hid->getInputReport(1);
    outKb   = hid->getOutputReport(1);
    inMedia = hid->getInputReport(2);
    inMouse = hid->getInputReport(3);
    hid->setManufacturer("DIY");
    hid->setPnp(0x02, 0xE502, 0xA111, 0x0220);
    hid->setHidInfo(0x00, 0x01);
    hid->setReportMap((uint8_t*)HID_MAP, sizeof(HID_MAP));
    hid->startServices();
    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->setAppearance(0x03C1);
    adv->addServiceUUID(hid->getHidService()->getUUID());
    adv->enableScanResponse(true);
    adv->start();
    hid->setBatteryLevel(100);
  }
  bool isConnected() const { return connected; }
  void setBattery(uint8_t p) { if (hid) hid->setBatteryLevel(p, true); }   // iPad Batteries widget

  void sendKeys(uint8_t mods, const uint8_t* keys, uint8_t n) {
    uint8_t r[8] = {mods, 0, 0, 0, 0, 0, 0, 0};
    for (uint8_t i = 0; i < n && i < 6; i++) r[2 + i] = keys[i];
    inKb->setValue(r, 8); inKb->notify();
  }
  void sendMedia(uint16_t mask) {
    uint8_t r[2] = {(uint8_t)(mask & 0xFF), (uint8_t)(mask >> 8)};
    inMedia->setValue(r, 2); inMedia->notify();
  }
  // wheel = vertical scroll, pan = horizontal scroll
  void mouseMove(int dx, int dy, int wheel, int pan = 0) {
    uint8_t r[5] = {mouseBtns, (uint8_t)(int8_t)dx, (uint8_t)(int8_t)dy, (uint8_t)(int8_t)wheel, (uint8_t)(int8_t)pan};
    inMouse->setValue(r, 5); inMouse->notify();
  }
  void mouseButton(uint8_t mask, bool down) {
    if (down) mouseBtns |= mask; else mouseBtns &= ~mask;
    mouseMove(0, 0, 0, 0);
  }
  void typeText(const char* s) {
    for (; *s; s++) {
      uint8_t code; bool sh;
      if (!asciiToHid(*s, code, sh)) continue;
      sendKeys(sh ? 0x02 : 0, &code, 1); delay(10);
      sendKeys(0, nullptr, 0);           delay(10);
    }
  }
  // NimBLE-Arduino 2.x callback signatures
  void onConnect(NimBLEServer* s, NimBLEConnInfo& info) override { connected = true; mouseBtns = 0; }
  void onDisconnect(NimBLEServer* s, NimBLEConnInfo& info, int reason) override {
    connected = false; mouseBtns = 0; amsConn = 0xFFFF; clearMedia(); NimBLEDevice::startAdvertising();
  }
  // link encrypted (after pairing, and on every reconnect): look for the iPad's media info
  void onAuthenticationComplete(NimBLEConnInfo& info) override {
    if (!info.isEncrypted() || amsBusy) return;
    amsConn = info.getConnHandle();
    amsBusy = true;
    if (xTaskCreate(amsTask, "ams", 6144, nullptr, 1, nullptr) != pdPASS) amsBusy = false;
  }
private:
  NimBLEHIDDevice* hid = nullptr;
  NimBLECharacteristic *inKb = nullptr, *outKb = nullptr, *inMedia = nullptr, *inMouse = nullptr;
  volatile bool connected = false;
  uint8_t mouseBtns = 0;
};
static BleHid* hid = nullptr;

// ============================ Actions ============================
enum ActType : uint8_t { ACT_NONE, ACT_KEYS, ACT_MEDIA, ACT_TEXT, ACT_MOUSE, ACT_LOCAL };
struct Action {
  ActType  type = ACT_NONE;
  uint8_t  mods = 0;
  uint8_t  keys[6] = {0};
  uint8_t  nkeys = 0;
  uint16_t media = 0;
  uint8_t  mouse = 0;      // 1 left 2 right 3 middle 4 scrollup 5 scrolldown 6 double
  uint8_t  local = 0;      // 1 led:next 2 led:prev 3 mat:next 4 mat:prev 5 oled:next 6 oled:toggle 7 led:toggle 8 mat:toggle
  String   text;
  char     label[21] = {0};
};

static void addKey(Action& a, uint8_t c) { if (a.nkeys < 6) a.keys[a.nkeys++] = c; }

static bool addToken(Action& a, String t) {
  t.trim(); t.toLowerCase();
  if (!t.length()) return false;
  if (t == "ctrl" || t == "control") { a.mods |= 0x01; return true; }
  if (t == "shift")                  { a.mods |= 0x02; return true; }
  if (t == "alt" || t == "opt" || t == "option") { a.mods |= 0x04; return true; }
  if (t == "gui" || t == "cmd" || t == "win" || t == "meta" || t == "super") { a.mods |= 0x08; return true; }
  if (t == "plus") { addKey(a, 0x2E); a.mods |= 0x02; return true; }
  struct NK { const char* n; uint8_t c; };
  static const NK map[] = {
    {"enter",0x28},{"esc",0x29},{"backspace",0x2A},{"tab",0x2B},{"space",0x2C},{"minus",0x2D},{"equal",0x2E},
    {"lbracket",0x2F},{"rbracket",0x30},{"backslash",0x31},{"semicolon",0x33},{"quote",0x34},{"grave",0x35},
    {"comma",0x36},{"period",0x37},{"slash",0x38},{"capslock",0x39},{"printscreen",0x46},{"insert",0x49},
    {"home",0x4A},{"pgup",0x4B},{"delete",0x4C},{"end",0x4D},{"pgdn",0x4E},
    {"right",0x4F},{"left",0x50},{"down",0x51},{"up",0x52}
  };
  for (const NK& k : map) if (t == k.n) { addKey(a, k.c); return true; }
  if (t[0] == 'f' && t.length() >= 2) {
    int n = t.substring(1).toInt();
    if (n >= 1 && n <= 12) { addKey(a, 0x3A + n - 1); return true; }
  }
  if (t.length() == 1) {
    uint8_t c; bool sh;
    if (asciiToHid(t[0], c, sh)) { addKey(a, c); if (sh) a.mods |= 0x02; return true; }
  }
  return false;
}

static uint16_t parseMedia(const String& n) {
  struct M { const char* n; uint16_t m; };
  static const M map[] = {
    {"next",1<<0},{"prev",1<<1},{"stop",1<<2},{"playpause",1<<3},{"mute",1<<4},{"volup",1<<5},{"voldown",1<<6},
    {"home",1<<7},{"osk",1<<8},{"search",1<<9},{"brightup",1<<10},{"brightdown",1<<11},
    {"ff",1<<12},{"rew",1<<13},{"lock",1<<14},{"back",1<<15}
  };
  for (const M& m : map) if (n == m.n) return m.m;
  return 0;
}

static Action parseAction(String s) {
  Action a; s.trim();
  strncpy(a.label, s.c_str(), 20); a.label[20] = 0;
  if (s.startsWith("key:")) {
    String rest = s.substring(4);
    int start = 0;
    while (start <= (int)rest.length()) {
      int p = rest.indexOf('+', start);
      if (p < 0) p = rest.length();
      addToken(a, rest.substring(start, p));
      start = p + 1;
    }
    if (a.mods || a.nkeys) a.type = ACT_KEYS;
  } else if (s.startsWith("media:")) {
    String n = s.substring(6); n.trim(); n.toLowerCase();
    a.media = parseMedia(n);
    if (a.media) a.type = ACT_MEDIA;
  } else if (s.startsWith("mouse:")) {
    String n = s.substring(6); n.trim(); n.toLowerCase();
    a.mouse = n == "left" ? 1 : n == "right" ? 2 : n == "middle" ? 3 : n == "scrollup" ? 4 : n == "scrolldown" ? 5 : n == "double" ? 6 : 0;
    if (a.mouse) a.type = ACT_MOUSE;
  } else if (s.startsWith("led:") || s.startsWith("mat:") || s.startsWith("oled:") || s.startsWith("profile:")) {
    s.toLowerCase();
    a.local = s == "led:next" ? 1 : s == "led:prev" ? 2 : s == "mat:next" ? 3 : s == "mat:prev" ? 4 :
              s == "oled:next" ? 5 : s == "oled:toggle" ? 6 : s == "led:toggle" ? 7 : s == "mat:toggle" ? 8 : s == "oled:main" ? 9 :
              s == "profile:toggle" ? 10 : s == "profile:ipad" ? 11 : s == "profile:pc" ? 12 : 0;
    if (a.local) a.type = ACT_LOCAL;
  } else if (s.startsWith("text:")) {
    a.text = s.substring(5);
    if (a.text.length()) a.type = ACT_TEXT;
  }
  return a;
}

// next/prev while a strip or matrix is off just turns it back on (without changing the effect)
static void runLocal(uint8_t l) {
  switch (l) {
    case 1: if (stripOff) stripOff = false; else curSFx = (curSFx + 1) % N_STRIP_FX; break;
    case 2: if (stripOff) stripOff = false; else curSFx = (curSFx + N_STRIP_FX - 1) % N_STRIP_FX; break;
    case 3: if (matOn) { if (matOff) matOff = false; else curMFx = (curMFx + 1) % N_MAT_FX; } break;
    case 4: if (matOn) { if (matOff) matOff = false; else curMFx = (curMFx + N_MAT_FX - 1) % N_MAT_FX; } break;
    case 5: oledPage = (oledPage + 1) % 5; return;
    case 6: oledOn = !oledOn; return;
    case 10: setProfile(profile ^ 1, true); return;        // iPad <-> PC (until the next connection in automatic mode)
    case 11: setProfile(0, true); return;
    case 12: setProfile(1, true); return;
    case 9: {                                              // next OLED main screen: clock -> animations -> cycle -> movies -> page
      int8_t h = oHome;
      if (h == HOME_CLOCK) h = 0;
      else if (h >= 0 && h < (int8_t)N_OANIM - 1) h++;
      else if (h == (int8_t)N_OANIM - 1) h = HOME_CYCLE;
      else if (h == HOME_CYCLE) h = HOME_MOVIES;
      else if (h == HOME_MOVIES) h = HOME_PAGE;
      else h = HOME_CLOCK;
      oHome = h;
      prefs.putChar("ohome", h);                           // remembered (cleared when the config is saved)
      return;
    }
    case 7: stripOff = !stripOff; break;
    case 8: if (matOn) matOff = !matOff; break;
    default: return;
  }
  fxDirty = true; fxChangedMs = millis(); fxPopupUntil = millis() + POPUP_MS;   // show effect name on the OLED
}

static void mouseClick(uint8_t mask) {
  hid->mouseButton(mask, true); delay(30); hid->mouseButton(mask, false);
}

// ---- Macros: a list of actions and pauses. Built once from the setup page text, run by the HID task.
struct Step  { Action a; uint16_t wait; };              // wait > 0: just pause that many ms
struct Macro { std::vector<Step> steps; char label[21] = {0}; uint16_t mediaKey = 0;
               bool any() const { return !steps.empty(); } };

// Plain names for common actions: { name, iPad action, PC action }
struct Alias { const char* name; const char* ipad; const char* pc; };
static const Alias ALIASES[] = {
  {"copy", "key:gui+c", "key:ctrl+c"},           {"paste", "key:gui+v", "key:ctrl+v"},
  {"cut", "key:gui+x", "key:ctrl+x"},            {"undo", "key:gui+z", "key:ctrl+z"},
  {"redo", "key:gui+shift+z", "key:ctrl+y"},     {"selectall", "key:gui+a", "key:ctrl+a"},
  {"find", "key:gui+f", "key:ctrl+f"},           {"save", "key:gui+s", "key:ctrl+s"},
  {"newtab", "key:gui+t", "key:ctrl+t"},         {"closetab", "key:gui+w", "key:ctrl+w"},
  {"reopentab", "key:gui+shift+t", "key:ctrl+shift+t"},
  {"home", "key:gui+h", "key:gui+d"},            {"appswitch", "key:gui+tab", "key:alt+tab"},
  {"search", "key:gui+space", "key:gui+s"},      {"screenshot", "key:gui+shift+3", "key:gui+shift+s"},
  {"lock", "media:lock", "key:gui+l"},           {"dock", "key:gui+alt+d", "key:gui"},
  {"keyboard", "media:osk", "media:osk"},
  {"enter", "key:enter", "key:enter"},           {"esc", "key:esc", "key:esc"},
  {"tab", "key:tab", "key:tab"},                 {"space", "key:space", "key:space"},
  {"backspace", "key:backspace", "key:backspace"}, {"delete", "key:delete", "key:delete"},
  {"up", "key:up", "key:up"},                    {"down", "key:down", "key:down"},
  {"left", "key:left", "key:left"},              {"right", "key:right", "key:right"},
  {"pageup", "key:pgup", "key:pgup"},            {"pagedown", "key:pgdn", "key:pgdn"},
  {"play", "media:playpause", "media:playpause"}, {"next", "media:next", "media:next"},
  {"prev", "media:prev", "media:prev"},          {"stop", "media:stop", "media:stop"},
  {"volup", "media:volup", "media:volup"},       {"voldown", "media:voldown", "media:voldown"},
  {"mute", "media:mute", "media:mute"},          {"brightup", "media:brightup", "media:brightup"},
  {"brightdown", "media:brightdown", "media:brightdown"},
  {"click", "mouse:left", "mouse:left"},         {"rightclick", "mouse:right", "mouse:right"},
  {"middleclick", "mouse:middle", "mouse:middle"}, {"doubleclick", "mouse:double", "mouse:double"},
  {"scrollup", "mouse:scrollup", "mouse:scrollup"}, {"scrolldown", "mouse:scrolldown", "mouse:scrolldown"},
};
static const Alias* findAlias(String t) {
  t.trim(); t.toLowerCase();
  for (const Alias& a : ALIASES) if (t == a.name) return &a;
  return nullptr;
}
// does this text start a new step? (used to split on commas, so "text:Hi, Omar" keeps its comma)
static bool stepStarts(String t) {
  t.trim(); t.toLowerCase();
  static const char* const pre[] = {"key:", "media:", "mouse:", "text:", "wait:", "led:", "mat:", "oled:", "profile:"};
  for (const char* p : pre) if (t.startsWith(p)) return true;
  int e = t.indexOf(',');
  return findAlias(e < 0 ? t : t.substring(0, e)) != nullptr;
}
// prof 0 = iPad, 1 = PC. explicitSide = the user wrote "iPad | PC", so no automatic Cmd -> Ctrl.
static void buildMacro(const String& text, uint8_t prof, bool explicitSide, Macro& m) {
  String t = text; t.trim();
  strlcpy(m.label, t.c_str(), sizeof m.label);
  int start = 0;
  for (int i = 0; i <= (int)t.length(); i++) {
    if (i < (int)t.length() && !(t[i] == ',' && stepStarts(t.substring(i + 1)))) continue;
    String part = t.substring(start, i); part.trim();
    start = i + 1;
    if (!part.length()) continue;
    String lo = part; lo.toLowerCase();
    if (lo.startsWith("wait:")) {                                        // pause, max 10 s
      int ms = constrain((int)part.substring(5).toInt(), 0, 10000);
      if (ms) m.steps.push_back({Action(), (uint16_t)ms});
      continue;
    }
    const Alias* al = findAlias(lo);
    Action a = parseAction(al ? String(prof ? al->pc : al->ipad) : part);
    if (!al && prof == 1 && !explicitSide && pcSwapCmd && a.type == ACT_KEYS && (a.mods & 0x08)) a.mods = (a.mods & ~0x08) | 0x01;
    if (a.type == ACT_MEDIA && !m.mediaKey) m.mediaKey = a.media;
    if (a.type != ACT_NONE) m.steps.push_back({a, 0});
  }
}

// A key press only puts the macro in this queue; the HID task sends it. So the loop keeps reading
// buttons and encoders while keys, macros or text are being sent.
static QueueHandle_t hidQueue = nullptr;
static void runAction(const Action& a);
static void hidTask(void*) {
  const Macro* m = nullptr;
  for (;;) {
    if (xQueueReceive(hidQueue, &m, portMAX_DELAY) != pdTRUE || !m) continue;
    for (const Step& st : m->steps) {
      if (st.wait) vTaskDelay(pdMS_TO_TICKS(st.wait));
      else runAction(st.a);
    }
  }
}

static void fire(const Macro& m) {
  if (!m.any()) return;
  noteInput(m.label);
  if (m.mediaKey) { lastMediaKey = m.mediaKey; popupMedia = true; }   // media keys show the Media page
  const Macro* p = &m;
  if (hidQueue) xQueueSend(hidQueue, &p, 0);               // queue full (16 waiting): this one is skipped
}

// runs in the HID task
static void runAction(const Action& a) {
  if (a.type == ACT_NONE) return;
  if (a.type == ACT_LOCAL) {
    runLocal(a.local);
    if (a.local == 9) pagePopupUntil = 0;              // show the new main screen right away
    if (a.local <= 4) triggerFlash();
    return;
  }
  if (!hid || !hid->isConnected()) return;
  switch (a.type) {
    case ACT_KEYS:
      hid->sendKeys(a.mods, a.keys, a.nkeys); delay(12);
      hid->sendKeys(0, nullptr, 0);
      break;
    case ACT_MEDIA:
      hid->sendMedia(a.media); delay(10);
      hid->sendMedia(0);
      break;
    case ACT_TEXT:
      hid->typeText(a.text.c_str());
      break;
    case ACT_MOUSE:
      if (a.mouse <= 3)       mouseClick(a.mouse == 1 ? 1 : a.mouse == 2 ? 2 : 4);
      else if (a.mouse == 4)  hid->mouseMove(0, 0, 1);
      else if (a.mouse == 5)  hid->mouseMove(0, 0, -1);
      else                    { mouseClick(1); delay(60); mouseClick(1); }
      break;
    default: break;
  }
  triggerFlash();
}

// ============================ Built-in effect buttons (GPIO39 strip / GPIO40 matrix) ============================
// Quick press = next effect. Hold 0.7-3 s and release = previous effect.
// Keep holding for 3 s = turn that strip/matrix off (or back on). While off, a quick press turns it back on.
struct FxBtn { uint8_t pin, nextAct, prevAct, offAct; bool stable, lastRaw; uint32_t lastChange, downAt; bool longDone; };
static FxBtn fxBtns[2] = {
  {PIN_STRIP_BTN, 1, 2, 7, HIGH, HIGH, 0, 0, false},
  {PIN_MAT_BTN,   3, 4, 8, HIGH, HIGH, 0, 0, false}
};
static void fxButtonsTick() {
  uint32_t now = millis();
  for (auto& b : fxBtns) {
    bool raw = digitalRead(b.pin);
    if (raw != b.lastRaw) { b.lastRaw = raw; b.lastChange = now; }
    if (now - b.lastChange > 25 && raw != b.stable) {
      b.stable = raw;
      markUsed();
      if (raw == LOW) { b.downAt = now; b.longDone = false; }
      else if (!b.longDone) {                                   // released
        runLocal((now - b.downAt) < 700 ? b.nextAct : b.prevAct);
        noteInput("effect"); triggerFlash();
      }
    }
    if (b.stable == LOW && !b.longDone && now - b.downAt >= 3000) {   // 3 s hold = off / on
      b.longDone = true; runLocal(b.offAct); noteInput("effect");
    }
  }
}

// ============================ Components ============================
class Component {
public:
  virtual ~Component() {}
  virtual void begin() = 0;
  virtual void update() = 0;
};

// picks the "with Fn held" action when the Fn button is held and that action is set
static const Macro& pickFn(const Macro& normal, const Macro& withFn) {
  return (fnDown && withFn.any()) ? withFn : normal;
}

// One macro for the iPad and one for the PC. "a | b" gives both; otherwise the same text is used for
// both (plain names pick the right shortcut themselves, gui becomes Ctrl in PC mode if set).
struct ActPair {
  Macro v[2];
  bool empty() const { return !v[0].any() && !v[1].any(); }
  const Macro& get() const { return v[profile ? 1 : 0]; }
};
static ActPair parsePair(const String& s) {
  ActPair p;
  int bar = s.indexOf('|');
  if (bar < 0) { buildMacro(s, 0, false, p.v[0]); buildMacro(s, 1, false, p.v[1]); }
  else         { buildMacro(s.substring(0, bar), 0, true, p.v[0]); buildMacro(s.substring(bar + 1), 1, true, p.v[1]); }
  return p;
}

// Button with optional long press (hold 0.5 s) and double tap (two presses within 0.3 s).
// Without them it acts the moment it is pressed. With them, a single press acts when released
// (or 0.3 s later if a double tap is set, to see if a second press comes).
class Button : public Component {
  static constexpr uint32_t LONG_MS = 500, DOUBLE_MS = 300, REP_DELAY = 400, REP_MS = 100;
  uint8_t pin; ActPair act, fnAct, longAct, dblAct; bool rep;
  bool stable = HIGH, lastRaw = HIGH, longDone = false, pending = false;
  uint32_t lastChange = 0, downAt = 0, pendingUntil = 0, lastRep = 0;
  void single() { fire(pickFn(act.get(), fnAct.get())); }
public:
  Button(uint8_t p, const ActPair& a, const ActPair& f, const ActPair& l, const ActPair& d, bool r = false)
    : pin(p), act(a), fnAct(f), longAct(l), dblAct(d), rep(r) {}
  void begin() override { pinMode(pin, INPUT_PULLUP); }
  void update() override {
    const uint32_t now = millis();
    if (rep) {                                             // hold to repeat: acts on press, then every 0.1 s after 0.4 s
      bool raw = digitalRead(pin);
      if (raw != lastRaw) { lastRaw = raw; lastChange = now; }
      if (now - lastChange > 20 && raw != stable) {
        stable = raw;
        if (stable == LOW) { downAt = now; lastRep = now; single(); }
      }
      if (stable == LOW && now - downAt >= REP_DELAY && now - lastRep >= REP_MS) { lastRep = now; markUsed(); single(); }
      return;
    }
    const bool hasLong = !longAct.empty(), hasDbl = !dblAct.empty();
    bool raw = digitalRead(pin);
    if (raw != lastRaw) { lastRaw = raw; lastChange = now; }
    if (now - lastChange > 20 && raw != stable) {
      stable = raw;
      if (stable == LOW) {                                 // pressed
        downAt = now; longDone = false;
        if (!hasLong && !hasDbl) single();                 // nothing to wait for: act right away
        else markUsed();
      } else if (hasLong || hasDbl) {                      // released
        if (longDone) {}                                   // the long press already ran
        else if (hasDbl && pending) { pending = false; fire(dblAct.get()); }
        else if (hasDbl) { pending = true; pendingUntil = now + DOUBLE_MS; }
        else single();
      }
    }
    if (hasLong && stable == LOW && !longDone && now - downAt >= LONG_MS) { longDone = true; pending = false; fire(longAct.get()); }
    if (pending && stable == HIGH && (int32_t)(now - pendingUntil) >= 0) { pending = false; single(); }
  }
};

// Fn (control) button: sends nothing by itself. Hold it and press a button / turn an encoder
// to run that button's "with Fn held" action.
class FnButton : public Component {
  uint8_t pin; bool stable = HIGH, lastRaw = HIGH; uint32_t lastChange = 0;
public:
  explicit FnButton(uint8_t p) : pin(p) {}
  void begin() override { pinMode(pin, INPUT_PULLUP); }
  void update() override {
    bool raw = digitalRead(pin);
    if (raw != lastRaw) { lastRaw = raw; lastChange = millis(); }
    if (millis() - lastChange > 20 && raw != stable) {
      stable = raw;
      markUsed();
      if (stable == LOW) fnDown = fnDown + 1;
      else if (fnDown)   fnDown = fnDown - 1;
    }
  }
};

// Timer button: tap = +1 min, hold = +5 s every 50 ms, double tap = -1 min.
// 3 s after the last press the timer starts. Press while it runs = back to setting. Press while it rings = stop.
// Triple click = switch to the stopwatch (and back). Stopwatch: tap = start / pause / go on, double tap = 0:00.
class TimerButton : public Component {
  uint8_t pin;
  bool stable = HIGH, lastRaw = HIGH, swallow = false, held = false, firstSwallowed = false;
  uint8_t taps = 0;
  uint32_t lastChange = 0, downAt = 0, lastRep = 0, pendingUntil = 0, firstAt = 0;
  static void touch() { timerTouchAt = millis(); timerPopupUntil = millis() + 2500; markUsed(); }
  static void add(int32_t ms) {
    int32_t v = timerSetMs + ms;
    if (v < 0) v = 0;
    if (v > 5999000) v = 5999000;                        // max 99:59
    timerSetMs = v; touch();
  }
  static void toggleMode() {                             // triple click: timer <-> stopwatch
    timerStop();
    swState = SW_IDLE; swAccum = 0;
    swMode = !swMode;
    if (swMode) timerShow();                             // matrix shows 0:00
    timerPopupUntil = millis() + 2000; markUsed();
    Serial.println(swMode ? "Timer button: stopwatch" : "Timer button: timer");
  }
  static void swTaps(uint8_t n, uint32_t at) {           // at = time of the first press, so start / pause is exact
    if (n >= 2) { swState = SW_IDLE; swAccum = 0; timerHide(); }                    // double tap: back to 0:00
    else if (swState == SW_RUN) { swAccum = swAccum + (at - swStartAt); swState = SW_PAUSE; }   // pause
    else { swStartAt = at; swState = SW_RUN; timerShow(); }                         // start / go on
    timerPopupUntil = millis() + 2500; markUsed();
  }
public:
  explicit TimerButton(uint8_t p) : pin(p) {}
  void begin() override { pinMode(pin, INPUT_PULLUP); }
  void update() override {
    const uint32_t now = millis();
    bool raw = digitalRead(pin);
    if (raw != lastRaw) { lastRaw = raw; lastChange = now; }
    if (now - lastChange > 20 && raw != stable) {
      stable = raw;
      if (stable == LOW) {                                 // pressed
        downAt = now; held = false; swallow = false;
        if (!taps) firstAt = now;
        if (!swMode) {
          if (timerState == T_ALARM)    { timerStop(); swallow = true; }
          else if (timerState == T_RUN) { timerSetMs = timerRemaining(); timerState = T_SET; timerResume = true; swallow = true; }   // pause: back to setting
          else if (timerState == T_IDLE){ timerState = T_SET; timerSetMs = 0; timerShow(); }
          touch();
          if (!taps) firstSwallowed = swallow;
        } else { timerPopupUntil = now + 2500; markUsed(); }
      } else {                                             // released
        if (!held) { taps++; pendingUntil = now + 350; }   // count quick presses (1, 2 or 3)
        if (!swMode && timerState == T_SET) touch();
      }
    }
    if (!swMode && stable == LOW && !swallow && timerState == T_SET && now - downAt >= 400) {   // held: +5 s every 50 ms
      if (!held) { held = true; taps = 0; lastRep = now - 50; }
      if (now - lastRep >= 50) { lastRep = now; add(5000); }
    }
    if (taps && stable == HIGH && (int32_t)(now - pendingUntil) >= 0) {   // no more presses: act on the count
      uint8_t n = taps; taps = 0;
      if (n >= 3) toggleMode();
      else if (swMode) swTaps(n, firstAt);
      else {
        if (firstSwallowed) n--;                           // the first press already paused / stopped
        if (n == 1) add(60000);                            // tap: +1 min
        else if (n == 2) add(-60000);                      // double tap: -1 min
      }
    }
  }
};

// Encoder counted by the S3's hardware pulse counter (PCNT): steps are counted even while the loop is busy.
// The S3 has 4 counters; a 5th encoder falls back to reading the pins in software.
// If an encoder turns the wrong way, swap its pin A and pin B in the setup page.
class Encoder : public Component {
  uint8_t pa, pb, div; ActPair cw, ccw, cwFn, ccwFn; Button* sw;
  pcnt_unit_handle_t unit = nullptr;
  int lastCount = 0, acc = 0;
  uint8_t state = 0;
  bool startCounter() {
    pcnt_unit_config_t uc = {};
    uc.low_limit = -30000; uc.high_limit = 30000;
    if (pcnt_new_unit(&uc, &unit) != ESP_OK) { unit = nullptr; return false; }
    pcnt_glitch_filter_config_t fc = {};
    fc.max_glitch_ns = 10000;                                              // ignore spikes shorter than 10 us
    pcnt_unit_set_glitch_filter(unit, &fc);
    pcnt_chan_config_t ca = {}; ca.edge_gpio_num = pa; ca.level_gpio_num = pb;
    pcnt_chan_config_t cb = {}; cb.edge_gpio_num = pb; cb.level_gpio_num = pa;
    pcnt_channel_handle_t chA = nullptr, chB = nullptr;
    if (pcnt_new_channel(unit, &ca, &chA) != ESP_OK || pcnt_new_channel(unit, &cb, &chB) != ESP_OK) {
      pcnt_del_unit(unit); unit = nullptr; return false;
    }
    // full quadrature: 4 counts per detent, like the software decoder
    pcnt_channel_set_edge_action(chA, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(chA, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    pcnt_channel_set_edge_action(chB, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(chB, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    gpio_pullup_en((gpio_num_t)pa); gpio_pullup_en((gpio_num_t)pb);
    pcnt_unit_enable(unit); pcnt_unit_clear_count(unit); pcnt_unit_start(unit);
    return true;
  }
public:
  Encoder(uint8_t a, uint8_t b, uint8_t d, const ActPair& c, const ActPair& cc, const ActPair& cf, const ActPair& ccf, Button* s)
    : pa(a), pb(b), div((uint8_t)constrain((int)(d ? d : 4), 1, 8)), cw(c), ccw(cc), cwFn(cf), ccwFn(ccf), sw(s) {}
  void begin() override {
    pinMode(pa, INPUT_PULLUP); pinMode(pb, INPUT_PULLUP);
    if (startCounter()) Serial.printf("Encoder %d/%d: hardware counter\n", pa, pb);
    else { Serial.printf("Encoder %d/%d: software (no free hardware counter)\n", pa, pb); state = (digitalRead(pa) << 1) | digitalRead(pb); }
    if (sw) sw->begin();
  }
  void update() override {
    if (unit) {
      int c = 0;
      pcnt_unit_get_count(unit, &c);
      acc += c - lastCount; lastCount = c;
      if (c > 20000 || c < -20000) { pcnt_unit_clear_count(unit); lastCount = 0; }   // keep away from the limits
    } else {
      static const int8_t tbl[16] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
      state = ((state << 2) | (digitalRead(pa) << 1) | digitalRead(pb)) & 0x0F;
      acc += tbl[state];
    }
    while (acc >= (int)div)  { acc -= div; encSpin = encSpin + 1; fire(pickFn(cw.get(), cwFn.get())); }
    while (acc <= -(int)div) { acc += div; encSpin = encSpin - 1; fire(pickFn(ccw.get(), ccwFn.get())); }
    if (sw) sw->update();
  }
};

class Pot : public Component {
  uint8_t pin; uint8_t steps; ActPair up, down;
  float filt = 0, committed = 0; bool primed = false; uint32_t lastRead = 0;
public:
  Pot(uint8_t p, uint8_t s, const ActPair& u, const ActPair& d)
    : pin(p), steps(constrain(s, 2, 100)), up(u), down(d) {}
  void begin() override {
    analogReadResolution(12);
    analogSetPinAttenuation(pin, ADC_11db);
    pinMode(pin, INPUT);
  }
  void update() override {
    if (millis() - lastRead < 10) return;
    lastRead = millis();
    float raw = analogRead(pin);
    if (!primed) { filt = committed = raw; primed = true; return; }
    filt += (raw - filt) * 0.2f;
    float w = 4096.0f / steps;
    float d = filt - committed;
    if (fabsf(d) >= w) {
      int n = (int)(d / w);
      committed += n * w;
      int cnt = min(abs(n), 8);
      for (int i = 0; i < cnt; i++) fire(n > 0 ? up.get() : down.get());
    }
  }
};

// ---- Wireless joystick (ESP-NOW)
// Old sticks (v2.3 and older) send JoyPkt (magic 0xA5). btn: bit0 = stick click, bit1 = scroll mode ON, bit2 = just woke up.
// Stick v2.4+ sends JoyPkt2 (magic 0xA6): also bit3 = hold the mouse button (drag / hold), bit4 = looking for a pad,
// ev / evSeq = its last gesture (evSeq counts up), cfgId = which settings it has.
// The pad answers with PadPkt (magic 0xB7): the stick learns the pad's address from it and gets its settings.
struct __attribute__((packed)) JoyPkt  { uint8_t magic; int16_t x, y; uint8_t btn; };
struct __attribute__((packed)) JoyPkt2 { uint8_t magic; int16_t x, y; uint8_t btn, ev, evSeq; uint16_t cfgId; uint8_t fx; };
struct __attribute__((packed)) PadPkt  { uint8_t magic; uint16_t cfgId; uint8_t bright, fx; uint16_t count, maxMa;
                                         uint8_t sleepMin, hue, speed, gest; };
// pad -> stick, 5 times a second while the stick is on: mode 0 = not connected, 1 = iPad, 2 = PC;
// timer = timerState (tLeft / tTotal in ms); bat -1 = unknown; song 0 none, 1 playing, 2 paused (sFrac 0-65535);
// night = brightness scale (255 = day).
struct __attribute__((packed)) PadInfo { uint8_t magic, mode, timer; uint32_t tLeft, tTotal; int8_t bat;
                                         uint8_t song; uint16_t sFrac; uint8_t sHue, night; };
enum { EV_NONE, EV_TAP, EV_DBL, EV_LONG, EV_SDBL };
static volatile int16_t  rjX = 0, rjY = 0;
static volatile uint8_t  rjBtn = 0, rjEv = 0, rjEvSeq = 0;
static volatile bool     rjV2 = false;                       // the stick sends gestures (v2.4+)
static volatile uint32_t rjLast = 0;
static volatile bool     stickWantsCfg = false;              // the stick doesn't have our settings yet
static volatile uint8_t  rjFx = 254;                          // the stick's effect: 255 = off, 254 = not known
// the stick's effect names (same order as in the stick code), for the OLED
static const char* const STICK_FX[] = {
  "solid", "blink", "breathe", "wipe", "wipe random", "random colors", "sweep", "dynamic",
  "colorloop", "rainbow", "scan", "dual scan", "fade", "theater chase", "theater rainbow", "running lights",
  "saw", "twinkle", "dissolve", "sparkle", "flash sparkle", "hyper sparkle", "strobe", "strobe rainbow",
  "blink rainbow", "chase", "chase rainbow", "chase flash", "comet", "fireworks", "rain", "fire 2012",
  "fire flicker", "candle", "colortwinkle", "meteor", "juggle", "bpm", "pride", "palette",
  "lake", "ripple", "plasma", "lightning", "gradient", "heartbeat", "noise", "aurora",
  "android", "bouncing balls", "sinelon", "popcorn", "larson scanner", "police", "tri wipe", "confetti",
  "joystick", "tilt color", "push meter", "warp", "click burst", "lava lamp", "ocean", "twin comets",
  "song progress", "off"};
constexpr uint8_t N_STICK_FX = sizeof(STICK_FX) / sizeof(STICK_FX[0]);
static bool needRemote = false;
static PadPkt  stickCfg = {};                                 // stick settings from the setup page
static ActPair rjClick, rjDbl, rjLong, rjSdbl;                // stick gestures -> actions

// Pairing: the pad remembers the first stick it hears (its MAC address) and ignores every other one.
// "Forget stick" in the setup page clears it, so the next stick that sends is paired instead.
static uint8_t       stickMac[6] = {0};
static volatile bool stickPaired = false, stickLearned = false;
static void loadStickMac() {
  Preferences pp;
  pp.begin("macropad", true);
  stickPaired = pp.getBytes("stickmac", stickMac, 6) == 6;
  pp.end();
}
static bool stickAllowed(const esp_now_recv_info_t* info) {
  if (!info || !info->src_addr) return false;
  if (stickPaired) return memcmp(info->src_addr, stickMac, 6) == 0;
  memcpy(stickMac, info->src_addr, 6);                     // first stick heard: pair with it
  stickPaired = true; stickLearned = true;                  // saved by loop()
  return true;
}

static void sotaRx(const esp_now_recv_info_t* info, const uint8_t* d, int len);
static void onJoyRecv(const esp_now_recv_info_t* info, const uint8_t* d, int len) {
  if (len >= 2 && d[0] == 0xC1) { sotaRx(info, d, len); return; }   // stick firmware update answer
  if (len == (int)sizeof(JoyPkt2) || len == (int)sizeof(JoyPkt2) - 1) {   // stick v2.5+ (v2.4 has no fx byte)
    JoyPkt2 p; p.fx = 254; memcpy(&p, d, len);
    if (p.magic != 0xA6 || !stickAllowed(info)) return;     // not our stick
    rjX = p.x; rjY = p.y; rjBtn = p.btn; rjEv = p.ev; rjEvSeq = p.evSeq; rjFx = p.fx; rjV2 = true; rjLast = millis();
    if (p.cfgId != stickCfg.cfgId) stickWantsCfg = true;    // answer it with our settings (from loop)
  } else if (len == (int)sizeof(JoyPkt)) {                  // older stick
    JoyPkt p; memcpy(&p, d, sizeof p);
    if (p.magic != 0xA5 || !stickAllowed(info)) return;
    rjX = p.x; rjY = p.y; rjBtn = p.btn; rjV2 = false; rjLast = millis();
  }
}
static void startRemoteJoy() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);   // must match the stick board
  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init failed"); return; }
  esp_now_register_recv_cb(onJoyRecv);
}
// How far the song is (0..1), if it is playing, and a colour from its title. false = nothing known.
static bool songProgress(float& frac, bool& playing, uint8_t& hue) {
  MediaInfo m;
  portENTER_CRITICAL(&mediaMux); m = media; portEXIT_CRITICAL(&mediaMux);
  if (!m.have || m.dur <= 0) return false;
  float pos = m.elapsed;
  if (m.state == 1 && m.rate > 0) pos += m.rate * (float)(millis() - m.infoAt) / 1000.0f;   // the iPad only sends changes
  frac = constrain(pos / m.dur, 0.0f, 1.0f);
  playing = m.state == 1;
  uint8_t h = 0; for (const char* c = m.title; *c; c++) h = (uint8_t)(h * 31 + (uint8_t)*c);
  hue = h;
  return true;
}
static void addStickPeer() {
  if (esp_now_is_peer_exist(stickMac)) return;
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, stickMac, 6);
  peer.channel = 0; peer.ifidx = WIFI_IF_STA; peer.encrypt = false;
  esp_now_add_peer(&peer);
}
// Tell the stick what the pad is doing (timer, battery, mode, song, night), 5 times a second while it is on.
static void stickInfoTick() {
  static uint32_t last = 0;
  const uint32_t now = millis();
  if (sotaBusy || !stickPaired || !rjV2 || now - rjLast > 1500 || now - last < 200) return;
  last = now;
  PadInfo p = {};
  p.magic = 0xB8;
  p.mode  = !(hid && hid->isConnected()) ? 0 : profile ? 2 : 1;
  p.timer = timerState;
  if (swMode && swState != SW_IDLE) { p.timer = swState == SW_RUN ? 4 : 5; p.tLeft = swElapsed(); } else   // stopwatch
  p.tLeft = timerState == T_RUN ? timerRemaining() : timerState == T_SET ? (uint32_t)((int32_t)timerSetMs > 0 ? (int32_t)timerSetMs : 0) : 0;
  p.tTotal = timerTotalMs;
  p.bat   = batPct;
  float fr; bool pl; uint8_t sh;
  if (songProgress(fr, pl, sh)) { p.song = pl ? 1 : 2; p.sFrac = (uint16_t)(fr * 65535); p.sHue = sh; }
  p.night = nightNow;
  addStickPeer();
  esp_now_send(stickMac, (const uint8_t*)&p, sizeof p);
}
// Answer the paired stick with its settings (this is also how the stick learns our address).
// Sent again every 250 ms until the stick reports it has them.
static void stickLinkTick() {
  static uint32_t lastSend = 0;
  if (sotaBusy || !stickWantsCfg || !stickPaired || millis() - lastSend < 250) return;
  lastSend = millis(); stickWantsCfg = false;
  addStickPeer();
  esp_now_send(stickMac, (const uint8_t*)&stickCfg, sizeof stickCfg);
}

// ---- Stick firmware update over ESP-NOW (stick code v3.0+)
// The stick file is uploaded in the setup page and kept on the pad (FATFS, /stick.bin). When the stick is on and
// left still for 2 s, a task sends it: hello -> begin (size, crc) -> data (240 bytes, each one confirmed) -> end.
// The stick writes it to its spare app slot, checks it and restarts. A failed or cut transfer changes nothing.
// pad -> stick: magic 0xC0, op 1 hello, 2 begin (a = size, b = crc32), 3 data (a = offset, then the bytes), 4 end, 5 cancel
// stick -> pad: magic 0xC1, op (same), status 0 = ok, next = bytes it has, ver = its firmware version
struct __attribute__((packed)) OtaHdr { uint8_t magic, op; uint32_t a, b; };
struct __attribute__((packed)) OtaAck { uint8_t magic, op, status; uint32_t next; char ver[12]; };
constexpr uint8_t OTA_CHUNK = 240;
static uint32_t crc32Add(uint32_t c, const uint8_t* p, size_t n) {   // same CRC-32 as the stick
  c = ~c;
  while (n--) { c ^= *p++; for (uint8_t k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u))); }
  return ~c;
}
static bool     ffatOk = false;                           // the FATFS part is mounted
static bool     sotaPending = false, sotaNoSupport = false;
static uint32_t sotaSize = 0, sotaCrc = 0, sotaRetryAt = 0;
static volatile int8_t   sotaPct = -1;
static volatile uint8_t  sotaResult = 0;                  // set by the task: 1 done, 2 stick too old, 3 failed
static volatile bool     sotaGotAck = false;
static volatile uint8_t  sotaAckOp = 0, sotaAckSt = 0;
static volatile uint32_t sotaAckNext = 0;
static char     stickVer[12] = "";                        // the stick's firmware version (from its answer)
static volatile bool stickVerNew = false;
static char     sotaL1[17] = "", sotaL2[17] = "";         // result on the OLED
static volatile uint32_t sotaMsgUntil = 0;

static void sotaRx(const esp_now_recv_info_t* info, const uint8_t* d, int len) {
  if (!info || !info->src_addr || !stickPaired || memcmp(info->src_addr, stickMac, 6) != 0) return;
  if (len < (int)sizeof(OtaAck)) return;
  OtaAck a; memcpy(&a, d, sizeof a);
  a.ver[sizeof a.ver - 1] = 0;
  if (a.ver[0] && strcmp(a.ver, stickVer) != 0) { strlcpy(stickVer, a.ver, sizeof stickVer); stickVerNew = true; }
  sotaAckOp = a.op; sotaAckSt = a.status; sotaAckNext = a.next; sotaGotAck = true;
}
// send and wait for the stick's answer to that op
static bool sotaCall(const uint8_t* p, size_t n, uint8_t op, uint32_t waitMs, uint8_t tries) {
  for (uint8_t t = 0; t < tries; t++) {
    sotaGotAck = false;
    esp_now_send(stickMac, p, n);
    const uint32_t t0 = millis();
    while (millis() - t0 < waitMs) {
      if (sotaGotAck && sotaAckOp == op) return true;
      vTaskDelay(1);
    }
  }
  return false;
}
static void sotaFail(const char* l2) {
  OtaHdr c = {0xC0, 5, 0, 0};
  esp_now_send(stickMac, (const uint8_t*)&c, sizeof c);     // tell the stick to drop what it got
  strlcpy(sotaL1, "FAILED", sizeof sotaL1); strlcpy(sotaL2, l2, sizeof sotaL2);
  sotaResult = 3;
}
static void stickOtaTask(void*) {
  addStickPeer();
  OtaHdr h = {0xC0, 1, 0, 0};
  File f;
  if (!sotaCall((const uint8_t*)&h, sizeof h, 1, 200, 10)) {   // no answer: stick code older than v3.0
    strlcpy(sotaL1, "STICK NEEDS", sizeof sotaL1); strlcpy(sotaL2, "v3.0 BY USB", sizeof sotaL2);
    sotaResult = 2; goto done;
  }
  f = FFat.open("/stick.bin", FILE_READ);
  if (!f || f.size() != sotaSize) { sotaFail("FILE MISSING"); goto done; }
  h = {0xC0, 2, sotaSize, sotaCrc};
  if (!sotaCall((const uint8_t*)&h, sizeof h, 2, 3000, 3)) { sotaFail("NO ANSWER"); goto done; }
  if (sotaAckSt) { sotaFail("FILE TOO BIG"); goto done; }
  {
    uint8_t pkt[6 + OTA_CHUNK];
    pkt[0] = 0xC0; pkt[1] = 3;
    uint32_t off = 0; uint8_t fails = 0;
    while (off < sotaSize) {
      const uint32_t n = min<uint32_t>(OTA_CHUNK, sotaSize - off);
      memcpy(pkt + 2, &off, 4);
      if (!f.seek(off) || f.read(pkt + 6, n) != n) { sotaFail("READ ERROR"); goto done; }
      if (!sotaCall(pkt, 6 + n, 3, 400, 1)) { if (++fails > 25) { sotaFail("STICK GONE"); goto done; } continue; }
      if (sotaAckSt) { sotaFail("WRITE ERROR"); goto done; }
      const uint32_t nx = sotaAckNext;
      if (nx == off + n) { off = nx; fails = 0; }
      else if (nx <= sotaSize && nx % OTA_CHUNK == 0) { off = nx; if (++fails > 25) { sotaFail("OUT OF STEP"); goto done; } }
      else if (++fails > 25) { sotaFail("OUT OF STEP"); goto done; }
      sotaPct = (int8_t)((uint64_t)off * 100 / sotaSize);
      markUsed();
    }
  }
  h = {0xC0, 4, 0, 0};
  if (!sotaCall((const uint8_t*)&h, sizeof h, 4, 8000, 2)) { sotaFail("NO ANSWER"); goto done; }
  if (sotaAckSt) { sotaFail(sotaAckSt == 3 ? "DATA CHANGED" : "CHECK FAILED"); goto done; }
  sotaPct = 100;
  vTaskDelay(pdMS_TO_TICKS(4000));                        // the stick restarts with the new code
  stickVer[0] = 0;
  h = {0xC0, 1, 0, 0};
  sotaCall((const uint8_t*)&h, sizeof h, 1, 300, 10);     // ask its new version
  strlcpy(sotaL1, "DONE", sizeof sotaL1);
  snprintf(sotaL2, sizeof sotaL2, "STICK v%s", stickVer[0] ? stickVer : "?");
  sotaResult = 1;
done:
  if (f) f.close();
  sotaMsgUntil = millis() + 6000;
  sotaPct = -1; markUsed();
  sotaBusy = false;
  vTaskDelete(nullptr);
}
// from loop(): ask the stick its version once, start the update when the stick is on and still for 2 s
static void stickOtaTick() {
  if (!needRemote || !stickPaired || !rjV2 || sotaBusy) return;
  const uint32_t now = millis();
  const bool fresh = rjLast && now - rjLast < 300;
  static uint8_t asked = 0; static uint32_t lastAsk = 0;
  if (fresh && !stickVer[0] && asked < 3 && now - lastAsk > 1000) {   // version for the setup page
    asked++; lastAsk = now;
    addStickPeer();
    OtaHdr h = {0xC0, 1, 0, 0};
    esp_now_send(stickMac, (const uint8_t*)&h, sizeof h);
  }
  if (!sotaPending || sotaNoSupport || (int32_t)(now - sotaRetryAt) < 0) return;
  static uint32_t quietSince = 0;
  const bool quiet = fresh && !(rjBtn & 0x29) && abs(rjX) < 250 && abs(rjY) < 250;
  if (!quiet) { quietSince = 0; return; }
  if (!quietSince) { quietSince = now; return; }
  if (now - quietSince < 2000) return;
  quietSince = 0;
  sotaBusy = true; sotaPct = 0; sotaResult = 0; markUsed();
  Serial.println("Stick update: sending...");
  if (xTaskCreatePinnedToCore(stickOtaTask, "sota", 6144, nullptr, 1, nullptr, 1) != pdPASS) sotaBusy = false;
}
// from loop(): keep the result (flash / prefs are only touched here)
static void stickOtaResult() {
  if (stickVerNew) { stickVerNew = false; prefs.putString("sver", stickVer); }
  const uint8_t r = sotaResult;
  if (!r) return;
  sotaResult = 0;
  char msg[48];
  if (r == 1) {
    FFat.remove("/stick.bin"); prefs.putUChar("sbpend", 0); sotaPending = false;
    snprintf(msg, sizeof msg, "done: the stick runs v%s", stickVer[0] ? stickVer : "?");
  } else if (r == 2) {
    sotaNoSupport = true;                               // don't try again until the next start
    snprintf(msg, sizeof msg, "the stick needs code v3.0 by USB once");
  } else {
    sotaRetryAt = millis() + 60000;                     // try again in a minute
    snprintf(msg, sizeof msg, "failed (%s), tries again", sotaL2);
  }
  prefs.putString("sbmsg", msg);
  Serial.printf("Stick update: %s\n", msg);
}

// Analog joystick -> mouse movement (or scroll). Wired (ADC1 pins 1-10) or wireless (ESP-NOW).
// Wired: keep the stick centred while the pad boots.
class Joystick : public Component {
  uint8_t px, py; int swPin; float sens, deadPct; bool invX, invY, scroll; uint8_t btnMask;
  bool remote; bool remoteDown = false, remoteHold = false, remoteScroll = false, evInit = false; uint8_t lastSeq = 0;
  int cx = 2048, cy = 2048; float rng = 1800;
  float fx = 0, fy = 0, ax = 0, ay = 0, sv = 0, sh = 0;
  float boostMax = 1, boostCur = 1;                         // speed boost when the stick is pushed all the way
  float pk[4] = {1300, 1300, 1300, 1300};                   // how far the stick really goes: +x, -x, +y, -y (learned)
  float reach(float v, uint8_t axis) {                      // learns the furthest push in this direction and returns it
    const uint8_t i = axis * 2 + (v < 0 ? 1 : 0);
    float& o = pk[i ^ 1];                                   // the other direction slowly settles back (about 2 min),
    if (o > 1300) o -= (o - 1300) * 0.0001f;                // so a one-off spike can't block full speed for good
    float& p = pk[i];
    const float a = fabsf(v);
    if (a > p) p = a;                                       // pushed further: learned at once
    else if (a < p * 0.5f && p > 1300) p -= (p - 1300) * 0.0001f;
    return p;
  }
  uint32_t last = 0, lastChange = 0; bool swRaw = HIGH, swStable = HIGH;
  void hold_(bool h, bool conn) {                          // wireless stick: press / release the mouse button
    if (h == remoteHold) return;
    remoteHold = h;
    if (conn) hid->mouseButton(btnMask, h);
  }
  float shape(float v, float dz, float full) {              // full = the furthest push in this direction
    float a = fabsf(v);
    if (a < dz) return 0;
    if (full < dz + 100) full = dz + 100;
    float n = (a - dz) / (full - dz);
    if (n > 1) n = 1;
    float o = n * n * sens;
    return v < 0 ? -o : o;
  }
public:
  Joystick(uint8_t x, uint8_t y, int sw, int sensv, int dead, bool ix, bool iy, bool sc, uint8_t bm, int boost = 1)
    : px(x), py(y), swPin(sw), sens(constrain(sensv, 1, 40)), deadPct(constrain(dead, 0, 60)),
      invX(ix), invY(iy), scroll(sc), btnMask(bm), remote(x == 255) { boostMax = constrain(boost, 1, 5); }
  void begin() override {
    if (remote) return;
    analogReadResolution(12);
    analogSetPinAttenuation(px, ADC_11db); analogSetPinAttenuation(py, ADC_11db);
    pinMode(px, INPUT); pinMode(py, INPUT);
    if (swPin >= 0) pinMode(swPin, INPUT_PULLUP);
    delay(20);
    long sx = 0, sy = 0;
    for (int i = 0; i < 32; i++) { sx += analogRead(px); sy += analogRead(py); delay(2); }
    cx = sx / 32; cy = sy / 32;
    int m = min(min(cx, 4095 - cx), min(cy, 4095 - cy)) - 60;
    rng = m > 300 ? m : 300;
  }
  void update() override {
    uint32_t now = millis();
    const bool game = matOn && !matOff && !cfgMode && isGameFx(curMFx);   // a game on the matrix uses the stick
    const bool conn = hid && hid->isConnected();
    bool fresh = remote && (now - rjLast < 300);          // stick board silent for 300 ms = treat as released
    if (remote) {
      const bool down = fresh && (rjBtn & 1);                                  // the button itself (games)
      const bool hold = fresh && (rjV2 ? (rjBtn & 8) : (rjBtn & 1));           // keep the mouse button pressed
      if (down != remoteDown) {
        remoteDown = down;
        if (game) { gStickBtn = down; if (down) markUsed(); }
      }
      if (game) hold_(false, conn);
      else if (hold != remoteHold) { hold_(hold, conn); if (hold) { noteInput("mouse click"); triggerFlash(); } }
      if (fresh && rjV2) {                                 // gestures from the stick: one per new evSeq
        const uint8_t seq = rjEvSeq;
        if (!evInit) { evInit = true; lastSeq = seq; }
        else if (seq != lastSeq) {
          lastSeq = seq; markUsed();
          if (!game) switch (rjEv) {
            case EV_TAP:  fire(rjClick.get()); break;
            case EV_DBL:  fire(rjDbl.get());   break;
            case EV_LONG: fire(rjLong.get());  break;
            case EV_SDBL: fire(rjSdbl.get());  break;
            default: break;
          }
        }
      }
      bool sc = rjBtn & 2;                                 // scroll mode toggled on the stick = use
      if (fresh && sc != remoteScroll) { remoteScroll = sc; markUsed(); }
    } else if (swPin >= 0) {
      bool raw = digitalRead(swPin);
      if (raw != swRaw) { swRaw = raw; lastChange = now; }
      if (now - lastChange > 20 && raw != swStable) {
        swStable = raw;
        if (game) { gStickBtn = swStable == LOW; if (swStable == LOW) markUsed(); }
        else {
          if (conn) hid->mouseButton(btnMask, swStable == LOW);
          if (swStable == LOW) { noteInput("mouse click"); triggerFlash(); }
        }
      }
    }
    if (now - last < 8) return;                            // up to ~100 reads a second
    float k = last ? (now - last) / 15.0f : 1.0f;          // movement per read scaled to time: same speed at any rate
    if (k > 3) k = 3;
    last = now;
    float rx, ry;
    if (remote) { rx = fresh ? rjX : 0; ry = fresh ? rjY : 0; }
    else        { rx = analogRead(px) - cx; ry = analogRead(py) - cy; }
    fx += (rx - fx) * 0.4f;
    fy += (ry - fy) * 0.4f;
    const float fullX = reach(fx, 0), fullY = reach(fy, 1);
    float nx = fx / fullX, ny = fy / fullY;               // -1..1 for the games and the joystick light (same in every direction)
    if (nx > 1) nx = 1;
    if (nx < -1) nx = -1;
    if (ny > 1) ny = 1;
    if (ny < -1) ny = -1;
    gStickX = invX ? -nx : nx; gStickY = invY ? -ny : ny;
    if (game) {                                            // playing: the stick doesn't move the mouse
      if (fabsf(nx) > 0.3f || fabsf(ny) > 0.3f) markUsed();
      return;
    }
    if (!conn) return;
    float dz = rng * deadPct / 100.0f;
    const bool precise = remote && fresh && rjV2 && (rjBtn & 32);
    if (precise) k *= 0.25f;                               // precision mode (scroll button held on the stick)
    // speed boost: pushed all the way (92 %+) -> speeds up to boostMax over ~0.4 s, back to normal at once when eased off
    const bool edge = (fabsf(nx) > 0.9f || fabsf(ny) > 0.9f) && !precise && !scroll && !(remote && (rjBtn & 2));
    if (edge && boostMax > 1) boostCur += (boostMax - boostCur) * min(1.0f, k * 0.11f);   // k * 15 = ms since last read
    else boostCur = 1;
    k *= boostCur;
    float dx = shape(fx, dz, fullX) * k, dy = shape(fy, dz, fullY) * k;
    if (dx != 0 || dy != 0) markUsed();                    // stick pushed past the deadzone = use
    if (invX) dx = -dx;
    if (invY) dy = -dy;
    if (scroll || (remote && fresh && (rjBtn & 2))) {      // scroll mode: web page setting, or toggled on the stick board
      sv += -dy * 0.12f;                                   // up/down (own memory, so mouse leftovers don't leak in)
      sh +=  dx * 0.12f;                                   // left/right
      int w = (int)sv; sv -= w;
      int p = (int)sh; sh -= p;
      ax = ay = 0;
      if (w || p) hid->mouseMove(0, 0, constrain(w, -127, 127), constrain(p, -127, 127));
    } else {
      ax += dx; ay += dy;
      int mx = (int)ax, my = (int)ay;
      ax -= mx; ay -= my;
      sv = sh = 0;
      if (mx || my) hid->mouseMove(constrain(mx, -127, 127), constrain(my, -127, 127), 0, 0);
    }
  }
};

static Component* comps[MAX_COMPONENTS];
static uint8_t    ncomps = 0;

// ============================ LEDs ============================
static CRGB sBuf[MAX_STRIP], sOut[MAX_STRIP];            // sBuf = effect canvas, sOut = scaled output
static CRGB mBuf[MAX_MAT + 1], mOut[MAX_MAT];            // mBuf has 1 spare slot for out-of-range pixels
static uint32_t sT = 0, mT = 0;                          // effect clocks (scaled by speed)

// Beat helper driven by our own clock: returns lo..hi, 'bpm' cycles per minute
static uint8_t tbeat(uint32_t t, uint8_t bpm, uint8_t lo, uint8_t hi) {
  uint8_t ph = (uint8_t)(((uint64_t)t * bpm) / 234);
  return lo + (uint8_t)(((uint16_t)sin8(ph) * (uint8_t)(hi - lo)) >> 8);
}

// ---- matrix coordinate mapping: logical (0,0) = top-left as you look at it
static uint16_t XY(int x, int y) {
  if (x < 0 || y < 0 || x >= mCfg.w || y >= mCfg.h) return MAX_MAT;
  if (mCfg.flipX) x = mCfg.w - 1 - x;
  if (mCfg.flipY) y = mCfg.h - 1 - y;
  if (mCfg.vert)  return x * mCfg.h + ((mCfg.serp && (x & 1)) ? mCfg.h - 1 - y : y);
  return y * mCfg.w + ((mCfg.serp && (y & 1)) ? mCfg.w - 1 - x : x);
}
#define FOR_XY for (uint8_t y = 0; y < H; y++) for (uint8_t x = 0; x < W; x++)

// angle of (dx,dy) mapped to 0..255
static inline uint8_t ang8(float dy, float dx) {
  return (uint8_t)(((int)((atan2f(dy, dx) + PI) * 40.584f)) & 0xFF);
}

// ---- colour of the time of day (needs the clock; without it, midday)
static CRGB dayColor(int* minuteOut = nullptr) {
  int m = 12 * 60;
  if (timeValid()) { time_t t = time(nullptr); struct tm lt; localtime_r(&t, &lt); m = lt.tm_hour * 60 + lt.tm_min; }
  if (minuteOut) *minuteOut = m;
  struct K { int16_t m; uint8_t r, g, b; };
  static const K keys[] = {
    {0, 90, 20, 4}, {300, 90, 22, 6}, {390, 255, 110, 30}, {480, 255, 190, 120}, {600, 200, 220, 255},
    {960, 230, 230, 255}, {1080, 255, 160, 60}, {1170, 170, 60, 150}, {1290, 140, 40, 10}, {1440, 90, 20, 4}};
  for (uint8_t i = 0; i + 1 < sizeof(keys) / sizeof(keys[0]); i++) {
    if (m >= keys[i].m && m <= keys[i + 1].m) {
      uint8_t f = (uint8_t)((m - keys[i].m) * 255 / (keys[i + 1].m - keys[i].m));
      return blend(CRGB(keys[i].r, keys[i].g, keys[i].b), CRGB(keys[i + 1].r, keys[i + 1].g, keys[i + 1].b), f);
    }
  }
  return CRGB(200, 220, 255);
}

// ---- strip effects
static uint8_t heat1[MAX_STRIP];
static void stripFx(uint8_t fx, uint16_t n) {
  const uint32_t t = sT; const uint8_t hue = sCfg.hue;
  switch (fx) {
    case SF_BREATHE:   fill_solid(sBuf, n, CHSV(hue, 255, tbeat(t, 10, 25, 110))); break;
    case SF_STATIC:    fill_solid(sBuf, n, CHSV(hue, 255, 255)); break;
    case SF_RAINBOW:   fill_rainbow(sBuf, n, t / 60, max(1, 255 / n)); break;
    case SF_COLORLOOP: fill_solid(sBuf, n, CHSV(t / 40, 255, 255)); break;
    case SF_SCANNER: {
      fadeToBlackBy(sBuf, n, 70);
      uint16_t pos = (triwave8(t / 5) * n) >> 8;
      sBuf[pos] = CHSV(hue, 255, 255);
    } break;
    case SF_METEOR: {
      for (uint16_t i = 0; i < n; i++) if (random8() < 90) sBuf[i].nscale8(190);
      uint16_t head = (t / 45) % (n + 6);
      for (int j = 0; j < 4; j++) {
        int p = (int)head - j;
        if (p >= 0 && p < n) sBuf[p] = CHSV(hue, j ? 255 : 140, 255 - j * 50);
      }
    } break;
    case SF_TWINKLE:
      fadeToBlackBy(sBuf, n, 40);
      if (random8() < 20 + (sCfg.speed >> 1)) sBuf[random16(n)] = CHSV(hue + random8(40), 200, 255);
      break;
    case SF_CONFETTI:
      fadeToBlackBy(sBuf, n, 12);
      if (random8() < 90) sBuf[random16(n)] += CHSV(t / 50 + random8(64), 200, 255);
      break;
    case SF_FIRE: {
      static uint32_t last = 0;
      if (t - last >= 25) {
        last = t;
        uint16_t cl = 550 / n + 2; if (cl > 255) cl = 255;
        for (uint16_t i = 0; i < n; i++) heat1[i] = qsub8(heat1[i], random8(0, cl));
        for (int k = n - 1; k >= 2; k--) heat1[k] = (heat1[k - 1] + heat1[k - 2] + heat1[k - 2]) / 3;
        if (random8() < 120) { uint8_t y = random8(n < 7 ? n : 7); heat1[y] = qadd8(heat1[y], random8(160, 255)); }
      }
      for (uint16_t i = 0; i < n; i++) sBuf[i] = HeatColor(heat1[i]);
    } break;
    case SF_JUGGLE: {
      fadeToBlackBy(sBuf, n, 20);
      uint8_t dh = 0;
      for (uint8_t i = 0; i < 8; i++) { sBuf[(uint16_t)tbeat(t, i + 7, 0, 255) * (n - 1) / 255] |= CHSV(dh, 255, 255); dh += 32; }
    } break;
    case SF_BPM: {
      uint8_t beat = tbeat(t, 62, 64, 255), g = t / 40;
      for (uint16_t i = 0; i < n; i++) sBuf[i] = ColorFromPalette(PartyColors_p, g + i * 2, (uint8_t)(beat - g + i * 10));
    } break;
    case SF_THEATER: {
      uint8_t q = (t / 90) % 3;
      for (uint16_t i = 0; i < n; i++) sBuf[i] = ((i + q) % 3 == 0) ? CRGB(CHSV(hue, 255, 255)) : CRGB(0, 0, 0);
    } break;
    case SF_RUNNING:
      for (uint16_t i = 0; i < n; i++) sBuf[i] = CHSV(hue, 255, sin8(i * 32 - (t >> 2)));
      break;
    case SF_AURORA:
      for (uint16_t i = 0; i < n; i++) {
        uint8_t a = inoise8(i * 50, t >> 2), b = inoise8(i * 30 + 5000, t >> 3);
        sBuf[i] = CHSV(hue + (b >> 2), 255, qadd8(a, 40));
      }
      break;
    case SF_WIPE: {
      uint32_t step = t / 70; uint16_t cyc = step / (2 * n), p = step % (2 * n);
      bool filling = p < n; uint16_t k = filling ? p : p - n; uint8_t c = hue + cyc * 40;
      for (uint16_t i = 0; i < n; i++) {
        bool on = filling ? (i <= k) : (i > k);
        if (on) sBuf[i] = CHSV(c, 255, 255); else sBuf[i] = CRGB(0, 0, 0);
      }
    } break;

    // ---- newer strip effects
    case SF_COMET:
      fadeToBlackBy(sBuf, n, 60);
      sBuf[(t / 35) % n] = CHSV(hue, 255, 255);
      break;
    case SF_SPARKLE:
      fill_solid(sBuf, n, CRGB(0, 0, 0));
      sBuf[random16(n)] = CHSV(hue, 255, 255);
      break;
    case SF_FIREWORKS:
      fadeToBlackBy(sBuf, n, 45);
      if (random8() < 22) {
        uint16_t p = random16(n); CRGB c = CHSV(random8(), 255, 255);
        sBuf[p] = c;
        CRGB d = c; d.nscale8(110);
        if (p > 0) sBuf[p - 1] += d;
        if (p < n - 1) sBuf[p + 1] += d;
      }
      break;
    case SF_CHASE: {
      uint16_t pos = (t / 50) % n; CRGB bg = CHSV(hue + 128, 255, 40), fg = CHSV(hue, 255, 255);
      for (uint16_t i = 0; i < n; i++) sBuf[i] = (((i + n - pos) % n) < 6) ? fg : bg;
    } break;
    case SF_PRIDE:
      for (uint16_t i = 0; i < n; i++)
        sBuf[i] = CHSV((uint8_t)(t / 30 + i * 8 + (sin8((uint8_t)(i * 10 + t / 25)) >> 2)), 255,
                       110 + (sin8((uint8_t)(i * 20 - t / 12)) >> 1));
      break;
    case SF_LAKE:
      for (uint16_t i = 0; i < n; i++)
        sBuf[i] = CHSV(130 + (sin8((uint8_t)(i * 9 + t / 12)) >> 3), 255, 90 + (sin8((uint8_t)(i * 14 - t / 8)) >> 1));
      break;
    case SF_HEARTBEAT: {
      uint16_t ph = t % 1300; uint8_t v = 0;
      if (ph < 200) v = triwave8((ph * 255) / 200);
      else if (ph >= 320 && ph < 520) v = scale8(triwave8(((ph - 320) * 255) / 200), 190);
      fill_solid(sBuf, n, CHSV(hue, 255, v));
    } break;
    case SF_POLICE: {
      uint8_t ph = (t / 110) % 6;
      for (uint16_t i = 0; i < n; i++) {
        if ((ph == 0 || ph == 2) && i < n / 2) sBuf[i] = CRGB(255, 0, 0);
        else if ((ph == 3 || ph == 5) && i >= n / 2) sBuf[i] = CRGB(0, 0, 255);
        else sBuf[i] = CRGB(0, 0, 0);
      }
    } break;
    case SF_CANDLE: {
      static uint32_t last = 0; static int lvl = 150;
      if (t - last >= 40) {
        last = t; lvl += (int)random8(21) - 10;
        if (lvl < 90) lvl = 90;
        if (lvl > 255) lvl = 255;
      }
      fill_solid(sBuf, n, CHSV(25, 240, (uint8_t)lvl));
    } break;
    case SF_GRADIENT: {
      uint8_t base = hue + (sin8((uint8_t)(t / 25)) >> 2);
      for (uint16_t i = 0; i < n; i++) sBuf[i] = CHSV((uint8_t)(base + i * 60 / n), 255, 255);
    } break;
    case SF_RIPPLE: {                                   // every key press: light runs out from the middle
      static uint32_t start[6] = {0}; static uint8_t rh[6]; static uint8_t nextR = 0; static uint16_t seen = 0;
      const uint16_t kp = keyPresses;
      if (kp != seen) { seen = kp; const uint32_t ms = millis(); start[nextR] = ms ? ms : 1; rh[nextR] = hue + random8(70); nextR = (nextR + 1) % 6; }
      fill_solid(sBuf, n, CHSV(hue, 255, 14));
      const float mid = (n - 1) / 2.0f;
      for (uint8_t k = 0; k < 6; k++) {
        if (!start[k]) continue;
        uint32_t age = millis() - start[k];
        if (age > 900) { start[k] = 0; continue; }
        float r = age * (n / 2.0f + 2.0f) / 700.0f;
        uint8_t v = 255 - age * 255 / 900;
        for (uint16_t i = 0; i < n; i++) {
          float d = fabsf(fabsf(i - mid) - r);
          if (d < 1.5f) sBuf[i] += CHSV(rh[k], 220, (uint8_t)(v * (1.5f - d) / 1.5f));
        }
      }
    } break;
    case SF_ENCRAIN: {                                  // rainbow that the encoder spins
      static float off = 0;
      off += ((float)encSpin * 12.0f - off) * 0.15f;
      fill_rainbow(sBuf, n, (uint8_t)((int)off + (int)(t / 300)), max(1, 255 / n));
    } break;
    case SF_JOYLIGHT: {                                 // a light that follows the joystick left / right
      fadeToBlackBy(sBuf, n, 60);
      float p = (gStickX + 1.0f) * 0.5f * (n - 1);
      int i0 = (int)p; float fr = p - i0;
      uint8_t hh = hue + (int8_t)(gStickY * 60.0f);
      sBuf[i0] += CHSV(hh, 230, (uint8_t)(255 * (1.0f - fr)));
      if (i0 + 1 < n) sBuf[i0 + 1] += CHSV(hh, 230, (uint8_t)(255 * fr));
    } break;
    case SF_HEAT: {                                     // the faster you type, the hotter it glows
      static float heat = 0; static uint16_t seen = 0; static uint32_t lastT = 0;
      const uint32_t now = millis();
      float dts = lastT ? (now - lastT) / 1000.0f : 0; lastT = now;
      const uint16_t kp = keyPresses;
      heat += (uint16_t)(kp - seen) * 0.12f; seen = kp;
      heat -= dts * 0.18f;
      if (heat < 0) heat = 0;
      if (heat > 1) heat = 1;
      for (uint16_t i = 0; i < n; i++) {
        uint8_t flick = inoise8(i * 60, now / 4) / 4;   // a little shimmer
        CRGB c = heat < 0.12f ? CRGB(CHSV(160, 255, 30 + (uint8_t)(heat * 900)))
                              : HeatColor((uint8_t)min(255, 60 + (int)(heat * 195) + flick / 2 - 32));
        sBuf[i] = c;
      }
    } break;
    case SF_DAY: {                                      // colour of the time of day, softly waving
      CRGB c = dayColor();
      for (uint16_t i = 0; i < n; i++) { sBuf[i] = c; sBuf[i].nscale8(200 + (sin8((uint8_t)(i * 20 + t / 30)) >> 2) - 10); }
    } break;
    case SF_SONG: {                                     // fills up as the song plays (colour per song), dims when paused
      float fr; bool pl; uint8_t sh;
      if (!songProgress(fr, pl, sh)) { fill_solid(sBuf, n, CHSV(hue, 200, tbeat(t, 8, 15, 60))); break; }   // nothing playing: soft glow
      const uint8_t v = pl ? 255 : tbeat(t, 15, 50, 110);
      const float f = fr * n;
      for (uint16_t i = 0; i < n; i++) {
        if (i + 1 <= f)  sBuf[i] = CHSV(sh + i * 3, 230, v);
        else if (i < f)  sBuf[i] = CHSV(sh, 150, scale8(v, (uint8_t)((f - i) * 255)));
        else             sBuf[i] = CHSV(sh, 230, 6);
      }
      if (pl) { uint16_t hd = (uint16_t)f; if (hd > n - 1) hd = n - 1; sBuf[hd] += CHSV(sh, 60, tbeat(t, 40, 40, 160)); }   // the head glows
    } break;
    default: fill_solid(sBuf, n, CRGB::Black); break;
  }
}

// ---- matrix state
static uint8_t heat2[MAX_W * 16];
static int8_t rainY[MAX_W];
struct Rip { float cx, cy; uint32_t t0; uint8_t hue; bool on; };
static Rip rips[4];
static uint8_t lifeA[256], lifeB[256], lifeAge[256]; static uint16_t lifeGen = 0;

// particles (PS fire, fireworks, starfield, fireflies)
struct Part { float x, y, vx, vy, life, dlife; uint8_t hue; bool on; };
static Part  parts[256];
static float hbuf[MAX_W * 16], htmp[MAX_W * 16];
static float fireAcc = 0, starAcc = 0;
static float bb[4]; static bool bbInit = false;                                   // bounce
static bool  fwRocket = false; static float fwx, fwy, fwvy, fwty; static uint8_t fwhue; static uint32_t fwNext = 0;   // fireworks

static void lifeSeed() {
  for (uint16_t i = 0; i < 256; i++) lifeA[i] = ((i % 16) < mCfg.w && (i / 16) < mCfg.h && random8() < 90) ? 1 : 0;
  memset(lifeAge, 0, sizeof lifeAge); lifeGen = 0;
}
// ---- games: shared state
static bool     gameNew = true;                           // set when the matrix effect changes
static uint32_t gMsgUntil = 0;                            // showing a score message until then
static char     gMsg[16];
static bool     gRestart = false;

static void resetMat() {
  gameNew = true; gMsgUntil = 0; gRestart = false;
  fill_solid(mBuf, MAX_MAT + 1, CRGB::Black);
  memset(heat2, 0, sizeof heat2);
  memset(hbuf, 0, sizeof hbuf);
  fireAcc = 0; starAcc = 0; bbInit = false; fwRocket = false; fwNext = 0;
  for (auto& p : parts) p.on = false;
  for (auto& r : rainY) r = -1;
  for (auto& r : rips) r.on = false;
  lifeSeed();
}

// PS Fire: embers rise from the bottom at a steady rate, cool as they climb, and are splatted into a
// heat field that fades and is blurred every frame. That gives soft flame tongues instead of sparkles.
// Tuning: FIRE_GAIN = brightness, FIRE_FADE = how long the glow lingers (closer to 1 = longer),
//         spawn rate = the 0.18f in fireAcc.
static void psFire(uint8_t W, uint8_t H) {
  const float FIRE_GAIN = 85.0f, FIRE_FADE = 0.78f;
  const float spd = (mCfg.speed + 16) / 144.0f;
  const uint32_t t = mT;

  fireAcc += W * 0.18f * spd;
  while (fireAcc >= 1.0f) {
    fireAcc -= 1.0f;
    for (auto& p : parts) {
      if (p.on) continue;
      p.on = true;
      p.x = random16((W - 1) * 100 + 1) / 100.0f;
      p.y = (H - 1) + random8() / 320.0f;
      p.vx = ((int)random8() - 128) / 4000.0f;
      p.vy = -(0.14f + random8() / 2200.0f);
      float rise = H * (0.55f + random8() / 600.0f);       // each ember climbs 55%..97% of the height
      p.life = 1.0f; p.dlife = -p.vy / rise;
      break;
    }
  }

  for (uint8_t y = 0; y < H; y++) for (uint8_t x = 0; x < W; x++) hbuf[y * 16 + x] *= FIRE_FADE;

  auto add = [&](int ax, int ay, float v) { if (ax >= 0 && ax < W && ay >= 0 && ay < H) hbuf[ay * 16 + ax] += v; };
  for (auto& p : parts) {
    if (!p.on) continue;
    uint8_t ph = (uint8_t)(((int)(t >> 3) + (int)(p.y * 24.0f)) & 0xFF);
    p.vx += ((int)sin8(ph) - 128) / 128.0f * 0.004f;       // smooth sway, no random jitter
    p.vx *= 0.96f;
    p.x += p.vx * spd; p.y += p.vy * spd; p.life -= p.dlife * spd;
    if (p.life <= 0 || p.y < -0.5f) { p.on = false; continue; }
    if (p.x < 0) p.x = 0;
    if (p.x > W - 1) p.x = W - 1;
    float hv = FIRE_GAIN * p.life * sqrtf(sqrtf(p.life));
    int ix = (int)p.x, iy = (int)floorf(p.y);
    float fx = p.x - ix, fy = p.y - iy;
    add(ix,     iy,     (1 - fx) * (1 - fy) * hv);
    add(ix + 1, iy,     fx * (1 - fy) * hv);
    add(ix,     iy + 1, (1 - fx) * fy * hv);
    add(ix + 1, iy + 1, fx * fy * hv);
  }

  for (uint8_t y = 0; y < H; y++) for (uint8_t x = 0; x < W; x++) {
    float c = hbuf[y * 16 + x];
    float l = x > 0     ? hbuf[y * 16 + x - 1] : c;
    float r = x < W - 1 ? hbuf[y * 16 + x + 1] : c;
    float u = y > 0     ? hbuf[(y - 1) * 16 + x] : c;
    float d = y < H - 1 ? hbuf[(y + 1) * 16 + x] : c;
    htmp[y * 16 + x] = c * 0.5f + (l + r) * 0.15f + (u + d) * 0.10f;
  }
  memcpy(hbuf, htmp, sizeof hbuf);

  FOR_XY {
    float v = hbuf[y * 16 + x];
    if (v > 255.0f) v = 255.0f;
    mBuf[XY(x, y)] = HeatColor((uint8_t)(v * 0.66f));
  }
}

// 5x8 pixel font for the matrix, ASCII 32-126 (from the X11 koi5x8 bitmap font). 5 columns per letter,
// bit 0 = top row. FONT_W = used width of each letter (proportional spacing).
static const uint8_t FONT5[95][5] PROGMEM = {
  {0x00,0x00,0x00,0x00,0x00},  //  
  {0x5E,0x00,0x00,0x00,0x00},  // !
  {0x0E,0x00,0x0E,0x00,0x00},  // "
  {0x14,0x7F,0x14,0x7F,0x14},  // #
  {0x04,0x2A,0x7F,0x2A,0x10},  // $
  {0x16,0x08,0x34,0x00,0x00},  // %
  {0x36,0x49,0x36,0x40,0x00},  // &
  {0x0E,0x00,0x00,0x00,0x00},  // '
  {0x3C,0x42,0x00,0x00,0x00},  // (
  {0x42,0x3C,0x00,0x00,0x00},  // )
  {0x54,0x38,0x38,0x54,0x00},  // *
  {0x10,0x10,0x7C,0x10,0x10},  // +
  {0x80,0x60,0x20,0x00,0x00},  // ,
  {0x10,0x10,0x10,0x10,0x00},  // -
  {0x40,0xE0,0x40,0x00,0x00},  // .
  {0x60,0x10,0x08,0x06,0x00},  // /
  {0x3C,0x42,0x3C,0x00,0x00},  // 0
  {0x44,0x7E,0x40,0x00,0x00},  // 1
  {0x64,0x52,0x52,0x4C,0x00},  // 2
  {0x22,0x4A,0x4E,0x32,0x00},  // 3
  {0x18,0x14,0x7E,0x10,0x00},  // 4
  {0x2E,0x4A,0x4A,0x32,0x00},  // 5
  {0x3C,0x4A,0x4A,0x30,0x00},  // 6
  {0x02,0x62,0x1A,0x06,0x00},  // 7
  {0x34,0x4A,0x4A,0x34,0x00},  // 8
  {0x0C,0x52,0x52,0x3C,0x00},  // 9
  {0x6C,0x6C,0x00,0x00,0x00},  // :
  {0x80,0x6C,0x2C,0x00,0x00},  // ;
  {0x18,0x24,0x42,0x00,0x00},  // <
  {0x28,0x28,0x28,0x28,0x00},  // =
  {0x42,0x24,0x18,0x00,0x00},  // >
  {0x04,0x52,0x0C,0x00,0x00},  // ?
  {0x3C,0x42,0x99,0xA5,0x1E},  // @
  {0x7C,0x12,0x12,0x7C,0x00},  // A
  {0x7E,0x4A,0x4A,0x34,0x00},  // B
  {0x3C,0x42,0x42,0x24,0x00},  // C
  {0x7E,0x42,0x42,0x3C,0x00},  // D
  {0x7E,0x4A,0x4A,0x42,0x00},  // E
  {0x7E,0x0A,0x0A,0x02,0x00},  // F
  {0x3C,0x42,0x52,0x34,0x00},  // G
  {0x7E,0x08,0x08,0x7E,0x00},  // H
  {0x42,0x7E,0x42,0x00,0x00},  // I
  {0x20,0x42,0x3E,0x02,0x00},  // J
  {0x7E,0x08,0x34,0x42,0x00},  // K
  {0x7E,0x40,0x40,0x40,0x00},  // L
  {0x7E,0x0C,0x0C,0x7E,0x00},  // M
  {0x7E,0x0C,0x38,0x7E,0x00},  // N
  {0x3C,0x42,0x42,0x3C,0x00},  // O
  {0x7E,0x12,0x12,0x0C,0x00},  // P
  {0x3C,0x52,0x62,0xBC,0x00},  // Q
  {0x7E,0x12,0x12,0x6C,0x00},  // R
  {0x24,0x4A,0x52,0x24,0x00},  // S
  {0x02,0x7E,0x02,0x00,0x00},  // T
  {0x3E,0x40,0x40,0x3E,0x00},  // U
  {0x1E,0x60,0x60,0x1E,0x00},  // V
  {0x7E,0x30,0x30,0x7E,0x00},  // W
  {0x66,0x18,0x18,0x66,0x00},  // X
  {0x06,0x08,0x70,0x08,0x06},  // Y
  {0x62,0x52,0x4A,0x46,0x00},  // Z
  {0x7E,0x42,0x42,0x00,0x00},  // [
  {0x06,0x08,0x10,0x60,0x00},  // backslash
  {0x42,0x42,0x7E,0x00,0x00},  // ]
  {0x04,0x02,0x04,0x00,0x00},  // ^
  {0x80,0x80,0x80,0x80,0x00},  // _
  {0x02,0x04,0x00,0x00,0x00},  // `
  {0x30,0x48,0x48,0x78,0x00},  // a
  {0x7E,0x48,0x48,0x30,0x00},  // b
  {0x30,0x48,0x48,0x00,0x00},  // c
  {0x30,0x48,0x48,0x7E,0x00},  // d
  {0x30,0x68,0x58,0x10,0x00},  // e
  {0x10,0x7C,0x12,0x04,0x00},  // f
  {0x10,0xA8,0xA8,0x70,0x00},  // g
  {0x7E,0x08,0x08,0x70,0x00},  // h
  {0x48,0x7A,0x40,0x00,0x00},  // i
  {0x40,0x80,0x7A,0x00,0x00},  // j
  {0x7E,0x10,0x10,0x68,0x00},  // k
  {0x42,0x7E,0x40,0x00,0x00},  // l
  {0x78,0x08,0x70,0x08,0x70},  // m
  {0x78,0x08,0x08,0x70,0x00},  // n
  {0x30,0x48,0x48,0x30,0x00},  // o
  {0xF8,0x28,0x28,0x10,0x00},  // p
  {0x10,0x28,0x28,0xF8,0x00},  // q
  {0x78,0x10,0x08,0x10,0x00},  // r
  {0x50,0x58,0x28,0x00,0x00},  // s
  {0x08,0x3E,0x48,0x20,0x00},  // t
  {0x38,0x40,0x40,0x78,0x00},  // u
  {0x38,0x40,0x38,0x00,0x00},  // v
  {0x38,0x40,0x30,0x40,0x38},  // w
  {0x48,0x30,0x30,0x48,0x00},  // x
  {0x58,0xA0,0xA0,0x78,0x00},  // y
  {0x48,0x68,0x58,0x48,0x00},  // z
  {0x08,0x2A,0x55,0x41,0x00},  // {
  {0x7E,0x00,0x00,0x00,0x00},  // |
  {0x41,0x55,0x2A,0x08,0x00},  // }
  {0x04,0x02,0x04,0x02,0x00},  // ~
};
static const uint8_t FONT_W[95] PROGMEM = {2,1,3,5,5,3,4,1,2,2,4,5,3,4,3,4,3,3,4,4,4,4,4,4,4,4,2,3,3,4,3,3,5,4,4,4,4,4,4,4,4,3,4,4,4,4,4,4,4,4,4,4,3,4,4,4,4,5,4,3,4,3,3,4,2,4,4,3,4,4,4,4,4,3,3,4,3,5,4,4,4,4,4,3,4,4,3,5,4,4,4,4,1,4,4};

// draw text on the matrix at (x0, y0); returns the width in pixels
static int textWidth(const char* s) {
  int w = 0;
  for (const char* p = s; *p; p++) { uint8_t c = (uint8_t)*p; if (c < 32 || c > 126) c = '?'; w += pgm_read_byte(&FONT_W[c - 32]) + 1; }
  return w;
}
static void drawText(const char* s, int x, int y0, uint8_t W, uint8_t H, uint8_t hue0, bool rainbow) {
  for (const char* p = s; *p && x < W; p++) {
    uint8_t c = (uint8_t)*p; if (c < 32 || c > 126) c = '?';
    const uint8_t idx = c - 32, w = pgm_read_byte(&FONT_W[idx]);
    for (uint8_t col = 0; col < w; col++, x++) {
      if (x < 0 || x >= W) continue;
      const uint8_t bits = pgm_read_byte(&FONT5[idx][col]);
      for (uint8_t r = 0; r < 8; r++)
        if ((bits >> r) & 1) { int y = y0 + r; if (y >= 0 && y < H) mBuf[XY(x, y)] = CHSV(rainbow ? (uint8_t)(hue0 + x * 10) : hue0, 255, 255); }
    }
    x++;                                                    // gap between letters
  }
}
// 3x5 digits for the timer (index 10 = ':')
static const uint8_t DIG3[11][5] = {{7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},{7,4,7,1,7},
                                    {7,4,7,5,7},{7,1,1,1,1},{7,5,7,5,7},{7,5,7,1,7},{0,1,0,1,0}};
static int drawDigit(uint8_t d, int x, int y, CRGB c, uint8_t W, uint8_t H) {   // returns width
  const int w = d == 10 ? 1 : 3;
  for (int r = 0; r < 5; r++) for (int k = 0; k < w; k++)
    if ((DIG3[d][r] >> (w - 1 - k)) & 1) { int xx = x + k, yy = y + r; if (xx >= 0 && xx < W && yy >= 0 && yy < H) mBuf[XY(xx, yy)] = c; }
  return w;
}

static uint32_t gMsgStart = 0; static bool gMsgSized = false;
static void gameMsg(const char* m, uint32_t ms, bool restart) {
  strlcpy(gMsg, m, sizeof gMsg); gMsgStart = millis(); gMsgUntil = gMsgStart + ms; gRestart = restart; gMsgSized = false;
}
static bool showGameMsg(uint8_t W, uint8_t H, uint16_t N) {   // true while a message is shown
  if ((int32_t)(gMsgUntil - millis()) > 0) {
    fill_solid(mBuf, N, CRGB::Black);
    int w = textWidth(gMsg), y0 = (H - 8) / 2;
    if (!gMsgSized) {                                     // long message: keep it up until it has scrolled across once
      gMsgSized = true;
      if (w > W) { uint32_t need = (uint32_t)(w + W) * 2000 / (matTextSpeed * 5) + 300; if (gMsgUntil - gMsgStart < need) gMsgUntil = gMsgStart + need; }
    }
    int x = w <= W ? (W - w) / 2 : textScroll(millis() - gMsgStart, w, W);
    drawText(gMsg, x, y0, W, H, 0, true);
    return true;
  }
  if (gRestart) { gRestart = false; gameNew = true; }
  return false;
}
static float gameStep() {                                 // frame time, 1.0 = 20 ms
  static uint32_t last = 0; uint32_t now = millis();
  float k = last ? (now - last) / 20.0f : 1.0f; last = now;
  return (k > 3 ? 3 : k) * gameSpeedF();                   // everything that moves is scaled by the game speed
}

static void gameSnake(uint8_t W, uint8_t H, uint16_t N) {
  static uint8_t bx[256], by[256]; static uint16_t len, head; static int8_t dx, dy, ndx, ndy;
  static uint8_t fx2, fy2; static uint32_t lastStep;
  auto food = [&]() {
    for (int tries = 0; tries < 200; tries++) {
      uint8_t x = random8(W), y = random8(H); bool hit = false;
      for (uint16_t i = 0; i < len; i++) { uint16_t k = (head + 256 - i) & 255; if (bx[k] == x && by[k] == y) { hit = true; break; } }
      if (!hit) { fx2 = x; fy2 = y; return; }
    }
  };
  if (gameNew) {
    gameNew = false; len = 3; head = 2; dx = ndx = 1; dy = ndy = 0; lastStep = millis();
    for (uint8_t i = 0; i < 3; i++) { bx[i] = i; by[i] = H / 2; }
    food();
  }
  if (showGameMsg(W, H, N)) return;
  if (fabsf(gStickX) > 0.5f || fabsf(gStickY) > 0.5f) {    // turn (never straight back)
    int8_t tx = 0, ty = 0;
    if (fabsf(gStickX) > fabsf(gStickY)) tx = gStickX > 0 ? 1 : -1; else ty = gStickY > 0 ? 1 : -1;
    if (tx != -dx || ty != -dy) { ndx = tx; ndy = ty; }
  }
  const uint32_t stepMs = gms(max(90, 220 - (int)len * 5));
  if (millis() - lastStep >= stepMs) {
    lastStep = millis(); dx = ndx; dy = ndy;
    int nx = bx[head] + dx, ny = by[head] + dy;
    bool dead = nx < 0 || ny < 0 || nx >= W || ny >= H;
    for (uint16_t i = 0; !dead && i + 1 < len; i++) { uint16_t k = (head + 256 - i) & 255; if (bx[k] == nx && by[k] == ny) dead = true; }
    if (dead) { char m[16]; snprintf(m, sizeof m, "%d", len - 3); gameMsg(m, 2500, true); return; }
    head = (head + 1) & 255; bx[head] = nx; by[head] = ny;
    if (nx == fx2 && ny == fy2) { if (len < 250) len++; food(); }
  }
  fill_solid(mBuf, N, CRGB::Black);
  for (uint16_t i = 0; i < len; i++) { uint16_t k = (head + 256 - i) & 255; mBuf[XY(bx[k], by[k])] = i ? CRGB(CHSV(96, 255, 200 - min(150, (int)i * 6))) : CRGB(180, 255, 180); }
  mBuf[XY(fx2, fy2)] = ((millis() / 200) & 1) ? CRGB(255, 0, 0) : CRGB(120, 0, 0);
}

static void gamePong(uint8_t W, uint8_t H, uint16_t N) {
  static float py, ay2, bxp, byp, vx, vy; static uint8_t me, cpu;
  const int P = max(2, H / 4);
  auto serve = [&](int dir) { bxp = W / 2.0f; byp = H / 2.0f; vx = 0.22f * dir; vy = ((int)random8(2) ? 1 : -1) * 0.12f; };
  if (gameNew) { gameNew = false; py = ay2 = (H - P) / 2.0f; me = cpu = 0; serve(1); }
  if (showGameMsg(W, H, N)) return;
  const float k = gameStep();
  py += gStickY * 0.45f * k; py = constrain(py, 0.0f, (float)(H - P));
  float target = byp - P / 2.0f; float d = target - ay2;    // computer paddle, a bit slow so you can win
  ay2 += constrain(d, -0.16f * k, 0.16f * k); ay2 = constrain(ay2, 0.0f, (float)(H - P));
  bxp += vx * k; byp += vy * k;
  if (byp < 0) { byp = -byp; vy = -vy; }
  if (byp > H - 1) { byp = 2 * (H - 1) - byp; vy = -vy; }
  if (bxp < 1 && vx < 0) {
    if (byp >= py - 0.5f && byp <= py + P - 0.5f) { bxp = 1; vx = -vx * 1.05f; vy += (byp - (py + P / 2.0f)) * 0.08f; }
    else if (bxp < 0) { cpu++; char m[16]; snprintf(m, sizeof m, "%d-%d", me, cpu); gameMsg(m, 1500, cpu >= 5); serve(1); if (cpu >= 5) return; }
  }
  if (bxp > W - 2 && vx > 0) {
    if (byp >= ay2 - 0.5f && byp <= ay2 + P - 0.5f) { bxp = W - 2; vx = -vx * 1.05f; vy += (byp - (ay2 + P / 2.0f)) * 0.08f; }
    else if (bxp > W - 1) { me++; char m[16]; snprintf(m, sizeof m, "%d-%d", me, cpu); gameMsg(m, 1500, me >= 5); serve(-1); if (me >= 5) return; }
  }
  vx = constrain(vx, -0.6f, 0.6f); vy = constrain(vy, -0.35f, 0.35f);
  fill_solid(mBuf, N, CRGB::Black);
  for (int i = 0; i < P; i++) { mBuf[XY(0, (int)py + i)] = CRGB(0, 120, 255); mBuf[XY(W - 1, (int)ay2 + i)] = CRGB(255, 60, 0); }
  for (int y = 0; y < H; y += 2) mBuf[XY(W / 2, y)] = CRGB(20, 20, 20);
  mBuf[XY((int)(bxp + 0.5f), (int)(byp + 0.5f))] = CRGB::White;
}

static void gameBreakout(uint8_t W, uint8_t H, uint16_t N) {
  static uint8_t bricks[4][16]; static float pxp, bxp, byp, vx, vy; static bool onPaddle; static uint8_t lives, level; static uint16_t score;
  const int PW = max(3, W / 4), rows = H >= 12 ? 4 : 2, top = 1;
  auto fillBricks = [&]() { for (int r = 0; r < 4; r++) for (int c = 0; c < 16; c++) bricks[r][c] = r < rows; };
  if (gameNew) { gameNew = false; fillBricks(); pxp = (W - PW) / 2.0f; onPaddle = true; lives = 3; level = 0; score = 0; }
  if (showGameMsg(W, H, N)) return;
  const float k = gameStep();
  pxp += gStickX * 0.5f * k; pxp = constrain(pxp, 0.0f, (float)(W - PW));
  if (onPaddle) {
    bxp = pxp + PW / 2.0f; byp = H - 2;
    if (gStickBtn || fabsf(gStickY) > 0.6f) { onPaddle = false; float sp = 0.2f + level * 0.03f; vx = sp * 0.7f; vy = -sp; }
  } else {
    bxp += vx * k; byp += vy * k;
    if (bxp < 0) { bxp = -bxp; vx = -vx; }
    if (bxp > W - 1) { bxp = 2 * (W - 1) - bxp; vx = -vx; }
    if (byp < 0) { byp = -byp; vy = -vy; }
    int cx2 = (int)(bxp + 0.5f), cy2 = (int)(byp + 0.5f), r = cy2 - top;
    if (r >= 0 && r < rows && cx2 >= 0 && cx2 < W && bricks[r][cx2 / 2]) { bricks[r][cx2 / 2] = 0; vy = -vy; score++; }
    if (byp >= H - 2 && vy > 0 && bxp >= pxp - 0.5f && bxp <= pxp + PW - 0.5f) {   // paddle
      byp = H - 2; vy = -vy; vx += (bxp - (pxp + PW / 2.0f)) * 0.06f; vx = constrain(vx, -0.4f, 0.4f);
    }
    if (byp > H) {                                           // missed
      onPaddle = true;
      if (lives > 1) { lives--; char m[16]; snprintf(m, sizeof m, "x%d", lives); gameMsg(m, 1000, false); }
      else { char m[16]; snprintf(m, sizeof m, "%d", score); gameMsg(m, 2500, true); return; }
    }
    bool any = false;
    for (int rr = 0; rr < rows; rr++) for (int c = 0; c < (W + 1) / 2; c++) if (bricks[rr][c]) any = true;
    if (!any) { level++; fillBricks(); onPaddle = true; gameMsg("WIN!", 1200, false); }
  }
  fill_solid(mBuf, N, CRGB::Black);
  for (int rr = 0; rr < rows; rr++) for (int x = 0; x < W; x++) if (bricks[rr][x / 2]) mBuf[XY(x, top + rr)] = CHSV(rr * 40 + ((x / 2) & 1) * 12, 255, 220);
  for (int i = 0; i < PW; i++) mBuf[XY((int)pxp + i, H - 1)] = CRGB(0, 150, 255);
  mBuf[XY((int)(bxp + 0.5f), (int)(byp + 0.5f))] = CRGB::White;
}

static void gameDodge(uint8_t W, uint8_t H, uint16_t N) {
  static float pxp; static float ox[10], oy[10], ov[10]; static bool on[10]; static uint32_t startMs, nextSpawn;
  if (gameNew) { gameNew = false; pxp = W / 2.0f - 1; for (auto& o : on) o = false; startMs = millis(); nextSpawn = millis() + 600; }
  if (showGameMsg(W, H, N)) return;
  const float k = gameStep();
  const uint32_t alive = millis() - startMs;
  pxp += gStickX * 0.5f * k; pxp = constrain(pxp, 0.0f, (float)(W - 2));
  if ((int32_t)(millis() - nextSpawn) >= 0) {
    for (int i = 0; i < 10; i++) if (!on[i]) { on[i] = true; ox[i] = random8(W); oy[i] = -1; ov[i] = 0.08f + alive / 400000.0f + random8() / 4000.0f; break; }
    nextSpawn = millis() + gms(max(150, 650 - (int)(alive / 100)));
  }
  fill_solid(mBuf, N, CRGB::Black);
  for (int i = 0; i < 10; i++) {
    if (!on[i]) continue;
    oy[i] += ov[i] * k;
    if (oy[i] >= H) { on[i] = false; continue; }
    int x = (int)ox[i], y = (int)oy[i];
    if (y >= H - 1 && (x == (int)pxp || x == (int)pxp + 1)) { char m[16]; snprintf(m, sizeof m, "%lus", (unsigned long)(alive / 1000)); gameMsg(m, 2500, true); return; }
    if (y >= 0) mBuf[XY(x, y)] = CRGB(255, 80, 0);
  }
  mBuf[XY((int)pxp, H - 1)] = CRGB(0, 255, 120); mBuf[XY((int)pxp + 1, H - 1)] = CRGB(0, 255, 120);
}

// edge detection for the stick pushed up and the stick button (each called once per frame by the game)
static bool stickUpEdge() { static bool held = false; const bool now = gStickY < -0.55f; const bool e = now && !held; held = now; return e; }
static bool stickBtnEdge() { static bool held = false; const bool now = gStickBtn; const bool e = now && !held; held = now; return e; }

// Tetris: stick left / right = move, up or button = rotate, down = drop faster
static void gameTetris(uint8_t W, uint8_t H, uint16_t N) {
  static const uint16_t SH[7] = {0x0F00, 0x4E00, 0x6C00, 0xC600, 0x8E00, 0x2E00, 0x6600};   // I T S Z J L O (4x4, row by row)
  static uint8_t board[16][10]; static int8_t px, py; static uint8_t pc, rot; static uint32_t lastFall, lastMove; static uint16_t lines;
  const int BW = W < 10 ? W : 10, X0 = (W - BW) / 2;
  auto cells = [&](uint8_t p, uint8_t r, int8_t out[4][2]) {
    int k = 0;
    for (int y = 0; y < 4; y++) for (int x = 0; x < 4; x++) if (SH[p] & (0x8000 >> (y * 4 + x))) {
      int rx = x, ry = y;
      for (int i = 0; i < (p == 6 ? 0 : r); i++) { int t2 = rx; rx = 3 - ry; ry = t2; }   // the O piece doesn't turn
      out[k][0] = rx; out[k][1] = ry; k++;
    }
  };
  auto fits = [&](uint8_t p, uint8_t r, int x, int y) {
    int8_t c[4][2]; cells(p, r, c);
    for (auto& q : c) { int bx = x + q[0], by = y + q[1]; if (bx < 0 || bx >= BW || by >= H) return false; if (by >= 0 && board[by][bx]) return false; }
    return true;
  };
  auto spawn = [&]() {
    pc = random8(7); rot = 0; px = BW / 2 - 2; py = pc == 0 ? -1 : 0;
    if (!fits(pc, rot, px, py)) { char m[16]; snprintf(m, sizeof m, "%u", lines); gameMsg(m, 2500, true); }
  };
  if (gameNew) { gameNew = false; memset(board, 0, sizeof board); lines = 0; lastFall = lastMove = millis(); spawn(); }
  if (showGameMsg(W, H, N)) return;
  const uint32_t now = millis();
  if (stickUpEdge() | stickBtnEdge()) { uint8_t r2 = (rot + 1) & 3; if (fits(pc, r2, px, py)) rot = r2; else if (fits(pc, r2, px - 1, py)) { rot = r2; px--; } else if (fits(pc, r2, px + 1, py)) { rot = r2; px++; } }
  if (fabsf(gStickX) > 0.5f && now - lastMove >= 140) { int d = gStickX > 0 ? 1 : -1; if (fits(pc, rot, px + d, py)) px += d; lastMove = now; }
  if (fabsf(gStickX) <= 0.5f) lastMove = 0;
  const uint32_t fallMs = gStickY > 0.6f ? 50 : gms(max(120, 600 - (int)lines * 20));
  if (now - lastFall >= fallMs) {
    lastFall = now;
    if (fits(pc, rot, px, py + 1)) py++;
    else {                                                   // lock the piece, clear full lines
      int8_t c[4][2]; cells(pc, rot, c);
      for (auto& q : c) { int bx = px + q[0], by = py + q[1]; if (by >= 0 && by < H) board[by][bx] = pc + 1; }
      for (int y = H - 1; y >= 0; y--) {
        bool full = true; for (int x = 0; x < BW; x++) if (!board[y][x]) { full = false; break; }
        if (!full) continue;
        for (int yy = y; yy > 0; yy--) memcpy(board[yy], board[yy - 1], sizeof board[0]);
        memset(board[0], 0, sizeof board[0]); lines++; y++;
      }
      spawn();
    }
  }
  fill_solid(mBuf, N, CRGB::Black);
  if (X0 > 0) for (int y = 0; y < H; y++) { mBuf[XY(X0 - 1, y)] = CRGB(25, 25, 40); mBuf[XY(X0 + BW, y)] = CRGB(25, 25, 40); }
  for (int y = 0; y < H; y++) for (int x = 0; x < BW; x++) if (board[y][x]) mBuf[XY(X0 + x, y)] = CHSV((board[y][x] - 1) * 36, 255, 200);
  int8_t c[4][2]; cells(pc, rot, c);
  for (auto& q : c) { int by = py + q[1]; if (by >= 0) mBuf[XY(X0 + px + q[0], by)] = CHSV(pc * 36, 200, 255); }
}

// Flappy: push the stick up (or press its button) to flap through the gaps
static void gameFlappy(uint8_t W, uint8_t H, uint16_t N) {
  static float by, vy; static int16_t pxs[3]; static uint8_t gy[3]; static uint32_t lastStep; static uint16_t score;
  const int gap = H >= 12 ? 5 : 3, birdX = 2, spacing = W >= 12 ? W / 2 + 1 : 6;
  auto newGap = [&]() { return (uint8_t)(1 + random8(max(1, H - gap - 1))); };
  if (gameNew) {
    gameNew = false; by = H / 2.0f; vy = 0; score = 0; lastStep = millis();
    for (int i = 0; i < 3; i++) { pxs[i] = W + i * spacing; gy[i] = newGap(); }
  }
  if (showGameMsg(W, H, N)) return;
  const float k = gameStep();
  if (stickUpEdge() | stickBtnEdge()) vy = -0.5f;
  vy += 0.04f * k; if (vy > 0.6f) vy = 0.6f;
  by += vy * k;
  const uint32_t stepMs = gms(max(60, 130 - (int)score * 3));
  if (millis() - lastStep >= stepMs) {
    lastStep = millis();
    for (int i = 0; i < 3; i++) {
      pxs[i]--;
      if (pxs[i] == birdX - 1) score++;
      if (pxs[i] < -1) { int far = max(pxs[0], max(pxs[1], pxs[2])); pxs[i] = far + spacing; gy[i] = newGap(); }
    }
  }
  const int bi = (int)(by + 0.5f);
  bool dead = by < -0.5f || by > H - 0.5f;
  for (int i = 0; i < 3 && !dead; i++) if (pxs[i] == birdX && (bi < gy[i] || bi >= gy[i] + gap)) dead = true;
  if (dead) { char m[16]; snprintf(m, sizeof m, "%u", score); gameMsg(m, 2500, true); return; }
  fill_solid(mBuf, N, CRGB(0, 0, 18));
  for (int i = 0; i < 3; i++) if (pxs[i] >= 0 && pxs[i] < W)
    for (int y = 0; y < H; y++) if (y < gy[i] || y >= gy[i] + gap) mBuf[XY(pxs[i], y)] = CRGB(0, 160, 30);
  mBuf[XY(birdX, bi)] = CRGB(255, 220, 0); mBuf[XY(birdX + 1, bi)] = CRGB(255, 90, 0);
}

// Invaders: stick left / right = move, up or button = shoot
static void gameInvaders(uint8_t W, uint8_t H, uint16_t N) {
  static bool alive[3][8]; static int fx, fy, dir; static float plx; static uint32_t lastMarch, lastShot, lastBul, lastEnemy;
  static int8_t bx[2], byy[2]; static int8_t ex[3], ey[3]; static uint8_t lives, wave; static uint16_t score;
  const int rows = H >= 12 ? 3 : 2, cols = min(8, max(2, (W - 2) / 2));
  auto newWave = [&]() {
    for (int r = 0; r < 3; r++) for (int c = 0; c < 8; c++) alive[r][c] = r < rows && c < cols;
    fx = 0; fy = 0; dir = 1; for (auto& b : byy) b = -1; for (auto& e : ey) e = -1;
  };
  if (gameNew) { gameNew = false; lives = 3; wave = 0; score = 0; plx = W / 2.0f; newWave(); lastMarch = lastShot = lastBul = lastEnemy = millis(); }
  if (showGameMsg(W, H, N)) return;
  const uint32_t now = millis();
  const float k = gameStep();
  plx += gStickX * 0.45f * k; plx = constrain(plx, 1.0f, (float)(W - 2));
  const int pxi = (int)(plx + 0.5f);
  const bool fireEdge = stickUpEdge() | stickBtnEdge();
  if ((fireEdge || ((gStickY < -0.55f || gStickBtn) && now - lastShot > gms(350))))   // shoot (hold = auto fire)
    for (int i = 0; i < 2; i++) if (byy[i] < 0) { bx[i] = pxi; byy[i] = H - 2; lastShot = now; break; }
  int alivecount = 0;
  for (int r = 0; r < rows; r++) for (int c = 0; c < cols; c++) alivecount += alive[r][c];
  const uint32_t marchMs = gms(max(120, 150 + alivecount * 25 - wave * 40));
  if (now - lastMarch >= marchMs) {                        // the formation moves; at the edge it steps down
    lastMarch = now;
    int minC = 99, maxC = -1;
    for (int r = 0; r < rows; r++) for (int c = 0; c < cols; c++) if (alive[r][c]) { minC = min(minC, c); maxC = max(maxC, c); }
    const int left = fx + minC * 2 + dir, right = fx + maxC * 2 + dir;
    if (left < 0 || right > W - 1) { dir = -dir; fy++; } else fx += dir;
  }
  if (now - lastBul >= gms(40)) {                               // bullets
    lastBul = now;
    for (int i = 0; i < 2; i++) if (byy[i] >= 0) {
      byy[i]--;
      if (byy[i] < 0) continue;
      for (int r = 0; r < rows; r++) for (int c = 0; c < cols; c++)
        if (alive[r][c] && fx + c * 2 == bx[i] && fy + r * 2 == byy[i]) { alive[r][c] = false; byy[i] = -1; score++; }
    }
  }
  if (now - lastEnemy >= gms(90)) {                             // enemy shots
    lastEnemy = now;
    for (int i = 0; i < 3; i++) if (ey[i] >= 0) {
      ey[i]++;
      if (ey[i] >= H - 1 && abs(ex[i] - pxi) <= 1) {          // hit
        ey[i] = -1;
        if (--lives == 0) { char m[16]; snprintf(m, sizeof m, "%u", score); gameMsg(m, 2500, true); return; }
        char m[16]; snprintf(m, sizeof m, "x%u", lives); gameMsg(m, 1000, false);
      } else if (ey[i] >= H) ey[i] = -1;
    }
    if (random8() < 14 + wave * 4) {
      int c = random8(cols);
      for (int r = rows - 1; r >= 0; r--) if (alive[r][c]) {
        for (int i = 0; i < 3; i++) if (ey[i] < 0) { ex[i] = fx + c * 2; ey[i] = fy + r * 2 + 1; break; }
        break;
      }
    }
  }
  alivecount = 0; int lowest = -1;
  for (int r = 0; r < rows; r++) for (int c = 0; c < cols; c++) if (alive[r][c]) { alivecount++; lowest = max(lowest, fy + r * 2); }
  if (alivecount == 0) { wave++; newWave(); char m[16]; snprintf(m, sizeof m, "WAVE %u", wave + 1); gameMsg(m, 1500, false); return; }
  if (lowest >= H - 2) { char m[16]; snprintf(m, sizeof m, "%u", score); gameMsg(m, 2500, true); return; }
  fill_solid(mBuf, N, CRGB::Black);
  for (int r = 0; r < rows; r++) for (int c = 0; c < cols; c++)
    if (alive[r][c]) mBuf[XY(fx + c * 2, fy + r * 2)] = CHSV(200 - r * 40 + (((now / 300) & 1) ? 10 : 0), 255, 230);
  for (int i = 0; i < 2; i++) if (byy[i] >= 0) mBuf[XY(bx[i], byy[i])] = CRGB::White;
  for (int i = 0; i < 3; i++) if (ey[i] >= 0) mBuf[XY(ex[i], ey[i])] = CRGB(255, 40, 0);
  for (int d = -1; d <= 1; d++) mBuf[XY(pxi + d, H - 1)] = CRGB(0, 220, 80);
  mBuf[XY(pxi, H - 2)] = CRGB(0, 140, 50);
}

static void matFx(uint8_t fx, uint8_t W, uint8_t H, uint16_t N) {
  const uint32_t t = mT; const uint8_t hue = mCfg.hue;
  const float spd = (mCfg.speed + 16) / 144.0f;
  const float mcx = (W - 1) / 2.0f, mcy = (H - 1) / 2.0f;
  switch (fx) {
    case MF_PLASMA:
      FOR_XY {
        uint8_t v = (sin8(x * 20 + (t >> 4)) + sin8(y * 18 - (t >> 5)) + sin8((x + y) * 12 + (t >> 3))) / 3;
        mBuf[XY(x, y)] = ColorFromPalette(PartyColors_p, v + (t >> 6), 255);
      }
      break;
    case MF_PSFIRE: psFire(W, H); break;
    case MF_FIRE: {
      static uint32_t last = 0;
      if (t - last >= 30) {
        last = t;
        uint16_t cl = 550 / H + 2; if (cl > 255) cl = 255;
        for (uint8_t x = 0; x < W; x++) {
          uint8_t* c = &heat2[x * 16];
          for (uint8_t y = 0; y < H; y++) c[y] = qsub8(c[y], random8(0, cl));
          for (int y = H - 1; y >= 2; y--) c[y] = (c[y - 1] + c[y - 2] + c[y - 2]) / 3;
          if (random8() < 130) { uint8_t y = random8(H < 3 ? H : 3); c[y] = qadd8(c[y], random8(160, 255)); }
        }
      }
      FOR_XY mBuf[XY(x, H - 1 - y)] = HeatColor(heat2[x * 16 + y]);
    } break;
    case MF_RAIN: {
      static uint32_t last = 0;
      fadeToBlackBy(mBuf, N, 35);
      if (t - last >= 70) {
        last = t;
        for (uint8_t x = 0; x < W; x++) {
          if (rainY[x] >= 0) {
            mBuf[XY(x, rainY[x])] = CHSV(hue, 255, 200);
            rainY[x]++;
            if (rainY[x] >= H) rainY[x] = -1; else mBuf[XY(x, rainY[x])] = CHSV(hue, 70, 255);
          } else if (random8() < 25) { rainY[x] = 0; mBuf[XY(x, 0)] = CHSV(hue, 70, 255); }
        }
      }
    } break;
    case MF_WAVES:
      FOR_XY {
        uint8_t h = (t >> 5) + (sin8(x * 24 + (t >> 2)) >> 2) + y * 6;
        uint8_t v = 100 + (sin8(y * 28 - (t >> 2)) >> 1);
        mBuf[XY(x, y)] = CHSV(h, 255, v);
      }
      break;
    case MF_LAVA:
      FOR_XY {
        uint8_t nz = inoise8(x * 40, y * 40 + (t >> 3), t >> 4);
        nz = qadd8(qsub8(nz, 40), nz >> 2);
        mBuf[XY(x, y)] = ColorFromPalette(LavaColors_p, nz, 255);
      }
      break;
    case MF_RIPPLES: {
      fill_solid(mBuf, N, CRGB::Black);
      if (random8() < 5) for (auto& r : rips) if (!r.on) {
        r.on = true; r.cx = random16(W * 10) / 10.0f; r.cy = random16(H * 10) / 10.0f; r.t0 = t; r.hue = random8();
        break;
      }
      float maxR = (W + H) * 0.5f;
      for (auto& r : rips) {
        if (!r.on) continue;
        float rad = (t - r.t0) * 0.005f;
        if (rad > maxR) { r.on = false; continue; }
        float fade = 1.0f - rad / maxR;
        FOR_XY {
          float d = sqrtf((x - r.cx) * (x - r.cx) + (y - r.cy) * (y - r.cy));
          float dl = fabsf(d - rad);
          if (dl < 1.2f) { CRGB c = CHSV(r.hue, 255, (uint8_t)((1.2f - dl) / 1.2f * 255.0f * fade)); mBuf[XY(x, y)] += c; }
        }
      }
    } break;
    case MF_LIFE: {
      static uint32_t last = 0;
      if (t - last >= 140) {
        last = t;
        uint16_t alive = 0; bool same = true;
        for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
          uint8_t nb = 0;
          for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++)
            if (dx || dy) nb += lifeA[((y + dy + H) % H) * 16 + (x + dx + W) % W];
          uint16_t i = y * 16 + x; uint8_t c = lifeA[i];
          uint8_t nw = (c && (nb == 2 || nb == 3)) || (!c && nb == 3);
          lifeB[i] = nw;
          if (nw != c) same = false;
          if (nw) { alive++; lifeAge[i] = c ? qadd8(lifeAge[i], 8) : 0; }
        }
        memcpy(lifeA, lifeB, sizeof lifeA); lifeGen++;
        if (alive < 3 || same || lifeGen > 400) lifeSeed();
      }
      fadeToBlackBy(mBuf, N, 60);
      FOR_XY if (lifeA[y * 16 + x]) mBuf[XY(x, y)] = CHSV(hue + lifeAge[y * 16 + x], 255, 255);
    } break;
    case MF_STARS:
      fadeToBlackBy(mBuf, N, 25);
      if (random8() < 40 + (mCfg.speed >> 2)) mBuf[random16(N)] = CHSV(hue + random8(50), random8(120, 255), 255);
      break;
    case MF_LISSAJOUS: {
      fadeToBlackBy(mBuf, N, 40);
      uint8_t xs = sin8((t >> 2) * 3), ys = sin8((t >> 2) * 2 + 64);
      mBuf[XY((xs * W) >> 8, (ys * H) >> 8)] = CHSV(t >> 4, 255, 255);
    } break;

    // ---- newer matrix effects
    case MF_RAINBOW:
      FOR_XY mBuf[XY(x, y)] = CHSV((uint8_t)((t >> 4) + (x + y) * (255 / (W + H))), 255, 255);
      break;
    case MF_SWIRL:
      FOR_XY {
        float dx = x - mcx, dy = y - mcy;
        float d = sqrtf(dx * dx + dy * dy);
        uint8_t hv = ang8(dy, dx) + (uint8_t)((int)(d * 14.0f)) + (uint8_t)(t >> 4);
        mBuf[XY(x, y)] = CHSV(hv, 255, 255);
      }
      break;
    case MF_TUNNEL:
      FOR_XY {
        float dx = x - mcx, dy = y - mcy;
        float d = sqrtf(dx * dx + dy * dy);
        uint8_t v = sin8((uint8_t)((int)(d * 26.0f) - (int)(t >> 2)));
        mBuf[XY(x, y)] = CHSV((uint8_t)(hue + (int)(d * 9.0f)), 255, v);
      }
      break;
    case MF_RADAR: {
      uint8_t sweep = (uint8_t)(t >> 3);
      FOR_XY {
        float dx = x - mcx, dy = y - mcy;
        uint8_t diff = (uint8_t)(sweep - ang8(dy, dx));
        uint8_t v = diff > 127 ? 0 : (uint8_t)(255 - diff * 2);
        mBuf[XY(x, y)] = CHSV(hue, 255, v);
      }
    } break;
    case MF_NOISE:
      FOR_XY {
        uint8_t nz = inoise8(x * 45, y * 45, t >> 3);
        nz = qsub8(nz, 16); nz = qadd8(nz, scale8(nz, 39));
        mBuf[XY(x, y)] = ColorFromPalette(RainbowColors_p, (uint8_t)(nz + (t >> 5)), 255);
      }
      break;
    case MF_OCEAN:
      FOR_XY {
        uint8_t nz = inoise8(x * 35 + (t >> 4), y * 70, t >> 3);
        nz = qadd8(qsub8(nz, 20), nz >> 1);
        mBuf[XY(x, y)] = ColorFromPalette(OceanColors_p, nz, 255);
      }
      break;
    case MF_CHECKER:
      FOR_XY {
        bool on = (((x >> 1) + (y >> 1) + (t / 500)) & 1);
        if (on) mBuf[XY(x, y)] = CHSV((uint8_t)(hue + (t >> 6)), 255, 255);
        else    mBuf[XY(x, y)] = CHSV((uint8_t)(hue + 128 + (t >> 6)), 255, 110);
      }
      break;
    case MF_BOUNCE: {
      if (!bbInit) {
        bbInit = true;
        bb[0] = random16(W * 10) / 10.0f; bb[1] = random16(H * 10) / 10.0f;
        bb[2] = random8(2) ? 0.17f : -0.17f; bb[3] = random8(2) ? 0.11f : -0.11f;
      }
      fadeToBlackBy(mBuf, N, 45);
      bb[0] += bb[2] * spd; bb[1] += bb[3] * spd;
      if (bb[0] < 0)     { bb[0] = 0;     bb[2] = -bb[2]; }
      if (bb[0] > W - 1) { bb[0] = W - 1; bb[2] = -bb[2]; }
      if (bb[1] < 0)     { bb[1] = 0;     bb[3] = -bb[3]; }
      if (bb[1] > H - 1) { bb[1] = H - 1; bb[3] = -bb[3]; }
      mBuf[XY((int)(bb[0] + 0.5f), (int)(bb[1] + 0.5f))] = CHSV((uint8_t)(t >> 4), 255, 255);
    } break;
    case MF_FIREWORKS: {
      fadeToBlackBy(mBuf, N, 55);
      if (!fwRocket && t > fwNext) {
        fwRocket = true;
        fwx = random16((W - 1) * 10 + 1) / 10.0f; fwy = H - 1;
        fwvy = -(0.30f + random8() / 1200.0f) * (0.4f + (H / 16.0f) * 0.6f);
        fwty = 1 + random8(H / 3 + 1); fwhue = random8();
      }
      if (fwRocket) {
        fwy += fwvy * spd;
        mBuf[XY((int)(fwx + 0.5f), (int)(fwy + 0.5f))] = CHSV(fwhue, 60, 255);
        if (fwy <= fwty) {
          fwRocket = false; fwNext = t + random16(600, 1800);
          uint8_t cnt = 14 + random8(10);
          for (auto& p : parts) {
            if (cnt == 0) break;
            if (p.on) continue;
            float a = random8() * (TWO_PI / 256.0f), s = 0.06f + random8() / 1400.0f;
            p.on = true; p.x = fwx; p.y = fwy; p.vx = cosf(a) * s; p.vy = sinf(a) * s;
            p.life = 1.0f; p.dlife = 0.022f + random8() / 9000.0f; p.hue = fwhue + random8(24);
            cnt--;
          }
        }
      }
      for (auto& p : parts) {
        if (!p.on) continue;
        p.vy += 0.0035f * spd; p.vx *= 0.985f; p.vy *= 0.985f;
        p.x += p.vx * spd; p.y += p.vy * spd; p.life -= p.dlife * spd;
        if (p.life <= 0) { p.on = false; continue; }
        CRGB c = CHSV(p.hue, 255, 255); c.nscale8((uint8_t)(p.life * 255.0f));
        mBuf[XY((int)(p.x + 0.5f), (int)(p.y + 0.5f))] += c;
      }
    } break;
    case MF_STARFIELD: {
      fadeToBlackBy(mBuf, N, 80);
      starAcc += (W * H) / 380.0f * spd + 0.05f;
      while (starAcc >= 1.0f) {
        starAcc -= 1.0f;
        for (auto& p : parts) {
          if (p.on) continue;
          float a = random8() * (TWO_PI / 256.0f), s = 0.015f + random8() / 6000.0f;
          p.on = true; p.x = mcx; p.y = mcy; p.vx = cosf(a) * s; p.vy = sinf(a) * s;
          break;
        }
      }
      const float acc = 1.0f + 0.045f * spd;
      for (auto& p : parts) {
        if (!p.on) continue;
        p.vx *= acc; p.vy *= acc;
        p.x += p.vx * spd; p.y += p.vy * spd;
        if (p.x < -0.5f || p.x > W - 0.5f || p.y < -0.5f || p.y > H - 0.5f) { p.on = false; continue; }
        float dd = fabsf(p.x - mcx) + fabsf(p.y - mcy);
        float b = 50.0f + dd * 40.0f; if (b > 255.0f) b = 255.0f;
        CRGB c = CHSV(150, 80, (uint8_t)b);
        mBuf[XY((int)(p.x + 0.5f), (int)(p.y + 0.5f))] += c;
      }
    } break;
    case MF_EQ: {
      fill_solid(mBuf, N, CRGB::Black);
      for (uint8_t x = 0; x < W; x++) {
        int nz = (int)inoise8(x * 70, t >> 2) - 40; if (nz < 0) nz = 0;
        float target = nz * H / 170.0f; if (target > H) target = H;
        hbuf[x] += (target - hbuf[x]) * 0.35f;
        if (hbuf[x] > hbuf[16 + x]) hbuf[16 + x] = hbuf[x];
        else { hbuf[16 + x] -= 0.07f; if (hbuf[16 + x] < 0) hbuf[16 + x] = 0; }
        int bh = (int)(hbuf[x] + 0.5f);
        for (int yy = 0; yy < bh; yy++) mBuf[XY(x, H - 1 - yy)] = CHSV((uint8_t)(96 - yy * 96 / H), 255, 255);
        int pk = (int)(hbuf[16 + x] + 0.5f);
        if (pk > 0 && pk <= H) mBuf[XY(x, H - pk)] = CRGB(255, 255, 255);
      }
    } break;
    case MF_HEART: {
      float sc = 1.5f - 0.22f * (0.5f + 0.5f * sinf(t * 0.0065f));
      FOR_XY {
        float nx = (x - mcx) / (W * 0.5f) * sc, ny = (mcy - y) / (H * 0.5f) * sc + 0.12f;
        float a = nx * nx + ny * ny - 1.0f;
        float v = a * a * a - nx * nx * ny * ny * ny;
        if (v <= 0) {
          float dpt = -v * 4.0f; if (dpt > 1.0f) dpt = 1.0f;
          mBuf[XY(x, y)] = CHSV(250, 255, (uint8_t)(140.0f + dpt * 115.0f));
        } else mBuf[XY(x, y)] = CRGB(0, 0, 0);
      }
    } break;
    case MF_SINEWAVE: {
      fadeToBlackBy(mBuf, N, 65);
      for (uint8_t x = 0; x < W; x++) {
        float s = sinf(x * 0.6f + t * 0.005f);
        int y1 = (int)(mcy + s * (H - 1) * 0.42f + 0.5f), y2 = (int)(mcy - s * (H - 1) * 0.42f + 0.5f);
        CRGB c1 = CHSV((uint8_t)(hue + x * 6 + (t >> 5)), 255, 255);
        CRGB c2 = CHSV((uint8_t)(hue + 128 + x * 6 + (t >> 5)), 255, 255);
        mBuf[XY(x, y1)] += c1; mBuf[XY(x, y2)] += c2;
      }
    } break;
    case MF_ORBITS: {
      fadeToBlackBy(mBuf, N, 55);
      float rmax = ((W < H ? W : H) - 1) / 2.0f;
      for (uint8_t k = 0; k < 4; k++) {
        float a = t * 0.0035f * (k + 1) * ((k & 1) ? -1.0f : 1.0f) + k * 1.7f, r = rmax * (0.3f + 0.23f * k);
        CRGB c = CHSV((uint8_t)(hue + k * 55 + (t >> 5)), 255, 255);
        mBuf[XY((int)(mcx + cosf(a) * r + 0.5f), (int)(mcy + sinf(a) * r + 0.5f))] += c;
      }
    } break;
    case MF_AURORA:
      FOR_XY {
        uint8_t nz = inoise8(x * 35, t >> 3);
        float top = H * (0.1f + 0.6f * nz / 255.0f);
        float d = y - top;
        if (d < 0) { mBuf[XY(x, y)] = CRGB(0, 0, 0); continue; }
        float b = 1.0f - d / (H * 0.55f);
        if (b < 0) b = 0;
        uint8_t sh = inoise8(x * 25 + 3000, y * 45, t >> 2);
        uint8_t v = scale8((uint8_t)(b * b * 255.0f), qadd8(sh, 70));
        mBuf[XY(x, y)] = CHSV((uint8_t)(96 + (int)(d * 7.0f)), 255, v);
      }
      break;
    case MF_METABALLS: {
      float bx[3], by[3], rr = (W < H ? W : H) * 0.22f;
      rr *= rr;
      for (uint8_t k = 0; k < 3; k++) {
        bx[k] = mcx + cosf(t * 0.0021f * (k + 1) + k * 2.1f) * (W * 0.33f);
        by[k] = mcy + sinf(t * 0.0017f * (k + 2) + k) * (H * 0.33f);
      }
      FOR_XY {
        float f = 0;
        for (uint8_t k = 0; k < 3; k++) {
          float dx = x - bx[k], dy = y - by[k];
          f += rr / (dx * dx + dy * dy + 0.6f);
        }
        float br = (f - 0.35f) * 300.0f;
        if (br < 0) br = 0;
        if (br > 255) br = 255;
        if (f > 3.0f) f = 3.0f;
        mBuf[XY(x, y)] = ColorFromPalette(PartyColors_p, (uint8_t)(f * 45.0f + (t >> 6)), (uint8_t)br);
      }
    } break;
    case MF_FIREFLIES: {
      fadeToBlackBy(mBuf, N, 50);
      uint8_t want = 3 + (W * H) / 20, have = 0;
      for (auto& p : parts) if (p.on) have++;
      if (have < want && random8() < 40) {
        for (auto& p : parts) {
          if (p.on) continue;
          p.on = true; p.x = random16(W * 100) / 100.0f; p.y = random16(H * 100) / 100.0f;
          p.vx = ((int)random8() - 128) / 3000.0f; p.vy = ((int)random8() - 128) / 3000.0f;
          p.life = 0; p.dlife = 0.006f + random8() / 20000.0f; p.hue = 55 + random8(25);
          break;
        }
      }
      for (auto& p : parts) {
        if (!p.on) continue;
        p.x += p.vx * spd; p.y += p.vy * spd;
        if (p.x < 0) p.x += W;
        if (p.x >= W) p.x -= W;
        if (p.y < 0) p.y += H;
        if (p.y >= H) p.y -= H;
        p.life += p.dlife * spd;
        if (p.life >= 1.0f) { p.on = false; continue; }
        CRGB c = CHSV(p.hue, 220, (uint8_t)(255.0f * sinf(p.life * 3.14159f)));
        mBuf[XY((int)p.x, (int)p.y)] += c;
      }
    } break;

    case MF_STATIC: fill_solid(mBuf, N, CHSV(hue, 255, 255)); break;

    case MF_TEXT: {                                      // scrolling text from the setup page
      char txt[96] = "", song[66] = "";
      portENTER_CRITICAL(&mediaMux); strlcpy(song, media.title, sizeof song); portEXIT_CRITICAL(&mediaMux);
      for (const char* p = matText; *p && strlen(txt) < sizeof txt - 9; ) {
        if (!strncmp(p, "{time}", 6)) {
          char tb[8] = "--:--";
          if (timeValid()) { time_t tt = time(nullptr); struct tm lt; localtime_r(&tt, &lt); strftime(tb, sizeof tb, cCfg.h24 ? "%H:%M" : "%I:%M", &lt); }
          strlcat(txt, tb, sizeof txt); p += 6;
        } else if (!strncmp(p, "{song}", 6)) { strlcat(txt, song[0] ? song : "-", sizeof txt); p += 6; }
        else { size_t l = strlen(txt); txt[l] = *p++; txt[l + 1] = 0; }
      }
      fill_solid(mBuf, N, CRGB::Black);
      const int w = textWidth(txt), y0 = (H - 8) / 2;
      const int x = w <= W && !strstr(matText, "{song}") ? (W - w) / 2 : textScroll(millis(), w, W);
      drawText(txt, x, y0, W, H, (uint8_t)(hue + (t >> 5)), true);
    } break;

    case MF_DAY: {                                       // colour of the time of day, like a sky
      int m = 0; CRGB c = dayColor(&m);
      FOR_XY { CRGB p = c; p.nscale8(255 - y * 90 / H); mBuf[XY(x, y)] = p; }
      if (m < 360 || m > 1260) {                          // night: a few twinkling stars
        for (uint8_t k = 0; k < 4; k++) { uint32_t hsh = (k + 1) * 2654435761u; uint8_t sx = (hsh >> 8) % W, sy = (hsh >> 16) % max(1, H / 2);
          if (((t / 300) + k) % 5) mBuf[XY(sx, sy)] = CRGB(160, 160, 120); }
      }
    } break;

    case MF_TIMER: {                                     // the timer / stopwatch (timer button)
      fill_solid(mBuf, N, CRGB::Black);
      const uint8_t st = timerState;
      const bool sw = swMode; const uint8_t sws = swState;
      if (!sw && st == T_ALARM) { const bool on = (millis() / 250) & 1; fill_solid(mBuf, N, on ? CRGB(255, 0, 0) : CRGB(40, 0, 0)); }
      const int32_t rem = timerRemaining();
      const uint32_t el = sw ? swElapsed() : 0;
      const int secs = sw ? (int)(el / 1000) : (rem + 999) / 1000, mm = (secs / 60) % 100, ss = secs % 60;
      CRGB col;
      const uint32_t tot = timerTotalMs ? timerTotalMs : 1;
      if (sw) col = sws == SW_RUN ? CRGB(0, 200, 255) : sws == SW_PAUSE ? (((millis() / 400) & 1) ? CRGB(0, 200, 255) : CRGB(0, 70, 90))
                                                                    : CRGB(40, 40, 40);   // stopwatch: cyan, blinking when paused
      else if (st == T_IDLE) col = CRGB(40, 40, 40);
      else if (st == T_SET)  col = ((millis() / 400) & 1) ? CRGB(255, 140, 0) : CRGB(120, 60, 0);   // blinking while setting
      else if (st == T_RUN)  col = CHSV((uint8_t)(96 * (uint32_t)rem / tot), 255, 255);   // green -> red
      else                   col = CRGB::White;
      auto bar = [&](int row) {                           // timer: each LED = a slice of the time left. stopwatch: the seconds of this minute
        if (row < 0 || row >= H) return;
        if (sw ? sws == SW_IDLE : st != T_RUN) return;
        const float f = sw ? (float)(el % 60000) * W / 60000.0f : (float)rem * W / (float)tot;
        for (int x = 0; x < W; x++) {
          float part = f - x;
          if (part <= 0) continue;
          if (part > 1) part = 1;
          CRGB c = col; c.nscale8((uint8_t)(20 + part * 100));   // never fully on (at most about 45 %)
          mBuf[XY(x, row)] = c;
        }
      };
      if (H >= 12) {                                      // minutes on top, seconds below, progress bar at the bottom
        int x0 = (W - 7) / 2;
        drawDigit(mm / 10, x0, 1, col, W, H); drawDigit(mm % 10, x0 + 4, 1, col, W, H);
        drawDigit(ss / 10, x0, 7, col, W, H); drawDigit(ss % 10, x0 + 4, 7, col, W, H);
        bar(H - 1);
      } else if (W >= 13 && mm < 10) {                     // one line: m:ss
        int x = (W - 13) / 2, y = (H - 5) / 2;
        x += drawDigit(mm, x, y, col, W, H) + 1; x += drawDigit(10, x, y, col, W, H) + 1;
        x += drawDigit(ss / 10, x, y, col, W, H) + 1; drawDigit(ss % 10, x, y, col, W, H);
        bar(H - 1);
      } else {                                            // small matrix: minutes (or seconds in the last minute)
        int v = mm ? mm : ss, x = (W - 7) / 2, y = (H - 5) / 2;
        drawDigit(v / 10 % 10, x, y, col, W, H); drawDigit(v % 10, x + 4, y, col, W, H);
        bar(H - 1);                                       // on 8x8: the bottom row below the digits
      }
    } break;

    case MF_JOY: {                                       // a light that follows the joystick
      fadeToBlackBy(mBuf, N, 40);
      int x = (int)((gStickX + 1.0f) * 0.5f * (W - 1) + 0.5f), y = (int)((gStickY + 1.0f) * 0.5f * (H - 1) + 0.5f);
      CRGB c = CHSV((uint8_t)(hue + (t >> 4)), 230, 255);
      mBuf[XY(x, y)] = c;
      CRGB d = c; d.nscale8(90);
      mBuf[XY(x + 1, y)] += d; mBuf[XY(x - 1, y)] += d; mBuf[XY(x, y + 1)] += d; mBuf[XY(x, y - 1)] += d;
    } break;

    case MF_SNAKE:    gameSnake(W, H, N); break;
    case MF_PONG:     gamePong(W, H, N); break;
    case MF_BREAKOUT: gameBreakout(W, H, N); break;
    case MF_DODGE:    gameDodge(W, H, N); break;
    case MF_TETRIS:   gameTetris(W, H, N); break;
    case MF_FLAPPY:   gameFlappy(W, H, N); break;
    case MF_INVADERS: gameInvaders(W, H, N); break;
    default: fill_solid(mBuf, N, CRGB::Black); break;
  }
}

// Night mode: brightness scale for the LEDs (255 = day). Checked every 2 s.
static uint8_t nightScale() {
  static uint32_t last = 0; static uint8_t v = 255;
  if (last && millis() - last < 2000) return v;
  last = millis();
  if (!cCfg.night || !timeValid()) return v = 255;
  time_t tt = time(nullptr); struct tm lt; localtime_r(&tt, &lt);
  const int h = lt.tm_hour, a = cCfg.nfrom, b = cCfg.nto;
  const bool in = a == b ? false : a < b ? (h >= a && h < b) : (h >= a || h < b);   // e.g. 22 -> 7 crosses midnight
  return v = in ? (uint8_t)max(1, cCfg.nlvl * 255 / 100) : 255;
}

static void renderFrame(uint32_t dt) {
  static uint8_t lastSFx = 255, lastMFx = 255;
  static bool sWasOff = false, mWasOff = false;
  static uint8_t dimLvl = 255;
  sT += (dt * (sCfg.speed + 16u)) >> 7;
  mT += (dt * (mCfg.speed + 16u)) >> 7;
  const bool conn = hid && hid->isConnected();
  const uint8_t fl = flashLevel;
  const bool sOff = stripOff, mOff = matOff;

  // ---- idle dimming: fade down slowly after DIM_AFTER_MS, come back quickly on any use
  uint8_t target = (cfgMode || forceWhite || timerState != T_IDLE || swState != SW_IDLE || (matOn && isGameFx(curMFx)) || idleMs() < (int32_t)DIM_AFTER_MS) ? 255 : DIM_LEVEL;
  if (dimLvl < target)      dimLvl = (target - dimLvl > 25) ? dimLvl + 25 : target;
  else if (dimLvl > target) dimLvl = (dimLvl - target > 2)  ? dimLvl - 2  : target;
  const uint8_t bs = batBrightScale();                  // battery saver: dimmer as the battery gets low
  const uint8_t ns = nightScale(); nightNow = ns;      // night mode
  uint8_t sB = scale8(scale8(scale8(sCfg.bright, dimLvl), bs), ns); if (!sB) sB = 1;
  uint8_t mB = scale8(scale8(scale8(mCfg.bright, dimLvl), bs), ns); if (!mB) mB = 1;

  // ---- strip
  uint16_t n = sCfg.count;
  if (forceWhite)    fill_solid(sBuf, n, CRGB::White);
  else if (cfgMode)  fill_solid(sBuf, n, CHSV(25, 255, tbeat(millis(), 45, 30, 255)));       // amber = config mode
  else if (sOff)     { fill_solid(sBuf, n, CRGB(0, 0, 0)); sWasOff = true; }                 // switched off by 3 s hold
  else if (!conn)    fill_solid(sBuf, n, CHSV(160, 255, tbeat(millis(), 20, 10, 140)));      // blue = waiting for host
  else {
    uint8_t fx = curSFx;
    if (fx != lastSFx || sWasOff) { lastSFx = fx; sWasOff = false; fill_solid(sBuf, MAX_STRIP, CRGB::Black); memset(heat1, 0, sizeof heat1); }
    stripFx(fx, n);
  }
  CRGB sw = CRGB::White; sw.nscale8_video(sB);
  for (uint16_t i = 0; i < n; i++) {
    sOut[i] = sBuf[i]; sOut[i].nscale8_video(sB);
    if (fl && !sOff) nblend(sOut[i], sw, fl >> 1);
  }

  // ---- matrix
  if (matOn) {
    uint8_t W = mCfg.w, H = mCfg.h; uint16_t N = W * H;
    if (forceWhite)   fill_solid(mBuf, N, CRGB::White);
    else if (cfgMode) fill_solid(mBuf, N, CHSV(25, 255, tbeat(millis(), 45, 30, 255)));
    else if (mOff)    { fill_solid(mBuf, N, CRGB(0, 0, 0)); mWasOff = true; }
    else {
      uint8_t fx = curMFx;
      if (fx != lastMFx || mWasOff) { lastMFx = fx; mWasOff = false; resetMat(); }
      matFx(fx, W, H, N);
    }
    CRGB mw = CRGB::White; mw.nscale8_video(mB);
    for (uint16_t i = 0; i < N; i++) {
      mOut[i] = mBuf[i]; mOut[i].nscale8_video(mB);
      if (fl && !mOff) nblend(mOut[i], mw, fl >> 1);
    }
  }
  // ---- about to sleep: strip and matrix flash amber twice a second (at full brightness, not dimmed)
  if (!cfgMode && !forceWhite && sleepAllowed() && idleMs() > (int32_t)(sleepAfterMs() - WARN_MS)) {
    const bool on = (millis() / 250) & 1;
    if (!sOff) { CRGB c = on ? CRGB(CHSV(25, 255, 255)) : CRGB(0, 0, 0); c.nscale8_video(sCfg.bright); fill_solid(sOut, n, c); }
    if (matOn && !mOff) { CRGB c = on ? CRGB(CHSV(25, 255, 255)) : CRGB(0, 0, 0); c.nscale8_video(mCfg.bright); fill_solid(mOut, mCfg.w * mCfg.h, c); }
  }
  const int8_t bp = batPct;
  if (!cfgMode && bp >= 0 && bp <= 10) {                // low battery: red flashes (3 s at first, then every 30 s)
    const uint32_t now = millis(), lowAt = batLowAt;
    if ((lowAt && now - lowAt < 3000) || (now % 30000) < 900) {
      const bool on = (now / 150) & 1;
      CRGB c = on ? CRGB(255, 0, 0) : CRGB(0, 0, 0);
      CRGB cs = c; cs.nscale8_video(max<uint8_t>(sCfg.bright, 80)); fill_solid(sOut, n, cs);
      if (matOn && !mOff) { CRGB cm = c; cm.nscale8_video(mCfg.bright); fill_solid(mOut, mCfg.w * mCfg.h, cm); }
    }
  }
  if (timerState == T_ALARM && !cfgMode) {              // time is up: strip (and matrix) flash red
    const bool on = (millis() / 250) & 1;
    CRGB c = on ? CRGB(255, 0, 0) : CRGB(0, 0, 0);
    CRGB cs = c; cs.nscale8_video(max<uint8_t>(sCfg.bright, 80)); fill_solid(sOut, n, cs);
    if (matOn && curMFx != MF_TIMER) { CRGB cm = c; cm.nscale8_video(mCfg.bright); fill_solid(mOut, mCfg.w * mCfg.h, cm); }
  }
  if (fl) flashLevel = scale8(fl, 180);                 // key flash fade: lower = faster (was 225)
  FastLED.show();
}

// ============================ OLED (0.66" 64x48, SSD1306, I2C) ============================
#if USE_OLED
  #if OLED_HW_I2C
static U8G2_SSD1306_64X48_ER_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);
  #else
static U8G2_SSD1306_64X48_ER_F_SW_I2C oled(U8G2_R0, /*clock=*/PIN_OLED_SCL, /*data=*/PIN_OLED_SDA, U8X8_PIN_NONE);
  #endif
static void showStatus(const String& l1, const String& l2) {          // used only during setup()
  oled.clearBuffer();
  oled.setFont(u8g2_font_8x13B_tr);
  oled.drawStr(0, 14, l1.substring(0, 8).c_str());
  oled.setFont(u8g2_font_5x7_tr);
  oled.drawStr(0, 30, l2.substring(0, 12).c_str());
  oled.sendBuffer();
}
#else
static void showStatus(const String&, const String&) {}
#endif

// Starts the OLED first thing in setup() and prints to Serial whether it answers on the I2C bus
static void oledBegin() {
#if USE_OLED
  #if OLED_HW_I2C
  oled.setBusClock(400000);
  #endif
  oled.begin();
  showStatus("BOOT", "starting...");
  #if OLED_HW_I2C
  const uint8_t addrs[] = {0x3C, 0x3D};
  for (uint8_t a : addrs) {
    Wire.beginTransmission(a);
    Serial.printf("OLED at 0x%02X: %s\n", a, Wire.endTransmission() == 0 ? "found" : "not found");
  }
  #endif
#endif
}

// ============================ Clock (internet time over the home Wi-Fi) ============================
// Only used when the OLED main screen is "Clock". Joins the home Wi-Fi for a few seconds, gets the time,
// then turns Wi-Fi off again, so it never disturbs Bluetooth or the wireless joystick.
// The ESP32 keeps the time through deep sleep and restarts, so it only reconnects when the time is
// unknown or more than CLOCK_RESYNC_S old. After a failed try it waits CLOCK_RETRY_FAIL_S.
static bool clockWanted() {                              // the time is needed for the clock, day colour or {time}
  return cCfg.night || (cCfg.on && (oHome == HOME_CLOCK || curSFx == SF_DAY || (matOn && (curMFx == MF_DAY || (curMFx == MF_TEXT && strstr(matText, "{time}"))))));
}

static volatile bool ckBusy = false;                     // getting the time right now (in the background)
static void clockTask(void*);
// Called once at start: decides if the time is needed and, if so, gets it in the background
// so Bluetooth and the wireless stick start straight away.
static void syncClock() {
  setenv("TZ", cCfg.tz, 1); tzset();                     // timezone has to be set again after every boot / wake
  if (ckMagic != CK_MAGIC) { ckMagic = CK_MAGIC; ckLastOk = 0; ckLastFail = 0; ckState = CK_NEVER; }   // first power-on
  if (!clockWanted()) return;                            // main screen is not the clock: no Wi-Fi at all
  if (!cCfg.ssid[0]) return;                             // no Wi-Fi set (the OLED says so)
  const time_t now = time(nullptr);
  if (timeValid() && ckLastOk && now >= ckLastOk && now - ckLastOk < (time_t)CLOCK_RESYNC_S) return;   // time is fresh
  if (ckLastFail && now >= ckLastFail && now - ckLastFail < (time_t)CLOCK_RETRY_FAIL_S) return;          // failed a moment ago
  ckBusy = true;
  xTaskCreatePinnedToCore(clockTask, "clock", 6144, nullptr, 1, nullptr, 1);
}
static void clockTask(void*) {
  Serial.printf("Clock: joining '%s'...\n", cCfg.ssid);
  WiFi.persistent(false);                                // don't store the home Wi-Fi in flash
  WiFi.mode(WIFI_STA);
  WiFi.begin(cCfg.ssid, cCfg.pass);
  uint32_t t0 = millis(); int lastShown = -1;
  (void)lastShown;
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < CLOCK_WIFI_TIMEOUT_MS) delay(100);
  bool ok = false;
  if (WiFi.status() == WL_CONNECTED) {
    sntp_set_sync_status(SNTP_SYNC_STATUS_RESET);
    configTzTime(cCfg.tz, "pool.ntp.org", "time.google.com");
    t0 = millis();
    lastShown = -1;
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && millis() - t0 < CLOCK_NTP_TIMEOUT_MS) delay(100);
    ok = sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
    if (ok) { ckState = CK_OK; ckLastOk = time(nullptr); ckLastFail = 0; Serial.println("Clock: time set"); }
    else    { ckState = CK_NOSERVER; Serial.println("Clock: no answer from the time server"); }
  } else {
    ckState = CK_NOWIFI;
    Serial.println("Clock: unable to connect to the Wi-Fi within 30 s (2.4 GHz network? right password?)");
  }
  if (!ok) { ckLastFail = time(nullptr); if (!ckLastFail) ckLastFail = 1; }
  if (esp_sntp_enabled()) esp_sntp_stop();
  if (needRemote) {                                      // keep the radio on for the stick: back to its channel
    WiFi.disconnect();
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  } else {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
  setenv("TZ", cCfg.tz, 1); tzset();
  ckBusy = false;
  vTaskDelete(nullptr);
}

// ============================ OLED animations (64x48, drawn straight into the display buffer) ============================
#if USE_OLED
static uint8_t* ob = nullptr;            // u8g2 frame buffer: 6 pages of 8 rows, 1 bit per pixel
static int obw = 64;
static inline void px(int x, int y)    { if ((unsigned)x < 64u && (unsigned)y < 48u) ob[(y >> 3) * obw + x] |=  (uint8_t)(1 << (y & 7)); }
static inline void pxOff(int x, int y) { if ((unsigned)x < 64u && (unsigned)y < 48u) ob[(y >> 3) * obw + x] &= (uint8_t)~(1 << (y & 7)); }
static inline void pxSet(int x, int y, bool on) { if (on) px(x, y); else pxOff(x, y); }
static uint32_t h32(uint32_t x) { x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16; return x; }

static void ln(int x0, int y0, int x1, int y1) {                       // clipped line
  if (abs(x0) > 300 || abs(y0) > 300 || abs(x1) > 300 || abs(y1) > 300) return;
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1, e = dx + dy;
  for (;;) {
    px(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * e;
    if (e2 >= dy) { e += dy; x0 += sx; }
    if (e2 <= dx) { e += dx; y0 += sy; }
  }
}
static void hln(int x0, int x1, int y, bool on = true) {
  if ((unsigned)y >= 48u) return;
  if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
  if (x0 < 0) x0 = 0;
  if (x1 > 63) x1 = 63;
  for (int x = x0; x <= x1; x++) pxSet(x, y, on);
}
static void fbox(int x, int y, int w, int h, bool on = true) { for (int j = 0; j < h; j++) hln(x, x + w - 1, y + j, on); }
static void rframe(int x, int y, int w, int h) {
  hln(x, x + w - 1, y); hln(x, x + w - 1, y + h - 1);
  for (int j = 0; j < h; j++) { px(x, y + j); px(x + w - 1, y + j); }
}
static void circ(int cx, int cy, int r) {
  int x = r, y = 0, e = 1 - r;
  while (x >= y) {
    px(cx + x, cy + y); px(cx + y, cy + x); px(cx - y, cy + x); px(cx - x, cy + y);
    px(cx - x, cy - y); px(cx - y, cy - x); px(cx + y, cy - x); px(cx + x, cy - y);
    y++;
    if (e < 0) e += 2 * y + 1; else { x--; e += 2 * (y - x) + 1; }
  }
}
static void fell(int cx, int cy, int rx, int ry, bool on = true) {      // filled ellipse
  if (ry <= 0) { hln(cx - rx, cx + rx, cy, on); return; }
  for (int dy = -ry; dy <= ry; dy++) {
    float f = 1.0f - (float)(dy * dy) / (float)(ry * ry);
    int hw = (int)(rx * sqrtf(f > 0 ? f : 0) + 0.5f);
    hln(cx - hw, cx + hw, cy + dy, on);
  }
}
static void disc(int cx, int cy, int r, bool on = true) { fell(cx, cy, r, r, on); }

// ---- small scenes
static void drawRocket(int cx, int yb, int flame) {                     // yb = bottom of the rocket
  for (int j = 0; j < 5; j++) { int hw = j * 3 / 4; hln(cx - hw, cx + hw, yb - 19 + j); }   // nose
  fbox(cx - 3, yb - 14, 7, 12);                                          // body
  disc(cx, yb - 10, 1, false);                                           // window
  for (int j = 0; j < 4; j++) { px(cx - 4, yb - 1 - j); px(cx + 4, yb - 1 - j); }          // fins
  px(cx - 5, yb - 1); px(cx - 5, yb - 2); px(cx + 5, yb - 1); px(cx + 5, yb - 2);
  hln(cx - 2, cx + 2, yb - 2);
  for (int k = 0; k < flame; k++) {                                       // flickering flame
    int w = 2 - k / 3; if (w < 0) w = 0;
    for (int x = cx - w; x <= cx + w; x++) if (random8() < 190) px(x, yb - 1 + k);
  }
}
static void drawFish(int x, int y, int dir, int rx) {
  int ry = rx * 3 / 5;
  fell(x, y, rx, ry);
  int tx = x - dir * (rx + 1);
  ln(tx, y, tx - dir * 3, y - 3); ln(tx, y, tx - dir * 3, y + 3); ln(tx - dir * 3, y - 3, tx - dir * 3, y + 3);
  pxOff(x + dir * (rx - 2), y - 1);                                       // eye
}

static void drawAnim(uint8_t a, uint32_t t) {
  static uint8_t cur = 255; static uint32_t lastT = 0;
  const bool init = (a != cur);
  cur = a;
  float dt = init ? 1.0f : (t - lastT) / 33.0f;
  if (dt > 3.0f) dt = 3.0f;
  lastT = t;
  ob = oled.getBufferPtr(); obw = oled.getBufferTileWidth() * 8;

  switch (a) {
    case OA_STRIPES: {                                                   // diagonal stripes, width breathes
      int off = (t / 30) & 15, w = 3 + (sin8((uint8_t)(t / 40)) >> 6);
      for (int y = 0; y < 48; y++) for (int x = 0; x < 64; x++) if (((x + y + off) & 15) < w) px(x, y);
    } break;

    case OA_RAIN: {                                                      // falling streaks
      static float ry[16], rv[16]; static uint8_t rl[16];
      for (int i = 0; i < 16; i++) {
        if (init) { ry[i] = -(float)random8(48); rv[i] = 0.8f + random8() / 128.0f; rl[i] = 4 + random8(10); }
        ry[i] += rv[i] * dt;
        if (ry[i] - rl[i] > 48) { ry[i] = -(float)random8(20); rv[i] = 0.8f + random8() / 128.0f; rl[i] = 4 + random8(10); }
        int x = i * 4 + 1, y = (int)ry[i];
        for (int k = 0; k < rl[i]; k++) if (k < rl[i] / 2 || ((y - k) & 1)) px(x, y - k);
      }
    } break;

    case OA_STARS: {                                                     // flying through stars
      static float sx[40], sy[40], sz[40];
      for (int i = 0; i < 40; i++) {
        if (init) { sx[i] = ((int)random16(2000) - 1000) / 1000.0f; sy[i] = ((int)random16(2000) - 1000) / 1000.0f; sz[i] = 0.1f + random8() / 280.0f; }
        sz[i] -= 0.012f * dt;
        int X = 32 + (int)(sx[i] / (sz[i] > 0.02f ? sz[i] : 0.02f) * 20), Y = 24 + (int)(sy[i] / (sz[i] > 0.02f ? sz[i] : 0.02f) * 20);
        if (sz[i] <= 0.05f || X < 0 || X > 63 || Y < 0 || Y > 47) {
          sx[i] = ((int)random16(2000) - 1000) / 1000.0f; sy[i] = ((int)random16(2000) - 1000) / 1000.0f; sz[i] = 1.0f;
          continue;
        }
        px(X, Y);
        if (sz[i] < 0.3f) { px(X + 1, Y); px(X, Y + 1); px(X + 1, Y + 1); }
      }
    } break;

    case OA_CUBE: {                                                      // rotating wireframe cube
      static const int8_t V[8][3] = {{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};
      static const uint8_t E[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
      float A = t * 0.0011f, B = t * 0.0007f, C = t * 0.0005f;
      float ca = cosf(A), sa = sinf(A), cb = cosf(B), sb = sinf(B), cc = cosf(C), sc = sinf(C);
      int P[8][2];
      for (int i = 0; i < 8; i++) {
        float x = V[i][0], y = V[i][1], z = V[i][2], t1;
        t1 = y * ca - z * sa; z = y * sa + z * ca; y = t1;               // around X
        t1 = x * cb + z * sb; z = -x * sb + z * cb; x = t1;              // around Y
        t1 = x * cc - y * sc; y = x * sc + y * cc; x = t1;               // around Z
        z += 4.0f;
        P[i][0] = 32 + (int)(x * 30.0f / z); P[i][1] = 24 + (int)(y * 30.0f / z);
      }
      for (auto& e : E) ln(P[e[0]][0], P[e[0]][1], P[e[1]][0], P[e[1]][1]);
    } break;

    case OA_WAVES:                                                       // three sine waves
      for (int k = 0; k < 3; k++) {
        int py = 0;
        for (int x = 0; x < 64; x++) {
          int y = 24 + (int)(sinf(x * 0.12f + t * 0.003f * (k + 1) + k * 2.1f) * (6 + k * 5));
          if (x) ln(x - 1, py, x, y); else px(x, y);
          py = y;
        }
      }
      break;

    case OA_BOUNCE: {                                                    // bouncing balls with gravity
      static float bx[4], by[4], vx[4], vy[4]; static uint8_t br[4];
      for (int i = 0; i < 4; i++) {
        if (init) { br[i] = 3 + (i & 1) * 2; bx[i] = 8 + i * 14; by[i] = 6 + random8(20); vx[i] = (random8(2) ? 1 : -1) * (0.6f + random8() / 300.0f); vy[i] = 0; }
        vy[i] += 0.15f * dt; bx[i] += vx[i] * dt; by[i] += vy[i] * dt;
        int r = br[i];
        if (bx[i] < r)      { bx[i] = r;      vx[i] = -vx[i]; }
        if (bx[i] > 63 - r) { bx[i] = 63 - r; vx[i] = -vx[i]; }
        if (by[i] > 47 - r) { by[i] = 47 - r; vy[i] = -vy[i] * 0.92f; if (fabsf(vy[i]) < 1.5f) vy[i] = -(3.0f + random8(3)); }
        if (i & 1) disc((int)bx[i], (int)by[i], r); else circ((int)bx[i], (int)by[i], r);
      }
    } break;

    case OA_LIFE: {                                                      // Game of Life, 32x24 cells of 2x2 px
      static uint8_t g[24][32], n2[24][32]; static uint32_t lastStep = 0; static uint16_t gen = 0;
      auto seed = [&]() { for (int y = 0; y < 24; y++) for (int x = 0; x < 32; x++) g[y][x] = random8() < 70; gen = 0; };
      if (init) seed();
      if (t - lastStep >= 120) {
        lastStep = t;
        uint16_t alive = 0; bool same = true;
        for (int y = 0; y < 24; y++) for (int x = 0; x < 32; x++) {
          int n = 0;
          for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++)
            if (dx || dy) n += g[(y + dy + 24) % 24][(x + dx + 32) % 32];
          uint8_t c = g[y][x], nw = (c && (n == 2 || n == 3)) || (!c && n == 3);
          n2[y][x] = nw; alive += nw; if (nw != c) same = false;
        }
        memcpy(g, n2, sizeof g); gen++;
        if (alive < 5 || same || gen > 300) seed();
      }
      for (int y = 0; y < 24; y++) for (int x = 0; x < 32; x++) if (g[y][x]) fbox(x * 2, y * 2, 2, 2);
    } break;

    case OA_SNOW: {                                                      // snow that piles up, then melts
      static float fx[40], fy[40]; static uint8_t heap[64];
      if (init) { memset(heap, 0, sizeof heap); for (int i = 0; i < 40; i++) { fx[i] = random8(64); fy[i] = -(float)random8(48); } }
      uint16_t sum = 0;
      for (int x = 0; x < 64; x++) sum += heap[x];
      if (sum > 64 * 12) memset(heap, 0, sizeof heap);
      for (int i = 0; i < 40; i++) {
        fy[i] += (0.35f + (i % 3) * 0.2f) * dt;
        fx[i] += sinf(t * 0.002f + i) * 0.3f * dt;
        if (fx[i] < 0) fx[i] += 64;
        if (fx[i] >= 64) fx[i] -= 64;
        int x = (int)fx[i];
        if (fy[i] >= 47 - heap[x]) {
          if (x > 0 && heap[x] > heap[x - 1] + 1) x--;
          else if (x < 63 && heap[x] > heap[x + 1] + 1) x++;
          if (heap[x] < 30) heap[x]++;
          fx[i] = random8(64); fy[i] = -(float)random8(10);
          continue;
        }
        px(x, (int)fy[i]);
        if (i % 5 == 0) px(x + 1, (int)fy[i]);
      }
      for (int x = 0; x < 64; x++) for (int k = 0; k < heap[x]; k++) px(x, 47 - k);
    } break;

    case OA_TUNNEL:                                                      // zooming rectangles
      for (int k = 0; k < 6; k++) {
        int p = (int)((t / 18 + k * 12) % 72), hw = p * 34 / 72, hh = hw * 26 / 34;
        if (hw > 1) rframe(32 - hw, 24 - hh, hw * 2, hh * 2);
      }
      ln(0, 0, 28, 21); ln(63, 0, 35, 21); ln(0, 47, 28, 26); ln(63, 47, 35, 26);
      break;

    case OA_EYES: {                                                      // two eyes looking around and blinking
      static float ex = 0, ey = 0, tx = 0, ty = 0; static uint32_t nextMove = 0, nextBlink = 0, blinkStart = 0;
      if (init) { nextMove = t; nextBlink = t + 2000; }
      if (t >= nextMove)  { tx = (int)random8(13) - 6; ty = (int)random8(9) - 4; nextMove = t + 600 + random16(1800); }
      if (t >= nextBlink) { blinkStart = t; nextBlink = t + 2000 + random16(3000); }
      ex += (tx - ex) * 0.25f * dt; ey += (ty - ey) * 0.25f * dt;
      if (ex > 6) ex = 6;
      if (ex < -6) ex = -6;
      const bool closed = t - blinkStart < 140;
      for (int cx : {17, 47}) {
        if (closed) { hln(cx - 12, cx + 12, 24); hln(cx - 12, cx + 12, 25); continue; }
        circ(cx, 24, 13); circ(cx, 24, 12);
        int qx = cx + (int)ex, qy = 24 + (int)ey;
        disc(qx, qy, 5);
        pxOff(qx - 2, qy - 2); pxOff(qx - 1, qy - 2); pxOff(qx - 2, qy - 1);   // shine
      }
    } break;

    case OA_ROCKET: {                                                    // short movie: launch, then flight through space (9 s loop)
      uint32_t m = t % 9000;
      for (int i = 0; i < 18; i++) {                                      // stars
        int sx = (int)(h32(i * 2 + 7) % 64), sy = (int)(h32(i * 2 + 8) % 40);
        if (m >= 4500) sy = (int)((sy + (m - 4500) / 8) % 48);
        if (((t / 200) + i) % 7) px(sx, sy);
      }
      if (m < 4500) {
        hln(0, 63, 44);                                                   // ground + launch pad
        fbox(25, 42, 14, 2);
        int yb = 41, flame = 1 + random8(2);
        if (m > 1500) {
          float d = (m - 1500) / 1000.0f;
          yb = 41 - (int)(d * d * 12);
          flame = 4 + random8(5);
          int grow = (int)((m - 1500) / 200); if (grow > 9) grow = 9;     // smoke clouds billowing out
          static const int8_t SX[5] = {-15, -8, 0, 8, 15}, SR[5] = {4, 6, 7, 6, 4};
          for (int k = 0; k < 5; k++) {
            int r = SR[k] * grow / 9; if (r < 1) continue;
            int cx = 32 + SX[k] * grow / 9;
            disc(cx, 44 - r, r); disc(cx, 44 - r, r - 1, false);           // outlined puff
          }
        }
        drawRocket(32, yb, flame);
      } else {
        int py = -10 + (int)((m - 4500) / 60);                           // planet passing by
        circ(12, py, 8); hln(2, 22, py + 2);
        drawRocket(40, 34 + (((t / 150) & 1) ? 1 : 0), 4 + random8(5));
      }
    } break;

    case OA_DRIVE: {                                                     // short movie: car driving through a city at night
      disc(54, 7, 5); disc(56, 6, 5, false);                             // moon
      for (int i = 0; i < 6; i++) px((i * 29 + 3) % 40, (i * 7 + 2) % 12);
      uint32_t scroll = t / 45;
      for (int i = (int)(scroll / 11) - 1; i <= (int)(scroll / 11) + 7; i++) {
        int x = i * 11 - (int)scroll, h = 8 + (int)(h32(i) % 16), top = 36 - h;
        rframe(x, top, 10, h + 1);
        for (int wy = top + 2; wy < 34; wy += 3)
          for (int wx = x + 2; wx < x + 8; wx += 3)
            if (h32(i * 131 + wy * 7 + wx) % 3 == 0) { px(wx, wy); px(wx + 1, wy); }
      }
      hln(0, 63, 37);                                                     // road
      for (int x = 0; x < 64; x++) if (((x + t / 12) % 10) < 5) px(x, 42);
      hln(0, 63, 47);
      int b = (t / 120) & 1;                                              // car, bobbing a little
      fbox(10, 36 + b, 24, 5);
      for (int j = 0; j < 4; j++) hln(16 - j, 27 + j, 32 + j + b);
      fbox(17, 33 + b, 4, 2, false); fbox(23, 33 + b, 4, 2, false);       // windows
      for (int wx : {15, 29}) {                                           // wheels with a turning spoke
        disc(wx, 42, 3); pxOff(wx, 42);
        float an = t * 0.02f; pxOff(wx + (int)(cosf(an) * 2), 42 + (int)(sinf(an) * 2));
      }
      for (int k = 0; k < 8; k++) if ((k + t / 60) % 2) px(36 + k * 3, 38 + b + k / 3);   // headlight beam
    } break;

    case OA_FISH: {                                                      // fish tank
      static float fxp[3], fyp[3], fv[3]; static float bbx[6], bby[6];
      if (init) {
        for (int i = 0; i < 3; i++) { fxp[i] = random8(64); fyp[i] = 10 + random8(24); fv[i] = (random8(2) ? 1 : -1) * (0.4f + random8() / 500.0f); }
        for (int i = 0; i < 6; i++) { bbx[i] = 44 + random8(6); bby[i] = 47 - random8(48); }
      }
      for (int x = 0; x < 64; x++) { px(x, 47); if (h32(x) % 3 == 0) px(x, 46); }   // sand
      for (int s = 0; s < 3; s++) {                                       // seaweed
        int base = 8 + s * 23, len = 14 + s * 3;
        for (int k = 0; k < len; k++) px(base + (int)(sinf(k * 0.3f + t * 0.003f + s) * 2), 46 - k);
      }
      for (int i = 0; i < 3; i++) {
        fxp[i] += fv[i] * dt;
        if (fxp[i] < -12 || fxp[i] > 76) { fv[i] = -fv[i]; fyp[i] = 8 + random8(26); }
        drawFish((int)fxp[i], (int)(fyp[i] + sinf(t * 0.003f + i) * 1.5f), fv[i] > 0 ? 1 : -1, 4 + i);
      }
      for (int i = 0; i < 6; i++) {                                       // bubbles
        bby[i] -= 0.5f * dt; bbx[i] += sinf(t * 0.005f + i) * 0.2f;
        if (bby[i] < -2) { bbx[i] = 44 + random8(6); bby[i] = 46; }
        circ((int)bbx[i], (int)bby[i], (i % 2) + 1);
      }
    } break;
  }
}
#endif

// ============================ OLED movies ============================
// Four short movies played one after another. Each one: animated title card (border draws itself,
// "MOVIE n" and the title are typed, then an iris opens on the first scene), the story, then the
// ending (iris closes, "THE END" is typed, the screen dissolves) and the next movie starts.
#if USE_OLED
static void typeStr(int x, int y, const char* s, int n, bool cursor) {   // first n characters + blinking cursor
  char b[16]; int len = (int)strlen(s); if (n > len) n = len; if (n < 0) n = 0; if (n > 15) n = 15;
  memcpy(b, s, n); b[n] = 0;
  oled.drawStr(x, y, b);
  if (cursor) { int cx = x + oled.getStrWidth(b); oled.drawBox(cx + 1, y - 1, oled.getMaxCharWidth() - 2, 2); }
}
static void irisMask(int r) {                                            // clear everything outside a circle
  for (int y = 0; y < 48; y++) for (int x = 0; x < 64; x++) {
    int dx = x - 32, dy = y - 24;
    if (dx * dx + dy * dy > r * r) pxOff(x, y);
  }
  if (r > 0 && r < 42) circ(32, 24, r);
}
static void drawStars(uint8_t n, int shift, uint32_t t, int maxY = 48) {
  for (int i = 0; i < n; i++) {
    int sx = (int)(h32(i * 2 + 7) % 64), sy = (int)((h32(i * 2 + 8) % maxY + shift) % maxY);
    if (((t / 200) + i) % 7) px(sx, sy);
  }
}
static void drawTree(int x, int base) {
  ln(x, base, x, base - 3);
  for (int j = 0; j < 8; j++) { int hw = (7 - j) / 2; hln(x - hw, x + hw, base - 4 - j); }
}
static void drawHouse(int x, int base, bool lit, uint32_t t) {
  rframe(x, base - 12, 18, 13);
  for (int j = 0; j < 10; j++) { if (x + 18 - j < x - 1 + j) break; hln(x - 1 + j, x + 18 - j, base - 13 - j); }
  fbox(x + 13, base - 20, 3, 5);                                           // chimney
  rframe(x + 3, base - 6, 4, 7);                                           // door
  if (lit) fbox(x + 10, base - 9, 5, 4); else rframe(x + 10, base - 9, 5, 4);
  if (lit) for (int k = 0; k < 3; k++) {                                   // chimney smoke
    int ph = (int)((t / 40 + k * 15) % 45);
    circ(x + 14 + ph / 10, base - 22 - ph / 3, 1 + ph / 20);
  }
}
static void drawCar(int b, bool moving, uint32_t t, bool lights) {
  fbox(10, 36 + b, 24, 5);
  for (int j = 0; j < 4; j++) hln(16 - j, 27 + j, 32 + j + b);
  fbox(17, 33 + b, 4, 2, false); fbox(23, 33 + b, 4, 2, false);
  for (int wx : {15, 29}) {
    disc(wx, 42, 3); pxOff(wx, 42);
    float an = (moving ? t : 0) * 0.02f; pxOff(wx + (int)(cosf(an) * 2), 42 + (int)(sinf(an) * 2));
  }
  if (lights) for (int k = 0; k < 8; k++) if ((k + t / 60) % 2) px(36 + k * 3, 38 + b + k / 3);
}
static void drawHeart(int x, int y) {
  static const uint8_t H[6] = {0x36, 0x7F, 0x7F, 0x3E, 0x1C, 0x08};       // 7x6
  for (int r = 0; r < 6; r++) for (int c = 0; c < 7; c++) if (H[r] & (0x40 >> c)) px(x + c, y + r);
}
static void smallText(int x, int y, const char* s) { oled.setFont(u8g2_font_5x8_tr); oled.drawStr(x, y, s); }

// ---- Movie 1: MOON TRIP  (countdown, launch, space, landing on the moon, flag)
static void mvMoon(uint32_t ms, bool) {
  int shift = 0;
  if (ms >= 4800 && ms < 9800) shift = (int)((ms - 4800) / 8);
  else if (ms >= 9800) shift = 625 + (int)((ms - 9800) / 60);
  drawStars(20, ms < 4800 ? 0 : shift, ms, ms < 4800 ? 40 : 48);
  if (ms < 4800) {
    hln(0, 63, 44); fbox(25, 42, 14, 2);
    int yb = 41, flame = 1 + random8(2);
    if (ms < 1800) {                                                      // 3, 2, 1
      char c[2] = {(char)('3' - ms / 600), 0};
      oled.setFont(u8g2_font_7x13B_tr); oled.drawStr(50, 20, c);
    } else {
      float d = (ms - 1800) / 1000.0f;
      yb = 41 - (int)(d * d * 12); flame = 4 + random8(5);
      int grow = (int)((ms - 1800) / 200); if (grow > 9) grow = 9;
      static const int8_t SX[5] = {-15, -8, 0, 8, 15}, SR[5] = {4, 6, 7, 6, 4};
      for (int k = 0; k < 5; k++) {
        int r = SR[k] * grow / 9; if (r < 1) continue;
        int cx = 32 + SX[k] * grow / 9;
        disc(cx, 44 - r, r); disc(cx, 44 - r, r - 1, false);
      }
    }
    drawRocket(32, yb, flame);
  } else if (ms < 9800) {
    int py = -10 + (int)((ms - 4800) / 60);                               // planet passing by
    circ(12, py, 8); hln(2, 22, py + 2);
    drawRocket(32, 34 + (((ms / 150) & 1) ? 1 : 0), 4 + random8(5));
  } else {
    uint32_t e = ms - 9800;
    int sy = 48 - (int)((e < 1500 ? e : 1500) * 12 / 1500);              // moon surface comes up to y 36
    circ(10, 8, 4); hln(8, 10, 7); hln(9, 12, 9);                          // the earth, far away
    int yb, flame;
    if (e < 1000)      { yb = 34; flame = 4 + random8(4); }
    else if (e < 3000) { float f = (e - 1000) / 2000.0f; f = f * (2 - f); yb = 34 + (int)((sy - 34) * f); flame = 2 + random8(4); }
    else               { yb = sy; flame = 0; }
    drawRocket(32, yb, flame);
    fbox(0, sy + 1, 64, 48 - sy - 1, false);
    hln(0, 63, sy);
    for (int y = sy + 1; y < 48; y++) for (int x = 0; x < 64; x++) if (((x * 3 + y * 5) % 7) == 0) px(x, y);
    fell(12, sy + 5, 4, 2); fell(12, sy + 5, 3, 1, false);               // craters
    fell(54, sy + 7, 5, 2); fell(54, sy + 7, 4, 1, false);
    if (e > 2600 && e < 3600) { int r = (int)((e - 2600) / 150); circ(26 - r, sy - 1, 1); circ(38 + r, sy - 1, 1); }   // dust
    if (e > 3800) {                                                       // flag goes up and waves
      int h = (int)((e - 3800) / 60); if (h > 10) h = 10;
      int fy = sy - h;
      ln(48, sy, 48, fy);
      if (h >= 10) { fbox(49, fy, 7, 4); if ((ms / 250) & 1) pxOff(55, fy + 3); else pxOff(55, fy); }
    }
  }
}

// ---- Movie 2: NIGHT RUN  (drive out of the city, past the trees, home; the light goes on, Zz)
static void mvNight(uint32_t ms, bool) {
  disc(54, 7, 5); disc(56, 6, 5, false);
  for (int i = 0; i < 6; i++) px((i * 29 + 3) % 40, (i * 7 + 2) % 12);
  const bool moving = ms < 11000;
  const uint32_t mv = moving ? ms : 11000;
  const int scroll = (int)(mv / 45);
  for (int i = 0; i < 12; i++) {                                           // city
    int x = i * 11 - scroll; if (x > 64 || x < -12) continue;
    int h = 8 + (int)(h32(i) % 16), top = 36 - h;
    rframe(x, top, 10, h + 1);
    for (int wy = top + 2; wy < 34; wy += 3) for (int wx = x + 2; wx < x + 8; wx += 3)
      if (h32(i * 131 + wy * 7 + wx) % 3 == 0) { px(wx, wy); px(wx + 1, wy); }
  }
  for (int x = 0; x < 64; x++) { int wx = x + scroll; if (wx > 135) px(x, 30 + (int)(sinf(wx * 0.05f) * 3)); }   // hills
  for (int k = 0; k < 7; k++) { int x = 150 + k * 18 - scroll; if (x > -8 && x < 72) drawTree(x, 36); }        // trees
  drawHouse(290 - scroll, 36, ms > 12500, ms);
  hln(0, 63, 37);
  uint32_t dd = mv / 12;
  for (int x = 0; x < 64; x++) if (((x + dd) % 10) < 5) px(x, 42);
  hln(0, 63, 47);
  drawCar(moving ? (int)((ms / 120) & 1) : 0, moving, mv, ms < 12000);
  if (ms > 14000) {                                                         // Zz from the window
    uint32_t z = ms - 14000;
    smallText(40 + (int)(z / 300), 22 - (int)(z / 250), "z");
    if (z > 600) smallText(44 + (int)((z - 600) / 300), 18 - (int)((z - 600) / 250), "Z");
  }
}

// ---- Movie 3: FISH TALE  (small fish, big fish chases it, it hides in the seaweed, big fish gives up)
static void mvFish(uint32_t ms, bool init) {
  static float bbx[5], bby[5];
  if (init) for (int i = 0; i < 5; i++) { bbx[i] = 10 + random8(40); bby[i] = 47 - random8(48); }
  for (int x = 0; x < 64; x++) { px(x, 47); if (h32(x) % 3 == 0) px(x, 46); }
  for (int s = 0; s < 2; s++) { int base = 6 + s * 10; for (int k = 0; k < 14; k++) px(base + (int)(sinf(k * 0.3f + ms * 0.003f + s) * 2), 46 - k); }
  for (int i = 0; i < 5; i++) {
    bby[i] -= 0.5f; bbx[i] += sinf(ms * 0.005f + i) * 0.2f;
    if (bby[i] < -2) { bbx[i] = 10 + random8(40); bby[i] = 46; }
    circ((int)bbx[i], (int)bby[i], 1);
  }
  // small fish
  float sx, sy; int sd = 1;
  if (ms < 4000)       { sx = -6 + 34 * ms / 4000.0f; sy = 24; }
  else if (ms < 5000)  { sx = 28; sy = 24; sd = -1; }
  else if (ms < 6500)  { float f = (ms - 5000) / 1500.0f; sx = 28 + 25 * f; sy = 24 + 6 * f; }
  else if (ms < 12500) { sx = 53; sy = 30; }
  else if (ms < 14500) { float f = (ms - 12500) / 2000.0f; sx = 53 - 17 * f; sy = 30 - 6 * f; sd = -1; }
  else                 { sx = 36 + sinf((ms - 14500) * 0.004f) * 6; sy = 24; sd = cosf((ms - 14500) * 0.004f) > 0 ? 1 : -1; }
  sy += sinf(ms * 0.004f) * 1.0f;
  drawFish((int)sx, (int)sy, sd, 3);
  // big fish
  if (ms >= 4000 && ms < 13000) {
    float bx; int bd = 1;
    if (ms < 8000)       bx = -20 + 50 * (ms - 4000) / 4000.0f;
    else if (ms < 10500) bx = 30;
    else               { bx = 30 - 55 * (ms - 10500) / 2500.0f; bd = -1; }
    drawFish((int)bx, 20 + (int)(sinf(ms * 0.003f) * 2), bd, 8);
    if (ms > 8500 && ms < 10500) smallText((int)bx - 2, 9, "?");
  }
  // seaweed clump (in front, so the small fish can hide behind it)
  for (int s = 0; s < 6; s++) {
    int base = 48 + s * 2;
    for (int k = 0; k < 22; k++) {
      int x = base + (int)(sinf(k * 0.35f + ms * 0.003f + s) * 2);
      px(x, 46 - k); if (k < 18) px(x + 1, 46 - k);
    }
  }
  if (ms > 4300 && ms < 5300) smallText((int)sx - 1, (int)sy - 6, "!");
  if (ms > 14500) drawHeart((int)sx - 3, (int)sy - 13);
}

// ---- Movie 4: SNOW DAY  (snow falls, a snowman builds itself, the sun comes out, it waves)
static void mvSnow(uint32_t ms, bool init) {
  static float fx[30], fy[30]; static uint8_t heap[64];
  if (init) { memset(heap, 0, sizeof heap); for (int i = 0; i < 30; i++) { fx[i] = random8(64); fy[i] = -(float)random8(48); } }
  int active = ms < 8000 ? 30 : 30 - (int)((ms - 8000) / 100); if (active < 0) active = 0;
  for (int i = 0; i < 30; i++) {
    if (i >= active && fy[i] < 0) continue;                             // snow slowly stops
    fy[i] += 0.35f + (i % 3) * 0.2f;
    fx[i] += sinf(ms * 0.002f + i) * 0.3f;
    if (fx[i] < 0) fx[i] += 64;
    if (fx[i] >= 64) fx[i] -= 64;
    int x = (int)fx[i];
    if (fy[i] >= 46 - heap[x]) {
      if (heap[x] < 5) heap[x]++;
      fx[i] = random8(64); fy[i] = i < active ? -(float)random8(10) : -100;
      continue;
    }
    px(x, (int)fy[i]);
  }
  for (int x = 0; x < 64; x++) for (int k = 0; k <= heap[x]; k++) px(x, 47 - k);
  if (ms > 8500) {                                                        // sun rises with turning rays
    int cy = -8 + (int)((ms - 8500 < 1500 ? ms - 8500 : 1500) * 16 / 1500);
    disc(8, cy, 4);
    for (int k = 0; k < 8; k++) {
      float a = k * 0.785f + ms * 0.002f;
      ln(8 + (int)(cosf(a) * 6), cy + (int)(sinf(a) * 6), 8 + (int)(cosf(a) * 9), cy + (int)(sinf(a) * 9));
    }
  }
  auto grow = [&](uint32_t from, uint32_t dur, int r) { if (ms < from) return 0; uint32_t e = ms - from; return e >= dur ? r : (int)(r * e / dur); };
  int r1 = grow(2500, 1500, 8), r2 = grow(4000, 1500, 6), r3 = grow(5500, 1000, 4);
  if (r1) disc(32, 37, r1);
  if (r2) disc(32, 24, r2);
  if (r3) disc(32, 14, r3);
  if (ms > 6500) { pxOff(30, 13); pxOff(34, 13); pxOff(32, 15); ln(33, 15, 37, 16); }      // eyes, carrot nose
  if (ms > 7000) { pxOff(32, 22); pxOff(32, 26); pxOff(32, 34); pxOff(32, 38); }          // buttons
  if (ms > 7500) ln(26, 24, 18, 19);                                                       // left arm
  if (ms > 7500) {                                                                          // right arm (waves later)
    float a = -0.6f + (ms > 11000 ? sinf(ms * 0.01f) * 0.6f : 0);
    ln(38, 24, 38 + (int)(cosf(a) * 9), 24 + (int)(sinf(a) * 9));
  }
  if (ms > 8000) { hln(27, 37, 9); fbox(29, 4, 7, 5); }                                    // hat
  if (ms > 11000) { pxOff(30, 16); pxOff(31, 17); pxOff(32, 17); pxOff(33, 17); pxOff(34, 16); }   // smile
}

struct MovieDef { const char* title; uint16_t len; void (*scene)(uint32_t, bool); };
static const MovieDef MOVIES[] = {
  {"MOON TRIP", 16000, mvMoon}, {"NIGHT RUN", 16000, mvNight}, {"FISH TALE", 16000, mvFish}, {"SNOW DAY", 15000, mvSnow}};
constexpr uint8_t N_MOVIES = sizeof(MOVIES) / sizeof(MOVIES[0]);
constexpr uint32_t OPEN_MS = 3400, END_MS = 3000;

static void drawMovies(uint32_t now) {
  static uint32_t clk = 0, lastNow = 0; static uint8_t mi = 0, sceneOf = 255; static bool started = false;
  uint32_t dt = now - lastNow; lastNow = now;
  if (!started || dt > 300) dt = 33;               // after a popup or another screen: carry on where it was
  started = true;
  clk += dt;
  while (clk >= OPEN_MS + MOVIES[mi].len + END_MS) { clk -= OPEN_MS + MOVIES[mi].len + END_MS; mi = (mi + 1) % N_MOVIES; }
  const MovieDef& M = MOVIES[mi];
  ob = oled.getBufferPtr(); obw = oled.getBufferTileWidth() * 8;
  oled.setFontMode(1); oled.setDrawColor(1);
  auto scene = [&](uint32_t ms) { bool init = (sceneOf != mi); sceneOf = mi; M.scene(ms, init); };

  if (clk < OPEN_MS) {                             // ---- opening
    uint32_t e = clk;
    if (e < 2800) {
      int n = (int)((e > 700 ? 700 : e) * 220 / 700);                      // border draws itself
      for (int k = 0; k < n; k++) {
        if (k < 64) px(k, 0); else if (k < 111) px(63, k - 63); else if (k < 174) px(173 - k, 47); else px(0, 220 - k);
      }
      char top[10]; snprintf(top, sizeof top, "MOVIE %d", mi + 1);
      oled.setFont(u8g2_font_5x8_tr);
      int tw = oled.getStrWidth(top);
      if (e > 500) typeStr((64 - tw) / 2, 12, top, (int)((e - 500) / 70), false);
      oled.setFont(u8g2_font_7x13B_tr);
      int w = oled.getStrWidth(M.title);
      if (e > 1100) {
        int n2 = (int)((e - 1100) / 110);
        typeStr((64 - w) / 2, 32, M.title, n2, ((e / 250) & 1) && e < 2700);
      }
      if (e > 2200) { int u = (int)((e - 2200) / 15); if (u > 24) u = 24; hln(32 - u, 32 + u, 36); }
    } else {
      scene(0);
      irisMask((int)((e - 2800) * 42 / 600));                              // iris opens on the first scene
    }
  } else if (clk < OPEN_MS + M.len) {              // ---- the movie
    scene(clk - OPEN_MS);
  } else {                                          // ---- ending
    uint32_t e = clk - OPEN_MS - M.len;
    if (e < 700) {
      scene(M.len + e);
      irisMask(42 - (int)(e * 42 / 700));                                  // iris closes
    } else {
      oled.setFont(u8g2_font_7x13B_tr);
      int w = oled.getStrWidth("THE END");
      typeStr((64 - w) / 2, 28, "THE END", e > 800 ? (int)((e - 800) / 120) : 0, ((e / 250) & 1) && e < 2000);
      if (e > 1700) { int u = (int)((e - 1700) / 15); if (u > 26) u = 26; hln(32 - u, 32 + u, 32); }
      for (int k = 0; k < 10; k++) {                                       // twinkles around the text
        uint32_t hsh = h32(k * 977 + e / 150);
        int x = (int)(hsh % 64), y = (int)((hsh >> 8) % 48);
        if (y < 12 || y > 36) px(x, y);
      }
      if (e > 2400) {                                                       // dissolve away
        uint32_t lim = e - 2400;
        for (int y = 0; y < 48; y++) for (int x = 0; x < 64; x++) if (h32(x * 64 + y) % 600 < lim) pxOff(x, y);
      }
    }
  }
}
#endif

// ---- Media page: song, artist, play state and volume slider (iPad), or just the media key (other devices)
#if USE_OLED
static const char* mediaKeyName(uint16_t m) {
  switch (m) {
    case 1 << 0: return "NEXT";       case 1 << 1: return "PREVIOUS";   case 1 << 2: return "STOP";
    case 1 << 3: return "PLAY/PAUSE"; case 1 << 4: return "MUTE";       case 1 << 5: return "VOLUME +";
    case 1 << 6: return "VOLUME -";   case 1 << 12: return "FORWARD";   case 1 << 13: return "REWIND";
    default:     return "MEDIA";
  }
}
// ---- Arabic text: letter joining (shaping) + right-to-left order, then smooth scrolling
// Each Arabic letter has up to 4 shapes (alone / end / start / middle) in Unicode block FE70-FEFF.
struct ArForm { uint16_t base, iso; uint8_t type; };   // type 0 = never joins, 1 = joins only to the letter before, 2 = both sides
static const ArForm AR_FORMS[] = {
  {0x0621,0xFE80,0},{0x0622,0xFE81,1},{0x0623,0xFE83,1},{0x0624,0xFE85,1},{0x0625,0xFE87,1},{0x0626,0xFE89,2},
  {0x0627,0xFE8D,1},{0x0628,0xFE8F,2},{0x0629,0xFE93,1},{0x062A,0xFE95,2},{0x062B,0xFE99,2},{0x062C,0xFE9D,2},
  {0x062D,0xFEA1,2},{0x062E,0xFEA5,2},{0x062F,0xFEA9,1},{0x0630,0xFEAB,1},{0x0631,0xFEAD,1},{0x0632,0xFEAF,1},
  {0x0633,0xFEB1,2},{0x0634,0xFEB5,2},{0x0635,0xFEB9,2},{0x0636,0xFEBD,2},{0x0637,0xFEC1,2},{0x0638,0xFEC5,2},
  {0x0639,0xFEC9,2},{0x063A,0xFECD,2},{0x0641,0xFED1,2},{0x0642,0xFED5,2},{0x0643,0xFED9,2},{0x0644,0xFEDD,2},
  {0x0645,0xFEE1,2},{0x0646,0xFEE5,2},{0x0647,0xFEE9,2},{0x0648,0xFEED,1},{0x0649,0xFEEF,1},{0x064A,0xFEF1,2}};
static const ArForm* arFind(uint16_t c) { for (const ArForm& a : AR_FORMS) if (a.base == c) return &a; return nullptr; }
static uint8_t arJoin(uint16_t c) {                    // 255 = not a joining Arabic letter
  if (c == 0x0640) return 2;                           // tatweel joins both sides
  const ArForm* f = arFind(c); return f ? f->type : 255;
}
static bool isRtlChar(uint16_t c) { return (c >= 0x0590 && c <= 0x08FF) || (c >= 0xFB1D && c <= 0xFDFF) || (c >= 0xFE70 && c <= 0xFEFF); }
static bool isLtrChar(uint16_t c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= 0xC0 && c < 0x0590); }

static size_t utf8Decode(const char* s, uint16_t* out, size_t max) {
  size_t n = 0; const uint8_t* p = (const uint8_t*)s;
  while (*p && n < max) {
    uint32_t c; int len;
    if (*p < 0x80) { c = *p; len = 1; }
    else if ((*p & 0xE0) == 0xC0) { c = *p & 0x1F; len = 2; }
    else if ((*p & 0xF0) == 0xE0) { c = *p & 0x0F; len = 3; }
    else { c = '?'; len = (*p & 0xF8) == 0xF0 ? 4 : 1; p += len; out[n++] = (uint16_t)c; continue; }   // emoji etc.
    for (int i = 1; i < len; i++) { if ((p[i] & 0xC0) != 0x80) { len = i; break; } c = (c << 6) | (p[i] & 0x3F); }
    p += len;
    if ((c >= 0x064B && c <= 0x065F) || c == 0x0670) continue;   // drop vowel marks (harakat)
    out[n++] = (uint16_t)c;
  }
  return n;
}
static size_t utf8Encode(const uint16_t* in, size_t n, char* out, size_t outSz) {
  size_t o = 0;
  for (size_t i = 0; i < n; i++) {
    uint16_t c = in[i];
    if (c < 0x80)       { if (o + 1 >= outSz) break; out[o++] = (char)c; }
    else if (c < 0x800) { if (o + 2 >= outSz) break; out[o++] = (char)(0xC0 | (c >> 6)); out[o++] = (char)(0x80 | (c & 0x3F)); }
    else                { if (o + 3 >= outSz) break; out[o++] = (char)(0xE0 | (c >> 12)); out[o++] = (char)(0x80 | ((c >> 6) & 0x3F)); out[o++] = (char)(0x80 | (c & 0x3F)); }
  }
  out[o] = 0;
  return o;
}
// logical order in, shaped letters in visual (left-to-right drawing) order out. Returns true if the line is right-to-left.
static bool shapeLine(const char* utf8, char* out, size_t outSz, bool fontHasForms) {
  static uint16_t a[160], b[160];
  size_t n = utf8Decode(utf8, a, 160), m = 0;
  // 1) shaping
  for (size_t i = 0; i < n; i++) {
    uint16_t c = a[i];
    uint8_t t = arJoin(c);
    if (t == 255 || c == 0x0640 || !fontHasForms) { b[m++] = c; continue; }
    const bool prevJoins = i > 0 && arJoin(a[i - 1]) == 2;
    if (c == 0x0644 && i + 1 < n && (a[i + 1] == 0x0622 || a[i + 1] == 0x0623 || a[i + 1] == 0x0625 || a[i + 1] == 0x0627)) {
      uint16_t lig = a[i + 1] == 0x0622 ? 0xFEF5 : a[i + 1] == 0x0623 ? 0xFEF7 : a[i + 1] == 0x0625 ? 0xFEF9 : 0xFEFB;   // lam-alef
      b[m++] = lig + (prevJoins ? 1 : 0);
      i++;
      continue;
    }
    const ArForm* f = arFind(c);
    const bool nextJoins = t == 2 && i + 1 < n && (arJoin(a[i + 1]) == 1 || arJoin(a[i + 1]) == 2);
    uint16_t g = f->iso;
    if (t == 1 && prevJoins) g = f->iso + 1;                                   // end
    if (t == 2) g = f->iso + (prevJoins && nextJoins ? 3 : prevJoins ? 1 : nextJoins ? 2 : 0);   // middle / end / start / alone
    b[m++] = g;
  }
  // 2) direction of the line = first letter that has one
  bool rtl = false;
  for (size_t i = 0; i < m; i++) { if (isRtlChar(b[i])) { rtl = true; break; } if (isLtrChar(b[i])) break; }
  // 3) resolve spaces / punctuation to the direction around them, then reorder for drawing
  static int8_t dir[160];                                                       // 1 = RTL, 0 = LTR
  for (size_t i = 0; i < m; i++) dir[i] = isRtlChar(b[i]) ? 1 : isLtrChar(b[i]) ? 0 : -1;
  for (size_t i = 0; i < m; i++) if (dir[i] < 0) {
    int8_t before = -1, after = -1;
    for (int j = (int)i - 1; j >= 0; j--) if (dir[j] >= 0 && (isRtlChar(b[j]) || isLtrChar(b[j]))) { before = dir[j]; break; }
    for (size_t j = i + 1; j < m; j++) if (isRtlChar(b[j]) || isLtrChar(b[j])) { after = isRtlChar(b[j]) ? 1 : 0; break; }
    dir[i] = (before >= 0 && before == after) ? before : (rtl ? 1 : 0);
  }
  auto rev = [&](size_t from, size_t to) { while (from < to) { uint16_t t = b[from]; b[from] = b[to]; b[to] = t; int8_t d = dir[from]; dir[from] = dir[to]; dir[to] = d; from++; to--; } };
  const int8_t inner = rtl ? 0 : 1;                                             // runs that must keep their own order
  if (rtl && m) rev(0, m - 1);                                                  // whole line right-to-left
  for (size_t i = 0; i < m;) {                                                  // flip back runs of the other direction
    if (dir[i] != inner) { i++; continue; }
    size_t j = i; while (j + 1 < m && dir[j + 1] == inner) j++;
    rev(i, j);
    i = j + 1;
  }
  utf8Encode(b, m, out, outSz);
  return rtl;
}

// One text line of the media page: shaped once per change, then drawn (scrolling if it doesn't fit).
struct TextLine { char raw[129]; char vis[420]; bool rtl; int w; };
static void drawTextLine(TextLine& L, const char* raw, int top, int bottom, int baseline, uint32_t now) {
  if (strcmp(L.raw, raw) != 0) {                                                // new text: shape it once
    strlcpy(L.raw, raw, sizeof L.raw);
    L.rtl = shapeLine(raw, L.vis, sizeof L.vis, true);          // the cu12 Arabic font has the joined letter shapes
    L.w = oled.getUTF8Width(L.vis);
  }
  int x;
  if (L.w <= 64) x = L.rtl ? 64 - L.w : 0;                                       // fits: right-aligned for Arabic
  else {                                                                        // scroll back and forth: wait, glide, wait, glide back
    const int travel = L.w - 64;
    const uint32_t glide = (uint32_t)travel * 40;                               // 25 px per second
    const uint32_t cycle = 2 * (1500 + glide);
    uint32_t t = now % cycle;
    int pos;
    if (t < 1500)                 pos = 0;
    else if (t < 1500 + glide)    pos = (int)((t - 1500) / 40);
    else if (t < 3000 + glide)    pos = travel;
    else                          pos = travel - (int)((t - 3000 - glide) / 40);
    x = L.rtl ? (64 - L.w) + pos : -pos;                                        // Arabic starts at the right and moves right
  }
  oled.setClipWindow(0, top, 64, bottom);
  oled.drawUTF8((u8g2_uint_t)x, baseline, L.vis);
  oled.setMaxClipWindow();
}
static void fmtTime(float sec, char* out, size_t n) {              // 83.4 -> "1:23", 3725 -> "1:02:05"
  int t = (int)sec; if (t < 0) t = 0;
  if (t >= 3600) snprintf(out, n, "%d:%02d:%02d", t / 3600, (t / 60) % 60, t % 60);
  else           snprintf(out, n, "%d:%02d", t / 60, t % 60);
}
static void drawSpeaker(int x, int y, bool muted) {               // 9x9 speaker icon
  oled.drawBox(x, y + 3, 3, 3);
  oled.drawTriangle(x + 2, y + 4, x + 6, y, x + 6, y + 8);
  if (muted) { oled.drawLine(x + 7, y + 2, x + 9, y + 6); oled.drawLine(x + 9, y + 2, x + 7, y + 6); }
}
static void drawMediaPage(uint32_t now) {
  MediaInfo m;
  portENTER_CRITICAL(&mediaMux); m = media; portEXIT_CRITICAL(&mediaMux);
  const uint16_t key = lastMediaKey;
  oled.setFontMode(1); oled.setDrawColor(1);

  if (m.have || m.vol >= 0) {                                       // iPad / iPhone: real information
    static TextLine tTitle = {};
    oled.setFont(u8g2_font_cu12_t_arabic);                          // 12 px, Latin + Arabic: song name only
    drawTextLine(tTitle, m.title[0] ? m.title : "Nothing playing", 0, 15, 11, now);
    oled.setFont(u8g2_font_4x6_tr);
    if (m.dur > 0) {                                                // progress bar + time played / song length
      float pos = m.elapsed;
      if (m.state == 1 && m.rate > 0) pos += m.rate * (float)(now - m.infoAt) / 1000.0f;   // the iPad only sends changes
      if (pos > m.dur) pos = m.dur;
      if (pos < 0) pos = 0;
      for (int x = 0; x < 64; x += 2) oled.drawPixel(x, 17);       // dotted track
      int w = (int)(64.0f * pos / m.dur);
      if (w > 0) oled.drawBox(0, 16, w, 3);
      char t1[10], t2[10];
      fmtTime(pos, t1, sizeof t1); fmtTime(m.dur, t2, sizeof t2);
      oled.drawStr(0, 25, t1);
      oled.drawStr(64 - oled.getStrWidth(t2), 25, t2);
    }
    if (m.state == 1)      oled.drawTriangle(30, 20, 30, 26, 34, 23);   // playing
    else if (m.state >= 0) { oled.drawBox(29, 20, 2, 7); oled.drawBox(32, 20, 2, 7); }   // paused
    oled.drawStr(0, 34, key ? mediaKeyName(key) : (m.state == 1 ? "PLAYING" : "PAUSED"));
    if (m.vol >= 0) {                                               // volume slider
      char pc[6]; snprintf(pc, sizeof pc, "%d", m.vol);
      oled.drawStr(64 - oled.getStrWidth(pc), 34, pc);
      drawSpeaker(0, 38, m.vol == 0);
      oled.drawFrame(12, 40, 52, 7);
      oled.drawBox(14, 42, (48 * m.vol) / 100, 3);
    }
  } else {                                                          // PC / Android: only the key that was pressed
    const char* k = mediaKeyName(key);
    const bool vol = key == (1 << 5) || key == (1 << 6) || key == (1 << 4);
    if (vol) drawSpeaker(27, 2, key == (1 << 4));
    oled.setFont(u8g2_font_7x13B_tr);
    int w = oled.getStrWidth(k);
    if (w > 64) { oled.setFont(u8g2_font_5x8_tr); w = oled.getStrWidth(k); }
    oled.drawStr((64 - w) / 2, 30, k);
    oled.setFont(u8g2_font_4x6_tr);
    const char* hint = "song info: iPad";
    oled.drawStr((64 - oled.getStrWidth(hint)) / 2, 45, hint);
  }
}
#endif

// Normal view = the main screen chosen in the web page: clock, a page, one animation or all animations.
// After a press: the page chosen in the web page for POPUP_MS, then back to the main screen.
// Pages: 0 status, 1 effects, 2 last action, 3 custom text. Clock without a valid time (or with the clock
// turned off) shows that page instead. The sleep countdown is shown over everything.
// Small dot in the top right corner while the wireless stick is connected
static bool stickDotOn(uint32_t now) {
  return needRemote && !cfgMode && rjLast && (int32_t)(now - rjLast) < 1500;
}
#if USE_OLED
static void drawStickDot(uint32_t now) {
  if (!stickDotOn(now)) return;
  oled.setDrawColor(0); oled.drawBox(58, 0, 6, 6);  // black edge so it stands out on anything
  oled.setDrawColor(1); oled.drawDisc(61, 2, 2);
}
#endif

static void oledTick() {
#if USE_OLED
  static uint32_t last = 0; static char prev[8][16]; static bool asleep = false;
  uint32_t now = millis();
  if (now - last < 33) return;                      // up to ~30 frames per second for animations
  last = now;
  const int32_t idle = idleMs();
  if (batDead) {                                    // battery empty: last message before the endless sleep
    oled.setPowerSave(0); asleep = false;
    oled.clearBuffer(); oled.setFontMode(1); oled.setDrawColor(1);
    oled.drawFrame(0, 0, 64, 48);
    oled.setFont(u8g2_font_5x8_tr);
    oled.drawStr((64 - oled.getStrWidth("BATTERY")) / 2, 12, "BATTERY");
    oled.drawStr((64 - oled.getStrWidth("EMPTY")) / 2, 21, "EMPTY");
    oled.setFont(u8g2_font_4x6_tr);
    oled.drawStr((64 - oled.getStrWidth("charge me, then")) / 2, 33, "charge me, then");
    oled.drawStr((64 - oled.getStrWidth("press reset")) / 2, 41, "press reset");
    oled.sendBuffer();
    memset(prev, 0, sizeof prev);
    return;
  }
  const bool warn = !cfgMode && sleepAllowed() && idle > (int32_t)(sleepAfterMs() - WARN_MS);
  bool wantOff = !warn && (!oledOn || (oCfg.sleepS && idle > (int32_t)oCfg.sleepS * 1000));
  if (wantOff != asleep) { oled.setPowerSave(wantOff ? 1 : 0); asleep = wantOff; }
  if (asleep) return;

  if (!cfgMode && (sotaBusy || (int32_t)(sotaMsgUntil - now) > 0)) {   // stick firmware update
    oled.clearBuffer();
    oled.setFontMode(1); oled.setDrawColor(1);
    oled.setFont(u8g2_font_5x8_tr);
    oled.drawStr((64 - oled.getStrWidth("STICK UPDATE")) / 2, 9, "STICK UPDATE");
    if (sotaBusy) {
      const int p = sotaPct < 0 ? 0 : sotaPct;
      char tb[8]; snprintf(tb, sizeof tb, "%d%%", p);
      oled.setFont(u8g2_font_7x13B_tr);
      oled.drawStr((64 - oled.getStrWidth(tb)) / 2, 27, tb);
      oled.drawFrame(2, 32, 60, 7);
      oled.drawBox(4, 34, 56 * p / 100, 3);
      oled.setFont(u8g2_font_4x6_tr);
      oled.drawStr((64 - oled.getStrWidth("leave the stick")) / 2, 47, "leave the stick");
    } else {
      oled.setFont(u8g2_font_7x13B_tr);
      oled.drawStr((64 - oled.getStrWidth(sotaL1)) / 2, 27, sotaL1);
      oled.setFont(u8g2_font_5x8_tr);
      oled.drawStr((64 - oled.getStrWidth(sotaL2)) / 2, 41, sotaL2);
    }
    oled.sendBuffer();
    memset(prev, 0, sizeof prev);
    return;
  }

  const bool haveTime = cCfg.on && timeValid();
  if (!cfgMode && swMode && (int32_t)(timerPopupUntil - now) > 0) {   // stopwatch
    oled.clearBuffer();
    oled.setFontMode(1); oled.setDrawColor(1);
    oled.setFont(u8g2_font_5x8_tr);
    const uint8_t sws = swState;
    const char* lbl = sws == SW_PAUSE ? "PAUSED" : "STOPWATCH";
    oled.drawStr((64 - oled.getStrWidth(lbl)) / 2, 9, lbl);
    const uint32_t el = swElapsed(); char tb[14];
    snprintf(tb, sizeof tb, "%lu:%02lu.%lu", (unsigned long)(el / 60000), (unsigned long)(el / 1000 % 60), (unsigned long)(el / 100 % 10));
    oled.setFont(u8g2_font_7x13B_tr);
    oled.drawStr((64 - oled.getStrWidth(tb)) / 2, 28, tb);
    oled.setFont(u8g2_font_4x6_tr);
    const char* hint = sws == SW_RUN ? "tap = pause" : sws == SW_PAUSE ? "2x tap = reset" : "3x = timer";
    oled.drawStr((64 - oled.getStrWidth(hint)) / 2, 42, hint);
    drawStickDot(now);
    oled.sendBuffer();
    memset(prev, 0, sizeof prev);
    return;
  }
  if (!cfgMode && !swMode && (timerState == T_ALARM || (int32_t)(timerPopupUntil - now) > 0)) {   // timer
    oled.clearBuffer();
    oled.setFontMode(1); oled.setDrawColor(1);
    const bool alarm = timerState == T_ALARM;
    if (alarm && ((now / 300) & 1)) oled.drawBox(0, 0, 64, 48);
    oled.setDrawColor(alarm && ((now / 300) & 1) ? 0 : 1);
    oled.setFont(u8g2_font_5x8_tr);
    const char* lbl = alarm ? "TIME'S UP!" : timerState == T_SET ? "SET TIMER" : "TIMER";
    oled.drawStr((64 - oled.getStrWidth(lbl)) / 2, 9, lbl);
    int secs = (timerRemaining() + 999) / 1000; char tb[10];
    snprintf(tb, sizeof tb, "%d:%02d", secs / 60, secs % 60);
    oled.setFont(u8g2_font_7x13B_tr);
    oled.drawStr((64 - oled.getStrWidth(tb)) / 2, 28, tb);
    oled.setFont(u8g2_font_4x6_tr);
    const char* hint = alarm ? "press to stop" : timerState == T_SET ? "starts in 3 s" : timerState == T_IDLE ? "3x = stopwatch" : "press = pause";
    oled.drawStr((64 - oled.getStrWidth(hint)) / 2, 42, hint);
    oled.setDrawColor(1);
    oled.sendBuffer();
    memset(prev, 0, sizeof prev);
    return;
  }
  if (!warn && !cfgMode && (int32_t)(profilePopupUntil - now) > 0) {   // "iPad mode" / "PC mode" for 2 s
    oled.clearBuffer();
    oled.setFontMode(1); oled.setDrawColor(1);
    oled.drawFrame(0, 0, 64, 48);
    const char* big = profile ? "PC" : "iPad";
    oled.setFont(u8g2_font_7x13B_tr);
    oled.drawStr((64 - oled.getStrWidth(big)) / 2, 24, big);
    oled.setFont(u8g2_font_5x8_tr);
    oled.drawStr((64 - oled.getStrWidth("mode")) / 2, 37, "mode");
    oled.sendBuffer();
    memset(prev, 0, sizeof prev);
    return;
  }
  const bool popup = (int32_t)(pagePopupUntil - now) > 0 || (int32_t)(fxPopupUntil - now) > 0;
  const bool normal = !warn && !cfgMode && !popup;
  const int8_t home = oHome;
  int8_t anim = -1;
  if (home == HOME_CYCLE) anim = (int8_t)((now / CYCLE_MS) % N_OANIM);
  else if (home >= 0 && home < (int8_t)N_OANIM) anim = home;
  const bool showClock = normal && home == HOME_CLOCK && haveTime;
  const bool clockMsg  = normal && home == HOME_CLOCK && cCfg.on && !haveTime;   // clock wanted, but no time

  if (normal && (anim >= 0 || home == HOME_MOVIES)) {   // animation or movies: redraw every frame
    oled.clearBuffer();
    if (home == HOME_MOVIES) drawMovies(now); else drawAnim((uint8_t)anim, now);
    if (fnDown) {                                   // FN tag bottom right
      oled.setFont(u8g2_font_5x8_tr);
      oled.setDrawColor(0); oled.drawBox(48, 38, 16, 10);
      oled.setDrawColor(1); oled.drawBox(49, 39, 15, 9);
      oled.setDrawColor(0); oled.drawStr(51, 47, "FN"); oled.setDrawColor(1);
    }
    drawStickDot(now);
    oled.sendBuffer();
    memset(prev, 0, sizeof prev);                   // so a page or the clock is drawn fresh afterwards
    return;
  }

  // Media page: after a media key / song or volume change, or when it is the chosen page
  if (!showClock && !clockMsg && !warn && !cfgMode && (int32_t)(fxPopupUntil - now) <= 0 &&
      ((popup && popupMedia) || oledPage == 4)) {
    oled.clearBuffer();
    drawMediaPage(now);
    drawStickDot(now);
    oled.sendBuffer();
    memset(prev, 0, sizeof prev);
    return;
  }

  // L[0] = mode tag, then up to 5 text lines (also used to notice changes, so we only redraw when needed)
  char L[8][16]; memset(L, 0, sizeof L);   // L[6], L[7] = stick effect (6-line effect page)

  if (clockMsg) {                                   // why there is no clock
    snprintf(L[0], 16, "MSG");
    if (ckBusy)                      { snprintf(L[1], 16, "GETTING"); snprintf(L[2], 16, "TIME..."); snprintf(L[3], 16, "%.12s", cCfg.ssid); }
    else if (!cCfg.ssid[0])          { snprintf(L[1], 16, "NO WI-FI SET"); snprintf(L[2], 16, "add it in"); snprintf(L[3], 16, "the setup"); }
    else if (ckState == CK_NOWIFI)   { snprintf(L[1], 16, "UNABLE TO"); snprintf(L[2], 16, "CONNECT"); snprintf(L[3], 16, "%.12s", cCfg.ssid); snprintf(L[4], 16, "retry in 15min"); }
    else if (ckState == CK_NOSERVER) { snprintf(L[1], 16, "NO ANSWER"); snprintf(L[2], 16, "FROM TIME"); snprintf(L[3], 16, "SERVER"); snprintf(L[4], 16, "retry in 15min"); }
    else                             { snprintf(L[1], 16, "NO TIME YET"); snprintf(L[2], 16, "gets it on"); snprintf(L[3], 16, "next wake-up"); }
    if (fnDown) snprintf(L[5], 16, "FN");
  } else if (showClock) {
    time_t t = time(nullptr); struct tm lt; localtime_r(&t, &lt);
    snprintf(L[0], 16, "CLOCK");
    if (cCfg.h24) strftime(L[1], 16, "%H:%M", &lt);
    else { int h = lt.tm_hour % 12; if (!h) h = 12; snprintf(L[1], 16, "%d:%02d", h, lt.tm_min); }
    strftime(L[2], 16, "%a %d %b", &lt);                              // "Tue 10 Nov"
    if (!cCfg.h24) snprintf(L[3], 16, "%s", lt.tm_hour < 12 ? "AM" : "PM");
    if (fnDown) snprintf(L[4], 16, "FN");
    if (batPct >= 0) snprintf(L[5], 16, "%d%%", (int)batPct);   // battery, bottom right
  } else {
    snprintf(L[0], 16, "PAGE");
    if (cfgMode) {                                  // setup page addresses
      snprintf(L[0], 16, "CFG");
      snprintf(L[1], 16, "SETUP PAGE");
      const uint8_t hs = homeState;
      if (hs == 2)      { snprintf(L[2], 16, "%s", homeIp); snprintf(L[3], 16, "macropad.local"); }
      else if (hs == 1) { snprintf(L[2], 16, "joining home"); snprintf(L[3], 16, "Wi-Fi..."); }
      else if (hs == 3) { snprintf(L[2], 16, "home Wi-Fi"); snprintf(L[3], 16, "not found"); }
      snprintf(L[4], 16, "or 192.168.4.1");
    } else if (warn) {
      int32_t left = ((int32_t)sleepAfterMs() - idle + 999) / 1000;
      if (left < 0) left = 0;
      snprintf(L[1], 16, "SLEEP IN %lds", (long)left);
      snprintf(L[2], 16, "press a key");
      snprintf(L[3], 16, "to stay on");
    } else {
      uint8_t page = ((int32_t)(fxPopupUntil - now) > 0) ? 1 : (uint8_t)oledPage;
      bool conn = hid && hid->isConnected();
      const char* sn = stripOff ? "OFF" : STRIP_FX[curSFx];
      const char* mn = !matOn ? "(none)" : matOff ? "OFF" : MAT_FX[curMFx];
      switch (page) {
        case 0:
          snprintf(L[1], 13, "%.12s", deviceName.c_str());
          snprintf(L[2], 13, "%s", cfgMode ? "CONFIG MODE" : !conn ? "Waiting..." : profile ? "PC mode" : "iPad mode");
          if (cfgMode) snprintf(L[3], 13, "192.168.4.1");
          else { snprintf(L[3], 13, "S:%.10s", sn); snprintf(L[4], 13, "M:%.10s", mn); }
          break;
        case 1:
          snprintf(L[1], 13, "STRIP FX");  snprintf(L[2], 13, "%.12s", sn);
          snprintf(L[3], 13, "MATRIX FX"); snprintf(L[4], 13, "%.12s", mn);
          if (rjFx != 254 && (int32_t)(now - rjLast) < 15000) {     // wireless stick on: its effect too
            const uint8_t f = rjFx;
            snprintf(L[6], 13, "STICK FX");
            snprintf(L[7], 13, "%.12s", f == 255 ? "OFF" : f < N_STICK_FX ? STICK_FX[f] : "?");
          }
          break;
        case 2:
          snprintf(L[1], 13, "LAST ACTION");
          if (!lastLabel[0]) snprintf(L[2], 13, "-");
          else { snprintf(L[2], 13, "%.12s", lastLabel); if (strlen(lastLabel) > 12) snprintf(L[3], 13, "%.12s", lastLabel + 12); }
          break;
        default:
          for (uint8_t i = 0; i < 3; i++) if (strlen(oCfg.text) > i * 12u) snprintf(L[1 + i], 13, "%.12s", oCfg.text + i * 12);
          break;
      }
      if (fnDown) snprintf(L[5], 16, "FN");
    }
  }

  if (stickDotOn(now)) strlcat(L[0], "*", 16);    // redraw when the stick connects / goes away
  if (memcmp(L, prev, sizeof L) == 0) return;
  memcpy(prev, L, sizeof L);
  oled.clearBuffer();
  oled.setFontMode(1);
  oled.setDrawColor(1);

  if (showClock) {
    // big time: the largest font that fits the 64 px width
    oled.setFont(u8g2_font_logisoso22_tn);
    if (oled.getStrWidth(L[1]) > 64) oled.setFont(u8g2_font_logisoso18_tn);
    int w = oled.getStrWidth(L[1]);
    oled.drawStr((64 - w) / 2, 26, L[1]);
    // date line under it
    oled.setFont(u8g2_font_5x8_tr);
    w = oled.getStrWidth(L[2]);
    oled.drawStr((64 - w) / 2, 37, L[2]);
    // bottom line: AM/PM on the left, FN in the middle (inverted), battery % on the right
    if (L[3][0]) oled.drawStr(0, 47, L[3]);
    if (L[5][0]) { w = oled.getStrWidth(L[5]); oled.drawStr(64 - w, 47, L[5]); }
    if (L[4][0]) {
      w = oled.getStrWidth(L[4]);
      oled.drawBox(32 - (w + 3) / 2, 39, w + 3, 9);
      oled.setDrawColor(0); oled.drawStr(32 - (w + 3) / 2 + 2, 47, L[4]); oled.setDrawColor(1);
    }
  } else if (clockMsg) {
    oled.setFont(u8g2_font_5x8_tr);
    oled.drawFrame(0, 0, 64, 48);
    for (uint8_t i = 1; i <= 4; i++) {
      if (!L[i][0]) continue;
      int w = oled.getStrWidth(L[i]);
      oled.drawStr((64 - w) / 2, 11 + 10 * (i - 1), L[i]);
    }
    if (L[5][0]) { oled.drawBox(50, 39, 14, 9); oled.setDrawColor(0); oled.drawStr(52, 47, "FN"); oled.setDrawColor(1); }
  } else if (!strcmp(L[0], "CFG")) {               // config mode: title + addresses (small font fits an IP)
    oled.setFont(u8g2_font_5x8_tr);
    oled.drawBox(0, 0, 64, 9);
    oled.setDrawColor(0); oled.drawStr((64 - oled.getStrWidth(L[1])) / 2, 8, L[1]); oled.setDrawColor(1);
    oled.setFont(u8g2_font_4x6_tr);
    if (L[2][0]) oled.drawStr(0, 19, L[2]);
    if (L[3][0]) oled.drawStr(0, 27, L[3]);
    oled.drawHLine(0, 32, 64);
    oled.drawStr(0, 40, "MacroPad-Setup");
    oled.drawStr(0, 47, L[4]);
  } else if (L[6][0]) {                            // 6 lines: strip, matrix and stick effects
    oled.setFont(u8g2_font_5x8_tr);
    const uint8_t idx[6] = {1, 2, 3, 4, 6, 7};
    for (uint8_t k = 0; k < 6; k++) {
      const char* t = L[idx[k]];
      if (k % 2 == 0) {                             // headers inverted, so the names stand apart
        int w = oled.getStrWidth(t);
        oled.drawBox(0, 8 * k, w + 2, 8);
        oled.setDrawColor(0); oled.drawStr(1, 8 * k + 7, t); oled.setDrawColor(1);
      } else oled.drawStr(0, 8 * k + 7, t);
    }
    if (L[5][0]) {                                   // FN tag bottom right
      int w = oled.getStrWidth(L[5]);
      oled.drawBox(64 - w - 3, 39, w + 3, 9);
      oled.setDrawColor(0); oled.drawStr(64 - w - 1, 47, L[5]); oled.setDrawColor(1);
    }
  } else {
    oled.setFont(u8g2_font_5x8_tr);
    for (uint8_t i = 1; i <= 5; i++) {
      if (i == 5 && L[5][0]) {                     // FN tag bottom right
        int w = oled.getStrWidth(L[5]);
        oled.drawBox(64 - w - 3, 39, w + 3, 9);
        oled.setDrawColor(0); oled.drawStr(64 - w - 1, 47, L[5]); oled.setDrawColor(1);
      } else if (i < 5) oled.drawStr(0, 8 + 10 * (i - 1), L[i]);
    }
  }
  drawStickDot(now);
  oled.sendBuffer();
#endif
}

// LEDs + OLED run in their own task so slow LED/OLED updates never delay button/encoder polling.
// When loop() asks for sleep, this task blacks out the LEDs, turns the OLED off and parks itself.
static void fxTask(void*) {
  uint32_t lastFrame = millis();
  for (;;) {
    if (goSleep) {
      fill_solid(sOut, MAX_STRIP, CRGB::Black);
      fill_solid(mOut, MAX_MAT, CRGB::Black);
      FastLED.show();
#if USE_OLED
      oled.setPowerSave(1);
#endif
      sleepAck = true;
      for (;;) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    uint32_t now = millis();
    if (now - lastFrame >= 20) { uint32_t dt = now - lastFrame; lastFrame = now; renderFrame(dt); }
    oledTick();
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

// ============================ Config: validate / apply / store ============================
static bool pinAllowed(int p) {
  if (PIN_STRIP_PWR >= 0 && p == PIN_STRIP_PWR) return false;
  if (PIN_MAT_PWR >= 0 && p == PIN_MAT_PWR) return false;
  for (uint8_t a : ALLOWED_PINS) if (a == p) return true;
  return false;
}
static bool claimPin(uint64_t& used, int p, String& err, bool adc1Only = false) {
  if (!pinAllowed(p))             { err = "GPIO " + String(p) + " is not allowed"; return false; }
  if (adc1Only && p > 10)         { err = "Analog inputs need ADC1 pins (GPIO 1-10), got " + String(p); return false; }
  if (used & (1ULL << p))         { err = "GPIO " + String(p) + " is used twice"; return false; }
  used |= (1ULL << p);
  return true;
}

static bool validateConfig(JsonDocument& d, String& err) {
  JsonArray arr = d["components"].as<JsonArray>();
  if (arr.isNull())               { err = "Missing components list"; return false; }
  if (arr.size() > MAX_COMPONENTS){ err = "Too many components"; return false; }
  int lc = d["led"]["count"] | 8;
  if (lc < 1 || lc > MAX_STRIP)   { err = "LED strip: 1-256 LEDs"; return false; }
  int mw = d["matrix"]["w"] | 0, mh = d["matrix"]["h"] | 0;
  if (mw < 0 || mh < 0 || mw > MAX_W || mh > MAX_H) { err = "Matrix: max 16x16"; return false; }
  if (strlen(d["clock"]["ssid"] | "") > 32) { err = "Wi-Fi name: max 32 characters"; return false; }
  if (strlen(d["clock"]["pass"] | "") > 64) { err = "Wi-Fi password: max 64 characters"; return false; }
  if (strlen(d["clock"]["tz"] | "") > 47)   { err = "Timezone: max 47 characters"; return false; }
  uint64_t used = 0;
  int bp = d["batPin"] | -1;
  if (bp >= 0 && !claimPin(used, bp, err, true)) return false;
  for (JsonObject c : arr) {
    String t = c["type"] | "";
    if (t == "button") {
      if (!claimPin(used, c["pin"] | -1, err)) return false;
    } else if (t == "encoder") {
      if (!claimPin(used, c["a"] | -1, err)) return false;
      if (!claimPin(used, c["b"] | -1, err)) return false;
      int sw = c["sw"] | -1;
      if (sw >= 0 && !claimPin(used, sw, err)) return false;
    } else if (t == "pot") {
      if (!claimPin(used, c["pin"] | -1, err, true)) return false;
    } else if (t == "joystick") {
      if (!claimPin(used, c["x"] | -1, err, true)) return false;
      if (!claimPin(used, c["y"] | -1, err, true)) return false;
      int sw = c["sw"] | -1;
      if (sw >= 0 && !claimPin(used, sw, err)) return false;
    } else if (t == "fn" || t == "timer") {
      if (!claimPin(used, c["pin"] | -1, err)) return false;
    } else if (t == "remotejoy") {
      // wireless joystick: no pins needed on this board
    } else { err = "Unknown component type"; return false; }
  }
  return true;
}

static void clearComponents() {
  for (uint8_t i = 0; i < ncomps; i++) { delete comps[i]; comps[i] = nullptr; }
  ncomps = 0;
}

static uint8_t findFx(const char* const* names, uint8_t n, const String& s) {
  for (uint8_t i = 0; i < n; i++) if (s.equalsIgnoreCase(names[i])) return i;
  return 0;
}

// buttons on RTC GPIOs (0-21) can wake the S3 from deep sleep
static void addWake(int p) { if (p >= 0 && p <= 21) wakeMask |= (1ULL << p); }

static void applyConfig(JsonDocument& d) {
  deviceName = String(d["name"] | "MacroPad");
  if (!deviceName.length()) deviceName = "MacroPad";
  powerMA = constrain((int)(d["power"] | 500), 100, 4000);

  JsonObject l = d["led"];
  sCfg.count = constrain((int)(l["count"] | 8), 1, (int)MAX_STRIP);
  sCfg.bright = constrain((int)(l["bright"] | 60), 5, 255);
  sCfg.hue = (uint8_t)(int)(l["hue"] | 160);
  sCfg.speed = constrain((int)(l["speed"] | 128), 0, 255);
  curSFx = findFx(STRIP_FX, N_STRIP_FX, String(l["fx"] | "breathe"));

  JsonObject m = d["matrix"];
  mCfg.w = constrain((int)(m["w"] | 0), 0, (int)MAX_W);
  mCfg.h = constrain((int)(m["h"] | 0), 0, (int)MAX_H);
  mCfg.bright = constrain((int)(m["bright"] | 40), 5, 255);
  mCfg.hue = (uint8_t)(int)(m["hue"] | 96);
  mCfg.speed = constrain((int)(m["speed"] | 128), 0, 255);
  mCfg.serp = (int)(m["serp"] | 1) != 0;
  mCfg.vert = (int)(m["vert"] | 0) != 0;
  mCfg.flipX = (int)(m["flipX"] | 0) != 0;
  mCfg.flipY = (int)(m["flipY"] | 0) != 0;
  strlcpy(matText, m["text"] | "MacroPad", sizeof matText);
  matGameSpeed = constrain((int)(m["gspeed"] | 3), 1, 10);
  matTextSpeed = constrain((int)(m["tspeed"] | 3), 1, 10);
  matOn = mCfg.w > 0 && mCfg.h > 0;
  curMFx = findFx(MAT_FX, N_MAT_FX, String(m["fx"] | "plasma"));

  JsonObject o = d["oled"];
  oCfg.on = (int)(o["on"] | 1) != 0;
  oCfg.bright = constrain((int)(o["bright"] | 128), 0, 255);
  oCfg.rot = (int)(o["rot"] | 0) != 0;
  oCfg.sleepS = constrain((int)(o["sleep"] | 0), 0, 3600);
  oCfg.page = constrain((int)(o["page"] | 0), 0, 4);
  strncpy(oCfg.text, String(o["text"] | "MacroPad").c_str(), 36); oCfg.text[36] = 0;
  oledOn = oCfg.on; oledPage = oCfg.page;
  oHome = parseHome(String(o["home"] | "clock"));

  batPin = (int8_t)constrain((int)(d["batPin"] | -1), -1, 10);
  batSaver = (int)(d["batSave"] | 1) != 0;
  if (batPin >= 0) analogSetPinAttenuation(batPin, ADC_11db);
  profileMode = constrain((int)(d["pmode"] | 0), 0, 2);   // before the actions: PC actions depend on pswap
  pcSwapCmd = (int)(d["pswap"] | 1) != 0;
  if (profileMode == 1) profile = 0;
  if (profileMode == 2) profile = 1;

  JsonObject ck = d["clock"];
  cCfg.on  = (int)(ck["on"] | 1) != 0;
  cCfg.h24 = (int)(ck["h24"] | 1) != 0;
  cCfg.night = (int)(ck["night"] | 0) != 0;
  cCfg.nfrom = constrain((int)(ck["nfrom"] | 22), 0, 23);
  cCfg.nto   = constrain((int)(ck["nto"] | 7), 0, 23);
  cCfg.nlvl  = constrain((int)(ck["nlvl"] | 25), 1, 100);
  strlcpy(cCfg.ssid, ck["ssid"] | "", sizeof cCfg.ssid);
  strlcpy(cCfg.pass, ck["pass"] | "", sizeof cCfg.pass);
  strlcpy(cCfg.tz,   ck["tz"] | "", sizeof cCfg.tz);
  if (!cCfg.tz[0]) strlcpy(cCfg.tz, "EET-2EEST,M3.5.0/0,M10.5.0/0", sizeof cCfg.tz);

  clearComponents();
  fnDown = 0;
  wakeMask = 0;
  addWake(PIN_MODE_BTN);
  for (JsonObject c : d["components"].as<JsonArray>()) {
    if (ncomps >= MAX_COMPONENTS) break;
    String t = c["type"] | "";
    if (t == "button") {
      comps[ncomps++] = new Button(c["pin"].as<int>(), parsePair(c["action"] | ""), parsePair(c["fnAction"] | ""),
                                   parsePair(c["long"] | ""), parsePair(c["dbl"] | ""), (int)(c["rep"] | 0) != 0);
      addWake(c["pin"].as<int>());
    } else if (t == "fn") {
      comps[ncomps++] = new FnButton(c["pin"].as<int>());
      addWake(c["pin"].as<int>());
    } else if (t == "timer") {
      comps[ncomps++] = new TimerButton(c["pin"].as<int>());
      addWake(c["pin"].as<int>());
    } else if (t == "encoder") {
      int sw = c["sw"] | -1;
      Button* swb = sw >= 0 ? new Button(sw, parsePair(c["swAction"] | ""), parsePair(c["swFnAction"] | ""),
                                         parsePair(c["swLong"] | ""), parsePair(c["swDbl"] | "")) : nullptr;
      comps[ncomps++] = new Encoder(c["a"].as<int>(), c["b"].as<int>(), c["div"] | 4,
                                    parsePair(c["cw"] | ""), parsePair(c["ccw"] | ""),
                                    parsePair(c["cwFn"] | ""), parsePair(c["ccwFn"] | ""), swb);
      addWake(sw);
    } else if (t == "pot") {
      comps[ncomps++] = new Pot(c["pin"].as<int>(), c["steps"] | 32,
                                parsePair(c["up"] | ""), parsePair(c["down"] | ""));
    } else if (t == "joystick" || t == "remotejoy") {
      bool rem = (t == "remotejoy");
      if (rem) needRemote = true;
      else addWake((int)(c["sw"] | -1));
      String b = String(c["btn"] | "left");
      uint8_t mask = b == "right" ? 2 : b == "middle" ? 4 : 1;
      if (rem) {                                            // wireless stick: gestures and its own settings
        rjClick = parsePair(b == "right" ? "rightclick" : b == "middle" ? "middleclick" : "click");
        rjDbl   = parsePair(String(c["dbl"]  | "doubleclick"));
        rjLong  = parsePair(String(c["long"] | "rightclick"));
        rjSdbl  = parsePair(String(c["sdbl"] | ""));
        PadPkt& k = stickCfg;
        k.magic    = 0xB7;
        k.bright   = constrain((int)(c["lbright"] | 60), 0, 255);
        k.fx       = constrain((int)(c["lfx"] | 255), 0, 255);
        k.count    = constrain((int)(c["lcount"] | 12), 1, 256);
        k.maxMa    = constrain((int)(c["lma"] | 250), 50, 5000);
        k.sleepMin = constrain((int)(c["lsleep"] | 5), 0, 120);
        k.hue      = constrain((int)(c["lhue"] | 160), 0, 255);
        k.speed    = constrain((int)(c["lspeed"] | 128), 0, 255);
        String dblTxt = String(c["dbl"] | ""); dblTxt.trim(); dblTxt.toLowerCase();
        const bool dblPlain = dblTxt == "doubleclick";   // two quick clicks already double-click: no need to wait
        k.gest     = (rjDbl.empty() || dblPlain ? 0 : 1) | (rjLong.empty() ? 0 : 2) | (rjSdbl.empty() ? 0 : 4);
        k.cfgId    = 0;
        uint16_t h = prefs.getUShort("sgen", 0) * 7919u + 0x2F1B;  // changes on every save, so the stick takes them again
        const uint8_t* q = (const uint8_t*)&k;
        for (size_t i = 0; i < sizeof k; i++) h = (uint16_t)(h * 31u) ^ q[i];
        k.cfgId = h ? h : 1;
      }
      comps[ncomps++] = new Joystick(rem ? 255 : c["x"].as<int>(), rem ? 255 : c["y"].as<int>(),
                                     rem ? -1 : (int)(c["sw"] | -1),
                                     c["sens"] | 12, c["dead"] | 12, (int)(c["invX"] | 0) != 0,
                                     (int)(c["invY"] | 0) != 0, String(c["mode"] | "mouse") == "scroll", mask,
                                     rem ? (int)(c["boost"] | 3) : 1);
    }
  }
}

static String loadJson() { return prefs.getString("cfg", DEFAULT_JSON); }

// ============================ Web UI (served from flash) ============================
static const char PAGE[] PROGMEM = R"HTML(<!DOCTYPE html><html><head><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1"><title>MacroPad Setup</title>
<style>
body{font-family:system-ui,sans-serif;background:#111;color:#eee;margin:0 auto;padding:12px;max-width:640px}
.card{background:#1d1d22;border-radius:10px;padding:12px;margin:10px 0}
h3{margin:0 0 8px;display:flex;justify-content:space-between;align-items:center;text-transform:capitalize}
label{display:block;font-size:13px;color:#aaa;margin:6px 0}
input,select{width:100%;box-sizing:border-box;padding:9px;margin-top:3px;border-radius:6px;border:1px solid #444;background:#26262c;color:#fff;font-size:16px}
select[data-pick]{margin-top:0;background:#30303a}
button{padding:11px 14px;border:0;border-radius:8px;font-size:15px;margin:4px 4px 4px 0;background:#3a3a44;color:#fff}
button.p{background:#2d7ff9}button.x{background:#7a2b2b;padding:4px 10px}button.w{background:#8a5a12}
small{color:#888;display:block;margin-top:6px}
.err{color:#ff7a6b}.err:empty{display:none}
textarea{width:100%;box-sizing:border-box;min-height:120px;background:#26262c;color:#fff;border:1px solid #444;border-radius:6px;font:12px monospace}
</style></head><body>
<h2>MacroPad Setup</h2><div id=app>Loading...</div>
<script>
let cfg={},FX={strip:[],matrix:[]};
const esc=v=>String(v??'').replace(/"/g,'&quot;');
const fld=(l,p,v,n,x='')=>`<label>${l}<input data-p="${p}" ${n?'data-n=1 type=number':''} value="${esc(v)}" ${x}></label>`;
const sel=(l,p,v,o,n)=>`<label>${l}<select data-p="${p}" ${n?'data-n=1':''}>${o.map(q=>{const a=Array.isArray(q)?q:[q,q];return `<option value="${a[0]}" ${a[0]==v?'selected':''}>${a[1]}</option>`}).join('')}</select></label>`;
const PRE=[
['Editing (works on iPad and PC)',[['Copy','copy'],['Paste','paste'],['Cut','cut'],['Undo','undo'],['Redo','redo'],['Select all','selectall'],['Find','find'],['Save','save'],['New tab','newtab'],['Close tab','closetab'],['Reopen closed tab','reopentab']]],
['iPad / system (works on iPad and PC)',[['Home screen / show desktop','home'],['App switcher','appswitch'],['Search','search'],['Screenshot','screenshot'],['Lock screen','lock'],['Dock / Start menu','dock'],['On-screen keyboard on/off','keyboard'],['Brightness +','brightup'],['Brightness -','brightdown']]],
['Media',[['Play / Pause','play'],['Next track','next'],['Previous track','prev'],['Stop','stop'],['Mute','mute'],['Volume +','volup'],['Volume -','voldown'],['Fast forward','media:ff'],['Rewind','media:rew']]],
['Keys',[['Enter','enter'],['Escape','esc'],['Tab','tab'],['Space','space'],['Backspace','backspace'],['Delete','delete'],['Up','up'],['Down','down'],['Left','left'],['Right','right'],['Page up','pageup'],['Page down','pagedown'],['F5','key:f5']]],
['Mouse',[['Left click','click'],['Right click','rightclick'],['Middle click','middleclick'],['Double click','doubleclick'],['Scroll up','scrollup'],['Scroll down','scrolldown']]],
['LED strip / matrix / OLED',[['Strip: next effect','led:next'],['Strip: previous effect','led:prev'],['Strip: on / off','led:toggle'],['Matrix: next effect','mat:next'],['Matrix: previous effect','mat:prev'],['Matrix: on / off','mat:toggle'],['OLED: next page','oled:next'],['OLED: next main screen (clock / animations)','oled:main'],['OLED: on / off','oled:toggle']]],
['iPad / PC mode',[['Switch iPad / PC mode','profile:toggle'],['iPad mode','profile:ipad'],['PC mode','profile:pc']]],
['Text and pauses',[['Type text (edit after text:)','text:Hello'],['Pause 0.2 s (for macros)','wait:200']]],
['Macro examples',[['Copy all','selectall, copy'],['Copy, switch app, paste','copy, wait:200, appswitch, wait:400, paste'],['Play and skip','play, wait:500, next']]]];
const PRESEL='<option value="">Presets...</option>'+PRE.map(g=>`<optgroup label="${g[0]}">${g[1].map(o=>`<option value="${o[1]}">${o[0]}</option>`).join('')}</optgroup>`).join('');
let ADD=false;
// ---- typo check for actions (same rules as the pad)
const MODS=['ctrl','control','shift','alt','opt','option','gui','cmd','win','meta','super'];
const KN=['enter','esc','backspace','tab','space','minus','equal','lbracket','rbracket','backslash','semicolon','quote','grave','comma','period','slash','capslock','printscreen','insert','home','pgup','delete','end','pgdn','right','left','down','up','plus'];
const MEDIA=['next','prev','stop','playpause','mute','volup','voldown','home','osk','search','brightup','brightdown','ff','rew','lock','back'];
const MOUSE=['left','right','middle','scrollup','scrolldown','double'];
const LOCAL=['led:next','led:prev','led:toggle','mat:next','mat:prev','mat:toggle','oled:next','oled:toggle','oled:main','profile:toggle','profile:ipad','profile:pc'];
function lev(a,b){const d=[];for(let i=0;i<=a.length;i++){d[i]=[i];for(let j=1;j<=b.length;j++)d[i][j]=i?Math.min(d[i-1][j]+1,d[i][j-1]+1,d[i-1][j-1]+(a[i-1]==b[j-1]?0:1)):j;}return d[a.length][b.length];}
function near(w,list){let best='',bd=9;for(const x of list){const v=lev(w,x);if(v<bd){bd=v;best=x;}}return bd<=(w.length>=6?3:2)?' (did you mean '+best+'?)':'';}
function stepStart(t){t=t.trim().toLowerCase();if(/^(key|media|mouse|text|wait|led|mat|oled|profile):/.test(t))return true;const e=t.indexOf(',');return (FX.alias||[]).includes((e<0?t:t.slice(0,e)).trim());}
function steps(v){const o=[];let st=0;for(let i=0;i<=v.length;i++){if(i<v.length&&!(v[i]==','&&stepStart(v.slice(i+1))))continue;const p=v.slice(st,i).trim();st=i+1;if(p)o.push(p);}return o;}
function checkStep(t){const lo=t.toLowerCase().trim(),A=FX.alias||[];
  if(A.includes(lo))return '';
  if(lo.startsWith('text:'))return t.length>5?'':'text: is empty';
  if(lo.startsWith('wait:')){const n=Number(lo.slice(5));return n>0&&n<=10000?'':'wait: needs a number from 1 to 10000 (ms)';}
  if(lo.startsWith('key:')){const ps=lo.slice(4).split('+').map(x=>x.trim()).filter(x=>x);if(!ps.length)return 'key: is empty';
    for(const k of ps){if(MODS.includes(k)||KN.includes(k)||/^f([1-9]|1[0-2])$/.test(k)||k.length==1)continue;return 'unknown key "'+k+'"'+near(k,MODS.concat(KN));}return '';}
  if(lo.startsWith('media:')){const m=lo.slice(6).trim();return MEDIA.includes(m)?'':'unknown media key "'+m+'"'+near(m,MEDIA);}
  if(lo.startsWith('mouse:')){const m=lo.slice(6).trim();return MOUSE.includes(m)?'':'unknown mouse action "'+m+'"'+near(m,MOUSE);}
  if(LOCAL.includes(lo.replace(/\s/g,'')))return '';
  return 'unknown action "'+t+'"'+near(lo,A.concat(LOCAL,MEDIA.map(x=>'media:'+x),MOUSE.map(x=>'mouse:'+x)));}
function checkAction(v){if(!v||!String(v).trim())return '';const sides=String(v).split('|');if(sides.length>2)return 'use | only once (iPad | PC)';
  const e=[];for(const s of sides)for(const p of steps(s)){const m=checkStep(p);if(m)e.push(m);}return e.join('; ');}
function upErr(p,v){const e=document.querySelector('[data-err="'+p+'"]');if(e)e.textContent=checkAction(v);}
const act=(l,p,v)=>`<label>${l}<input data-p="${p}" data-act=1 value="${esc(v)}" placeholder="copy / volup / key:gui+h / text:hi"></label><small class=err data-err="${p}">${checkAction(v)}</small><select data-pick="${p}">${PRESEL}</select>`;
function setp(p,v){const k=p.split('.');let o=cfg;for(let i=0;i<k.length-1;i++)o=o[k[i]];o[k[k.length-1]]=v;}
document.addEventListener('input',e=>{
  const t=e.target,pk=t.dataset.pick;
  if(pk){if(!t.value)return;const f=document.querySelector('input[data-p="'+pk+'"]');
    const cur=f?f.value.trim():'';const v=(cur&&ADD)?cur+', '+t.value:t.value;
    setp(pk,v);if(f)f.value=v;t.selectedIndex=0;upErr(pk,v);return;}
  const p=t.dataset.p;if(!p)return;
  setp(p,t.dataset.n?Number(t.value):t.value);
  if(t.dataset.act)upErr(p,t.value);
});
const yn=[[0,'No'],[1,'Yes']];
const SFXJ=[[255,"Keep the stick's own choice"]].concat(["solid", "blink", "breathe", "wipe", "wipe random", "random colors", "sweep", "dynamic", "colorloop", "rainbow", "scan", "dual scan", "fade", "theater chase", "theater rainbow", "running lights", "saw", "twinkle", "dissolve", "sparkle", "flash sparkle", "hyper sparkle", "strobe", "strobe rainbow", "blink rainbow", "chase", "chase rainbow", "chase flash", "comet", "fireworks", "rain", "fire 2012", "fire flicker", "candle", "colortwinkle", "meteor", "juggle", "bpm", "pride", "palette", "lake", "ripple", "plasma", "lightning", "gradient", "heartbeat", "noise", "aurora", "android", "bouncing balls", "sinelon", "popcorn", "larson scanner", "police", "tri wipe", "confetti", "joystick", "tilt color", "push meter", "warp", "click burst", "lava lamp", "ocean", "twin comets", "song progress", "off"].map((n,i)=>[i,n]));
const SPD=[1,2,3,4,5,6,7,8,9,10].map(v=>[v,v==1?'1 (slowest)':v==3?'3 (default)':v==10?'10 (fastest)':String(v)]);
const TZ=[['Beirut','EET-2EEST,M3.5.0/0,M10.5.0/0'],['UTC','UTC0'],['London','GMT0BST,M3.5.0/1,M10.5.0'],['Paris / Berlin','CET-1CEST,M3.5.0,M10.5.0/3'],['Istanbul / Riyadh','<+03>-3'],['Dubai','<+04>-4'],['New York','EST5EDT,M3.2.0,M11.1.0'],['Los Angeles','PST8PDT,M3.2.0,M11.1.0']];
function render(){
  const L=cfg.led,M=cfg.matrix,O=cfg.oled,K=cfg.clock;
  let h=`<div class=card><h3>General</h3>${fld('Device name (Bluetooth)','name',cfg.name)}${fld('LED power limit in mA (USB is about 500)','power',cfg.power,1)}
  ${sel('Device mode','pmode',cfg.pmode,[[0,'Automatic (iPad found = iPad mode, else PC mode)'],[1,'Always iPad'],[2,'Always PC']],1)}
  ${sel('In PC mode, turn gui (Cmd) into Ctrl','pswap',cfg.pswap,[[1,'Yes (gui+c becomes ctrl+c)'],[0,'No']],1)}
  ${fld('Battery pin (ADC1: GPIO 1-10, -1 = no battery)','batPin',cfg.batPin,1)}
  ${sel('Battery saver below 50 % (sleeps sooner, dimmer LEDs)','batSave',cfg.batSave,[[1,'On'],[0,'Off']],1)}
  <small>Battery wiring: battery + to 100k to the battery pin, and 100k from that pin to GND. At 10 % the LEDs flash red, at 5 % the pad shows BATTERY EMPTY and sleeps until you restart it.</small>
  <label><input type=checkbox id=addmode style="width:auto" ${ADD?'checked':''} onchange="ADD=this.checked"> Presets add to the field (to build a macro) instead of replacing it</label>
  <small><b>Macros:</b> write several actions separated by commas, with <b>wait:200</b> for a 0.2 s pause, for example <b>selectall, copy, wait:200, appswitch, paste</b>.
  Plain names (copy, paste, home, play, volup, click...) work on both iPad and PC by themselves.
  For a different action on the PC write both, separated by | , for example <b>key:gui+h | key:gui+d</b> (iPad | PC). Long press = hold 0.5 s, double tap = two presses within 0.3 s; leave them empty if not needed. A Mac counts as PC: choose Always iPad if you use a Mac.</small></div>
  <div class=card><h3>LED strip (max 256)</h3>${fld('LED count (1-256). Long strips: set the LED power limit above to what your power can give','led.count',L.count,1)}${fld('Brightness (5-255)','led.bright',L.bright,1)}
  ${fld('Colour / hue (0-255)','led.hue',L.hue,1)}${fld('Speed (0-255)','led.speed',L.speed,1)}${sel('Effect (start-up)','led.fx',L.fx,FX.strip)}
  <small>Button on GPIO 39: press = next effect, hold 0.7-3 s then release = previous, hold 3 s = off / on.</small></div>
  <div class=card><h3>LED matrix (max 16x16)</h3>${fld('Width (0 = no matrix)','matrix.w',M.w,1)}${fld('Height','matrix.h',M.h,1)}
  ${fld('Brightness (5-255)','matrix.bright',M.bright,1)}${fld('Colour / hue (0-255)','matrix.hue',M.hue,1)}${fld('Speed (0-255)','matrix.speed',M.speed,1)}
  ${sel('Effect (start-up)','matrix.fx',M.fx,FX.matrix)}
  ${sel('Wiring','matrix.serp',M.serp,[[1,'Zigzag (serpentine)'],[0,'All rows same direction']],1)}
  ${sel('LED chain runs','matrix.vert',M.vert,[[0,'Along rows'],[1,'Along columns']],1)}
  ${sel('Mirror horizontally','matrix.flipX',M.flipX,yn,1)}${sel('Mirror vertically','matrix.flipY',M.flipY,yn,1)}
  ${fld('Scrolling text (effect "text"). {time} = the clock, {song} = the song playing','matrix.text',M.text,0,'maxlength=60')}
  ${sel('Text speed','matrix.tspeed',M.tspeed,SPD,1)}${sel('Game speed','matrix.gspeed',M.gspeed,SPD,1)}
  <small>Games (snake, pong, breakout, dodge, tetris, flappy, invaders) are played with the joystick while they are on the matrix; the mouse works again when you pick another effect. Effect "timer" shows the timer button's countdown.</small>
  <small>Button on GPIO 40: press = next effect, hold 0.7-3 s then release = previous, hold 3 s = off / on.</small></div>
  <div class=card><h3>OLED</h3>${sel('Display','oled.on',O.on,[[1,'On'],[0,'Off']],1)}${fld('Brightness (0-255)','oled.bright',O.bright,1)}
  ${sel('Rotation','oled.rot',O.rot,[[0,'Normal'],[1,'180 degrees']],1)}${fld('Sleep after N seconds idle (0 = never)','oled.sleep',O.sleep,1)}
  ${sel('Main screen','oled.home',O.home,[['clock','Clock'],['movies','Movies (4 short movies, one after another)'],['cycle','All animations (8 s each)'],['page','The page below']].concat((FX.oled||[]).map(n=>[n,'Animation: '+n])))}${sel('Page shown for 5 s after a press','oled.page',O.page,[[0,'Status'],[1,'Effects'],[2,'Last action'],[3,'Custom text'],[4,'Media (song + volume)']],1)}<small>Media keys (volume, play, next...) always show the Media page. Song and volume come from an iPad / iPhone; other devices only show the key.</small>${fld('Custom text (page 4)','oled.text',O.text,0,'maxlength=36')}
  <small>Buttons can use oled:main (next main screen), oled:next (next page) and oled:toggle (OLED on / off).</small></div>
  <div class=card><h3>Clock</h3>${sel('Show the time on the OLED','clock.on',K.on,[[1,'Yes'],[0,'No']],1)}${sel('Format','clock.h24',K.h24,[[1,'24 hour'],[0,'12 hour (AM / PM)']],1)}
  ${fld('Home Wi-Fi name','clock.ssid',K.ssid,0,'maxlength=32 autocapitalize=off autocorrect=off')}${fld('Home Wi-Fi password','clock.pass',K.pass,0,'type=password maxlength=64 placeholder="'+(K.passSet?'saved - leave empty to keep it':'')+'"')}
  ${fld('Timezone (pick from the list or type your own)','clock.tz',K.tz,0,'list=tzl maxlength=47 autocapitalize=off autocorrect=off')}
  ${sel('Night mode (LEDs dim at night)','clock.night',K.night,[[0,'Off'],[1,'On']],1)}${sel('Night starts at','clock.nfrom',K.nfrom,[...Array(24).keys()].map(h=>[h,(h<10?'0':'')+h+':00']),1)}${sel('Night ends at','clock.nto',K.nto,[...Array(24).keys()].map(h=>[h,(h<10?'0':'')+h+':00']),1)}${fld('Night brightness % (1-100)','clock.nlvl',K.nlvl,1)}
  <small>Night mode dims the strip, the matrix and the stick's strip. It needs the time, so the Wi-Fi above must be set (it also works when the clock is not shown).</small><datalist id=tzl>${TZ.map(z=>`<option value="${z[1]}">${z[0]}</option>`).join('')}</datalist>
  <small>Only used when Main screen is Clock. The pad joins this Wi-Fi for a few seconds to get the time (at most once an hour), then turns Wi-Fi off again. It must be a 2.4 GHz network (the ESP32 can't use 5 GHz). In config mode the pad also joins it, so you can open the setup page at http://macropad.local from any device on that Wi-Fi. If it can't connect within 30 s, the OLED shows UNABLE TO CONNECT and it tries again 15 min later.</small></div>`;
  cfg.components.forEach((c,i)=>{
    const P='components.'+i+'.';
    h+=`<div class=card><h3>${c.type}<button class=x onclick="del(${i})">&#10005;</button></h3>`;
    if(c.type=='button') h+=fld('GPIO pin',P+'pin',c.pin,1)+act('Action (press)',P+'action',c.action)
      +sel('Hold to repeat',P+'rep',c.rep||0,[[0,'No'],[1,'Yes: holding keeps repeating the action (long press and double tap are then not used)']],1)+act('Long press (optional)',P+'long',c.long)
      +act('Double tap (optional)',P+'dbl',c.dbl)+act('Action with Fn held (optional)',P+'fnAction',c.fnAction);
    if(c.type=='encoder') h+=fld('Pin A',P+'a',c.a,1)+fld('Pin B',P+'b',c.b,1)+fld('Steps per click (1-8, usually 2 or 4)',P+'div',c.div,1)
      +act('Clockwise',P+'cw',c.cw)+act('Counter-clockwise',P+'ccw',c.ccw)
      +act('Clockwise with Fn held',P+'cwFn',c.cwFn)+act('Counter-clockwise with Fn held',P+'ccwFn',c.ccwFn)
      +fld('Push-switch pin (-1 = none)',P+'sw',c.sw,1)+act('Push action',P+'swAction',c.swAction)+act('Push: long press (optional)',P+'swLong',c.swLong)+act('Push: double tap (optional)',P+'swDbl',c.swDbl)+act('Push with Fn held',P+'swFnAction',c.swFnAction);
    if(c.type=='timer') h+=fld('GPIO pin',P+'pin',c.pin,1)+'<small>Timer button: tap = +1 min, hold = +5 s every 50 ms, double tap = -1 min. It starts 3 s after the last press. Press while it runs = back to setting, press when it rings = stop. The matrix shows the timer by itself.<br>Triple click = stopwatch (triple click again = timer). Stopwatch: tap = start / pause / go on, double tap = back to 0:00.</small>';
    if(c.type=='fn') h+=fld('GPIO pin',P+'pin',c.pin,1)+'<small>Fn (control) button: hold it and press a button or turn an encoder to run its "with Fn held" action. If that action is empty, the normal action runs.</small>';
    if(c.type=='pot') h+=fld('ADC1 pin (GPIO 1-10)',P+'pin',c.pin,1)+fld('Steps over full travel',P+'steps',c.steps,1)
      +act('Moving up',P+'up',c.up)+act('Moving down',P+'down',c.down);
    if(c.type=='joystick') h+=fld('X axis pin (ADC1: GPIO 1-10)',P+'x',c.x,1)+fld('Y axis pin (ADC1: GPIO 1-10)',P+'y',c.y,1)
      +fld('Stick button pin (-1 = none)',P+'sw',c.sw,1)+sel('Stick button clicks',P+'btn',c.btn,['left','right','middle'])
      +sel('Mode',P+'mode',c.mode,[['mouse','Move mouse'],['scroll','Scroll (up/down/left/right)']])
      +fld('Speed (1-40)',P+'sens',c.sens,1)+fld('Deadzone % (0-60)',P+'dead',c.dead,1)
      +sel('Invert X',P+'invX',c.invX,yn,1)+sel('Invert Y',P+'invY',c.invY,yn,1);
    if(c.type=='remotejoy') h+=`<small>Paired stick: <b>${STICK.mac||'none yet: the first stick that is switched on gets paired'}</b></small>${STICK.mac?'<button onclick="forget()">Forget stick</button>':''}`
      +sel('Stick button clicks',P+'btn',c.btn,['left','right','middle'])
      +sel('Mode',P+'mode',c.mode,[['mouse','Move mouse'],['scroll','Scroll (up/down/left/right)']])
      +fld('Speed (1-40)',P+'sens',c.sens,1)+fld('Deadzone % (0-60)',P+'dead',c.dead,1)
      +sel('Speed boost (push the stick all the way)',P+'boost',c.boost,[[1,'Off'],[2,'2x faster'],[3,'3x faster'],[4,'4x faster'],[5,'5x faster']],1)
      +sel('Invert X',P+'invX',c.invX,yn,1)+sel('Invert Y',P+'invY',c.invY,yn,1)
      +'<small>Click the scroll button on the stick board to turn scroll mode on / off (the stick board LED turns white while it is on). Hold it 3 s with your hand off the stick to recalibrate the centre (the strip flashes green).</small>'
      +'<small><b>Stick button gestures</b> (stick code v2.4+)</small>'
      +act('Stick double click (optional)',P+'dbl',c.dbl)+act('Stick long press (optional)',P+'long',c.long)
      +act('Scroll button double tap (optional)',P+'sdbl',c.sdbl)
      +'<small>A single click does the click chosen above. Press the stick and move it = drag. Two quick clicks already double-click, so leave "double click" empty for the fastest clicks (any other action there makes each click wait 0.3 s). Leave double click and long press empty to make the stick button a plain mouse button (hold = hold).</small>'
      +'<small><b>Stick LEDs</b> (sent to the stick after you save)</small>'
      +sel('Effect',P+'lfx',c.lfx,SFXJ,1)+fld('Brightness (0-255)',P+'lbright',c.lbright,1)
      +fld('LED count (1-256)',P+'lcount',c.lcount,1)+fld('LED power limit in mA',P+'lma',c.lma,1)
      +fld('Colour (hue 0-255)',P+'lhue',c.lhue,1)+fld('Effect speed (0-255)',P+'lspeed',c.lspeed,1)
      +fld('Sleep after (minutes, 0 = never sleep)',P+'lsleep',c.lsleep,1)
      +'<small>The effect button on the stick still works; saving this page sends these settings again. To pair the stick with another pad: hold its scroll button while you plug it in, until the strip flashes blue.</small>';
    h+='</div>';
  });
  h+=`<button onclick="add('button')">+ Button</button><button onclick="add('fn')">+ Fn button</button><button onclick="add('timer')">+ Timer button</button><button onclick="add('encoder')">+ Encoder</button><button onclick="add('pot')">+ Pot</button><button onclick="add('joystick')">+ Joystick</button><button onclick="add('remotejoy')">+ Wireless joystick</button>
  <div><button class=p onclick="save()">Save &amp; Restart</button><button class=w onclick="post('/api/reset','Restore defaults?')">Defaults</button>
  <button onclick="post('/api/cancel')">Exit without saving</button></div>
  <div class=card><h3>Firmware update</h3><small id=fwv>${fwInfo()}</small>
  <label>Main board (ESP32-S3) file<input type=file id=fpad accept=".bin"></label><button class=p onclick="upFw('pad')">Update main board</button>
  <label>Wireless stick (ESP32-C6) file<input type=file id=fstick accept=".bin"></label><button class=p onclick="upFw('stick')">Send to the stick</button>${OTA.pend?'<button onclick="cancelStick()">Cancel stick update</button>':''}
  <div id=fwst class=err></div>
  <small>In the Arduino IDE pick the right board in Tools, then Sketch &gt; Export Compiled Binary. Use the file that ends in <b>.ino.bin</b> in the sketch's build folder (not .merged.bin, .bootloader.bin or .partitions.bin). Each area only takes the file for its own board.<br>
  Main board: installed at once, then the pad restarts in Bluetooth mode (your settings stay).<br>
  Stick: kept on the pad and sent over the wireless link the next time the stick is on and left still for 2 s (about a minute; its strip fills up purple, the OLED shows the progress). The stick needs code v3.0 or newer, uploaded by USB once. The stick's own Partition Scheme must have room for updates (the default one does).</small></div>
  <div class=card><h3>Flash a board on the USB port</h3><small id=flv>${flInfo()}</small>
  <label>Board file: ESP32, ESP32-S3, C3 or C6 (.ino.bin, or .merged.bin for a blank board)<input type=file id=ftg accept=".bin"></label><button class=p onclick="upTg()">Save file</button>${FL.file?'<button class=p onclick="startFlash()">Start flashing</button><button onclick="delTg()">Delete file</button>':''}
  <div id=flst class=err></div>
  <small>For ESP32, ESP32-S3, C3 and C6 boards: on a port with a CP2102, CH340 or CH9102 USB chip, or on the S3 / C3 / C6 chip's own USB port. Export the board's sketch with its own board picked in Tools. <b>Start flashing</b> restarts the pad as a USB flasher (no Bluetooth meanwhile). Plug the board into the pad's USB-C port marked <b>USB</b> (not UART / COM) with an OTG adapter or USB-C cable; follow the OLED. The board needs 5 V from that port: bridge the USB-OTG pads on the S3 board if it has them, or use an OTG adapter with power in. When it is done the board restarts with its new code and the pad goes back to Bluetooth mode. A .ino.bin keeps the board's bootloader and partitions (it must have had an Arduino sketch before); use .merged.bin for a brand-new board.</small></div>
  <div class=card><h3>Backup / restore</h3><button onclick="backup()">Show backup</button><button onclick="restoreBox()">Restore a backup</button><div id=bk></div>
  <small>The backup has your whole setup except the Wi-Fi password. Restoring loads it into this page; press Save & Restart to keep it.</small></div>
  <small>Allowed pins: 1,2,4-11,14-18. Pots and joystick axes: 1-10 only. Reserved: 21 (mode), 38 (strip), 39/40 (effect buttons), 47 (matrix), 41/42 (OLED), 12/13 (LED power switches).</small>`;
  app.innerHTML=h;
}
const D={button:{type:'button',pin:4,action:'copy',rep:0,long:'',dbl:'',fnAction:''},
 fn:{type:'fn',pin:11},
 timer:{type:'timer',pin:14},
 encoder:{type:'encoder',a:8,b:9,div:4,cw:'volup',ccw:'voldown',cwFn:'',ccwFn:'',sw:-1,swAction:'',swLong:'',swDbl:'',swFnAction:''},
 pot:{type:'pot',pin:1,steps:32,up:'media:volup',down:'media:voldown'},
 joystick:{type:'joystick',x:1,y:2,sw:-1,btn:'left',mode:'mouse',sens:12,dead:12,invX:0,invY:0},
 remotejoy:{type:'remotejoy',btn:'left',mode:'mouse',sens:12,dead:12,invX:0,invY:0,dbl:'',long:'rightclick',sdbl:'',
  lfx:255,lbright:60,lcount:12,lma:250,lhue:160,lspeed:128,lsleep:5,boost:3}};
function add(t){cfg.components.push({...D[t]});render()}
function del(i){cfg.components.splice(i,1);render()}
function norm(c){
  c.name=c.name||'MacroPad';c.power=c.power||500;c.pmode=c.pmode??0;c.pswap=c.pswap??1;c.batPin=c.batPin??-1;c.batSave=c.batSave??1;
  c.led=Object.assign({count:8,bright:60,hue:160,fx:'breathe',speed:128},c.led);
  c.matrix=Object.assign({w:0,h:0,bright:40,hue:96,fx:'plasma',speed:128,serp:1,vert:0,flipX:0,flipY:0,text:'MacroPad',gspeed:3,tspeed:3},c.matrix);
  c.oled=Object.assign({on:1,bright:128,rot:0,sleep:0,page:0,home:'clock',text:'MacroPad'},c.oled);
  c.clock=Object.assign({on:1,h24:1,ssid:'',pass:'',passSet:0,tz:'EET-2EEST,M3.5.0/0,M10.5.0/0',night:0,nfrom:22,nto:7,nlvl:25},c.clock);
  c.components=c.components||[];
  c.components.forEach(o=>{if(o.type=='remotejoy')for(const k in D.remotejoy)if(o[k]===undefined)o[k]=D.remotejoy[k];});
  return c;
}
async function save(){
  const bad=[...document.querySelectorAll('input[data-act]')].map(i=>[i.closest('label').firstChild.textContent,checkAction(i.value)]).filter(x=>x[1]);
  if(bad.length&&!confirm('Some actions look wrong:\n\n'+bad.map(b=>'- '+b[0]+': '+b[1]).join('\n')+'\n\nSave anyway?'))return;
  const out=JSON.parse(JSON.stringify(cfg));                 // leave out empty fields so more fits in memory
  out.components.forEach(o=>{if(o.type!='remotejoy')for(const k in o) if(o[k]==='') delete o[k];});   // stick: keep '' (= no action)
  const r=await fetch('/api/save',{method:'POST',body:JSON.stringify(out)});
  const t=await r.text();
  if(r.ok)app.innerHTML='<h3>Saved. Restarting in Bluetooth mode...</h3>';else alert(t);
}
async function post(u,q){
  if(q&&!confirm(q))return;
  await fetch(u,{method:'POST'});app.innerHTML='<h3>Restarting in Bluetooth mode...</h3>';
}
// ---- backup / restore (everything except the Wi-Fi password)
function backup(){bk.innerHTML='<textarea id=bkt readonly></textarea><button onclick="copyBk()">Copy</button><small id=bkm></small>';
  const c=JSON.parse(JSON.stringify(cfg));delete c.clock.pass;delete c.clock.passSet;bkt.value=JSON.stringify(c);}
function copyBk(){bkt.select();let ok=false;try{ok=document.execCommand('copy');}catch(e){}
  bkm.textContent=ok?'Copied. Keep it in your Notes app.':'Select all the text above and copy it (long-press on a phone).';}
function restoreBox(){bk.innerHTML='<textarea id=bkt placeholder="Paste a backup here"></textarea><button class=p onclick="loadBk()">Load backup</button>';}
function loadBk(){let c;try{c=JSON.parse(bkt.value);}catch(e){alert('That is not a valid backup (the text is cut or changed).');return;}
  if(!c||!Array.isArray(c.components)){alert('That is not a MacroPad backup.');return;}
  c.clock=Object.assign({},c.clock,{pass:'',passSet:cfg.clock.passSet});cfg=norm(c);render();
  alert('Backup loaded. Check it, then press Save & Restart.');}
let STICK={mac:''},OTA={},FL={};
function flInfo(){const o=FL;let s=o.file?'Saved: <b>'+o.file+'</b> ('+(o.chip||'ESP32')+', '+Math.round(o.size/1024)+' KB, written at 0x'+Number(o.addr).toString(16)+')':'No file saved yet.';if(o.msg)s+='<br>Last time: '+o.msg;return s;}
async function loadFl(){try{FL=await (await fetch('/api/flash')).json();}catch(e){FL={};}render();}
function upTg(){
  const f=document.getElementById('ftg').files[0],st=document.getElementById('flst');
  if(!f){alert('Choose a .bin file first.');return;}
  const fd=new FormData();fd.append('f',f,f.name);
  const x=new XMLHttpRequest();x.open('POST','/api/flash/file');
  x.upload.onprogress=e=>{if(e.lengthComputable)st.textContent='Uploading '+Math.round(e.loaded*100/e.total)+' % ...';};
  x.onload=()=>{if(x.status==200)loadFl();else st.textContent='Not saved: '+x.responseText;};
  x.onerror=()=>{st.textContent='Upload failed (the connection was lost). Try again.';};
  st.textContent='Uploading 0 % ...';x.send(fd);
}
async function startFlash(){
  if(!confirm('The pad restarts as a USB flasher (no Bluetooth meanwhile). Then plug the board into its USB port and follow the OLED. Start?'))return;
  const r=await fetch('/api/flash/start',{method:'POST'});
  if(r.ok)app.innerHTML='<h3>Restarting as a USB flasher. Plug the board in and follow the OLED.</h3>';else alert(await r.text());
}
async function delTg(){await fetch('/api/flash/delete',{method:'POST'});loadFl();}
function fwInfo(){const o=OTA;if(!o.pad)return '';return 'Main board: v'+o.pad+' &middot; Stick: '+(o.stick?'v'+o.stick:'not heard yet')
  +(o.pend?'<br><b>Stick update waiting</b> ('+Math.round(o.size/1024)+' KB): it is sent when the stick is on.'+(o.remote?'':' Add the Wireless joystick card first.'):'')
  +(o.msg?'<br>Last stick update: '+o.msg:'')+(o.fs?'':'<br>Stick updates need the Partition Scheme "16M Flash (3MB APP/9.9MB FATFS)".');}
async function loadOta(){try{OTA=await (await fetch('/api/ota')).json();}catch(e){OTA={};}render();}
async function cancelStick(){await fetch('/api/ota/stickcancel',{method:'POST'});loadOta();}
function upFw(k){
  const f=document.getElementById(k=='pad'?'fpad':'fstick').files[0],st=document.getElementById('fwst');
  if(!f){alert('Choose a .bin file first.');return;}
  if(/merged|bootloader|partitions/i.test(f.name)&&!confirm(f.name+' looks like the wrong file: the right one ends in .ino.bin. Send it anyway?'))return;
  if(k=='pad'&&!confirm('Update the main board now? It restarts when it is done.'))return;
  const fd=new FormData();fd.append('f',f,f.name);
  const x=new XMLHttpRequest();x.open('POST','/api/ota/'+k);
  x.upload.onprogress=e=>{if(e.lengthComputable)st.textContent='Uploading '+Math.round(e.loaded*100/e.total)+' % ...';};
  x.onload=()=>{if(x.status==200){if(k=='pad')app.innerHTML='<h3>Main board updated. Restarting in Bluetooth mode...</h3>';else{alert('Stick file saved on the pad. It is sent to the stick the next time both are on (outside this setup mode).');loadOta();}}else st.textContent='Not updated: '+x.responseText;};
  x.onerror=()=>{st.textContent='Upload failed (the connection was lost). Try again.';};
  st.textContent='Uploading 0 % ...';x.send(fd);
}
async function forget(){await fetch('/api/forgetstick',{method:'POST'});STICK.mac='';render();}
Promise.all([fetch('/api/config').then(r=>r.json()),fetch('/api/fx').then(r=>r.json()),fetch('/api/stick').then(r=>r.json()).catch(()=>({mac:''})),fetch('/api/ota').then(r=>r.json()).catch(()=>({})),fetch('/api/flash').then(r=>r.json()).catch(()=>({}))]).then(([c,f,k,o,l])=>{FX=f;STICK=k;OTA=o;FL=l;cfg=norm(c);render()});
</script></body></html>)HTML";

static void restartInto(bool toConfig) {
  prefs.putBool("cfgmode", toConfig);
  delay(600);               // let the HTTP response flush
  ESP.restart();
}

// Config mode on the home Wi-Fi too (if one is set in the Clock card): 0 = not set, 1 = joining, 2 = on it, 3 = failed
static void homeWifiTick() {
  if (homeState != 1) return;
  if (WiFi.status() == WL_CONNECTED) {
    strlcpy(homeIp, WiFi.localIP().toString().c_str(), sizeof homeIp);
    if (MDNS.begin("macropad")) MDNS.addService("http", "tcp", 80);   // http://macropad.local
    homeState = 2;
    Serial.printf("Setup page also at http://%s and http://macropad.local\n", homeIp);
  } else if (millis() - homeSince > 30000) {                        // gave up: only the pad's own Wi-Fi then
    homeState = 3;
    WiFi.disconnect();
    Serial.println("Home Wi-Fi: unable to connect (setup page only on MacroPad-Setup)");
  }
}

// ---- firmware files (setup page)
// An ESP32 app file starts with 0xE9, has the chip id at byte 12 and its app description (magic ABCD5432) at byte 32.
static String otaErr; static bool otaFirst = false; static File otaFile; static uint32_t otaBytes = 0, otaCrcAcc = 0;
static String imageProblem(const uint8_t* d, size_t n, uint16_t want) {
  if (n < 36 || d[0] != 0xE9) return "This is not a firmware file. Use the .ino.bin file from Sketch > Export Compiled Binary.";
  const uint16_t id = d[12] | (d[13] << 8);
  if (id != want) {
    if (id == 0x0009) return "This is the main board (ESP32-S3) file. Put it in the main board area.";
    if (id == 0x000D) return "This is the stick (ESP32-C6) file. Put it in the stick area.";
    return "This file is for another chip. Pick the right board in Tools, then export again.";
  }
  uint32_t mg; memcpy(&mg, d + 32, 4);
  if (mg != 0xABCD5432u) return "This is not the app file. Use the one that ends in .ino.bin (not merged, bootloader or partitions).";
  return "";
}
static void otaPadUpload() {
  HTTPUpload& u = server.upload();
  if (u.status == UPLOAD_FILE_START) {
    otaErr = ""; otaFirst = true;
    Serial.printf("Main board update: %s\n", u.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) otaErr = String("No room for an update (") + Update.errorString() + "). Check the Partition Scheme.";
  } else if (u.status == UPLOAD_FILE_WRITE) {
    if (otaErr.length()) return;
    if (otaFirst) { otaFirst = false; otaErr = imageProblem(u.buf, u.currentSize, 0x0009); if (otaErr.length()) { Update.abort(); return; } }
    if (Update.write(u.buf, u.currentSize) != u.currentSize) { otaErr = String("Write failed: ") + Update.errorString(); Update.abort(); }
  } else if (u.status == UPLOAD_FILE_END) {
    if (!otaErr.length() && !Update.end(true)) otaErr = String("The file did not pass the check: ") + Update.errorString();
  } else if (u.status == UPLOAD_FILE_ABORTED) {
    Update.abort(); otaErr = "Upload interrupted";
  }
}
static void otaStickUpload() {
  HTTPUpload& u = server.upload();
  if (u.status == UPLOAD_FILE_START) {
    otaErr = ""; otaFirst = true; otaBytes = 0; otaCrcAcc = 0;
    if (!ffatOk) { otaErr = "No room to keep the stick file: set Tools > Partition Scheme to \"16M Flash (3MB APP/9.9MB FATFS)\" and upload the main board by USB."; return; }
    FFat.remove("/stick.tmp");
    otaFile = FFat.open("/stick.tmp", FILE_WRITE);
    if (!otaFile) otaErr = "Can't save the file on the pad";
  } else if (u.status == UPLOAD_FILE_WRITE) {
    if (otaErr.length()) return;
    if (otaFirst) { otaFirst = false; otaErr = imageProblem(u.buf, u.currentSize, 0x000D); if (otaErr.length()) return; }
    if (otaBytes + u.currentSize > 4000000UL) { otaErr = "The file is too big"; return; }
    if (otaFile.write(u.buf, u.currentSize) != u.currentSize) { otaErr = "Can't save the file on the pad (full?)"; return; }
    otaCrcAcc = crc32Add(otaCrcAcc, u.buf, u.currentSize); otaBytes += u.currentSize;
  } else if (u.status == UPLOAD_FILE_END || u.status == UPLOAD_FILE_ABORTED) {
    if (otaFile) otaFile.close();
    if (u.status == UPLOAD_FILE_ABORTED) otaErr = "Upload interrupted";
    if (!otaErr.length() && otaBytes < 50000) otaErr = "The file is too small to be the stick code";
    if (otaErr.length()) { FFat.remove("/stick.tmp"); return; }
    FFat.remove("/stick.bin");
    if (!FFat.rename("/stick.tmp", "/stick.bin")) { otaErr = "Can't save the file on the pad"; return; }
    prefs.putUInt("sbsize", otaBytes); prefs.putUInt("sbcrc", otaCrcAcc); prefs.putUChar("sbpend", 1);
    prefs.putString("sbmsg", "");
    Serial.printf("Stick update saved: %lu bytes\n", (unsigned long)otaBytes);
  }
}

// "Flash a board on the USB port": the classic ESP32 file is kept in /target.bin. App files go to 0x10000
// (the board keeps its bootloader); a merged file (Arduino .merged.bin) holds everything and goes to 0x0.
static void tgUpload() {
  HTTPUpload& u = server.upload();
  if (u.status == UPLOAD_FILE_START) {
    otaErr = ""; otaBytes = 0;
    if (!ffatOk) { otaErr = "No room to keep the file: set Tools > Partition Scheme to \"16M Flash (3MB APP/9.9MB FATFS)\"."; return; }
    FFat.remove("/target.tmp");
    otaFile = FFat.open("/target.tmp", FILE_WRITE);
    if (!otaFile) otaErr = "Can't save the file on the pad";
  } else if (u.status == UPLOAD_FILE_WRITE) {
    if (otaErr.length()) return;
    if (otaBytes + u.currentSize > 4194304UL) { otaErr = "The file is too big (max 4 MB)"; return; }
    if (otaFile.write(u.buf, u.currentSize) != u.currentSize) { otaErr = "Can't save the file on the pad (full?)"; return; }
    otaBytes += u.currentSize;
  } else if (u.status == UPLOAD_FILE_END || u.status == UPLOAD_FILE_ABORTED) {
    if (otaFile) otaFile.close();
    if (u.status == UPLOAD_FILE_ABORTED) otaErr = "Upload interrupted";
    uint32_t addr = 0xFFFFFFFF; uint16_t tgId = 0;
    if (!otaErr.length()) {
      uint8_t h[40] = {0}, h2[16] = {0}, h3[40] = {0};
      File r = FFat.open("/target.tmp", FILE_READ);
      if (r) {
        r.read(h, sizeof h);
        if (otaBytes > 0x1010 && r.seek(0x1000)) r.read(h2, sizeof h2);
        if (otaBytes > 0x10040 && r.seek(0x10000)) r.read(h3, sizeof h3);
        r.close();
      }
      auto chipOk = [](uint16_t c) { return c == 0 || c == 5 || c == 9 || c == 13; };   // ESP32, C3, S3, C6
      uint32_t mg, mg3; memcpy(&mg, h + 32, 4); memcpy(&mg3, h3 + 32, 4);
      const uint16_t id = h[12] | (h[13] << 8), id2 = h2[12] | (h2[13] << 8);
      if (h[0] == 0xE9 && mg == 0xABCD5432u && chipOk(id)) { addr = 0x10000; tgId = id; }                 // app file
      else if (h[0] == 0xFF && h2[0] == 0xE9 && id2 == 0) { addr = 0; tgId = 0; }                          // merged, classic ESP32
      else if (h[0] == 0xE9 && id != 0 && chipOk(id) && h3[0] == 0xE9 && mg3 == 0xABCD5432u) { addr = 0; tgId = id; }   // merged, S3 / C3 / C6
      else if (h[0] == 0xE9 && !chipOk(id)) otaErr = "This file is for a chip the flasher doesn't know (it does ESP32, S3, C3 and C6).";
      else if (h[0] == 0xE9) otaErr = "This is not the app file. Use .ino.bin (or .merged.bin for a blank board).";
      else otaErr = "This is not an ESP32 firmware file.";
    }
    if (otaErr.length()) { FFat.remove("/target.tmp"); return; }
    FFat.remove("/target.bin");
    if (!FFat.rename("/target.tmp", "/target.bin")) { otaErr = "Can't save the file on the pad"; return; }
    prefs.putUInt("tgsize", otaBytes); prefs.putUInt("tgaddr", addr); prefs.putUShort("tgchip", tgId);
    prefs.putString("tgname", u.filename); prefs.putString("tgmsg", "");
  }
}

static void startConfigPortal() {
  if (cCfg.ssid[0]) {                           // pad's own Wi-Fi + the home Wi-Fi at the same time
    WiFi.persistent(false);
    WiFi.setHostname("macropad");
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(cCfg.ssid, cCfg.pass);
    homeState = 1; homeSince = millis();
  } else WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress ip = WiFi.softAPIP();
  dns.start(53, "*", ip);                       // captive portal: all names -> us
  ffatOk = FFat.begin(true);                    // keeps the stick file (formatted the first time, takes a few s)
  if (!ffatOk) Serial.println("FATFS not found: stick updates need the 16M Flash (3MB APP/9.9MB FATFS) scheme");
  Serial.printf("Config AP '%s' up at http://%s\n", AP_SSID, ip.toString().c_str());
  showStatus("CONFIG", "192.168.4.1");

  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", PAGE); });
  server.on("/api/config", HTTP_GET, []() {             // the Wi-Fi password is never sent to the page
    String js = loadJson();
    JsonDocument d;
    if (deserializeJson(d, js)) { server.send(200, "application/json", js); return; }
    const bool has = strlen(d["clock"]["pass"] | "") > 0;
    d["clock"]["pass"] = "";
    d["clock"]["passSet"] = has ? 1 : 0;
    String o; serializeJson(d, o);
    server.send(200, "application/json", o);
  });
  server.on("/api/fx", HTTP_GET, []() {
    String s = "{\"strip\":[";
    for (uint8_t i = 0; i < N_STRIP_FX; i++) { if (i) s += ','; s += '"'; s += STRIP_FX[i]; s += '"'; }
    s += "],\"matrix\":[";
    for (uint8_t i = 0; i < N_MAT_FX; i++)   { if (i) s += ','; s += '"'; s += MAT_FX[i];   s += '"'; }
    s += "],\"oled\":[";
    for (uint8_t i = 0; i < N_OANIM; i++)    { if (i) s += ','; s += '"'; s += OANIM_NAMES[i]; s += '"'; }
    s += "],\"alias\":[";
    for (size_t i = 0; i < sizeof(ALIASES) / sizeof(ALIASES[0]); i++) { if (i) s += ','; s += '"'; s += ALIASES[i].name; s += '"'; }
    s += "]}";
    server.send(200, "application/json", s);
  });

  server.on("/api/save", HTTP_POST, []() {
    String body = server.arg("plain");
    if (!body.length() || body.length() > MAX_JSON_BYTES) { server.send(400, "text/plain", "Config empty or too large"); return; }
    JsonDocument doc;
    if (deserializeJson(doc, body)) { server.send(400, "text/plain", "Invalid JSON"); return; }
    String err;
    if (!validateConfig(doc, err)) { server.send(400, "text/plain", err); return; }
    prefs.putUShort("sgen", prefs.getUShort("sgen", 0) + 1);   // the wireless stick takes its settings again
    if (strlen(doc["clock"]["pass"] | "") == 0) {          // password left empty: keep the saved one
      JsonDocument old;
      if (!deserializeJson(old, loadJson())) { String op = old["clock"]["pass"] | ""; if (op.length()) doc["clock"]["pass"] = op; }
    }
    doc["clock"].remove("passSet");
    String out; serializeJson(doc, out);
    prefs.putString("cfg", out);
    prefs.remove("sfx"); prefs.remove("mfx"); prefs.remove("ohome");   // saved config wins over button choices
    server.send(200, "text/plain", "OK");
    restartInto(false);
  });
  server.on("/api/reset", HTTP_POST, []() {
    prefs.remove("cfg"); prefs.remove("sfx"); prefs.remove("mfx"); prefs.remove("ohome");
    server.send(200, "text/plain", "OK");
    restartInto(false);
  });
  server.on("/api/stick", HTTP_GET, []() {
    char b[48] = "";
    if (stickPaired) snprintf(b, sizeof b, "%02X:%02X:%02X:%02X:%02X:%02X", stickMac[0], stickMac[1], stickMac[2], stickMac[3], stickMac[4], stickMac[5]);
    server.send(200, "application/json", String("{\"mac\":\"") + b + "\"}");
  });
  server.on("/api/forgetstick", HTTP_POST, []() {
    prefs.remove("stickmac"); stickPaired = false;
    server.send(200, "text/plain", "OK");
  });
  server.on("/api/ota", HTTP_GET, []() {
    JsonDocument d;
    d["pad"] = FW_VERSION;
    d["stick"] = prefs.getString("sver", "");
    d["pend"] = (ffatOk && prefs.getUChar("sbpend", 0) && FFat.exists("/stick.bin")) ? 1 : 0;
    d["size"] = prefs.getUInt("sbsize", 0);
    d["msg"] = prefs.getString("sbmsg", "");
    d["fs"] = ffatOk ? 1 : 0;
    d["remote"] = needRemote ? 1 : 0;
    String o; serializeJson(d, o);
    server.send(200, "application/json", o);
  });
  server.on("/api/ota/pad", HTTP_POST, []() {
    if (otaErr.length()) { server.send(400, "text/plain", otaErr); return; }
    server.send(200, "text/plain", "OK");
    Serial.println("Main board updated -> restart");
    restartInto(false);
  }, otaPadUpload);
  server.on("/api/ota/stick", HTTP_POST, []() {
    if (otaErr.length()) { server.send(400, "text/plain", otaErr); return; }
    server.send(200, "text/plain", "OK");
  }, otaStickUpload);
  server.on("/api/flash", HTTP_GET, []() {
    JsonDocument d;
    const bool has = ffatOk && FFat.exists("/target.bin") && prefs.getUInt("tgsize", 0);
    d["file"] = has ? prefs.getString("tgname", "board.bin") : String("");
    d["size"] = prefs.getUInt("tgsize", 0);
    d["addr"] = prefs.getUInt("tgaddr", 0x10000);
    d["msg"] = prefs.getString("tgmsg", "");
    const uint16_t c = prefs.getUShort("tgchip", 0);
    d["chip"] = c == 9 ? "ESP32-S3" : c == 5 ? "ESP32-C3" : c == 13 ? "ESP32-C6" : "ESP32";
    String o; serializeJson(d, o);
    server.send(200, "application/json", o);
  });
  server.on("/api/flash/file", HTTP_POST, []() {
    if (otaErr.length()) { server.send(400, "text/plain", otaErr); return; }
    server.send(200, "text/plain", "OK");
  }, tgUpload);
  server.on("/api/flash/start", HTTP_POST, []() {
    if (!ffatOk || !FFat.exists("/target.bin")) { server.send(400, "text/plain", "Save a file first"); return; }
    prefs.putBool("flashmode", true);
    server.send(200, "text/plain", "OK");
    restartInto(false);
  });
  server.on("/api/flash/delete", HTTP_POST, []() {
    if (ffatOk) FFat.remove("/target.bin");
    prefs.putUInt("tgsize", 0); prefs.putString("tgmsg", "");
    server.send(200, "text/plain", "OK");
  });
  server.on("/api/ota/stickcancel", HTTP_POST, []() {
    if (ffatOk) FFat.remove("/stick.bin");
    prefs.putUChar("sbpend", 0); prefs.putString("sbmsg", "cancelled");
    server.send(200, "text/plain", "OK");
  });
  server.on("/api/cancel", HTTP_POST, []() {
    server.send(200, "text/plain", "OK");
    restartInto(false);
  });
  server.onNotFound([]() {                       // captive-portal redirect
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();
}

// ============================ Mode button ============================
static void modeButtonTick() {
  static bool stable = HIGH, lastRaw = HIGH; static uint32_t lastChange = 0;
  bool raw = digitalRead(PIN_MODE_BTN);
  if (raw != lastRaw) { lastRaw = raw; lastChange = millis(); }
  if (millis() - lastChange > 40 && raw != stable) {
    stable = raw;
    if (stable == LOW) {
      forceWhite = true;                        // LED task shows white while we reboot
      restartInto(!cfgMode);                    // toggle
    }
  }
}

// ============================ LED power switches ============================
// Order matters: data line low BEFORE the LEDs lose ground, so they can't power themselves through DIN.
static void ledPowerOn() {
  const int8_t pw[] = {PIN_STRIP_PWR, PIN_MAT_PWR};
  const uint8_t dl[] = {PIN_STRIP, PIN_MATRIX};
  for (uint8_t i = 0; i < 2; i++) {
    gpio_hold_dis((gpio_num_t)dl[i]);                   // release the sleep hold first
    pinMode(dl[i], OUTPUT); digitalWrite(dl[i], LOW);
    if (pw[i] < 0) continue;
    gpio_hold_dis((gpio_num_t)pw[i]);
    pinMode(pw[i], OUTPUT); digitalWrite(pw[i], HIGH);
  }
#if !SOC_GPIO_SUPPORT_HOLD_SINGLE_IO_IN_DSLP
  gpio_deep_sleep_hold_dis();
#endif
  delay(10);                                            // LEDs start up before they get data
}
static void ledPowerOffForSleep() {
  const int8_t pw[] = {PIN_STRIP_PWR, PIN_MAT_PWR};
  const uint8_t dl[] = {PIN_STRIP, PIN_MATRIX};
  for (uint8_t i = 0; i < 2; i++) {
    pinMode(dl[i], OUTPUT); digitalWrite(dl[i], LOW);   // 1) data low
    gpio_hold_en((gpio_num_t)dl[i]);
    if (pw[i] < 0) continue;
    pinMode(pw[i], OUTPUT); digitalWrite(pw[i], LOW);   // 2) power off
    gpio_hold_en((gpio_num_t)pw[i]);                    //    and keep it off during deep sleep
  }
#if !SOC_GPIO_SUPPORT_HOLD_SINGLE_IO_IN_DSLP
  gpio_deep_sleep_hold_en();
#endif
}

// ============================ Deep sleep ============================
static bool anyWakePinDown() {
  for (uint8_t p = 0; p <= 21; p++) if ((wakeMask & (1ULL << p)) && digitalRead(p) == LOW) return true;
  return false;
}

// Wake sources: the buttons (ext1). With a wireless joystick also a timer: every PEEK_EVERY_MS the board
// wakes for ~0.4 s, listens for the stick and goes straight back to sleep if the stick is not being used.
static void armAndSleep() {
  esp_sleep_enable_ext1_wakeup(rtcWakeMask, ESP_EXT1_WAKEUP_ANY_LOW);
  for (uint8_t p = 0; p <= 21; p++) {
    if (!(rtcWakeMask & (1ULL << p))) continue;
    rtc_gpio_pullup_en((gpio_num_t)p);
    rtc_gpio_pulldown_dis((gpio_num_t)p);
  }
  if (rtcRemote) esp_sleep_enable_timer_wakeup((uint64_t)PEEK_EVERY_MS * 1000ULL);
#if SOC_PM_SUPPORT_RTC_PERIPH_PD
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);   // keeps the pull-ups alive in sleep
#endif
  Serial.flush();
  esp_deep_sleep_start();
}

// Quick listen for the wireless stick. Wakes the pad when the stick was just woken up (bit2),
// its button is pressed (bit0) or it is pushed away from the centre.
static volatile bool peekHit = false;
static void onPeekRecv(const esp_now_recv_info_t* info, const uint8_t* d, int len) {
  if (!info || !info->src_addr || (stickPaired && memcmp(info->src_addr, stickMac, 6) != 0)) return;   // not our stick
  if (len != (int)sizeof(JoyPkt) && len != (int)sizeof(JoyPkt2) && len != (int)sizeof(JoyPkt2) - 1) return;
  JoyPkt p; memcpy(&p, d, sizeof p);                  // both versions start the same way
  if (p.magic != 0xA5 && p.magic != 0xA6) return;
  if ((p.btn & 0x05) || abs(p.x) > 200 || abs(p.y) > 200) peekHit = true;
}
static bool peekRemote() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);    // same channel as the stick
  if (esp_now_init() != ESP_OK) { WiFi.mode(WIFI_OFF); return false; }
  esp_now_register_recv_cb(onPeekRecv);
  uint32_t t0 = millis();
  while (!peekHit && millis() - t0 < 250) delay(5);  // the stick sends at least every 100 ms
  esp_now_deinit();
  WiFi.mode(WIFI_OFF);
  return peekHit;
}

static void sleepNow(bool forever);
static void goToSleep() {
  if (anyWakePinDown()) { markUsed(); return; }  // a button is being held: that is use, and it would wake us at once
  Serial.println("Not used for a while -> deep sleep (press a button to wake)");
  sleepNow(false);
}
// forever = battery empty: nothing can wake it, only a restart (reset button or power off / on)
static void sleepNow(bool forever) {

  if (fxDirty) { prefs.putUChar("sfx", curSFx); prefs.putUChar("mfx", curMFx); fxDirty = false; }
  rtcValid = true; rtcStripOff = stripOff; rtcMatOff = matOff;

  if (hid && hid->isConnected()) {               // release everything on the host
    hid->sendKeys(0, nullptr, 0);
    hid->sendMedia(0);
    hid->mouseButton(0xFF, false);
    delay(30);
  }

  goSleep = true;                                // LED task: black out LEDs, OLED off
  uint32_t t0 = millis();
  while (!sleepAck && millis() - t0 < 500) delay(5);

  if (needRemote) esp_now_deinit();
  WiFi.mode(WIFI_OFF);

  ledPowerOffForSleep();                         // data lines low, then strip + matrix power off

  if (forever) {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    Serial.println("Battery empty -> sleeping until restarted");
    Serial.flush();
    esp_deep_sleep_start();
  }
  rtcWakeMask = wakeMask; rtcRemote = needRemote;
  armAndSleep();
}

// ---- battery: % from the voltage of a single LiPo cell
static int lipoPercent(float mv) {
  static const uint16_t MV[]  = {3270, 3610, 3690, 3710, 3730, 3750, 3770, 3790, 3800, 3820, 3840, 3850, 3870, 3910, 3950, 3980, 4020, 4080, 4110, 4150, 4200};
  static const uint8_t  PCT[] = {   0,    5,   10,   15,   20,   25,   30,   35,   40,   45,   50,   55,   60,   65,   70,   75,   80,   85,   90,   95,  100};
  if (mv <= MV[0]) return 0;
  for (uint8_t i = 1; i < sizeof(MV) / sizeof(MV[0]); i++)
    if (mv <= MV[i]) return PCT[i - 1] + (int)((mv - MV[i - 1]) * (PCT[i] - PCT[i - 1]) / (MV[i] - MV[i - 1]));
  return 100;
}
static void batteryTick() {                        // once a second, from loop()
  static uint32_t last = 0, critSince = 0; static float mvF = 0; static int8_t sent = -1;
  if (batPin < 0) { batPct = -1; return; }
  const uint32_t now = millis();
  if (last && now - last < 1000) return;
  last = now;
  uint32_t sum = 0;
  for (uint8_t i = 0; i < 16; i++) sum += analogReadMilliVolts(batPin);
  const float mv = sum / 16.0f * 2.0f;                     // 100k + 100k divider halves the voltage
  if (mv < 2500) { batPct = -1; mvF = 0; critSince = 0; return; }   // nothing connected
  mvF = mvF > 0 ? mvF + (mv - mvF) * 0.05f : mv;           // smooth it (the LEDs make it jump)
  const int8_t old = batPct, p = (int8_t)lipoPercent(mvF);
  batPct = p;
  if (old > 10 && p <= 10) batLowAt = now ? now : 1;       // just got low: flash red now
  if (hid && p != sent) { hid->setBattery(p); sent = p; }
  if (p <= 5) {                                            // 30 s at 5 % or less: endless sleep
    if (!critSince) critSince = now ? now : 1;
    else if (now - critSince > 30000) {
      batDead = true;
      delay(3000);                                         // the OLED shows "BATTERY EMPTY"
      sleepNow(true);
    }
  } else critSince = 0;
}

// ============================ USB flasher (classic ESP32 boards on the S3's own USB port) ============================
// The S3's USB port works as a USB host. It talks to the board's USB-serial chip, puts the ESP32 in download mode
// with DTR / RTS (like the Arduino IDE does), and writes the file with the ESP32's built-in (ROM) loader:
// sync -> faster speed -> attach flash -> erase -> 1 KB blocks -> MD5 check -> reset.
static usb_host_client_handle_t uhClient = nullptr;
static usb_device_handle_t uhDev = nullptr;
static volatile uint8_t uhNewAddr = 0;
static volatile bool uhGone = false, uhInRun = false;
static SemaphoreHandle_t uhCtrlSem = nullptr, uhOutSem = nullptr;
static StreamBufferHandle_t uhRx = nullptr;
static usb_transfer_t *uhCtrlX = nullptr, *uhOutX = nullptr, *uhInX = nullptr;
static uint8_t  uhKind = 0;                               // 1 CP210x, 2 CH34x, 3 CDC-ACM (CH9102 / CH343 / the chip's own USB)
static bool     uhJtag = false;                           // the board's own USB port (ESP32-S3 / C3 / C6 USB-Serial-JTAG)
static uint16_t tgChip = 0;                               // chip of the file: 0 ESP32, 9 S3, 5 C3, 13 C6
static uint8_t  uhIfData = 0, uhIfComm = 0, uhEpIn = 0, uhEpOut = 0;
static uint16_t uhMpsIn = 64;

static void uhClientCb(const usb_host_client_event_msg_t* m, void*) {
  if (m->event == USB_HOST_CLIENT_EVENT_NEW_DEV) uhNewAddr = m->new_dev.address;
  else if (m->event == USB_HOST_CLIENT_EVENT_DEV_GONE) uhGone = true;
}
static void uhLibTask(void*) { for (;;) { uint32_t fl = 0; usb_host_lib_handle_events(portMAX_DELAY, &fl); } }
static void uhClientTask(void*) { for (;;) usb_host_client_handle_events(uhClient, portMAX_DELAY); }
static void uhDoneCb(usb_transfer_t* t) { xSemaphoreGive((SemaphoreHandle_t)t->context); }
static void uhInCb(usb_transfer_t* t) {                   // bytes from the board -> stream buffer, then ask again
  if (t->status == USB_TRANSFER_STATUS_COMPLETED && t->actual_num_bytes)
    xStreamBufferSend(uhRx, t->data_buffer, t->actual_num_bytes, 0);
  if (uhInRun && !uhGone && t->status != USB_TRANSFER_STATUS_NO_DEVICE && t->status != USB_TRANSFER_STATUS_CANCELED)
    usb_host_transfer_submit(t);
}
static bool uhCtrl(uint8_t type, uint8_t req, uint16_t val, uint16_t idx, const uint8_t* data, uint16_t len) {
  usb_setup_packet_t* s = (usb_setup_packet_t*)uhCtrlX->data_buffer;
  s->bmRequestType = type; s->bRequest = req; s->wValue = val; s->wIndex = idx; s->wLength = len;
  if (len) memcpy(uhCtrlX->data_buffer + sizeof(usb_setup_packet_t), data, len);
  uhCtrlX->num_bytes = sizeof(usb_setup_packet_t) + len;
  uhCtrlX->device_handle = uhDev; uhCtrlX->bEndpointAddress = 0;
  uhCtrlX->callback = uhDoneCb; uhCtrlX->context = uhCtrlSem; uhCtrlX->timeout_ms = 1000;
  xSemaphoreTake(uhCtrlSem, 0);
  if (usb_host_transfer_submit_control(uhClient, uhCtrlX) != ESP_OK) return false;
  if (xSemaphoreTake(uhCtrlSem, pdMS_TO_TICKS(1500)) != pdTRUE) return false;
  return uhCtrlX->status == USB_TRANSFER_STATUS_COMPLETED;
}
static bool uhWrite(const uint8_t* d, size_t n) {
  if (n > uhOutX->data_buffer_size) return false;
  memcpy(uhOutX->data_buffer, d, n);
  uhOutX->num_bytes = n; uhOutX->device_handle = uhDev; uhOutX->bEndpointAddress = uhEpOut;
  uhOutX->callback = uhDoneCb; uhOutX->context = uhOutSem; uhOutX->timeout_ms = 2000;
  xSemaphoreTake(uhOutSem, 0);
  if (usb_host_transfer_submit(uhOutX) != ESP_OK) return false;
  if (xSemaphoreTake(uhOutSem, pdMS_TO_TICKS(3000)) != pdTRUE) return false;
  return uhOutX->status == USB_TRANSFER_STATUS_COMPLETED;
}
// CH340 / CH341 speed setting (same maths as the Linux ch341 driver)
static int ch34xDivisor(uint32_t speed) {
  const uint32_t CLK = 48000000;
  int ps;
  for (ps = 3; ps >= 0; ps--) if (speed > CLK / ((1u << (12 - 3 * ps - 1)) * 512)) break;
  if (ps < 0) return -1;
  uint32_t cd = 1u << (12 - 3 * ps - 1), div = CLK / (cd * speed), fact;
  if (div < 9 || div > 255) { div /= 2; cd *= 2; fact = 0; } else fact = 1;
  if (div < 2) return -1;
  if (16 * CLK / (cd * div) - 16 * speed >= 16 * speed - 16 * CLK / (cd * (div + 1))) div++;
  if (fact == 1 && div % 2 == 0) { div /= 2; fact = 0; }
  return (int)(((0x100 - div) << 8) | (fact << 2) | (uint32_t)ps);
}
static bool uhSetBaud(uint32_t baud) {
  if (uhKind == 1) { uint8_t b[4]; memcpy(b, &baud, 4); return uhCtrl(0x41, 0x1E, 0, uhIfData, b, 4); }
  if (uhKind == 2) {
    const int d = ch34xDivisor(baud);
    return d >= 0 && uhCtrl(0x40, 0x9A, 0x1312, (uint16_t)(d | 0x80), nullptr, 0) && uhCtrl(0x40, 0x9A, 0x2518, 0x00C3, nullptr, 0);
  }
  uint8_t lc[7]; memcpy(lc, &baud, 4); lc[4] = 0; lc[5] = 0; lc[6] = 8;   // 8N1
  return uhCtrl(0x21, 0x20, 0, uhIfComm, lc, 7);
}
static bool uhLines(bool dtr, bool rts) {                 // true = line active (like "DTR on" in a serial program)
  if (uhKind == 1) return uhCtrl(0x41, 0x07, (dtr ? 1 : 0) | (rts ? 2 : 0) | 0x0300, uhIfData, nullptr, 0);
  if (uhKind == 2) return uhCtrl(0x40, 0xA4, (uint16_t)(~((dtr ? 0x20 : 0) | (rts ? 0x40 : 0)) & 0xFF), 0, nullptr, 0);
  return uhCtrl(0x21, 0x22, (dtr ? 1 : 0) | (rts ? 2 : 0), uhIfComm, nullptr, 0);
}
static void uhDrain() { uint8_t junk[64]; while (xStreamBufferReceive(uhRx, junk, sizeof junk, 0) > 0) {} }

// opens the board and finds its serial interface; err says why not
static bool uhOpen(uint8_t addr, String& err) {
  if (usb_host_device_open(uhClient, addr, &uhDev) != ESP_OK) { err = "USB: can't open"; return false; }
  const usb_device_desc_t* dd = nullptr; const usb_config_desc_t* cd = nullptr;
  if (usb_host_get_device_descriptor(uhDev, &dd) != ESP_OK || usb_host_get_active_config_descriptor(uhDev, &cd) != ESP_OK) { err = "USB: no descriptor"; return false; }
  const uint16_t vid = dd->idVendor, pid = dd->idProduct;
  uhJtag = vid == 0x303A;
  const uint8_t* p = (const uint8_t*)cd;
  const uint16_t total = cd->wTotalLength;
  int8_t ifNum[8]; uint8_t ifCls[8], eIn[8], eOut[8]; uint16_t mps[8]; int ni = -1;
  bool alt0 = true;
  for (uint16_t i = 0; i + 2 <= total && p[i] >= 2; i += p[i]) {
    if (p[i + 1] == 4 && p[i] >= 9) {                     // interface
      alt0 = p[i + 3] == 0;
      if (alt0 && ni < 7) { ni++; ifNum[ni] = p[i + 2]; ifCls[ni] = p[i + 5]; eIn[ni] = 0; eOut[ni] = 0; mps[ni] = 64; }
    } else if (p[i + 1] == 5 && p[i] >= 7 && ni >= 0 && alt0 && (p[i + 3] & 3) == 2) {   // bulk endpoint
      if (p[i + 2] & 0x80) { eIn[ni] = p[i + 2]; mps[ni] = p[i + 4] | (p[i + 5] << 8); } else eOut[ni] = p[i + 2];
    }
  }
  int use = -1;
  for (int k = 0; k <= ni; k++) if (ifCls[k] == 0x0A && eIn[k] && eOut[k]) { use = k; uhKind = 3; }   // standard CDC (CH9102 / CH343)
  if (use < 0) for (int k = 0; k <= ni; k++) if (eIn[k] && eOut[k]) { use = k; break; }
  if (use >= 0 && uhKind != 3) {
    if (vid == 0x10C4) uhKind = 1;
    else if (vid == 0x1A86) uhKind = 2;
    else use = -1;
  }
  if (use < 0) {
    char b[48];
    snprintf(b, sizeof b, vid == 0x0403 ? "FTDI chip not supported" : "unknown USB chip %04X:%04X", vid, pid);
    err = b; return false;
  }
  uhIfData = ifNum[use]; uhEpIn = eIn[use]; uhEpOut = eOut[use]; uhMpsIn = mps[use] ? mps[use] : 64;
  uhIfComm = uhIfData ? uhIfData - 1 : 0;
  for (int k = 0; k <= ni; k++) if (ifCls[k] == 0x02) uhIfComm = ifNum[k];
  if (usb_host_interface_claim(uhClient, uhDev, uhIfData, 0) != ESP_OK) { err = "USB: interface busy"; return false; }
  if (usb_host_transfer_alloc(64 + sizeof(usb_setup_packet_t), 0, &uhCtrlX) != ESP_OK ||
      usb_host_transfer_alloc(2 * (0x400 + 32) + 8, 0, &uhOutX) != ESP_OK ||
      usb_host_transfer_alloc(uhMpsIn * 8, 0, &uhInX) != ESP_OK) { err = "USB: out of memory"; return false; }
  if (uhKind == 1) { uhCtrl(0x41, 0x00, 0x0001, uhIfData, nullptr, 0); uhCtrl(0x41, 0x03, 0x0800, uhIfData, nullptr, 0); }   // enable, 8N1
  if (uhKind == 2) uhCtrl(0x40, 0xA1, 0, 0, nullptr, 0);                                                                       // serial init
  if (!uhSetBaud(115200)) { err = "USB: can't set the speed"; return false; }
  uhLines(false, false);
  uhInX->device_handle = uhDev; uhInX->bEndpointAddress = uhEpIn; uhInX->callback = uhInCb; uhInX->context = nullptr;
  uhInX->num_bytes = uhMpsIn * 8; uhInX->timeout_ms = 0;
  uhInRun = true;
  if (usb_host_transfer_submit(uhInX) != ESP_OK) { err = "USB: can't read"; return false; }
  return true;
}

// ---- ESP32 ROM loader (SLIP frames)
static uint8_t flBlock[16 + 0x400], slipBuf[2 * (0x400 + 32) + 8], romResp[128];
static bool romSend(uint8_t op, const uint8_t* data, uint16_t len, uint32_t chk) {
  size_t o = 0;
  slipBuf[o++] = 0xC0;
  const uint8_t hdr[8] = {0x00, op, (uint8_t)len, (uint8_t)(len >> 8), (uint8_t)chk, (uint8_t)(chk >> 8), (uint8_t)(chk >> 16), (uint8_t)(chk >> 24)};
  for (uint16_t i = 0; i < 8 + len; i++) {
    const uint8_t b = i < 8 ? hdr[i] : data[i - 8];
    if (b == 0xC0)      { slipBuf[o++] = 0xDB; slipBuf[o++] = 0xDC; }
    else if (b == 0xDB) { slipBuf[o++] = 0xDB; slipBuf[o++] = 0xDD; }
    else slipBuf[o++] = b;
  }
  slipBuf[o++] = 0xC0;
  return uhWrite(slipBuf, o);
}
static int romFrame(uint32_t timeoutMs) {                 // one answer frame into romResp, or -1
  const uint32_t t0 = millis(); bool in = false, esc = false; int n = 0; uint8_t b;
  while (millis() - t0 < timeoutMs) {
    if (xStreamBufferReceive(uhRx, &b, 1, pdMS_TO_TICKS(5)) != 1) { if (uhGone) return -1; continue; }
    if (!in) { if (b == 0xC0) { in = true; n = 0; } continue; }
    if (b == 0xC0) { if (n == 0) continue; return n; }
    if (esc) { esc = false; b = b == 0xDC ? 0xC0 : b == 0xDD ? 0xDB : b; }
    else if (b == 0xDB) { esc = true; continue; }
    if (n < (int)sizeof romResp) romResp[n++] = b;
  }
  return -1;
}
static bool romCmd(uint8_t op, const uint8_t* data, uint16_t len, uint32_t chk, uint32_t timeoutMs, int* rlen) {
  if (!romSend(op, data, len, chk)) return false;
  const uint32_t t0 = millis();
  while (millis() - t0 < timeoutMs) {
    const int n = romFrame(timeoutMs - (millis() - t0));
    if (n < 0) return false;
    if (n < 8 || romResp[0] != 0x01 || romResp[1] != op) continue;   // not the answer to this command
    const int sz = romResp[2] | (romResp[3] << 8);
    if (sz < 2 || 8 + sz > n) continue;
    if (rlen) *rlen = sz;
    return romResp[8 + sz - (sz >= 4 ? 4 : 2)] == 0;                 // ESP32 ROM: 4 status bytes, first = 0 means OK
  }
  return false;
}
static bool romSync() {
  uint8_t d[36] = {0x07, 0x07, 0x12, 0x20};
  memset(d + 4, 0x55, 32);
  for (uint8_t i = 0; i < 7; i++) {
    if (romCmd(0x08, d, sizeof d, 0, 120, nullptr)) {
      while (romFrame(80) >= 0) {}                        // the ROM answers a sync several times
      return true;
    }
  }
  return false;
}
static bool romEnter() {                                  // reset into download mode (same pulses as the Arduino IDE)
  for (uint8_t attempt = 0; attempt < 6; attempt++) {
    if (uhJtag) {                                         // the chip's own USB port: its USB block does the reset
      uhLines(false, false); delay(100);
      uhLines(true, false);  delay(100);                  // boot pin low
      uhLines(true, true);   uhLines(false, true); delay(100);   // reset (through 1,1 like esptool)
      uhLines(false, false); delay(attempt & 1 ? 500 : 200);
    } else {
      uhLines(false, true);  delay(100);                  // EN low = reset
      uhLines(true, false);  delay((attempt & 1) ? 400 : 50);   // IO0 low while EN goes high = download mode
      uhLines(false, false); delay(50);
    }
    uhDrain();
    if (romSync()) return true;
    if (uhGone) return false;
  }
  return false;
}

#if USE_OLED
static void flShow(const char* l1, const char* l2, int pct) {
  oled.setPowerSave(0);
  oled.clearBuffer();
  oled.setFontMode(1); oled.setDrawColor(1);
  oled.setFont(u8g2_font_5x8_tr);
  oled.drawBox(0, 0, 64, 9);
  oled.setDrawColor(0); oled.drawStr((64 - oled.getStrWidth("USB FLASHER")) / 2, 8, "USB FLASHER"); oled.setDrawColor(1);
  oled.drawStr((64 - oled.getStrWidth(l1)) / 2, 21, l1);
  oled.setFont(u8g2_font_4x6_tr);
  char a[17], b[17]; strlcpy(a, l2, sizeof a); strlcpy(b, strlen(l2) > 16 ? l2 + 16 : "", sizeof b);
  oled.drawStr((64 - oled.getStrWidth(a)) / 2, 31, a);
  if (pct < 0 && b[0]) oled.drawStr((64 - oled.getStrWidth(b)) / 2, 39, b);
  if (pct >= 0) { oled.drawFrame(2, 37, 60, 8); oled.drawBox(4, 39, 56 * constrain(pct, 0, 100) / 100, 4); }
  oled.sendBuffer();
}
#else
static void flShow(const char*, const char*, int) {}
#endif

static void flDone(bool ok, const String& msg) {
  prefs.putString("tgmsg", String(ok ? "done: " : "failed: ") + msg);
  flShow(ok ? "DONE" : "FAILED", msg.c_str(), -1);
  const uint32_t t0 = millis();
  while (millis() - t0 < 10000 && digitalRead(PIN_MODE_BTN) == HIGH) delay(20);   // 10 s, or the mode button
  while (digitalRead(PIN_MODE_BTN) == LOW) delay(10);
  ESP.restart();
  for (;;) {}
}

// writes the saved file to the board; returns "" when it worked
static String romFlashFile(File& f, uint32_t size, uint32_t addr) {
  uint32_t p[6] = {0, 0, 0, 0, 0, 0};
  uint32_t nb[2] = {460800, 0};
  if (romCmd(0x0F, (uint8_t*)nb, 8, 0, 3000, nullptr)) {   // faster: 460800 baud (stays at 115200 if refused)
    uhSetBaud(460800); delay(50); uhDrain();
  }
  if (!romCmd(0x0D, (uint8_t*)p, 8, 0, 3000, nullptr)) return "flash not found";   // SPI attach
  p[0] = 0; p[1] = 4UL * 1024 * 1024; p[2] = 64 * 1024; p[3] = 4096; p[4] = 256; p[5] = 0xFFFF;
  if (!romCmd(0x0B, (uint8_t*)p, 24, 0, 3000, nullptr)) return "flash setup failed";
  const uint32_t BS = 0x400, blocks = (size + BS - 1) / BS;
  p[0] = size; p[1] = blocks; p[2] = BS; p[3] = addr; p[4] = 0;   // newer chips: 5th word = not encrypted
  flShow("ERASING", "takes a few seconds", 0);
  if (!romCmd(0x02, (uint8_t*)p, tgChip == 0 ? 16 : 20, 0, 20000 + size / 1024 * 40, nullptr)) return "erase failed";
  for (uint32_t seq = 0; seq < blocks; seq++) {
    const uint32_t off = seq * BS, n = min(BS, size - off);
    const uint32_t hdr[4] = {BS, seq, 0, 0};
    memcpy(flBlock, hdr, 16);
    if (!f.seek(off) || f.read(flBlock + 16, n) != n) return "can't read the file";
    if (n < BS) memset(flBlock + 16 + n, 0xFF, BS - n);
    uint32_t ck = 0xEF;
    for (uint32_t i = 0; i < BS; i++) ck ^= flBlock[16 + i];
    if (!romCmd(0x03, flBlock, 16 + BS, ck, 5000, nullptr)) return uhGone ? "board unplugged" : "write failed";
    if ((seq & 7) == 0 || seq + 1 == blocks) {
      char b[24]; snprintf(b, sizeof b, "%lu / %lu KB", (unsigned long)((off + n) / 1024), (unsigned long)(size / 1024));
      flShow("WRITING", b, (int)((uint64_t)(seq + 1) * 100 / blocks));
    }
  }
  flShow("CHECKING", "MD5 of what was written", 100);
  const uint32_t m[4] = {addr, size, 0, 0};
  int rl = 0;
  if (!romCmd(0x13, (const uint8_t*)m, 16, 0, 10000 + size / 1024 * 10, &rl) || rl < 36) return "check failed";
  char theirs[33]; memcpy(theirs, romResp + 8, 32); theirs[32] = 0;
  MD5Builder md; md.begin();
  for (uint32_t off = 0; off < size; off += BS) {
    const uint32_t n = min(BS, size - off);
    if (!f.seek(off) || f.read(flBlock, n) != n) return "can't read the file";
    md.add(flBlock, n);
  }
  md.calculate();
  if (strcasecmp(md.toString().c_str(), theirs) != 0) return "check failed (MD5)";
  uhLines(false, true); delay(200); uhLines(false, false);   // reset: the board starts its new code
  return "";
}

// the whole flasher mode (started from the setup page). Never returns: the pad restarts at the end.
static void runUsbFlasher() {
  FastLED.clear(true);
  const uint32_t size = prefs.getUInt("tgsize", 0), addr = prefs.getUInt("tgaddr", 0x10000);
  tgChip = prefs.getUShort("tgchip", 0);
  ffatOk = FFat.begin(false);
  File f;
  if (ffatOk) f = FFat.open("/target.bin", FILE_READ);
  if (!f || !size || f.size() != size) flDone(false, "no file saved");
#if defined(ARDUINO_USB_MODE) && ARDUINO_USB_MODE == 0
  flDone(false, "set Tools > USB Mode to Hardware CDC");
#endif
  flShow("STARTING", "", -1);
  Serial.println("USB flasher: the USB port becomes a host now (serial output stops)");
  Serial.flush(); Serial.end();
  uhCtrlSem = xSemaphoreCreateBinary(); uhOutSem = xSemaphoreCreateBinary();
  uhRx = xStreamBufferCreate(4096, 1);
  usb_host_config_t hc = {};
  hc.skip_phy_setup = false; hc.intr_flags = ESP_INTR_FLAG_LEVEL1;
  if (usb_host_install(&hc) != ESP_OK) flDone(false, "USB host won't start");
  xTaskCreatePinnedToCore(uhLibTask, "usblib", 4096, nullptr, 3, nullptr, 0);
  usb_host_client_config_t cc = {};
  cc.is_synchronous = false; cc.max_num_event_msg = 5;
  cc.async.client_event_callback = uhClientCb; cc.async.callback_arg = nullptr;
  if (usb_host_client_register(&cc, &uhClient) != ESP_OK) flDone(false, "USB host won't start");
  xTaskCreatePinnedToCore(uhClientTask, "usbcli", 4096, nullptr, 3, nullptr, 0);

  flShow("PLUG IN BOARD", "USB port; mode button = stop", -1);
  const uint32_t t0 = millis();
  uint32_t lastLook = 0;
  while (!uhNewAddr) {
    if (millis() - lastLook >= 500) {                      // also find a board that was already plugged in
      lastLook = millis();
      uint8_t list[4]; int n = 0;
      if (usb_host_device_addr_list_fill(sizeof list, list, &n) == ESP_OK && n > 0) { uhNewAddr = list[0]; break; }
      usb_host_lib_info_t li = {};
      usb_host_lib_info(&li);
      char b[40];
      snprintf(b, sizeof b, "host on, %d dev, %lus left", li.num_devices, (unsigned long)((120000 - (millis() - t0)) / 1000));
      flShow("PLUG IN BOARD", b, -1);
    }
    if (digitalRead(PIN_MODE_BTN) == LOW) flDone(false, "stopped");
    if (millis() - t0 > 120000) flDone(false, "no board plugged in (2 min)");
    delay(20);
  }
  delay(300);
  String err;
  if (!uhOpen(uhNewAddr, err)) flDone(false, err);
  flShow("CONNECTING", uhJtag ? "board's own USB" : uhKind == 1 ? "CP210x chip" : uhKind == 2 ? "CH340 chip" : "CH9102 / CDC chip", -1);
  if (!romEnter()) flDone(false, "no answer: hold BOOT on the board, try again");
  err = romFlashFile(f, size, addr);
  f.close();
  if (err.length()) flDone(false, err);
  flDone(true, "the board runs its new code");
}

// ============================ setup / loop ============================
void setup() {
  Serial.begin(115200);
  const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  // quick listen wake: if the wireless stick is not being used, go straight back to sleep
  loadStickMac();                                       // paired stick (also needed for the quick listen)
  if (cause == ESP_SLEEP_WAKEUP_TIMER && rtcRemote && !peekRemote()) armAndSleep();
  const bool fromSleep = (cause == ESP_SLEEP_WAKEUP_EXT1 || cause == ESP_SLEEP_WAKEUP_TIMER);

  ledPowerOn();                                   // strip + matrix power on (data lines low first)

  oledBegin();                                    // OLED first, so you can see it is alive
  prefs.begin("macropad", false);

  cfgMode = prefs.getBool("cfgmode", false);
  if (cfgMode) prefs.putBool("cfgmode", false);   // one-shot: next boot returns to Bluetooth
  const bool flashMode = prefs.getBool("flashmode", false);   // USB flasher (started from the setup page)
  if (flashMode) prefs.putBool("flashmode", false);

  // Load config; fall back to defaults if the stored one is corrupt/invalid
  JsonDocument doc; String err;
  if (deserializeJson(doc, loadJson()) || !validateConfig(doc, err)) {
    Serial.println("Stored config invalid, using defaults");
    doc.clear(); deserializeJson(doc, DEFAULT_JSON);
  }
  applyConfig(doc);

  if (fromSleep) {
    // wake pins were RTC IO during sleep -> back to normal GPIO, then ignore the press that woke us
    for (uint8_t p = 0; p <= 21; p++) {
      if (!(wakeMask & (1ULL << p))) continue;
      rtc_gpio_deinit((gpio_num_t)p);
      pinMode(p, INPUT_PULLUP);
    }
    uint32_t t0 = millis();
    while (anyWakePinDown() && millis() - t0 < 5000) delay(10);
    delay(30);
    Serial.println("Woke up from sleep");
  }

  pinMode(PIN_MODE_BTN, INPUT_PULLUP);
  pinMode(PIN_STRIP_BTN, INPUT_PULLUP);
  pinMode(PIN_MAT_BTN, INPUT_PULLUP);

  // effect last chosen with the buttons (cleared whenever the config is saved in the web UI)
  if (profileMode == 0) profile = prefs.getUChar("prof", 0) ? 1 : 0;   // last mode until the device is recognised
  uint8_t s = prefs.getUChar("sfx", 255), m = prefs.getUChar("mfx", 255);
  if (s < N_STRIP_FX) curSFx = s;
  if (m < N_MAT_FX)   curMFx = m;
  int8_t oh = prefs.getChar("ohome", -100);                // main screen last chosen with oled:main
  if (oh == HOME_CLOCK || oh == HOME_PAGE || oh == HOME_CYCLE || oh == HOME_MOVIES || (oh >= 0 && oh < (int8_t)N_OANIM)) oHome = oh;
  if (rtcValid) { stripOff = rtcStripOff; matOff = rtcMatOff; }   // strip/matrix on/off kept across sleep

  ADD_STRIP();
  if (matOn) ADD_MATRIX();
  FastLED.setBrightness(255);                     // per-strip brightness is applied in renderFrame()
  FastLED.setMaxPowerInVoltsAndMilliamps(5, powerMA);

#if USE_OLED
  oled.setContrast(oCfg.bright);
  oled.setFlipMode(oCfg.rot ? 1 : 0);
#endif

  if (flashMode) runUsbFlasher();                 // never returns: restarts in Bluetooth mode when done

  if (cfgMode) {
    setenv("TZ", cCfg.tz, 1); tzset();            // show the clock in config mode too (if it was set before)
    startConfigPortal();
  } else {
    if (needRemote) startRemoteJoy();             // Wi-Fi/ESP-NOW only when a wireless joystick is configured
    hid = new BleHid();
    hid->begin(deviceName.c_str());               // Bluetooth starts right away
    for (uint8_t i = 0; i < ncomps; i++) comps[i]->begin();
    syncClock();                                  // time from the home Wi-Fi, in the background (only if needed)
    strlcpy(stickVer, prefs.getString("sver", "").c_str(), sizeof stickVer);
    if (needRemote && prefs.getUChar("sbpend", 0)) {   // a stick update is waiting
      ffatOk = FFat.begin(false);
      sotaSize = prefs.getUInt("sbsize", 0); sotaCrc = prefs.getUInt("sbcrc", 0);
      sotaPending = ffatOk && sotaSize && FFat.exists("/stick.bin");
      if (sotaPending) Serial.printf("Stick update waiting (%lu bytes): sent when the stick is on\n", (unsigned long)sotaSize);
    }
    hidQueue = xQueueCreate(16, sizeof(const Macro*));
    xTaskCreatePinnedToCore(hidTask, "hid", 4096, nullptr, 2, nullptr, 1);
    showStatus("BLE", deviceName);
    Serial.printf("BLE HID started as '%s'\n", deviceName.c_str());
  }
  markUsed();
  // LED/OLED task on core 0 so FastLED.show() never delays button/encoder polling in loop() (core 1)
  xTaskCreatePinnedToCore(fxTask, "fx", 12288, nullptr, 1, nullptr, 0);
}

void loop() {
  modeButtonTick();
  if (cfgMode) {
    dns.processNextRequest();
    server.handleClient();
    homeWifiTick();
  } else {
    for (uint8_t i = 0; i < ncomps; i++) comps[i]->update();
    fxButtonsTick();
    if (profileDirty) { profileDirty = false; prefs.putUChar("prof", profile); }   // remember iPad / PC
    if (needRemote) {
      stickLinkTick();                                        // send the stick its settings when it needs them
      stickOtaTick();                                         // stick firmware update (when one is waiting)
      stickOtaResult();
      stickInfoTick();                                        // and what the pad is doing
      static uint8_t seenFx = 254;                            // effect changed on the stick: show it on the OLED
      const uint8_t f = rjFx;
      if (f != seenFx) { if (seenFx != 254 && f != 254) fxPopupUntil = millis() + POPUP_MS; seenFx = f; }
    }
    if (stickLearned) {                                       // a stick was just paired: remember it
      stickLearned = false;
      prefs.putBytes("stickmac", stickMac, 6);
      Serial.printf("Stick paired: %02X:%02X:%02X:%02X:%02X:%02X\n", stickMac[0], stickMac[1], stickMac[2], stickMac[3], stickMac[4], stickMac[5]);
      noteInput("stick paired");
    }
    if (fxDirty && millis() - fxChangedMs > 2500) {   // save the chosen effect after the button stops being pressed
      fxDirty = false;
      prefs.putUChar("sfx", curSFx);
      prefs.putUChar("mfx", curMFx);
    }
    timerTick();
    batteryTick();
    if (sleepAllowed() && idleMs() >= (int32_t)sleepAfterMs()) goToSleep();   // not used for a while -> deep sleep
  }
  delay(1);
}
