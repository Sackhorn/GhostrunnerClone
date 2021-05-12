// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WSF : ModuleRules
{
	public WSF(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay", 
			"UMG", 
			"Slate",
			"SlateCore",
			"ProceduralMeshComponent", 
			"RenderCore", 
			"RHI", 
			"AIModule",
			"Niagara"
		});
	}
}
