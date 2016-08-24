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
					"NdiMedia/Private/Player",
				}
			);

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"MediaAssets",
				}
			);

			// add VLC libraries
			string SDKDir = System.Environment.GetEnvironmentVariable("NDI_SDK_DIR");

			if (string.IsNullOrEmpty(SDKDir))
			{
				System.Console.WriteLine("NdiMedia plug-in requires NDI_SDK_DIR environment variable to be set");

				return;
			}

			if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				PrivateIncludePaths.Add(Path.Combine(SDKDir, "include"));
				PublicLibraryPaths.Add(Path.Combine(SDKDir, "lib", "x86_x64-linux-gnu-5.3"));
				PublicAdditionalLibraries.Add("libndi.a");
			}
			else if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				PrivateIncludePaths.Add(Path.Combine(SDKDir, "include"));
				PublicLibraryPaths.Add(Path.Combine(SDKDir, "lib", "x64"));
				PublicAdditionalLibraries.Add("libndi.dylib");
			}
			else if (Target.Platform == UnrealTargetPlatform.Win32)
			{
				PrivateIncludePaths.Add(Path.Combine(SDKDir, "Include"));
				PublicLibraryPaths.Add(Path.Combine(SDKDir, "Lib", "x86"));
				PublicAdditionalLibraries.Add("Processing.NDI.Lib.x86.lib");
			}
			else if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				PrivateIncludePaths.Add(Path.Combine(SDKDir, "Include"));
				PublicLibraryPaths.Add(Path.Combine(SDKDir, "Lib", "x64"));
				PublicAdditionalLibraries.Add("Processing.NDI.Lib.x64.lib");
			}
			else
			{
				System.Console.WriteLine("NDI SDK does not supported this platform");
			}
		}
	}
}
