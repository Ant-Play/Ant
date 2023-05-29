import os
import subprocess
import CheckPython

# Make sure everything we need is installed
CheckPython.ValidatePackages()

import Utils
import colorama

from colorama import Fore
from colorama import Back
from colorama import Style

colorama.init()

# Change from Scripts directory to root
os.chdir('../')

print(f"{Style.BRIGHT}{Back.GREEN}Generating Visual Studio 2022 solution.{Style.RESET_ALL}")
subprocess.call(["vendor/premake/bin/premake5.exe", "vs2022"])
os.chdir('AntPlay/SandboxProject')
subprocess.call(["../../vendor/premake/bin/premake5.exe", "vs2022"])