// Copyright Epic Games, Inc. All Rights Reserved.
// Reconstructed from Fortnite UE 4.12 IDA
// Navigation system data structures and types

#pragma once

#include "CoreMinimal.h"
#include "AI/Navigation/NavigationTypes.h"
#include "FortNavigationTypes.generated.h"

/**
 * Navigation node reference type (wraps Detour NavMeshPolyRef)
 * In UE 4.12, this is typically a uint64
 */
typedef uint64 NavNodeRef;
static const NavNodeRef INVALID_NAVNODEREF = 0;

/**
 * Location on navigation mesh
 * Combines world space position with navigation node reference
 *
 * Evidence: Deduced from ANavigationData::ProjectPoint return type
 */
USTRUCT(BlueprintType)
struct FORTNITEGAME_API FNavLocation
{
	GENERATED_BODY()

	/** Location in world space, typically projected onto NavMesh surface */
	UPROPERTY(BlueprintReadWrite, Category = "Navigation")
	FVector Location;

	/** Reference to navigation mesh polygon containing this location */
	UPROPERTY(BlueprintReadWrite, Category = "Navigation")
	NavNodeRef NodeRef;

	FNavLocation()
		: Location(FVector::ZeroVector)
		, NodeRef(INVALID_NAVNODEREF)
	{}

	FNavLocation(const FVector& InLocation, NavNodeRef InNodeRef = INVALID_NAVNODEREF)
		: Location(InLocation)
		, NodeRef(InNodeRef)
	{}

	/** Check if this location has a valid NavMesh polygon reference */
	bool IsValid() const { return NodeRef != INVALID_NAVNODEREF; }

	/** Check if this location has geometry (may not have valid NavMesh reference) */
	bool HasNodeRef() const { return NodeRef != INVALID_NAVNODEREF; }
};

/**
 * Single point on a navigation path
 * Reconstructed from path following system requirements
 */
USTRUCT(BlueprintType)
struct FORTNITEGAME_API FNavPathPoint
{
	GENERATED_BODY()

	/** World space location of this path point */
	UPROPERTY(BlueprintReadWrite, Category = "Navigation")
	FVector Location;

	/** Navigation node reference for this point */
	UPROPERTY()
	NavNodeRef NodeRef;

	/** Custom flags for this path point (e.g., jump point, crouch required) */
	UPROPERTY(BlueprintReadWrite, Category = "Navigation")
	uint32 Flags;

	FNavPathPoint()
		: Location(FVector::ZeroVector)
		, NodeRef(INVALID_NAVNODEREF)
		, Flags(0)
	{}

	FNavPathPoint(const FVector& InLocation)
		: Location(InLocation)
		, NodeRef(INVALID_NAVNODEREF)
		, Flags(0)
	{}
};

/**
 * Batch point projection work item
 * Used for optimized batch NavMesh queries
 *
 * Evidence: ANavigationData::BatchProjectPoints
 */
struct FORTNITEGAME_API FNavigationProjectionWork
{
	/** Point to project onto NavMesh */
	FVector Point;

	/** Projection extent (search box size) */
	FVector Extent;

	/** Output: Projected location */
	FNavLocation OutLocation;

	/** Output: Whether projection succeeded */
	bool bResult;

	FNavigationProjectionWork()
		: Point(FVector::ZeroVector)
		, Extent(FVector::ZeroVector)
		, OutLocation()
		, bResult(false)
	{}

	FNavigationProjectionWork(const FVector& InPoint, const FVector& InExtent = FVector(50.0f, 50.0f, 100.0f))
		: Point(InPoint)
		, Extent(InExtent)
		, OutLocation()
		, bResult(false)
	{}
};

/**
 * Batch raycast work item
 * Used for line-of-sight checks against NavMesh
 *
 * Evidence: ANavigationData::BatchRaycast
 */
struct FORTNITEGAME_API FNavigationRaycastWork
{
	/** Raycast start point */
	FVector StartPos;

	/** Raycast end point */
	FVector EndPos;

	/** Output: Hit location if blocked */
	FVector HitLocation;

	/** Output: Whether raycast was blocked */
	bool bDidHit;

	FNavigationRaycastWork()
		: StartPos(FVector::ZeroVector)
		, EndPos(FVector::ZeroVector)
		, HitLocation(FVector::ZeroVector)
		, bDidHit(false)
	{}

	FNavigationRaycastWork(const FVector& InStart, const FVector& InEnd)
		: StartPos(InStart)
		, EndPos(InEnd)
		, HitLocation(FVector::ZeroVector)
		, bDidHit(false)
	{}
};

/**
 * Recast NavMesh generation properties
 * Defines parameters for NavMesh building
 *
 * Based on standard Recast parameters for UE 4.12
 */
USTRUCT(BlueprintType)
struct FORTNITEGAME_API FRecastNavMeshGenerationProperties
{
	GENERATED_BODY()

	/** Size of voxel used in heightfield (XY plane) */
	UPROPERTY(EditAnywhere, Category = "Generation", meta = (ClampMin = "1.0"))
	float CellSize;

	/** Size of voxel used in heightfield (Z axis) */
	UPROPERTY(EditAnywhere, Category = "Generation", meta = (ClampMin = "1.0"))
	float CellHeight;

	/** Minimum floor to ceiling height that will still allow navigation */
	UPROPERTY(EditAnywhere, Category = "Generation")
	float AgentHeight;

	/** Maximum walkable slope angle in degrees */
	UPROPERTY(EditAnywhere, Category = "Generation", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float AgentMaxSlope;

	/** Maximum ledge height that can be stepped up */
	UPROPERTY(EditAnywhere, Category = "Generation")
	float AgentMaxStepHeight;

	/** Minimum region size for isolated NavMesh islands (in voxels) */
	UPROPERTY(EditAnywhere, Category = "Generation")
	float MinRegionArea;

	/** Size of single tile in world units */
	UPROPERTY(EditAnywhere, Category = "Generation")
	float TileSize;

	FRecastNavMeshGenerationProperties()
		: CellSize(19.0f)          // Default for Fortnite-scale worlds
		, CellHeight(10.0f)        // Good vertical resolution
		, AgentHeight(144.0f)      // Standard character capsule height (UE units)
		, AgentMaxSlope(45.0f)     // Reasonable walkable slope
		, AgentMaxStepHeight(35.0f) // Can step up stairs
		, MinRegionArea(0.0f)      // Keep all regions
		, TileSize(1024.0f)        // 1024 UE units per tile
	{}
};

/**
 * Navigation agent properties
 * Defines physical characteristics of entities that use navigation
 *
 * Evidence: NavAgentProperties
 */
USTRUCT(BlueprintType)
struct FORTNITEGAME_API FNavAgentProperties
{
	GENERATED_BODY()

	/** Radius of the agent (for collision/clearance checks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
	float AgentRadius;

	/** Height of the agent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
	float AgentHeight;

	/** Maximum step height the agent can traverse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
	float AgentStepHeight;

	/** Maximum slope angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
	float AgentMaxSlope;

	/** Preferred navigation data class name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
	FName PreferredNavData;

	/** Whether agent can crouch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
	bool bCanCrouch;

	/** Whether agent can jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
	bool bCanJump;

	/** Whether agent can swim */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
	bool bCanSwim;

	/** Whether agent can fly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
	bool bCanFly;

	FNavAgentProperties()
		: AgentRadius(34.0f)       // Fortnite character capsule radius
		, AgentHeight(192.0f)      // Fortnite character capsule height
		, AgentStepHeight(35.0f)
		, AgentMaxSlope(45.0f)
		, PreferredNavData(NAME_None)
		, bCanCrouch(true)
		, bCanJump(true)
		, bCanSwim(true)
		, bCanFly(false)
	{}
};
