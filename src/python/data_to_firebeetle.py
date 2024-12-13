import asyncio
from bleak import BleakClient, BleakScanner
import struct

FIREBEETLE_MAC = "FBF96330-5137-08DC-A5E5-2B871E85BC5D"
WRITE_CHARACTERISTIC = "0000DEAD-0000-1000-8000-00805F9B34FB"
READ_CHARACTERISTIC = "0000FEF4-0000-1000-8000-00805F9B34FB"

def parse_ble_data(data):
    print(f"Raw data (hex): {' '.join(f'{byte:02X}' for byte in data)}")
    if len(data) < 12:
        return "Invalid data length"

    header = data[:3].decode('utf-8', errors='ignore')
    if header != "DHT":
        return f"Invalid header: {header}"

    try:
        # Unpack floats using little-endian
        temperature, humidity = struct.unpack("<ff", data[3:11])
        checksum = data[11]
        calculated_checksum = 0
        for byte in data[:11]:
            calculated_checksum ^= byte

        if checksum != calculated_checksum:
            return "Checksum mismatch"

        return f"Temperature: {temperature:.2f}°C, Humidity: {humidity:.2f}%"
    except Exception as e:
        return f"Error parsing data: {e}"

async def main():
    async with BleakClient(FIREBEETLE_MAC) as client:
        print(f"Connected: {client.is_connected}")
        while True:
            data = await client.read_gatt_char(READ_CHARACTERISTIC)
            print(parse_ble_data(data))
            await asyncio.sleep(2)


asyncio.run(main())
