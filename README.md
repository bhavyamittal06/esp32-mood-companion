# ESP32 Mood Companion

A tiny desk companion with an animated face. A script on your Mac decides its **mood** from how your computer is doing (CPU load, battery, time of day), and an **ESP32** shows that mood on an OLED screen with matching beeps.

![demo]
<img width="2940" height="1511" alt="WhatsApp Image 2026-11-19 at 17 35 01" src="https://github.com/user-attachments/assets/7a196882-98a3-4c96-a6e1-4c589095a1e2" />



**Status:** working in the Wokwi simulator. Real-hardware build coming next.

▶ **Try it in your browser:** [Wokwi simulation](https://wokwi.com/projects/481118620147792897)
## How it works

```
 Mac (mood_sender.py)                ESP32 (face.ino)
 ┌──────────────────┐   mood word   ┌─────────────────────┐
 │ CPU / battery /  │ ────────────► │ draws face on OLED  │
 │ time of day      │  e.g. "happy" │ plays buzzer sound  │
 └──────────────────┘  (USB serial) └─────────────────────┘
```

1. `mood_sender.py` reads the system state and picks one of six moods.
2. It sends the mood as a word followed by a newline (115200 baud).
3. `face.ino` reads the word, redraws the face, and plays a short sound.

## Moods

| Mood | Face | When the Mac script picks it |
|---|---|---|
| `sad` | frown + tear | battery below 20% and unplugged |
| `angry` | slanted brows, flat mouth | CPU at 85% or more |
| `excited` | big eyes, open mouth | CPU at 60% or more |
| `sleepy` | closed eyes, "zZ" | between 11 PM and 6 AM |
| `happy` | smile | CPU below 20% (idle) |
| `neutral` | flat mouth | everything else |

The first matching rule wins, in the order above. You can change the rules in `pick_mood()` in `mood_sender.py`.

## Hardware

| Part | Notes |
|---|---|
| ESP32 dev board | any common ESP32 DevKit |
| SSD1306 OLED, 128x64, I²C | address `0x3C` |
| Passive buzzer | |
| Breadboard + jumper wires | |

### Wiring

| Part pin | ESP32 pin |
|---|---|
| OLED GND | GND |
| OLED VCC | 3V3 |
| OLED SCL | GPIO 22 |
| OLED SDA | GPIO 21 |
| Buzzer (−) | GND |
| Buzzer (+) | GPIO 18 |

## Running it

### 1. The face (ESP32 / Wokwi)

- Libraries: **Adafruit SSD1306** and **Adafruit GFX Library**.
- Open `face.ino` in the Arduino IDE (with the ESP32 board package installed) or paste it into a Wokwi ESP32 project.
- Open the Serial Monitor (115200 baud) and type a mood to test it:
  `happy`, `sad`, `angry`, `sleepy`, `excited`, `neutral`

### 2. The mood script (Mac)

```bash
python3 -m venv venv
source venv/bin/activate
pip install psutil pyserial

# print the mood only (no hardware needed)
python3 mood_sender.py

# cycle through all moods to test
python3 mood_sender.py --demo --interval 1

# send moods to a real ESP32 over USB
python3 mood_sender.py --port /dev/cu.usbserial-XXXX
```

Options: `--interval` (seconds between checks), `--count` (stop after N readings), `--baud` (default 115200).

> In the Wokwi simulator the browser can't talk to your Mac directly, so type the mood printed by the script into Wokwi's Serial Monitor. With a real board, `--port` sends it automatically.

## Files

| File | What it is |
|---|---|
| `face.ino` | ESP32 firmware: face drawing, sounds, serial input |
| `mood_sender.py` | Mac script that decides and sends the mood |

## Roadmap

- [x] Faces and sounds for six moods
- [x] Mac script driven by CPU, battery and time of day
- [x] Simulation in Wokwi
- [ ] Build on real hardware
- [ ] Wi-Fi control (HTTP or MQTT) instead of USB serial
- [ ] Button to change mood by hand
- [ ] More moods and smoother animations
- [ ] Case (cardboard or 3D-printed)

## License

MIT (add a `LICENSE` file if you want to publish under this license).
