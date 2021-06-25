// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DreamTarget : TargetRules
{
	public DreamTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		bUsesSteam = true;
		ExtraModuleNames.AddRange(new string[] { "Dream" });
	}

}
