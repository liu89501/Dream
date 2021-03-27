// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PlayerDataStore : ModuleRules
{
    public PlayerDataStore(ReadOnlyTargetRules Target) : base(Target)
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
            "JsonUtilities",
            "Json",
            "RSA",
            "OnlineSubsystem",
            "OnlineSubsystemSteam",
            "HTTP",
            "CoreUObject",
            "Engine"
        });
    }
}
