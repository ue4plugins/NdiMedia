// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	using System.IO;

	public class NdiMedia : ModuleRules
	{
		public NdiMedia(TargetInfo Target)
		{
			DynamicallyLoadedModuleNames.AddRange(
				new string[] {
					"Media",
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"NdiMediaFactory",
					"Networking",
					"Projects",
					"RenderCore",
				}
			);

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"Media",
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"NdiMedia/Private",
					"NdiMedia/Private/Assets",
					"NdiMedia/Private/Ndi",
					"NdiMedia/Private/Player",
					"NdiMedia/Private/Shared",
				}
			);

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"MediaAssets",
				}
			);

			// add NDI libraries
			string NdiDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty"));
			string LibDir = Path.Combine(NdiDir, "lib");

			PrivateIncludePaths.Add(Path.Combine(NdiDir, "include"));

			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				PublicLibraryPaths.Add(Path.Combine(LibDir, "apple", "iOS"));
				PublicAdditionalLibraries.Add("libndi_ios.a");
			}
			else if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(LibDir, "linux", "x86_64-linux-gnu-5.4", "libndi.so.1.0.1")));
			}
			else if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				PublicDelayLoadDLLs.Add("libndi.dylib");
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(LibDir, "apple", "x86", "libndi.dylib")));
			}
			else if (Target.Platform == UnrealTargetPlatform.Win32)
			{
				PublicLibraryPaths.Add(Path.Combine(LibDir, "windows", "x86"));
				PublicAdditionalLibraries.Add("Processing.NDI.Lib.x86.lib");
				PublicDelayLoadDLLs.Add("Processing.NDI.Lib.x86.dll");
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(LibDir, "windows", "x86", "Processing.NDI.Lib.x86.dll")));
			}
			else if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				PublicLibraryPaths.Add(Path.Combine(LibDir, "windows", "x64"));
				PublicAdditionalLibraries.Add("Processing.NDI.Lib.x64.lib");
				PublicDelayLoadDLLs.Add("Processing.NDI.Lib.x64.dll");
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(LibDir, "windows", "x64", "Processing.NDI.Lib.x64.dll")));
			}
			else
			{
				System.Console.WriteLine("NDI SDK does not supported this platform");
			}
		}
	}
}
