// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class NdiMediaFactory : ModuleRules
	{
		public NdiMediaFactory(TargetInfo Target)
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
					"MediaAssets",
				}
			);

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"Media",
					"NdiMedia",
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"NdiMediaFactory/Private",
				}
			);

			if ((Target.Platform == UnrealTargetPlatform.Mac) ||
				(Target.Platform == UnrealTargetPlatform.Linux) ||
				(Target.Platform == UnrealTargetPlatform.Win32) ||
				(Target.Platform == UnrealTargetPlatform.Win64))
				{
					DynamicallyLoadedModuleNames.Add("NdiMedia");
				}
		}
	}
}
