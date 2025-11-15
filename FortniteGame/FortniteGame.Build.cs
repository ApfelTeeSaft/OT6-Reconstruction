// Copyright Epic Games, Inc. All Rights Reserved.
// FortniteGame Module Build Configuration for UE 4.12

using UnrealBuildTool;

public class FortniteGame : ModuleRules
{
    public FortniteGame(TargetInfo Target)
    {
        // Public module dependencies
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "AIModule",
                "NetworkingRebuild"
            }
        );

        // Private module dependencies
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "UMG"
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
        );

        PublicIncludePaths.AddRange(
            new string[]
            {
                "FortniteGame/Public"
            }
        );

        PrivateIncludePaths.AddRange(
            new string[]
            {
                "FortniteGame/Private"
            }
        );

        Definitions.Add("FORTNITEGAME_API=");
    }
}
