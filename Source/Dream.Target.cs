// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DreamTarget : TargetRules
{
	public DreamTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		bUsesSteam = true;
		bWithPushModel = true;
		ExtraModuleNames.AddRange(new[] { "Dream", "DreamLoadingScreen" });
	}

}
