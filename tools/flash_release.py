"""Validate and flash a released non-C clock package without ESP-IDF."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

EXPECTED = {"0x0": "bootloader/bootloader.bin", "0x8000": "partition_table/partition-table.bin", "0x10000": "thirteen_clock.bin"}

def validate(root):
    manifest = json.loads((root / "manifest.json").read_text(encoding="utf-8"))
    if manifest.get("board") != "ESP32-S3-Touch-AMOLED-1.75 (non-C)" or manifest.get("flash_size") != "16MB":
        raise ValueError("Not a non-C 1.75 / 16MB package")
    files = manifest["files"]
    if len(files) != 3 or {v["offset"]: k for k, v in files.items()} != EXPECTED:
        raise ValueError("Unexpected image layout; refusing to overwrite NVS")
    for name, entry in files.items():
        data = (root / name).read_bytes()
        if len(data) != entry["bytes"] or hashlib.sha256(data).hexdigest() != entry["sha256"]:
            raise ValueError(f"Damaged or incomplete download: {name}")
    if manifest.get("flash_mode") not in ("dio", "qio") or manifest.get("flash_freq") != "80m":
        raise ValueError("Unexpected flash settings")
    return manifest

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", help="USB serial port, for example COM3")
    parser.add_argument("--baud", type=int, choices=(115200, 460800), default=460800)
    parser.add_argument("--check", action="store_true", help="Check files without accessing USB")
    options = parser.parse_args()
    root = Path(__file__).resolve().parent
    try:
        manifest = validate(root)
    except (OSError, ValueError, KeyError, TypeError) as exc:
        parser.exit(1, f"Package validation failed: {exc}\n")
    print("Verified: Waveshare ESP32-S3-Touch-AMOLED-1.75 NON-C, 16MB, 25 faces")
    if options.check:
        return
    if not options.port:
        parser.error("--port is required")
    try:
        from importlib.metadata import version
        if version("esptool").split(".")[0] != "4":
            raise ValueError("esptool 4.x is required")
    except Exception as exc:
        parser.exit(1, f"{exc}\nInstall: python -m pip install -r requirements-flash.txt\n")
    command = [sys.executable, "-m", "esptool", "--chip", "esp32s3", "--port", options.port,
               "--baud", str(options.baud), "--before", "default_reset", "--after", "hard_reset",
               "write_flash", "--flash_mode", manifest["flash_mode"],
               "--flash_freq", manifest["flash_freq"], "--flash_size", "16MB"]
    for offset, name in EXPECTED.items():
        command.extend([offset, str(root / name)])
    subprocess.run(command, check=True)
    print("Installed. Configure Wi-Fi on the display; time synchronizes automatically.")

if __name__ == "__main__":
    main()
