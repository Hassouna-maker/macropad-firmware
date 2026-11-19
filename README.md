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
