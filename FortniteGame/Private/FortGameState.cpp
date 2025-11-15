// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Game State Implementation

#include "FortGameState.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// AFortGameState

AFortGameState::AFortGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MatchResult(EFortCompletionResult::Undefined)
	, MatchTimeRemaining(0.0f)
	, bMatchInProgress(false)
{
	bReplicates = true;
	bAlwaysRelevant = true;

	// Initialize team arrays
	TeamScores.SetNum(static_cast<int32>(EFortTeam::MAX));
	TeamPlayerCounts.SetNum(static_cast<int32>(EFortTeam::MAX));

	for (int32 i = 0; i < TeamScores.Num(); ++i)
	{
		TeamScores[i] = 0;
		TeamPlayerCounts[i] = 0;
	}
}

void AFortGameState::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogNet, Log, TEXT("FortGameState: BeginPlay"));
}

void AFortGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMatchInProgress && Role == ROLE_Authority)
	{
		MatchTimeRemaining -= DeltaTime;
		if (MatchTimeRemaining <= 0.0f)
		{
			MatchTimeRemaining = 0.0f;
			// Match time expired
		}
	}
}

void AFortGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortGameState, MatchResult);
	DOREPLIFETIME(AFortGameState, MatchTimeRemaining);
	DOREPLIFETIME(AFortGameState, bMatchInProgress);
	DOREPLIFETIME(AFortGameState, TeamScores);
	DOREPLIFETIME(AFortGameState, TeamPlayerCounts);
}

//////////////////////////////////////////////////////////////////////////
// AFortGameStateFrontEnd

AFortGameStateFrontEnd::AFortGameStateFrontEnd(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, NumPlayersInLobby(0)
{
	UE_LOG(LogNet, Log, TEXT("AFortGameStateFrontEnd: Initialized"));
}

void AFortGameStateFrontEnd::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortGameStateFrontEnd, NumPlayersInLobby);
}

//////////////////////////////////////////////////////////////////////////
// AFortGameStateZone

AFortGameStateZone::AFortGameStateZone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TheaterLevel(1)
	, ZoneLevel(1)
	, PrimaryObjectiveStatus(EFortObjectiveStatus::Created)
{
	UE_LOG(LogNet, Log, TEXT("AFortGameStateZone: Initialized"));

	// Initialize default difficulty
	ZoneDifficultyInfo.DifficultyLevel = 1;
	ZoneDifficultyInfo.DifficultyMultiplier = 1.0f;
	ZoneDifficultyInfo.RecommendedPlayerLevel = 1;
}

const FZoneDifficultyInfo& AFortGameStateZone::GetZoneDifficultyInfo() const
{
	return ZoneDifficultyInfo;
}

void AFortGameStateZone::SetZoneDifficultyInfoAndUpdateGameDifficulty(const FZoneDifficultyInfo& NewDifficulty)
{
	if (Role != ROLE_Authority)
	{
		return;
	}

	ZoneDifficultyInfo = NewDifficulty;

	UE_LOG(LogNet, Log, TEXT("AFortGameStateZone: SetZoneDifficultyInfoAndUpdateGameDifficulty - Level: %d, Multiplier: %.2f"),
		NewDifficulty.DifficultyLevel, NewDifficulty.DifficultyMultiplier);

	// Update game difficulty based on zone difficulty
	// This would trigger AI scaling, loot adjustments, etc.
}

void AFortGameStateZone::GetGameDifficultyTheaterZoneCalculationValues(int32& OutTheaterLevel, int32& OutZoneLevel) const
{
	OutTheaterLevel = TheaterLevel;
	OutZoneLevel = ZoneLevel;
}

void AFortGameStateZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortGameStateZone, ZoneDifficultyInfo);
	DOREPLIFETIME(AFortGameStateZone, TheaterLevel);
	DOREPLIFETIME(AFortGameStateZone, ZoneLevel);
	DOREPLIFETIME(AFortGameStateZone, ActiveObjectives);
	DOREPLIFETIME(AFortGameStateZone, PrimaryObjectiveStatus);
}

//////////////////////////////////////////////////////////////////////////
// AFortGameStateKeep

AFortGameStateKeep::AFortGameStateKeep(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, KeepHealthPercentage(100.0f)
{
	UE_LOG(LogNet, Log, TEXT("AFortGameStateKeep: Initialized"));
}

void AFortGameStateKeep::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortGameStateKeep, KeepHealthPercentage);
}

//////////////////////////////////////////////////////////////////////////
// AFortGameStateManor

AFortGameStateManor::AFortGameStateManor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ManorDefenseWave(0)
{
	UE_LOG(LogNet, Log, TEXT("AFortGameStateManor: Initialized"));
}

void AFortGameStateManor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortGameStateManor, ManorDefenseWave);
}

//////////////////////////////////////////////////////////////////////////
// AFortGameStateOutpost

AFortGameStateOutpost::AFortGameStateOutpost(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UE_LOG(LogNet, Log, TEXT("AFortGameStateOutpost: Initialized"));
}

void AFortGameStateOutpost::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortGameStateOutpost, OutpostOwnerID);
	DOREPLIFETIME(AFortGameStateOutpost, AllowedEditorIDs);
}

//////////////////////////////////////////////////////////////////////////
// AFortGameStatePvP

AFortGameStatePvP::AFortGameStatePvP(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PvPMatchResult(EFortPvPGameResult::Draw)
	, WinningTeam(-1)
{
	UE_LOG(LogNet, Log, TEXT("AFortGameStatePvP: Initialized"));

	// Initialize PvP team arrays
	TeamKills.SetNum(static_cast<int32>(EFortTeam::HumanPvP_Team10) + 1);
	TeamDeaths.SetNum(static_cast<int32>(EFortTeam::HumanPvP_Team10) + 1);

	for (int32 i = 0; i < TeamKills.Num(); ++i)
	{
		TeamKills[i] = 0;
		TeamDeaths[i] = 0;
	}
}

void AFortGameStatePvP::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortGameStatePvP, PvPMatchResult);
	DOREPLIFETIME(AFortGameStatePvP, WinningTeam);
	DOREPLIFETIME(AFortGameStatePvP, TeamKills);
	DOREPLIFETIME(AFortGameStatePvP, TeamDeaths);
}

//////////////////////////////////////////////////////////////////////////
// AFortGameStatePvPBaseDestruction

AFortGameStatePvPBaseDestruction::AFortGameStatePvPBaseDestruction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UE_LOG(LogNet, Log, TEXT("AFortGameStatePvPBaseDestruction: Initialized"));

	// Initialize base health percentages
	TeamBaseHealthPercentages.SetNum(static_cast<int32>(EFortTeam::HumanPvP_Team10) + 1);
	for (int32 i = 0; i < TeamBaseHealthPercentages.Num(); ++i)
	{
		TeamBaseHealthPercentages[i] = 100.0f;
	}
}

void AFortGameStatePvPBaseDestruction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortGameStatePvPBaseDestruction, TeamBaseHealthPercentages);
}
