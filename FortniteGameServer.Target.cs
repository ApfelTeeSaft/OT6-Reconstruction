// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Dedicated Server Target Configuration for UE 4.12

using UnrealBuildTool;
using System.Collections.Generic;

public class FortniteGameServerTarget : TargetRules
{
    public FortniteGameServerTarget(TargetInfo Target)
    {
        Type = TargetType.Server;
        bUsesSteam = false; // Set to true if using Steam
    }

    //
    // TargetRules interface
    //

    public override void SetupBinaries(
        TargetInfo Target,
        ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
        ref List<string> OutExtraModuleNames
    )
    {
        OutExtraModuleNames.Add("FortniteGame");
    }

    public override bool GetSupportedPlatforms(ref List<UnrealTargetPlatform> OutPlatforms)
    {
        // Dedicated server supports Windows and Linux
        OutPlatforms.Add(UnrealTargetPlatform.Win64);
        OutPlatforms.Add(UnrealTargetPlatform.Linux);
        return true;
    }

    public override List<UnrealTargetPlatform> GUBP_GetPlatforms_MonolithicOnly(UnrealTargetPlatform HostPlatform)
    {
        List<UnrealTargetPlatform> Platforms = null;

        if (HostPlatform == UnrealTargetPlatform.Win64)
        {
            Platforms = new List<UnrealTargetPlatform> { UnrealTargetPlatform.Win64, UnrealTargetPlatform.Linux };
        }

        return Platforms;
    }
}
