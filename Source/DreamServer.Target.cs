// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DreamServerTarget : TargetRules
{
	public DreamServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		bUsesSteam = true;
		ExtraModuleNames.AddRange(new string[] { "Dream", "PlayerDataStore" });
	}
}
