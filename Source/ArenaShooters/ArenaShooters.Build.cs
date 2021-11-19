// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ArenaShooters : ModuleRules
{
	public ArenaShooters(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", "CoreUObject", "Engine", "InputCore", "UMG", "SlateCore",
			"OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemSteam",
			"EngineSettings",
		});

		//DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}
