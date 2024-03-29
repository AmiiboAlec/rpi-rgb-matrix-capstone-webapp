#!/usr/bin/python3

import subprocess
import os

pid = subprocess.Popen(["/mnt/shared/LED-project-new/hello.py", ""], close_fds=True)

print(pid)

os.waitpid(pid.pid, 0)

print("Child exited!")
