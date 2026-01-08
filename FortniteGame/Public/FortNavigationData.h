// Copyright Epic Games, Inc. All Rights Reserved.
// Reconstructed from Fortnite UE 4.12 IDA
// Navigation data and NavMesh query API

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/Navigation/NavigationTypes.h"
#include "FortNavigationTypes.h"
#include "FortNavigationFilter.h"
#include "FortNavigationData.generated.h"

/**
 * Base navigation data class
 * Provides abstract interface for navigation queries
 *
 * Evidence: ANavigationData method list
 * Source Path: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\Engine\Source\Runtime\Engine\Classes\AI/Navigation/NavigationData.h
 */
UCLASS(Abstract)
class FORTNITEGAME_API ANavigationData : public AActor
{
	GENERATED_BODY()

public:
	ANavigationData(const FObjectInitializer& ObjectInitializer);

	//--------------------------------------------------------------------------
	// Point Queries
	//--------------------------------------------------------------------------

	/**
	 * Project point onto navigation mesh within extent
	 *
	 * @param Point World space point to project
	 * @param OutLocation Output projected location on NavMesh
	 * @param Extent Search box size (default 50,50,100)
	 * @return true if projection succeeded
	 */
	virtual bool ProjectPoint(const FVector& Point, FNavLocation& OutLocation, const FVector& Extent = FVector(50, 50, 100)) const;

	/**
	 * Get random point anywhere on navigation mesh
	 *
	 * @param ResultLocation Output random location
	 * @param QueryFilter Optional filter for area costs
	 * @return true if random point found
	 */
	virtual bool GetRandomPoint(FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const;

	/**
	 * Get random point within navigable radius of origin
	 *
	 * @param Origin Center point for search
	 * @param Radius Search radius
	 * @param ResultLocation Output random location
	 * @param QueryFilter Optional filter
	 * @return true if random point found
	 */
	virtual bool GetRandomPointInNavigableRadius(const FVector& Origin, float Radius, FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const;

	/**
	 * Get random reachable point within radius (requires pathfinding)
	 *
	 * @param Origin Starting point
	 * @param Radius Search radius
	 * @param ResultLocation Output location
	 * @param QueryFilter Optional filter
	 * @return true if reachable point found
	 */
	virtual bool GetRandomReachablePointInRadius(const FVector& Origin, float Radius, FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const;

	//--------------------------------------------------------------------------
	// Batch Operations
	//--------------------------------------------------------------------------

	/**
	 * Project multiple points in batch (performance optimization)
	 *
	 * @param Workload Array of projection work items (input/output)
	 * @param Extent Default search extent
	 */
	virtual void BatchProjectPoints(TArray<FNavigationProjectionWork>& Workload, const FVector& Extent = FVector(50, 50, 100)) const;

	/**
	 * Batch raycast for line-of-sight checks
	 *
	 * @param Workload Array of raycast work items (input/output)
	 * @param QueryFilter Optional filter
	 */
	virtual void BatchRaycast(TArray<FNavigationRaycastWork>& Workload, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const;

	//--------------------------------------------------------------------------
	// Path Queries
	//--------------------------------------------------------------------------

	/**
	 * Calculate path cost between two points
	 *
	 * @param PathStart Start location
	 * @param PathEnd End location
	 * @param QueryFilter Optional filter
	 * @return Path cost, or -1 if no path exists
	 */
	virtual float CalcPathCost(const FVector& PathStart, const FVector& PathEnd, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const;

	/**
	 * Calculate path length between two points
	 *
	 * @param PathStart Start location
	 * @param PathEnd End location
	 * @param QueryFilter Optional filter
	 * @return Path length in world units, or -1 if no path exists
	 */
	virtual float CalcPathLength(const FVector& PathStart, const FVector& PathEnd, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const;

	/**
	 * Check if navigation node contains world location
	 *
	 * @param NodeRef Navigation node reference
	 * @param WorldSpaceLocation Location to test
	 * @return true if node contains location
	 */
	virtual bool DoesNodeContainLocation(NavNodeRef NodeRef, const FVector& WorldSpaceLocation) const;

	//--------------------------------------------------------------------------
	// Bounds & Properties
	//--------------------------------------------------------------------------

	/**
	 * Get bounding box of all navigation data
	 *
	 * @return Bounding box in world space
	 */
	virtual FBox GetBounds() const;

	/**
	 * Get navigation agent properties for this NavMesh
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	const FNavAgentProperties& GetNavAgentProperties() const { return NavAgentProps; }

	/**
	 * Check if this navigation data supports specific agent
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	virtual bool SupportsAgent(const FNavAgentProperties& AgentProps) const;

protected:
	/** Navigation agent properties this NavMesh was built for */
	UPROPERTY(EditAnywhere, Category = "Navigation")
	FNavAgentProperties NavAgentProps;

	/** Bounding box of navigation data */
	UPROPERTY()
	FBox NavDataBounds;

	/** Whether this navigation data is currently usable */
	UPROPERTY()
	bool bEnableDrawing;
};

/**
 * Recast NavMesh implementation
 * Wraps Recast/Detour navigation mesh for UE4
 *
 * Evidence: ARecastNavMesh references
 * Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\Engine\Source\Runtime\Engine\Private\AI\Navigation\RecastNavMeshGenerator.cpp
 */
UCLASS(Config=Engine)
class FORTNITEGAME_API ARecastNavMesh : public ANavigationData
{
	GENERATED_BODY()

public:
	ARecastNavMesh(const FObjectInitializer& ObjectInitializer);

	//~ Begin ANavigationData Interface
	virtual bool ProjectPoint(const FVector& Point, FNavLocation& OutLocation, const FVector& Extent = FVector(50, 50, 100)) const override;
	virtual bool GetRandomPoint(FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const override;
	virtual bool GetRandomPointInNavigableRadius(const FVector& Origin, float Radius, FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const override;
	virtual bool GetRandomReachablePointInRadius(const FVector& Origin, float Radius, FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const override;
	virtual void BatchProjectPoints(TArray<FNavigationProjectionWork>& Workload, const FVector& Extent = FVector(50, 50, 100)) const override;
	virtual void BatchRaycast(TArray<FNavigationRaycastWork>& Workload, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const override;
	virtual float CalcPathCost(const FVector& PathStart, const FVector& PathEnd, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const override;
	virtual float CalcPathLength(const FVector& PathStart, const FVector& PathEnd, TSharedPtr<const FNavigationQueryFilter> QueryFilter = nullptr) const override;
	virtual bool DoesNodeContainLocation(NavNodeRef NodeRef, const FVector& WorldSpaceLocation) const override;
	virtual FBox GetBounds() const override;
	//~ End ANavigationData Interface

	/**
	 * Get underlying Recast NavMesh (access to engine's navigation system)
	 * Returns UE4's built-in ARecastNavMesh if available
	 */
	class ARecastNavMesh* GetRecastNavMeshImpl() const;

protected:
	/** NavMesh generation properties */
	UPROPERTY(EditAnywhere, Category = "Generation")
	FRecastNavMeshGenerationProperties GenerationProps;

	/** Whether to use tiled navigation mesh */
	UPROPERTY(EditAnywhere, Category = "Generation")
	bool bUseTiledNavMesh;

	/** Maximum number of tiles (for memory budgeting) */
	UPROPERTY(EditAnywhere, Category = "Generation", meta = (EditCondition = "bUseTiledNavMesh"))
	int32 MaxTiles;

	/** Maximum polygons per tile */
	UPROPERTY(EditAnywhere, Category = "Generation", meta = (EditCondition = "bUseTiledNavMesh"))
	int32 MaxPolysPerTile;

	/**
	 * Cached reference to world's navigation system NavMesh
	 * We delegate to UE4's built-in navigation system rather than reimplementing Recast
	 */
	mutable TWeakObjectPtr<ARecastNavMesh> CachedRecastNavMesh;

	/** Find and cache the world's RecastNavMesh */
	void CacheNavigationData() const;
};
