import subprocess
import os

git_version_info = (
    subprocess.check_output(["git", "describe", "--tags", "--always", "--dirty"])
    .strip()
    .decode("UTF-8")
    .lstrip("v")
)

print("VERSION/REVISION FROM GIT: " + git_version_info)

Import("env")
env.Append(CPPDEFINES=[("GIT_VERSION_INFO", f'\\"{git_version_info}\\"')])
