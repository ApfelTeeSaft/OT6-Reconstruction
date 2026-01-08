// Copyright Epic Games, Inc. All Rights Reserved.
// Reconstructed from Fortnite UE 4.12 IDA
// Navigation data implementation - delegates to UE4 NavigationSystem

#include "FortNavigationData.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/RecastNavMesh.h"
#include "Engine/World.h"

//==============================================================================
// ANavigationData
//==============================================================================

ANavigationData::ANavigationData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, NavDataBounds(ForceInit)
	, bEnableDrawing(false)
{
	bNetLoadOnClient = false;
	SetCanBeDamaged(false);
}

bool ANavigationData::ProjectPoint(const FVector& Point, FNavLocation& OutLocation, const FVector& Extent) const
{
	// Base implementation - derived classes override
	OutLocation = FNavLocation(Point);
	return false;
}

bool ANavigationData::GetRandomPoint(FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	// Base implementation
	return false;
}

bool ANavigationData::GetRandomPointInNavigableRadius(const FVector& Origin, float Radius, FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	// Base implementation
	return false;
}

bool ANavigationData::GetRandomReachablePointInRadius(const FVector& Origin, float Radius, FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	// Base implementation
	return false;
}

void ANavigationData::BatchProjectPoints(TArray<FNavigationProjectionWork>& Workload, const FVector& Extent) const
{
	// Process each point individually (derived classes can optimize)
	for (FNavigationProjectionWork& Work : Workload)
	{
		Work.bResult = ProjectPoint(Work.Point, Work.OutLocation, Work.Extent);
	}
}

void ANavigationData::BatchRaycast(TArray<FNavigationRaycastWork>& Workload, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	// Base implementation - no raycasting
	for (FNavigationRaycastWork& Work : Workload)
	{
		Work.bDidHit = false;
	}
}

float ANavigationData::CalcPathCost(const FVector& PathStart, const FVector& PathEnd, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	// Base implementation - direct distance
	return FVector::Dist(PathStart, PathEnd);
}

float ANavigationData::CalcPathLength(const FVector& PathStart, const FVector& PathEnd, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	// Base implementation - direct distance
	return FVector::Dist(PathStart, PathEnd);
}

bool ANavigationData::DoesNodeContainLocation(NavNodeRef NodeRef, const FVector& WorldSpaceLocation) const
{
	// Base implementation
	return false;
}

FBox ANavigationData::GetBounds() const
{
	return NavDataBounds;
}

bool ANavigationData::SupportsAgent(const FNavAgentProperties& AgentProps) const
{
	// Check if agent properties are compatible
	// For now, accept all agents
	return true;
}

//==============================================================================
// ARecastNavMesh
//==============================================================================

ARecastNavMesh::ARecastNavMesh(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUseTiledNavMesh(true)
	, MaxTiles(256)
	, MaxPolysPerTile(512)
{
	// Default properties for Fortnite-scale worlds
	NavAgentProps = FNavAgentProperties();
	GenerationProps = FRecastNavMeshGenerationProperties();

	UE_LOG(LogNavigation, Log, TEXT("ARecastNavMesh constructed - will delegate to world NavMesh"));
}

void ARecastNavMesh::CacheNavigationData() const
{
	// Find world's navigation system and cache its RecastNavMesh
	if (CachedRecastNavMesh.IsValid())
	{
		return;  // Already cached
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogNavigation, Warning, TEXT("ARecastNavMesh::CacheNavigationData - No world"));
		return;
	}

	UNavigationSystem* NavSys = World->GetNavigationSystem();
	if (!NavSys)
	{
		UE_LOG(LogNavigation, Warning, TEXT("ARecastNavMesh::CacheNavigationData - No navigation system"));
		return;
	}

	// Get main navigation data (should be ARecastNavMesh in UE4)
	ANavigationData* MainNavData = NavSys->GetMainNavData(ENavigationDataResolution::Default);
	if (MainNavData && MainNavData != this)
	{
		// Cache as RecastNavMesh if it's the correct type
		::ARecastNavMesh* RecastNav = Cast<::ARecastNavMesh>(MainNavData);
		if (RecastNav)
		{
			CachedRecastNavMesh = RecastNav;
			UE_LOG(LogNavigation, Log, TEXT("ARecastNavMesh::CacheNavigationData - Cached world RecastNavMesh"));
		}
		else
		{
			UE_LOG(LogNavigation, Warning, TEXT("ARecastNavMesh::CacheNavigationData - Main nav data is not RecastNavMesh"));
		}
	}
}

::ARecastNavMesh* ARecastNavMesh::GetRecastNavMeshImpl() const
{
	CacheNavigationData();
	return CachedRecastNavMesh.Get();
}

bool ARecastNavMesh::ProjectPoint(const FVector& Point, FNavLocation& OutLocation, const FVector& Extent) const
{
	// Delegate to world's RecastNavMesh
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (!NavMesh)
	{
		UE_LOG(LogNavigation, VeryVerbose, TEXT("ARecastNavMesh::ProjectPoint - No NavMesh available"));
		OutLocation = FNavLocation(Point);
		return false;
	}

	// Use engine's ProjectPoint
	FNavLocation EngineNavLoc;
	bool bSuccess = NavMesh->ProjectPoint(Point, EngineNavLoc, Extent);

	if (bSuccess)
	{
		OutLocation.Location = EngineNavLoc.Location;
		OutLocation.NodeRef = EngineNavLoc.NodeRef;

		UE_LOG(LogNavigation, VeryVerbose, TEXT("ARecastNavMesh::ProjectPoint - Success: %s -> %s"),
			*Point.ToString(), *OutLocation.Location.ToString());
	}
	else
	{
		OutLocation = FNavLocation(Point);
		UE_LOG(LogNavigation, VeryVerbose, TEXT("ARecastNavMesh::ProjectPoint - Failed for %s"), *Point.ToString());
	}

	return bSuccess;
}

bool ARecastNavMesh::GetRandomPoint(FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (!NavMesh)
	{
		return false;
	}

	FNavLocation EngineNavLoc;
	bool bSuccess = NavMesh->GetRandomPoint(EngineNavLoc, QueryFilter);

	if (bSuccess)
	{
		ResultLocation.Location = EngineNavLoc.Location;
		ResultLocation.NodeRef = EngineNavLoc.NodeRef;
	}

	return bSuccess;
}

bool ARecastNavMesh::GetRandomPointInNavigableRadius(const FVector& Origin, float Radius, FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (!NavMesh)
	{
		UE_LOG(LogNavigation, VeryVerbose, TEXT("ARecastNavMesh::GetRandomPointInNavigableRadius - No NavMesh"));
		return false;
	}

	FNavLocation EngineNavLoc;
	bool bSuccess = NavMesh->GetRandomPointInNavigableRadius(Origin, Radius, EngineNavLoc, QueryFilter);

	if (bSuccess)
	{
		ResultLocation.Location = EngineNavLoc.Location;
		ResultLocation.NodeRef = EngineNavLoc.NodeRef;

		UE_LOG(LogNavigation, VeryVerbose, TEXT("ARecastNavMesh::GetRandomPointInNavigableRadius - Found point at %s"),
			*ResultLocation.Location.ToString());
	}

	return bSuccess;
}

bool ARecastNavMesh::GetRandomReachablePointInRadius(const FVector& Origin, float Radius, FNavLocation& ResultLocation, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (!NavMesh)
	{
		return false;
	}

	FNavLocation EngineNavLoc;
	bool bSuccess = NavMesh->GetRandomReachablePointInRadius(Origin, Radius, EngineNavLoc, QueryFilter);

	if (bSuccess)
	{
		ResultLocation.Location = EngineNavLoc.Location;
		ResultLocation.NodeRef = EngineNavLoc.NodeRef;
	}

	return bSuccess;
}

void ARecastNavMesh::BatchProjectPoints(TArray<FNavigationProjectionWork>& Workload, const FVector& Extent) const
{
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (!NavMesh)
	{
		// Fall back to base implementation
		Super::BatchProjectPoints(Workload, Extent);
		return;
	}

	// Delegate to engine's batch projection
	TArray<FNavigationProjectionWork> EngineWorkload;
	EngineWorkload.Reserve(Workload.Num());

	for (const FNavigationProjectionWork& Work : Workload)
	{
		EngineWorkload.Add(Work);
	}

	NavMesh->BatchProjectPoints(EngineWorkload, Extent);

	// Copy results back
	for (int32 i = 0; i < Workload.Num(); ++i)
	{
		Workload[i].OutLocation.Location = EngineWorkload[i].OutLocation.Location;
		Workload[i].OutLocation.NodeRef = EngineWorkload[i].OutLocation.NodeRef;
		Workload[i].bResult = EngineWorkload[i].bResult;
	}
}

void ARecastNavMesh::BatchRaycast(TArray<FNavigationRaycastWork>& Workload, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (!NavMesh)
	{
		Super::BatchRaycast(Workload, QueryFilter);
		return;
	}

	// Delegate to engine's batch raycast
	TArray<FNavigationRaycastWork> EngineWorkload;
	EngineWorkload.Reserve(Workload.Num());

	for (const FNavigationRaycastWork& Work : Workload)
	{
		EngineWorkload.Add(Work);
	}

	NavMesh->BatchRaycast(EngineWorkload, QueryFilter);

	// Copy results back
	for (int32 i = 0; i < Workload.Num(); ++i)
	{
		Workload[i].HitLocation = EngineWorkload[i].HitLocation;
		Workload[i].bDidHit = EngineWorkload[i].bDidHit;
	}
}

float ARecastNavMesh::CalcPathCost(const FVector& PathStart, const FVector& PathEnd, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (!NavMesh)
	{
		return -1.0f;  // No path
	}

	return NavMesh->CalcPathCost(PathStart, PathEnd, QueryFilter);
}

float ARecastNavMesh::CalcPathLength(const FVector& PathStart, const FVector& PathEnd, TSharedPtr<const FNavigationQueryFilter> QueryFilter) const
{
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (!NavMesh)
	{
		return -1.0f;  // No path
	}

	return NavMesh->CalcPathLength(PathStart, PathEnd, QueryFilter);
}

bool ARecastNavMesh::DoesNodeContainLocation(NavNodeRef NodeRef, const FVector& WorldSpaceLocation) const
{
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (!NavMesh)
	{
		return false;
	}

	return NavMesh->DoesNodeContainLocation(NodeRef, WorldSpaceLocation);
}

FBox ARecastNavMesh::GetBounds() const
{
	::ARecastNavMesh* NavMesh = GetRecastNavMeshImpl();
	if (NavMesh)
	{
		return NavMesh->GetBounds();
	}

	return Super::GetBounds();
}
