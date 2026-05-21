#!/usr/bin/env python3
"""
gen_license.py - สร้างไฟล์ license.lc สำหรับ rAthena Hardware Fingerprint License
fingerprint format : "CPU:<cpu_serial>|MAC:<mac_address>"
cipher             : XOR กับ key "xai_s" แล้ว encode เป็น HEX (ตรงกับ xor_cipher ใน core.cpp)

วิธีใช้:
  1. รันบนเครื่องเซิร์ฟเวอร์ที่ต้องการออก license
     python3 gen_license.py

  2. หรือระบุ CPU serial / MAC เองด้วย argument
     python3 gen_license.py --cpu BFEBFBFF000906EA --mac AA:BB:CC:DD:EE:FF

  3. ระบุ output path
     python3 gen_license.py --output /path/to/license.lc
"""

import sys
import os
import struct
import argparse
import subprocess
import socket

# ----------------------------------------------------------------
# XOR cipher (ตรงกับ xor_cipher() ใน core.cpp)
# ----------------------------------------------------------------
KEY = b"xai_s"

def xor_encode(plaintext: str) -> str:
    """plaintext -> hex-encoded ciphertext"""
    data = plaintext.encode("utf-8")
    result = bytearray()
    for i, b in enumerate(data):
        result.append(b ^ KEY[i % len(KEY)])
    return result.hex().upper()

def xor_decode(hex_str: str) -> str:
    """hex-encoded ciphertext -> plaintext"""
    raw = bytes.fromhex(hex_str)
    result = bytearray()
    for i, b in enumerate(raw):
        result.append(b ^ KEY[i % len(KEY)])
    return result.decode("utf-8")

# ----------------------------------------------------------------
# ดึง CPU Serial
# ----------------------------------------------------------------
def get_cpu_serial() -> str:
    if sys.platform == "win32":
        try:
            out = subprocess.check_output(
                "wmic cpu get ProcessorId /value",
                shell=True, stderr=subprocess.DEVNULL
            ).decode(errors="ignore")
            for line in out.splitlines():
                if line.startswith("ProcessorId="):
                    return line.split("=", 1)[1].strip()
        except Exception:
            pass
        return ""
    else:
        # Linux: อ่าน /proc/cpuinfo
        try:
            with open("/proc/cpuinfo", "r") as f:
                physical_id = ""
                model_name  = ""
                for line in f:
                    if line.startswith("Serial"):
                        val = line.split(":", 1)[1].strip()
                        if val:
                            return val
                    if line.startswith("physical id") and not physical_id:
                        physical_id = line.split(":", 1)[1].strip()
                    if line.startswith("model name") and not model_name:
                        model_name = line.split(":", 1)[1].strip()

            # fallback: djb2 hash (ตรงกับ C++ fallback)
            combined = f"{physical_id}|{model_name}"
            if combined == "|":
                return ""
            h = 5381
            for c in combined.encode("utf-8"):
                h = ((h << 5) + h) ^ c
                h &= 0xFFFFFFFFFFFFFFFF  # 64-bit
            return f"{h:016X}"
        except Exception:
            return ""

# ----------------------------------------------------------------
# ดึง MAC Address จาก Primary NIC (interface ที่มี default gateway)
# ----------------------------------------------------------------
def get_mac_address() -> str:
    if sys.platform == "win32":
        try:
            import winreg, ctypes
            # ใช้ GetAdaptersInfo ผ่าน ctypes
            # วิธีง่ายกว่า: parse ipconfig /all แล้วจับคู่กับ route print
            gw_iface = _win_get_gateway_iface()
            if not gw_iface:
                return ""
            out = subprocess.check_output(
                "ipconfig /all", shell=True, stderr=subprocess.DEVNULL
            ).decode(errors="ignore")
            # หา block ที่ชื่อ adapter ตรงกับ gw_iface
            mac = _win_parse_mac(out, gw_iface)
            return mac
        except Exception:
            return ""
    else:
        # Linux: อ่าน /proc/net/route หา default gateway interface
        iface = _linux_get_gateway_iface()
        if not iface:
            return ""
        try:
            with open(f"/sys/class/net/{iface}/address", "r") as f:
                mac = f.read().strip().upper()
            if mac == "00:00:00:00:00:00":
                return ""
            return mac
        except Exception:
            return ""

def _linux_get_gateway_iface() -> str:
    """อ่าน /proc/net/route หา interface ที่มี Destination=0 (default route)"""
    try:
        with open("/proc/net/route", "r") as f:
            next(f)  # skip header
            for line in f:
                parts = line.split()
                if len(parts) < 4:
                    continue
                iface   = parts[0]
                dest    = int(parts[1], 16)
                gateway = int(parts[2], 16)
                if dest == 0 and gateway != 0:
                    return iface
    except Exception:
        pass
    return ""

def _win_get_gateway_iface() -> str:
    """ดึงชื่อ adapter ที่มี default gateway บน Windows ผ่าน 'route print'"""
    try:
        out = subprocess.check_output(
            "route print 0.0.0.0", shell=True, stderr=subprocess.DEVNULL
        ).decode(errors="ignore")
        # บรรทัดที่มี 0.0.0.0 เป็น Network Destination
        for line in out.splitlines():
            parts = line.split()
            if parts and parts[0] == "0.0.0.0":
                # parts: [dest, mask, gateway, interface, metric]
                if len(parts) >= 4:
                    return parts[3]  # IP ของ interface
    except Exception:
        pass
    return ""

def _win_parse_mac(ipconfig_out: str, iface_ip: str) -> str:
    """ค้นหา MAC ใน output ของ ipconfig /all โดยจับคู่จาก IP ของ interface"""
    current_mac = ""
    for line in ipconfig_out.splitlines():
        line = line.strip()
        if "Physical Address" in line or "Physical Address." in line:
            parts = line.split(":", 1)
            if len(parts) == 2:
                current_mac = parts[1].strip().replace("-", ":")
        if iface_ip in line and current_mac:
            return current_mac
    return ""

# ----------------------------------------------------------------
# สร้าง fingerprint + เขียนไฟล์
# ----------------------------------------------------------------
def make_fingerprint(cpu: str, mac: str) -> str:
    return f"CPU:{cpu}|MAC:{mac}"

def write_license(fingerprint: str, output_path: str) -> None:
    encoded = xor_encode(fingerprint)
    with open(output_path, "w", newline="\n") as f:
        f.write(encoded + "\n")
    print(f"[OK] fingerprint : {fingerprint}")
    print(f"[OK] encoded     : {encoded}")
    print(f"[OK] written to  : {os.path.abspath(output_path)}")

# ----------------------------------------------------------------
# main
# ----------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(description="Generate license.lc for rAthena")
    parser.add_argument("--cpu",    default=None, help="CPU serial (override auto-detect)")
    parser.add_argument("--mac",    default=None, help="MAC address (override auto-detect, format AA:BB:CC:DD:EE:FF)")
    parser.add_argument("--output", default="license.lc", help="Output file path (default: license.lc)")
    parser.add_argument("--verify", default=None, metavar="FILE", help="ทดสอบ decode ไฟล์ license.lc ที่มีอยู่")
    args = parser.parse_args()

    # โหมด verify
    if args.verify:
        try:
            with open(args.verify, "r") as f:
                encoded = f.read().strip()
            decoded = xor_decode(encoded)
            print(f"[VERIFY] encoded     : {encoded}")
            print(f"[VERIFY] fingerprint : {decoded}")
        except Exception as e:
            print(f"[ERROR] {e}", file=sys.stderr)
            sys.exit(1)
        return

    # ดึง CPU serial
    cpu = args.cpu
    if cpu is None:
        print("[*] Detecting CPU serial...")
        cpu = get_cpu_serial()
        if not cpu:
            print("[ERROR] Cannot detect CPU serial. Use --cpu to specify manually.", file=sys.stderr)
            sys.exit(1)
        print(f"    CPU serial : {cpu}")

    # ดึง MAC address
    mac = args.mac
    if mac is None:
        print("[*] Detecting MAC address (primary NIC)...")
        mac = get_mac_address()
        if not mac:
            print("[ERROR] Cannot detect MAC address. Use --mac to specify manually.", file=sys.stderr)
            sys.exit(1)
        print(f"    MAC address: {mac}")

    # MAC uppercase / validate format หยาบ ๆ
    mac = mac.upper()

    fingerprint = make_fingerprint(cpu, mac)
    write_license(fingerprint, args.output)

if __name__ == "__main__":
    main()
