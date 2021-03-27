// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DreamEditorTarget : TargetRules
{
	public DreamEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		bUsesSteam = true;
		ExtraModuleNames.AddRange(new string[] { "Dream", "PlayerDataStore", "DreamEditor" });
	}
}
