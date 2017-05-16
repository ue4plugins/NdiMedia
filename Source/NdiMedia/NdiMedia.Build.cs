// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	using System.IO;

	public class NdiMedia : ModuleRules
	{
		public NdiMedia(ReadOnlyTargetRules Target) : base(Target)
		{
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
 
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

			PrivateIncludePaths.Add(Path.Combine(NdiDir, "include"));

			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
                string LibDir = Path.Combine(NdiDir, "lib", "apple", "iOS");
                string LibPath = Path.Combine(LibDir, "libndi_ios.a");

                PublicAdditionalLibraries.Add(LibPath);
            }
            else if (Target.Platform == UnrealTargetPlatform.Linux)
			{
                string LibDir = Path.Combine(NdiDir, "lib", "linux", "x86_64-linux-gnu-5.4");
                string LibPath = Path.Combine(LibDir, "libndi.a");

                PublicAdditionalLibraries.Add(LibPath);
                PublicAdditionalLibraries.Add("stdc++");
            }
			else if (Target.Platform == UnrealTargetPlatform.Mac)
			{
                string LibDir = Path.Combine(NdiDir, "lib", "apple", "x64");
                string LibPath = Path.Combine(LibDir, "libndi.dylib");

                PublicLibraryPaths.Add(LibDir);
                PublicAdditionalLibraries.Add(LibPath);
                PublicDelayLoadDLLs.Add(LibPath);
                RuntimeDependencies.Add(new RuntimeDependency(LibPath));
			}
			else if (Target.Platform == UnrealTargetPlatform.Win32)
			{
                string LibDir = Path.Combine(NdiDir, "lib", "windows", "x86");
                string DllPath = Path.Combine(LibDir, "Processing.NDI.Lib.x86.dll");

                PublicLibraryPaths.Add(LibDir);
                PublicAdditionalLibraries.Add("Processing.NDI.Lib.x86.lib");
                PublicDelayLoadDLLs.Add("Processing.NDI.Lib.x86.dll");
                RuntimeDependencies.Add(new RuntimeDependency(DllPath));
            }
			else if (Target.Platform == UnrealTargetPlatform.Win64)
			{
                string LibDir = Path.Combine(NdiDir, "lib", "windows", "x64");
                string DllPath = Path.Combine(LibDir, "Processing.NDI.Lib.x64.dll");

                PublicLibraryPaths.Add(LibDir);
                PublicAdditionalLibraries.Add("Processing.NDI.Lib.x64.lib");
                PublicDelayLoadDLLs.Add("Processing.NDI.Lib.x64.dll");
                RuntimeDependencies.Add(new RuntimeDependency(DllPath));
			}
			else
			{
				System.Console.WriteLine("NDI SDK does not supported this platform");
			}
		}
	}
}
