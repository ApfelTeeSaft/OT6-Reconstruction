// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Game State - Replicated game state information
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\FortGameState.cpp

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "FortEnums.h"
#include "FortGameState.generated.h"

/**
 * Zone difficulty information structure
 * Source: AFortGameStateZone functions reference difficulty tracking
 */
USTRUCT(BlueprintType)
struct FZoneDifficultyInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 DifficultyLevel;

	UPROPERTY(BlueprintReadOnly)
	float DifficultyMultiplier;

	UPROPERTY(BlueprintReadOnly)
	int32 RecommendedPlayerLevel;

	FZoneDifficultyInfo()
		: DifficultyLevel(1)
		, DifficultyMultiplier(1.0f)
		, RecommendedPlayerLevel(1)
	{}
};

/**
 * AFortGameState - Base game state for Fortnite
 */
UCLASS()
class FORTNITEGAME_API AFortGameState : public AGameState
{
	GENERATED_BODY()

public:
	AFortGameState(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface

	/** Match state */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|GameState")
	EFortCompletionResult MatchResult;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|GameState")
	float MatchTimeRemaining;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|GameState")
	bool bMatchInProgress;

	/** Team information */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|GameState")
	TArray<int32> TeamScores;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|GameState")
	TArray<int32> TeamPlayerCounts;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortGameStateFrontEnd - Frontend/menu game state
 */
UCLASS()
class FORTNITEGAME_API AFortGameStateFrontEnd : public AFortGameState
{
	GENERATED_BODY()

public:
	AFortGameStateFrontEnd(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|GameState")
	int32 NumPlayersInLobby;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortGameStateZone - Main mission/zone game state
 * Functions: GetZoneDifficultyInfo, SetZoneDifficultyInfoAndUpdateGameDifficulty, GetGameDifficultyTheaterZoneCalculationValues
 */
UCLASS()
class FORTNITEGAME_API AFortGameStateZone : public AFortGameState
{
	GENERATED_BODY()

public:
	AFortGameStateZone(const FObjectInitializer& ObjectInitializer);

	/**
	 * Get current zone difficulty information
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|GameState")
	const FZoneDifficultyInfo& GetZoneDifficultyInfo() const;

	/**
	 * Set zone difficulty and update game difficulty
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|GameState")
	void SetZoneDifficultyInfoAndUpdateGameDifficulty(const FZoneDifficultyInfo& NewDifficulty);

	/**
	 * Get difficulty calculation values for theater zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|GameState")
	void GetGameDifficultyTheaterZoneCalculationValues(int32& OutTheaterLevel, int32& OutZoneLevel) const;

protected:
	/** Zone difficulty tracking */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Zone")
	FZoneDifficultyInfo ZoneDifficultyInfo;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Zone")
	int32 TheaterLevel;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Zone")
	int32 ZoneLevel;

	/** Mission objectives */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Mission")
	TArray<AActor*> ActiveObjectives;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Mission")
	EFortObjectiveStatus PrimaryObjectiveStatus;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortGameStateKeep - Keep mode game state
 */
UCLASS()
class FORTNITEGAME_API AFortGameStateKeep : public AFortGameStateZone
{
	GENERATED_BODY()

public:
	AFortGameStateKeep(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Keep")
	float KeepHealthPercentage;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortGameStateManor - Manor mode game state
 */
UCLASS()
class FORTNITEGAME_API AFortGameStateManor : public AFortGameStateZone
{
	GENERATED_BODY()

public:
	AFortGameStateManor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Manor")
	int32 ManorDefenseWave;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortGameStateOutpost - Outpost mode game state
 */
UCLASS()
class FORTNITEGAME_API AFortGameStateOutpost : public AFortGameStateZone
{
	GENERATED_BODY()

public:
	AFortGameStateOutpost(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Outpost")
	FString OutpostOwnerID;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Outpost")
	TArray<FString> AllowedEditorIDs;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortGameStatePvP - PvP base game state
 */
UCLASS()
class FORTNITEGAME_API AFortGameStatePvP : public AFortGameState
{
	GENERATED_BODY()

public:
	AFortGameStatePvP(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|PvP")
	EFortPvPGameResult PvPMatchResult;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|PvP")
	int32 WinningTeam;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|PvP")
	TArray<int32> TeamKills;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|PvP")
	TArray<int32> TeamDeaths;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortGameStatePvPBaseDestruction - PvP base destruction mode
 */
UCLASS()
class FORTNITEGAME_API AFortGameStatePvPBaseDestruction : public AFortGameStatePvP
{
	GENERATED_BODY()

public:
	AFortGameStatePvPBaseDestruction(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|PvP")
	TArray<float> TeamBaseHealthPercentages;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
