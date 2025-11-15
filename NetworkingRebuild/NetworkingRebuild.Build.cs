// Copyright Epic Games, Inc. All Rights Reserved.
// Build configuration for NetworkingRebuild module
// Reconstructed UE 4.12 networking subsystem

using UnrealBuildTool;

public class NetworkingRebuild : ModuleRules
{
	public NetworkingRebuild(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Networking",
				"Sockets",
				"OnlineSubsystem",
				"OnlineSubsystemUtils"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"PacketHandler",
				"ReliabilityHandlerComponent"
			}
		);

		// Enable optimizations for networking code
		bFasterWithoutUnity = true;

		// Circular reference for networking
		CircularlyReferencedDependentModules.Add("Engine");
	}
}
