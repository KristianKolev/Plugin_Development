// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Plugin_Development : ModuleRules
{
	public Plugin_Development(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Json", "JsonUtilities", "Niagara", "DeveloperSettings", "UMG" });
	}
}
