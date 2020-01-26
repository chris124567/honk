#!/usr/bin/env python3
"""
This script prevents gdb from recognizing the file as an executable, but Linux is still able to run it.
"""

import io
import mmap
with open("server", "r+b") as f:
	mm = mmap.mmap(f.fileno(), 0)
	mm[5] = 255

	mm.close()

with open("honk", "r+b") as f:
	mm = mmap.mmap(f.fileno(), 0)
	mm[5] = 255

	mm.close()
