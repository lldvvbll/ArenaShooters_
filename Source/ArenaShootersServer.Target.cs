// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ArenaShootersServerTarget : TargetRules
{
	public ArenaShootersServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("ArenaShooters");

		bUsesSteam = true;
		GlobalDefinitions.Add("UE4_PROJECT_STEAMPRODUCTNAME=\"ArenaShooters\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMGAMEDIR=\"ArenaShooters\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMGAMEDESC=\"Arena Shooters\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMSHIPPINGID=480");
	}
}
