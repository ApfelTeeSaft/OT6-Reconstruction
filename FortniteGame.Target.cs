// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Game Target Configuration for UE 4.12 (Client/Editor builds)

using UnrealBuildTool;
using System.Collections.Generic;

public class FortniteGameTarget : TargetRules
{
    public FortniteGameTarget(TargetInfo Target)
    {
        Type = TargetType.Game;
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
}

public class FortniteGameEditorTarget : TargetRules
{
    public FortniteGameEditorTarget(TargetInfo Target)
    {
        Type = TargetType.Editor;
        bUsesSteam = false;
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
}
