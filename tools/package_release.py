"""Package a 16 MiB production build without NVS, credentials or local logs."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build")
    options = parser.parse_args()
    build = options.build_dir.resolve()
    args = json.loads((build / "flasher_args.json").read_text(encoding="utf-8"))
    config = json.loads((build / "config/sdkconfig.json").read_text(encoding="utf-8"))
    if args["flash_settings"]["flash_size"] != "16MB" or config.get("CLOCK_SELF_TEST"):
        raise ValueError("Only 16MB production builds may be packaged")
    expected = {"0x0": "bootloader/bootloader.bin", "0x8000": "partition_table/partition-table.bin", "0x10000": "thirteen_clock.bin"}
    if args["flash_files"] != expected:
        raise ValueError("Unexpected image layout")
    for key, value in config.items():
        if re.search(r"(SSID|PASSWORD|PASSPHRASE)$", key) and value:
            raise ValueError(f"Refusing a build with embedded credentials: {key}")
    description = json.loads((build / "project_description.json").read_text(encoding="utf-8"))
    output = ROOT / "release"
    output.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="clock-package-", dir=output) as temporary:
        package = Path(temporary) / "clock-25faces-non-c-16mb"
        package.mkdir()
        entries = {}
        for offset, name in expected.items():
            dest = package / name
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(build / name, dest)
            entries[name] = {"offset": offset, "sha256": hashlib.sha256(dest.read_bytes()).hexdigest(), "bytes": dest.stat().st_size}
        manifest = {"board": "ESP32-S3-Touch-AMOLED-1.75 (non-C)", "idf": description["git_revision"],
                    "flash_size": "16MB", "flash_mode": args["flash_settings"]["flash_mode"],
                    "flash_freq": args["flash_settings"]["flash_freq"], "face_count": 25, "hour_dial": 12, "files": entries}
        (package / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        shutil.copyfile(ROOT / "tools/flash_release.py", package / "flash.py")
        shutil.copyfile(ROOT / "README.md", package / "README.md")
        (package / "tools").mkdir()
        shutil.copyfile(ROOT / "tools/sync_time.py", package / "tools/sync_time.py")
        (package / "requirements-flash.txt").write_text("esptool>=4.8,<5\n", encoding="ascii")
        for source in (ROOT / "docs").rglob("*"):
            if source.is_file() and (source.suffix in (".md", ".png", ".json") or source.name == "WAVESHARE-LICENSE"):
                dest = package / source.relative_to(ROOT)
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(source, dest)
        for source in (ROOT / "tools/fonts").glob("*-OFL.txt"):
            dest = package / "tools/fonts" / source.name
            dest.parent.mkdir(exist_ok=True)
            shutil.copyfile(source, dest)
        archive = output / "clock-25faces-non-c-16mb.zip"
        with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as z:
            for path in sorted(package.rglob("*")):
                if path.is_file():
                    z.write(path, path.relative_to(package.parent))
    checksum = hashlib.sha256(archive.read_bytes()).hexdigest()
    (output / "SHA256SUMS.txt").write_text(f"{checksum}  {archive.name}\n", encoding="ascii")
    print(archive)
    print(f"SHA256: {checksum}")

if __name__ == "__main__":
    main()
