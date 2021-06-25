// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PlayerDataInterface : ModuleRules
{
    public PlayerDataInterface(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core",
            "OnlineSubsystem",
            "OnlineSubsystemSteam",
            "JsonUtilities",
            "Json",
            "RSA",
            "HTTP",
            "CoreUObject",
            "Engine"
        });
    }
}
