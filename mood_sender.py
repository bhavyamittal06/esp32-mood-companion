#!/usr/bin/env python3
"""
mood_sender.py - decides a "mood" for your desk companion.

Mood comes from:
  - CPU load      (busy laptop -> excited / angry)
  - battery       (low and unplugged -> sad)
  - time of day   (late night -> sleepy)

By default it only PRINTS the mood (no hardware needed).
Later, add --port to send it to the ESP32 over USB serial.

Usage:
  python3 mood_sender.py                  # print mood every 2 s
  python3 mood_sender.py --demo           # cycle through every mood (for testing)
  python3 mood_sender.py --count 5        # stop after 5 readings
  python3 mood_sender.py --port /dev/cu.usbserial-0001   # send to real ESP32
"""
import argparse
import datetime
import time

import psutil

MOODS = ["happy", "sad", "angry", "sleepy", "excited", "neutral"]


def pick_mood(cpu: float, hour: int, battery_pct, plugged) -> str:
    # Order matters: the first matching rule wins.
    if battery_pct is not None and plugged is False and battery_pct < 20:
        return "sad"        # low battery, not charging
    if cpu >= 85:
        return "angry"      # CPU is suffering
    if cpu >= 60:
        return "excited"    # lots going on
    if hour >= 23 or hour < 6:
        return "sleepy"     # late night
    if cpu < 20:
        return "happy"      # calm and idle
    return "neutral"


def read_state():
    cpu = psutil.cpu_percent(interval=1)  # averaged over 1 second
    hour = datetime.datetime.now().hour
    batt = psutil.sensors_battery()       # None on desktops
    pct = batt.percent if batt else None
    plugged = batt.power_plugged if batt else None
    return cpu, hour, pct, plugged


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", help="serial port of the ESP32 (omit to just print)")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--interval", type=float, default=2.0, help="seconds between checks")
    ap.add_argument("--count", type=int, default=0, help="stop after N readings (0 = run forever)")
    ap.add_argument("--demo", action="store_true", help="cycle through all moods")
    args = ap.parse_args()

    ser = None
    if args.port:
        import serial  # pip install pyserial
        ser = serial.Serial(args.port, args.baud, timeout=1)
        time.sleep(2)  # ESP32 resets when the port opens; give it time to boot

    last = None
    n = 0
    try:
        while True:
            if args.demo:
                mood = MOODS[n % len(MOODS)]
                info = "demo"
            else:
                cpu, hour, pct, plugged = read_state()
                mood = pick_mood(cpu, hour, pct, plugged)
                info = f"cpu={cpu:.0f}% hour={hour} battery={pct}"

            if mood != last:  # only act when the mood changes
                print(f"[{datetime.datetime.now():%H:%M:%S}] mood -> {mood:8s} ({info})")
                if ser:
                    ser.write((mood + "\n").encode())
                last = mood

            n += 1
            if args.count and n >= args.count:
                break
            time.sleep(args.interval)
    except KeyboardInterrupt:
        print("\nbye!")
    finally:
        if ser:
            ser.close()


if __name__ == "__main__":
    main()
