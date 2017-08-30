// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class NdiMediaEditor : ModuleRules
	{
		public NdiMediaEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"MediaAssets",
					"NdiMedia",
					"PropertyEditor",
					"Slate",
					"SlateCore",
					"UnrealEd",
				});

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"AssetTools",
				});

			PrivateIncludePaths.AddRange(
				new string[] {
					"NdiMediaEditor/Private",
					"NdiMediaEditor/Private/Customizations",
					"NdiMediaEditor/Private/Factories",
				});
		}
	}
}
