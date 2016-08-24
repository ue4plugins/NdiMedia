# NdiMedia

Unreal Engine 4 Media Framework plug-in for NewTek's Network Device Interface (NDI).

![Logo](Docs/logo.png)


## About

NDI™ (Network Device Interface) is a standard created by NewTek to make it easy
to develop video-related products that share video on a local Ethernet network
(video over IP). This plug-in makes NDI media input streams available in Unreal
Engine 4.

Make sure to pull the *Tag* that matches your Unreal Engine version. If you sync
to *Master* the code may not compile, because it may depend on Engine changes
that are not yet available in the UE4 Master branch.


## Supported Platforms

This plug-in was last built against **Unreal Engine 4.13 Preview** and tested
against the following platforms:

- ~~Linux~~
- ~~Mac~~
- Windows


## Prerequisites

This plug-in requires the Newtek NDI SDK, which can be downloaded for free after
registering on the Newtek web site. The current license terms do not allow for
the SDK files to be redistributed with this plug-in, so that some manual
per-platform configuration is required. The plan for future versions of this
plug-in is to become self-contained, however.

### Linux

Download and unzip the NDI SDK for Linux into a directory of your choice and add
a *NDI_SDK_DIR* environment variable that points to this directory. The NDI
library will be linked statically.

### Mac

Download and unzip the NDI SDK for OSX into a directory of your choice and add
a *NDI_SDK_DIR* environment variable that points to this directory. The NDI
library will be linked statically.

### Windows

Install the NDI SDK for Windows into a directory of your choice. The installer
does not currently create any environment variables, so you have to add one
manually. Add *NDI_SDK_DIR* and point it to the installation directory,
i.e. *c:\Program Files\NewTek\NewTek NDI SDK*

The NDI library will be linked dynamically, which means that, for now you have
to copy the DLL to the plug-in's binaries directory, i.e. copy
*c:\Program Files\NewTek\NewTek NDI SDK\Bin\x64\Processing.NDI.Lib.x64.dll* to
*\Engine\Plugins\Media\NdiMedia\Binaries\Win64\*


## Dependencies

This plug-in requires Visual Studio and either a C++ code project or the full
Unreal Engine 4 source code from GitHub. If you are new to programming in UE4,
please see the official [Programming Guide](https://docs.unrealengine.com/latest/INT/Programming/index.html)! 


## Usage

You can use this plug-in as a project plug-in, or an Engine plug-in.

If you use it as a project plug-in, clone this repository into your project's
*/Plugins* directory and compile your game in Visual Studio. A C++ code project
is required for this to work.

If you use it as an Engine plug-in, clone this repository into the
*/Engine/Plugins/Media* directory and compile your game. Full Unreal Engine 4
source code from GitHub is required for this.


## Support

Please [file an issue](https://github.com/ue4plugins/NdiMedia/issues), submit a
[pull request](https://github.com/ue4plugins/NdiMedia/pulls?q=is%3Aopen+is%3Apr)
or email us at info@headcrash.industries


## References

* [NewTek NDI SDK Download](http://pages.newtek.com/NDI-Developers.html)
* [NewTek NDI Homepage](http://newtek.com/ndi.html)
* [Introduction to UE4 Plugins](https://wiki.unrealengine.com/An_Introduction_to_UE4_Plugins)
