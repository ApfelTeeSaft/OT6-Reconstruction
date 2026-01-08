// Copyright Epic Games, Inc. All Rights Reserved.
// Reconstructed from Fortnite UE 4.12 IDA
// Navigation query filters for AI pathfinding

#pragma once

#include "CoreMinimal.h"
#include "AI/Navigation/NavQueryFilter.h"
#include "FortNavigationFilter.generated.h"

/**
 * Recast navigation area types
 * Defines different surface types for pathfinding cost calculation
 */
namespace EFortNavArea
{
	enum Type
	{
		/** Default walkable area */
		Default = 0,

		/** Destructible objects (walls, buildings) */
		Destructible = 1,

		/** Dangerous terrain (storm, lava, etc.) */
		Dangerous = 2,

		/** Water areas */
		Water = 3,

		/** Jump/fall required */
		Jump = 4,

		/** Maximum number of areas supported by Recast */
		MaxAreas = 64
	};
}

/**
 * Recast query filter implementation
 * Wraps Detour's dtQueryFilter for UE4
 *
 * Evidence: FRecastQueryFilter error message
 */
struct FORTNITEGAME_API FRecastQueryFilter
{
	/** Maximum areas supported by Recast/Detour */
	static const int32 MaxAreas = 64;

	/** Traversal cost for each area type */
	float AreaCosts[MaxAreas];

	/** Include flags for polygon filtering */
	uint16 IncludeFlags;

	/** Exclude flags for polygon filtering */
	uint16 ExcludeFlags;

	FRecastQueryFilter()
		: IncludeFlags(0xFFFF)
		, ExcludeFlags(0)
	{
		// Initialize all area costs to 1.0 (neutral)
		for (int32 i = 0; i < MaxAreas; ++i)
		{
			AreaCosts[i] = 1.0f;
		}
	}

	/**
	 * Set traversal cost for specific area type
	 * Higher cost = less preferred path
	 *
	 * Evidence: Error message shows validation of area index
	 */
	void SetAreaCost(int32 AreaID, float Cost)
	{
		if (AreaID >= 0 && AreaID < MaxAreas)
		{
			AreaCosts[AreaID] = Cost;
		}
		else
		{
			UE_LOG(LogNavigation, Error,
				TEXT("FRecastQueryFilter: Trying to set cost to more areas than allowed! Discarding redundant values. AreaID=%d"),
				AreaID);
		}
	}

	/** Get cost for area type */
	float GetAreaCost(int32 AreaID) const
	{
		return (AreaID >= 0 && AreaID < MaxAreas) ? AreaCosts[AreaID] : 1.0f;
	}

	/** Set include flags */
	void SetIncludeFlags(uint16 Flags)
	{
		IncludeFlags = Flags;
	}

	/** Set exclude flags */
	void SetExcludeFlags(uint16 Flags)
	{
		ExcludeFlags = Flags;
	}
};

/**
 * Base navigation query filter for Fortnite AI
 * Evidence: UFortNavigationFilter string reference
 */
UCLASS(Abstract)
class FORTNITEGAME_API UFortNavigationFilter : public UNavQueryFilter
{
	GENERATED_BODY()

public:
	UFortNavigationFilter(const FObjectInitializer& ObjectInitializer);

	/** Recast filter implementation */
	FRecastQueryFilter RecastFilter;

	/** Apply Fortnite-specific filtering rules */
	virtual void InitializeFilter(const ANavigationData& NavData, const UObject* Querier) const;
};

/**
 * Navigation filter for hunting AI behavior
 * Allows AI to path through destructible objects to reach targets
 *
 * Evidence: UFortNavigationFilter_Hunting
 */
UCLASS()
class FORTNITEGAME_API UFortNavigationFilter_Hunting : public UFortNavigationFilter
{
	GENERATED_BODY()

public:
	UFortNavigationFilter_Hunting(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFilter(const ANavigationData& NavData, const UObject* Querier) const override;
};

/**
 * Navigation filter for non-destructive AI behavior
 * Prevents AI from pathing through destructible objects
 *
 * Evidence: UFortNavigationFilter_NoSmashing
 */
UCLASS()
class FORTNITEGAME_API UFortNavigationFilter_NoSmashing : public UFortNavigationFilter
{
	GENERATED_BODY()

public:
	UFortNavigationFilter_NoSmashing(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFilter(const ANavigationData& NavData, const UObject* Querier) const override;
};

/**
 * Fort nav agent cost data
 * Stores area traversal costs per agent type
 *
 * Evidence: UFortNavAgentCostData
 */
UCLASS(BlueprintType)
class FORTNITEGAME_API UFortNavAgentCostData : public UObject
{
	GENERATED_BODY()

public:
	UFortNavAgentCostData(const FObjectInitializer& ObjectInitializer);

	/** Cost multipliers for each navigation area */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	TMap<FName, float> AreaCosts;

	/** Whether this cost data is disabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	bool bDisableNavAgentCostData;

	/** Get cost for specific area name */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	float GetAreaCost(FName AreaName) const;

	/** Set cost for specific area name */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void SetAreaCost(FName AreaName, float Cost);
};
