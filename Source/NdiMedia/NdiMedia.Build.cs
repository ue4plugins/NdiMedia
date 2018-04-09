// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
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
				});

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"MediaUtils",
					"NdiMediaFactory",
					"Networking",
					"Projects",
					"RenderCore",
				});

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"Media",
				});

			PrivateIncludePaths.AddRange(
				new string[] {
					"NdiMedia/Private",
					"NdiMedia/Private/Assets",
					"NdiMedia/Private/Ndi",
					"NdiMedia/Private/Player",
					"NdiMedia/Private/Shared",
				});

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"MediaAssets",
				});

			// add NDI SDK
			if ((Target.Platform == UnrealTargetPlatform.Linux) ||
				(Target.Platform == UnrealTargetPlatform.Mac) ||
				(Target.Platform == UnrealTargetPlatform.Win32) ||
				(Target.Platform == UnrealTargetPlatform.Win64))
			{
				string SdkDir = Environment.ExpandEnvironmentVariables("%NDI_SDK_DIR%");
				PrivateIncludePaths.Add(Path.Combine(SdkDir, "Include"));
			}
			else
			{
				System.Console.WriteLine("NDI SDK does not supported this platform");
			}
		}
	}
}
