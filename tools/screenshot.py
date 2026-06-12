#!/usr/bin/env python3
"""Boot the kernel in QEMU headless, grab a framebuffer screendump, save PNG.

Usage: python tools/screenshot.py [out.png]

This is a developer/CI helper; it is not part of the kernel build.
"""
import os
import socket
import subprocess
import sys
import time

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KERNEL = os.path.join(REPO, "build", "kernel.elf")
QEMU = os.environ.get(
    "QEMU",
    r"C:\Program Files\qemu\qemu-system-riscv64.exe",
)
MON_HOST, MON_PORT = "127.0.0.1", 55556


def monitor_cmd(cmd: str) -> None:
    with socket.create_connection((MON_HOST, MON_PORT), timeout=5) as s:
        time.sleep(0.2)
        s.sendall((cmd + "\n").encode())
        time.sleep(0.5)
        try:
            s.recv(4096)
        except OSError:
            pass


def main() -> int:
    out_png = sys.argv[1] if len(sys.argv) > 1 else os.path.join(REPO, "fb.png")
    ppm = os.path.join(REPO, "fb.ppm")
    if os.path.exists(ppm):
        os.remove(ppm)

    proc = subprocess.Popen([
        QEMU, "-machine", "virt", "-m", "256M",
        "-bios", "default", "-kernel", KERNEL,
        "-device", "ramfb",
        "-serial", "null",
        "-monitor", f"tcp:{MON_HOST}:{MON_PORT},server,nowait",
        "-display", "none",
    ])
    try:
        time.sleep(2.5)              # let the kernel boot and render a frame
        monitor_cmd(f'screendump {ppm}')
        time.sleep(1.0)
        monitor_cmd("quit")
        time.sleep(0.5)
    finally:
        if proc.poll() is None:
            proc.terminate()

    if not os.path.exists(ppm):
        print("ERROR: screendump did not produce", ppm)
        return 1

    from PIL import Image
    Image.open(ppm).save(out_png)
    print("wrote", out_png)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
