﻿// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class DreamEditor : ModuleRules
{
    public DreamEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        OverridePackageType = PackageOverrideType.GameUncookedOnly;

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine( ModuleDirectory, "Public" )
            }
        );


        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine( ModuleDirectory, "Private" )
            }
        );

        // "SkeletonEditor",
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "PropertyEditor",
            "ContentBrowser",
            "Dream",
            "PlayerDataInterface",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "BlueprintGraph",
            "AIModule"
        });
    }
}
