# MacroPad firmware

- `macropad_s3/` – main board (ESP32-S3)
- `joystick_c6/` – wireless stick (ESP32-C6)
- `hiiiii_s3/` – small example controller (ESP32-S3), with `PadOTA.h`

Every change is compiled by GitHub (Actions tab). The update files are on the **Latest firmware** release:
https://github.com/Hassouna-maker/macropad-firmware/releases/tag/latest

In the pad's setup page: `macropad_s3.ino.bin` goes in the main board area, `joystick_c6.ino.bin` in the stick area,
and a controller's `.ino.bin` under **Wireless controllers**.

## Your own controllers (updates by radio from the pad)
Copy `hiiiii_s3/PadOTA.h` next to your sketch, then:

```cpp
#include "PadOTA.h"
void setup() { padOtaBegin("my board", "1.0"); }
void loop()  { padOtaLoop(); }
```

Upload it once by USB. From then on the pad lists the board and sends it new code by radio. See the notes at the top of `PadOTA.h`.

## WLED effects
The pad (v6.4) and the stick (v3.2) contain light effects copied from [WLED](https://github.com/wled/WLED) v16.0.1
(118 for the strip and the stick, 30 for the matrix; their names start with `W:`). Music reactive effects are left out.
WLED is licensed under the EUPL v. 1.2, so this repository is too: see `LICENSE` and `NOTICE`.

The files `Wled*.h` in `macropad_s3/` and `joystick_c6/` are generated. To rebuild them from another WLED version:

```
git clone --depth 1 --branch v16.0.1 https://github.com/wled/wled
python3 tools/gen_wled.py wled/wled00 out/
```
