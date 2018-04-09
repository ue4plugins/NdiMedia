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

This plug-in was last built against **Unreal Engine 4.19** and **NDI SDK 3** on
the following platforms:

- Linux (untested)
- macOS (untested)
- Windows


## Prerequisites

Strarting with 4.19, the required NDI header files and libraries are no longer
included in this plug-in. Instead you must download and install the NewTek SDK
for your platform. Verify that the *NDI_SDK_DIR* environment variable is set on
your computer. Follow the NDI SDK Documentation for installation instructions.

The users of your project must have the NDI Redist package for the desired
target platform package installed. It contains the NDI run-time libraries that
are required for this plug-in to work. You should bundle the redistributable
package installer with your project installer. Installation of the NDI SDK is
not necessary on end user machines.

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
* [NewTek NDI Redist Download](http://new.tk/NDIRedistV3)
* [NewTek NDI Homepage](http://newtek.com/ndi.html)
* [Introduction to UE4 Plugins](https://wiki.unrealengine.com/An_Introduction_to_UE4_Plugins)
