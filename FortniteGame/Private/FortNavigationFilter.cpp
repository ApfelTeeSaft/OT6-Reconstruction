// Copyright Epic Games, Inc. All Rights Reserved.
// Reconstructed from Fortnite UE 4.12 IDA
// Navigation query filters implementation

#include "FortNavigationFilter.h"
#include "AI/Navigation/NavigationData.h"

//==============================================================================
// UFortNavigationFilter
//==============================================================================

UFortNavigationFilter::UFortNavigationFilter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Initialize default costs
	RecastFilter = FRecastQueryFilter();
}

void UFortNavigationFilter::InitializeFilter(const ANavigationData& NavData, const UObject* Querier) const
{
	// Base initialization
	// Derived classes override to set specific costs
}

//==============================================================================
// UFortNavigationFilter_Hunting
//==============================================================================

UFortNavigationFilter_Hunting::UFortNavigationFilter_Hunting(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Hunting AI can path through destructibles but at higher cost
	RecastFilter.SetAreaCost(EFortNavArea::Default, 1.0f);
	RecastFilter.SetAreaCost(EFortNavArea::Destructible, 5.0f);  // Prefer not to destroy, but will if needed
	RecastFilter.SetAreaCost(EFortNavArea::Dangerous, 10.0f);    // Avoid dangerous areas
	RecastFilter.SetAreaCost(EFortNavArea::Water, 2.0f);         // Can path through water
	RecastFilter.SetAreaCost(EFortNavArea::Jump, 3.0f);          // Can jump

	UE_LOG(LogNavigation, Log, TEXT("UFortNavigationFilter_Hunting initialized - allows destructible pathing"));
}

void UFortNavigationFilter_Hunting::InitializeFilter(const ANavigationData& NavData, const UObject* Querier) const
{
	Super::InitializeFilter(NavData, Querier);

	// Additional runtime initialization if needed
	// Could query Querier to adjust costs based on AI state idk
}

//==============================================================================
// UFortNavigationFilter_NoSmashing
//==============================================================================

UFortNavigationFilter_NoSmashing::UFortNavigationFilter_NoSmashing(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Non-destructive AI cannot path through destructibles
	RecastFilter.SetAreaCost(EFortNavArea::Default, 1.0f);
	RecastFilter.SetAreaCost(EFortNavArea::Destructible, TNumericLimits<float>::Max());  // Infinite cost = blocked
	RecastFilter.SetAreaCost(EFortNavArea::Dangerous, TNumericLimits<float>::Max());     // Cannot enter dangerous areas
	RecastFilter.SetAreaCost(EFortNavArea::Water, 2.0f);         // Can path through water
	RecastFilter.SetAreaCost(EFortNavArea::Jump, 3.0f);          // Can jump

	UE_LOG(LogNavigation, Log, TEXT("UFortNavigationFilter_NoSmashing initialized - blocks destructible pathing"));
}

void UFortNavigationFilter_NoSmashing::InitializeFilter(const ANavigationData& NavData, const UObject* Querier) const
{
	Super::InitializeFilter(NavData, Querier);

	// Additional runtime initialization if needed
}

//==============================================================================
// UFortNavAgentCostData
//==============================================================================

UFortNavAgentCostData::UFortNavAgentCostData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bDisableNavAgentCostData(false)
{
	// Initialize with default area costs
	AreaCosts.Add(TEXT("Default"), 1.0f);
	AreaCosts.Add(TEXT("Destructible"), 5.0f);
	AreaCosts.Add(TEXT("Dangerous"), 10.0f);
	AreaCosts.Add(TEXT("Water"), 2.0f); // <- biggest hater right here
	AreaCosts.Add(TEXT("Jump"), 3.0f);
}

float UFortNavAgentCostData::GetAreaCost(FName AreaName) const
{
	if (bDisableNavAgentCostData)
	{
		return 1.0f;  // Neutral cost when disabled
	}

	const float* Cost = AreaCosts.Find(AreaName);
	return Cost ? *Cost : 1.0f;
}

void UFortNavAgentCostData::SetAreaCost(FName AreaName, float Cost)
{
	AreaCosts.Add(AreaName, Cost);
}
