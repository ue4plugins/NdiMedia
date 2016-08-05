// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class NdiMediaEditor : ModuleRules
	{
		public NdiMediaEditor(TargetInfo Target)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"MediaAssets",
					"NdiMedia",
					"UnrealEd",
                }
			);

			PrivateIncludePathModuleNames.AddRange(
				new string [] {
					"AssetTools",
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"NdiMediaEditor/Private",
				}
			);
		}
	}
}
