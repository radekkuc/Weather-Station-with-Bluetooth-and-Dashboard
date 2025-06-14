import asyncio
import datetime
import csv
import sys
import re
from bleak import BleakScanner, BleakClient, BleakError

UART_SERVICE = "0000ffe0-0000-1000-8000-00805f9b34fb"
UART_CHAR    = "0000ffe1-0000-1000-8000-00805f9b34fb"
CSV_PATH     = "log.csv"
MQ9_PATH     = "mq9.csv"
TARGET_NAME  = "HMSoft"
SCAN_TIMEOUT = 5.0

# Matches: The estimated value based on Rs/R0=9.95 is: 16.01 ppm
MQ9_PATTERN = re.compile(r"based on Rs/R0=.* is: ([\d.]+) ppm")


async def run_logger():
    print("Scanning for BLE devices …")
    devices = await BleakScanner.discover(timeout=SCAN_TIMEOUT)
    device  = next((d for d in devices if TARGET_NAME in (d.name or "")), None)
    if not device:
        print(f"No device with name containing '{TARGET_NAME}' found.")
        return

    print(f"Found {device.name}  {device.address}")
    try:
        async with BleakClient(device.address) as client:
            with open(CSV_PATH, "w", newline="") as f_all, \
                 open(MQ9_PATH, "w", newline="") as f_mq9:

                writer_all = csv.writer(f_all)
                writer_all.writerow(["timestamp", "payload"])

                writer_mq9 = csv.writer(f_mq9)
                writer_mq9.writerow(["timestamp", "mq9_ppm"])

                print("Connected – logging …  (Ctrl-C to stop)")
                buffer = bytearray()

                def handle(_, data: bytearray):
                    nonlocal buffer
                    buffer.extend(data)

                    while b'\n' in buffer:
                        line, _, buffer = buffer.partition(b'\n')
                        text = line.decode("utf-8", errors="replace").strip()

                        ts = datetime.datetime.now().isoformat(timespec="seconds")
                        print(text)
                        writer_all.writerow([ts, text])
                        f_all.flush()

                        # Try to extract MQ9 value
                        match = MQ9_PATTERN.search(text)
                        if match:
                            try:
                                ppm = float(match.group(1))
                                writer_mq9.writerow([ts, ppm])
                                f_mq9.flush()
                            except ValueError:
                                print(f"[Warning] Failed to parse ppm: {match.group(1)}")

                await client.start_notify(UART_CHAR, handle)
                while True:
                    await asyncio.sleep(3600)

    except BleakError as e:
        print("BLE error:", e)
    except KeyboardInterrupt:
        print("\nStopped by user.")


if __name__ == "__main__":
    try:
        asyncio.run(run_logger())
    except KeyboardInterrupt:
        print("\nStopped by user.")
        sys.exit(0)
