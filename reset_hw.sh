#!/usr/bin/python3
import serial
import time
import argparse

DEFAULT_PORT = "/dev/ttyUSB0"

def reset_esp32(port_name):
    print(f"[INFO] Connecting to {port_name}...")
    try:
        port = serial.Serial(port_name)
        print("[INFO] Triggering reset...")
        port.dtr = False
        port.rts = True
        time.sleep(0.1)
        port.rts = False
        port.close()
        print("[INFO] Reset signal sent successfully.")
    except serial.SerialException as e:
        print(f"[ERROR] Could not open port {port_name}: {e}")

def main():
    parser = argparse.ArgumentParser(description="Trigger a hardware reset on an ESP32 via serial RTS.")
    parser.add_argument("--port", default=DEFAULT_PORT, help="Serial port connected to ESP32 (default: /dev/ttyUSB0)")
    args = parser.parse_args()

    reset_esp32(args.port)

if __name__ == "__main__":
    main()
