// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Dream : ModuleRules
{
    public Dream(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
            "CoreUObject",
            "Engine",
            "InputCore",
            "GameplayTags",
            "GameplayTasks",
            "GameplayAbilities",
            "Niagara",
            "UMG",
            "Sockets",
            "Networking",
            "JsonUtilities",
            "Json",
            "HTTP",
            "Slate",
            "SlateCore",
            "MoviePlayer",
            "PhysicsCore",
            "OnlineSubsystem",
            "OnlineSubsystemSteam",
            "PlayerDataInterface",
            "AIModule"
        });

        AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
    }
}
