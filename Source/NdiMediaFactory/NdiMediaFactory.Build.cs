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
					"Projects",
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

			if (Target.Type == TargetRules.TargetType.Editor)
			{
				DynamicallyLoadedModuleNames.Add("Settings");
				PrivateIncludePathModuleNames.Add("Settings");
			}

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
