"""Check the distributable ZIP without touching a serial port."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import zipfile
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("flash_release", ROOT / "tools/flash_release.py")
flash = importlib.util.module_from_spec(spec)
spec.loader.exec_module(flash)

class ReleaseTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="clock-release-test-", dir=ROOT / "release")
        self.addCleanup(self.temporary.cleanup)
        with zipfile.ZipFile(ROOT / "release/clock-25faces-non-c-16mb.zip") as archive:
            names = archive.namelist()
            self.assertFalse(any(name.endswith(".log") or "sdkconfig" in name or "nvs.bin" in name for name in names))
            archive.extractall(self.temporary.name)
        self.folder = Path(self.temporary.name) / "clock-25faces-non-c-16mb"

    def test_valid_package_and_check_cli(self):
        data = flash.validate(self.folder)
        self.assertEqual(data["face_count"], 25)
        result = subprocess.run([sys.executable, str(self.folder / "flash.py"), "--check"], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_corrupt_image_rejected(self):
        path = self.folder / "thirteen_clock.bin"
        data = bytearray(path.read_bytes())
        data[-1] ^= 1
        path.write_bytes(data)
        with self.assertRaisesRegex(ValueError, "Damaged"):
            flash.validate(self.folder)

    def test_nvs_target_rejected(self):
        path = self.folder / "manifest.json"
        data = json.loads(path.read_text())
        data["files"]["thirteen_clock.bin"]["offset"] = "0x9000"
        path.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, "layout"):
            flash.validate(self.folder)

    def test_32mb_rejected(self):
        path = self.folder / "manifest.json"
        data = json.loads(path.read_text())
        data["flash_size"] = "32MB"
        path.write_text(json.dumps(data))
        with self.assertRaises(ValueError):
            flash.validate(self.folder)

    def test_flash_command_preserves_nvs(self):
        with patch.object(flash, "__file__", str(self.folder / "flash.py")), patch.object(sys, "argv", ["flash.py", "--port", "COM99"]), patch("importlib.metadata.version", return_value="4.12.0"), patch.object(flash.subprocess, "run") as run:
            flash.main()
        command = run.call_args.args[0]
        self.assertIn("COM99", command)
        self.assertIn("16MB", command)
        self.assertNotIn("erase_flash", command)
        self.assertEqual(command[-6::2], ["0x0", "0x8000", "0x10000"])

if __name__ == "__main__":
    unittest.main()
