// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Player State Implementation

#include "FortPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "FortPlayerController.h"

//////////////////////////////////////////////////////////////////////////
// AFortPlayerState

AFortPlayerState::AFortPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Team(EFortTeam::HumanCampaign)
	, MonsterKills(0)
	, PlayerKills(0)
	, Headshots(0)
	, Assists(0)
	, Deaths(0)
	, BuildingsBuilt(0)
	, BuildingsEdited(0)
	, BuildingsDestroyed(0)
	, HeroPowerLevel(0)
	, bHeroInitialized(false)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AFortPlayerState::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogNet, Log, TEXT("FortPlayerState: BeginPlay for %s"), *GetPlayerName());
}

void AFortPlayerState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int32 AFortPlayerState::GetHeroPowerPoints() const
{
	int32 TotalPower = PowerCalculationData.BasePower;
	TotalPower += PowerCalculationData.TechnologyBonus;
	TotalPower += PowerCalculationData.FortitudeBonus;
	TotalPower += PowerCalculationData.OffenseBonus;
	TotalPower += PowerCalculationData.ResistanceBonus;

	return TotalPower;
}

void AFortPlayerState::GetPowerPointCalculationData(FPowerPointCalculationData& OutData) const
{
	OutData = PowerCalculationData;
}

bool AFortPlayerState::InitializeHero(const FString& HeroId)
{

	if (Role != ROLE_Authority)
	{
		return false;
	}

	AFortPlayerController* FortPC = Cast<AFortPlayerController>(GetOwner());
	FString PCName = FortPC ? FortPC->GetName() : TEXT("NULL");
	FString PSName = GetName();

	if (HeroId.IsEmpty())
	{
		UE_LOG(LogNet, Error, TEXT("AFortPlayerState::InitializeHero failed. FortPC: %s, FortPC->PlayerState: %s, HeroId: %s"),
			*PCName, *PSName, *HeroId);
		return false;
	}

	UE_LOG(LogNet, Log, TEXT("AFortPlayerState::InitializeHero. FortPC: %s, FortPC->PlayerState: %s, HeroId: %s"),
		*PCName, *PSName, *HeroId);

	// Initialize hero
	ActiveHeroId = HeroId;
	bHeroInitialized = true;

	// Set initial power level
	HeroPowerLevel = 1;
	PowerCalculationData.BasePower = 10;
	PowerCalculationData.TechnologyBonus = 0;
	PowerCalculationData.FortitudeBonus = 0;
	PowerCalculationData.OffenseBonus = 0;
	PowerCalculationData.ResistanceBonus = 0;

	return true;
}

void AFortPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortPlayerState, Team);
	DOREPLIFETIME(AFortPlayerState, MonsterKills);
	DOREPLIFETIME(AFortPlayerState, PlayerKills);
	DOREPLIFETIME(AFortPlayerState, Headshots);
	DOREPLIFETIME(AFortPlayerState, Assists);
	DOREPLIFETIME(AFortPlayerState, Deaths);
	DOREPLIFETIME(AFortPlayerState, BuildingsBuilt);
	DOREPLIFETIME(AFortPlayerState, BuildingsEdited);
	DOREPLIFETIME(AFortPlayerState, BuildingsDestroyed);
	DOREPLIFETIME(AFortPlayerState, ActiveHeroId);
	DOREPLIFETIME(AFortPlayerState, HeroPowerLevel);
	DOREPLIFETIME(AFortPlayerState, PowerCalculationData);
	DOREPLIFETIME(AFortPlayerState, AccountId);
}

//////////////////////////////////////////////////////////////////////////
// AFortPlayerStateFrontEnd

AFortPlayerStateFrontEnd::AFortPlayerStateFrontEnd(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsReady(false)
	, PartyIndex(-1)
{
	UE_LOG(LogNet, Log, TEXT("AFortPlayerStateFrontEnd: Initialized"));
}

void AFortPlayerStateFrontEnd::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortPlayerStateFrontEnd, bIsReady);
	DOREPLIFETIME(AFortPlayerStateFrontEnd, PartyIndex);
}

//////////////////////////////////////////////////////////////////////////
// AFortPlayerStateZone

AFortPlayerStateZone::AFortPlayerStateZone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ResourceWood(0)
	, ResourceStone(0)
	, ResourceMetal(0)
	, MissionContributionScore(0.0f)
{
	UE_LOG(LogNet, Log, TEXT("AFortPlayerStateZone: Initialized"));
}

void AFortPlayerStateZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortPlayerStateZone, ResourceWood);
	DOREPLIFETIME(AFortPlayerStateZone, ResourceStone);
	DOREPLIFETIME(AFortPlayerStateZone, ResourceMetal);
	DOREPLIFETIME(AFortPlayerStateZone, MissionContributionScore);
}

//////////////////////////////////////////////////////////////////////////
// AFortPlayerStateKeep

AFortPlayerStateKeep::AFortPlayerStateKeep(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, KeepDefensePoints(0)
{
	UE_LOG(LogNet, Log, TEXT("AFortPlayerStateKeep: Initialized"));
}

void AFortPlayerStateKeep::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortPlayerStateKeep, KeepDefensePoints);
}

//////////////////////////////////////////////////////////////////////////
// AFortPlayerStateManor

AFortPlayerStateManor::AFortPlayerStateManor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ManorRepairPoints(0)
{
	UE_LOG(LogNet, Log, TEXT("AFortPlayerStateManor: Initialized"));
}

void AFortPlayerStateManor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortPlayerStateManor, ManorRepairPoints);
}

//////////////////////////////////////////////////////////////////////////
// AFortPlayerStateOutpost

AFortPlayerStateOutpost::AFortPlayerStateOutpost(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsOutpostOwner(false)
	, bCanEditOutpost(false)
	, OutpostLevel(1)
{
	UE_LOG(LogNet, Log, TEXT("AFortPlayerStateOutpost: Initialized"));
}

void AFortPlayerStateOutpost::SetCanEditOutpost(AFortPlayerState* RequestingPlayer, bool bCanEdit)
{

	if (!RequestingPlayer)
	{
		UE_LOG(LogNet, Error, TEXT("AFortPlayerStateOutpost::SetCanEditOutpost: no player specified"));
		return;
	}

	// Only the outpost owner can grant edit permissions
	if (!bIsOutpostOwner)
	{
		UE_LOG(LogNet, Error, TEXT("AFortPlayerStateOutpost::SetCanEditOutpost: requesting player is not outpost owner"));
		return;
	}

	// Update the requesting player's edit permissions
	AFortPlayerStateOutpost* OutpostPS = Cast<AFortPlayerStateOutpost>(RequestingPlayer);
	if (OutpostPS)
	{
		OutpostPS->bCanEditOutpost = bCanEdit;
		UE_LOG(LogNet, Log, TEXT("AFortPlayerStateOutpost: SetCanEditOutpost - Player: %s, CanEdit: %d"),
			*RequestingPlayer->GetPlayerName(), bCanEdit);
	}
}

void AFortPlayerStateOutpost::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortPlayerStateOutpost, bIsOutpostOwner);
	DOREPLIFETIME(AFortPlayerStateOutpost, bCanEditOutpost);
	DOREPLIFETIME(AFortPlayerStateOutpost, OutpostLevel);
}

//////////////////////////////////////////////////////////////////////////
// AFortPlayerStatePvP

AFortPlayerStatePvP::AFortPlayerStatePvP(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PvPKills(0)
	, PvPDeaths(0)
	, PvPDamageDealt(0.0f)
	, PvPDamageTaken(0.0f)
{
	UE_LOG(LogNet, Log, TEXT("AFortPlayerStatePvP: Initialized"));
}

void AFortPlayerStatePvP::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortPlayerStatePvP, PvPKills);
	DOREPLIFETIME(AFortPlayerStatePvP, PvPDeaths);
	DOREPLIFETIME(AFortPlayerStatePvP, PvPDamageDealt);
	DOREPLIFETIME(AFortPlayerStatePvP, PvPDamageTaken);
}
