#!/usr/bin/env python3

import sys
import serial
import time
import readline

readline.clear_history()


class ChunkedSerial(serial.Serial):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def write(self, data):
        for i in range(len(data) // 64):
            super().write(data[i * 64 : (i + 1) * 64])
            client.flush()
        data = data[len(data) // 64 * 64 :]
        if len(data) > 0:
            super().write(data)
            client.flush()


def read_frame(client):
    data = client.read(4)
    size = int.from_bytes(data, "little")
    print("Reading {} bytes...".format(size))
    data = client.read(size)
    return data


device = sys.argv[1]

try:
    client = ChunkedSerial(device, 115200)
except:
    print("Error: could not connect to device")
    sys.exit(1)

print("┌" + "─" * 60 + "┐")
print("│Connected to {}".format(device) + " " * (60 - 13 - len(device)) + "│")
print("│Type 'quit' or 'q' to exit" + " " * 34 + "│")
print("│Type 'encode <input_file> <output_file>' to encode a file" + " " * 3 + "│")
print("└" + "─" * 60 + "┘")

while True:
    s = input("$ ").strip()
    if s == "quit" or s == "q":
        break
    if s == "":
        continue
    if str.startswith(s, "encode "):
        args = s.split(" ")
        if len(args) != 3:
            print("Usage: encode <input_file> <output_file>")
            continue
        input_file = args[1].strip()
        output_file = args[2].strip()
        try:
            with open(input_file, "rb") as f:
                data = f.read()
        except:
            print("Error: file not found")
            continue
        client.write(b"\x01")
        time.sleep(0.1)
        data_out = b""
        for i in range(256 // 8):
            client.write(data[i * 8 * 256 : i * 8 * 256 + 256 * 8])
            data_out += read_frame(client)
        data_out += read_frame(client)
        with open(output_file, "wb") as f:
            f.write(data_out)
