# Ant
Ant is a self learn game engine, code base on Hazel Engine(<https://github.com/TheCherno/Hazel>)

 ---
## Getting   Started
Visual Studio 2017 or 2019 is recommended, Ant is officially untested on other development environments whilst we focus on a Windows build
<u>1. **Downloading the repository:**</u>
   Start by cloning the repository with `git clone --recursive https://github.com/Ant-Play/Ant.git`.

If the repository was cloned non-recursively previously, use `git submodule update --init` to clone the necessary submodules.

<u>2. **Configuring the dependencies:**</u>
   1. Run the [Setup.bat](scripts/Setup.bat) file found in `scripts` folder. This will download the required prerequisites for the project if they are not present yet.
   2. One prerequisite is the Vulkan SDK. If it is not installed, the script will execute the `VulkanSDK.exe` file, and will prompt the user to install the SDK.
   3. After installation, run the [Setup.bat](scripts/Setup.bat) file again. If the Vulkan SDK is installed properly, it will then download the Vulkan SDK Debug libraries. (This may take a longer amount of time)
   4. After downloading and unzipping the files, the [Win-GenProjects.bat](scripts/Win-GenProjects.bat) script file will get executed automatically, which will then generate a Visual Studio solution file for user's usage.

If changes are made, or if you want to regenerate project files, rerun the [Win-GenProjects.bat](scripts/Win-GenProjects.bat) script file found in `scripts` folder.