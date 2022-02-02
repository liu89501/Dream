// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DreamEditorTarget : TargetRules
{
	public DreamEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		bUsesSteam = true;
		ExtraModuleNames.AddRange(new[] { "Dream", "DreamEditor" });
	}
}
