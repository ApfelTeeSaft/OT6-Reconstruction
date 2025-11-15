// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Player State - Player session and statistics tracking
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Player\FortPlayerState.cpp

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FortEnums.h"
#include "FortPlayerState.generated.h"

/**
 * Hero power point calculation data
 * Source: AFortPlayerState::GetPowerPointCalculationData function reference
 */
USTRUCT(BlueprintType)
struct FPowerPointCalculationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 BasePower;

	UPROPERTY(BlueprintReadOnly)
	int32 TechnologyBonus;

	UPROPERTY(BlueprintReadOnly)
	int32 FortitudeBonus;

	UPROPERTY(BlueprintReadOnly)
	int32 OffenseBonus;

	UPROPERTY(BlueprintReadOnly)
	int32 ResistanceBonus;

	FPowerPointCalculationData()
		: BasePower(0)
		, TechnologyBonus(0)
		, FortitudeBonus(0)
		, OffenseBonus(0)
		, ResistanceBonus(0)
	{}
};

/**
 * AFortPlayerState - Base player state for Fortnite
 * Functions: GetHeroPowerPoints, GetPowerPointCalculationData, InitializeHero
 */
UCLASS()
class FORTNITEGAME_API AFortPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AFortPlayerState(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface

	/**
	 * Get hero power points for this player
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|PlayerState")
	int32 GetHeroPowerPoints() const;

	/**
	 * Get detailed power point calculation data
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|PlayerState")
	void GetPowerPointCalculationData(FPowerPointCalculationData& OutData) const;

	/**
	 * Initialize hero for this player
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|PlayerState")
	bool InitializeHero(const FString& HeroId);

	/** Team assignment */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Team")
	EFortTeam Team;

	/** Player statistics - replicated to all clients */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	int32 MonsterKills;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	int32 PlayerKills;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	int32 Headshots;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	int32 Assists;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	int32 Deaths;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	int32 BuildingsBuilt;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	int32 BuildingsEdited;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	int32 BuildingsDestroyed;

	/** Hero data */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Hero")
	FString ActiveHeroId;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Hero")
	int32 HeroPowerLevel;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Hero")
	FPowerPointCalculationData PowerCalculationData;

	/** Account information */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Account")
	FString AccountId;

protected:
	/** Internal hero initialization state */
	UPROPERTY()
	bool bHeroInitialized;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortPlayerStateFrontEnd - Frontend/menu player state
 */
UCLASS()
class FORTNITEGAME_API AFortPlayerStateFrontEnd : public AFortPlayerState
{
	GENERATED_BODY()

public:
	AFortPlayerStateFrontEnd(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|FrontEnd")
	bool bIsReady;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|FrontEnd")
	int32 PartyIndex;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortPlayerStateZone - Mission/zone player state
 */
UCLASS()
class FORTNITEGAME_API AFortPlayerStateZone : public AFortPlayerState
{
	GENERATED_BODY()

public:
	AFortPlayerStateZone(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Zone")
	int32 ResourceWood;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Zone")
	int32 ResourceStone;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Zone")
	int32 ResourceMetal;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Zone")
	float MissionContributionScore;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortPlayerStateKeep - Keep mode player state
 */
UCLASS()
class FORTNITEGAME_API AFortPlayerStateKeep : public AFortPlayerStateZone
{
	GENERATED_BODY()

public:
	AFortPlayerStateKeep(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Keep")
	int32 KeepDefensePoints;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortPlayerStateManor - Manor mode player state
 */
UCLASS()
class FORTNITEGAME_API AFortPlayerStateManor : public AFortPlayerStateZone
{
	GENERATED_BODY()

public:
	AFortPlayerStateManor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Manor")
	int32 ManorRepairPoints;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortPlayerStateOutpost - Outpost mode player state
 * Function: SetCanEditOutpost
 */
UCLASS()
class FORTNITEGAME_API AFortPlayerStateOutpost : public AFortPlayerStateZone
{
	GENERATED_BODY()

public:
	AFortPlayerStateOutpost(const FObjectInitializer& ObjectInitializer);

	/**
	 * Set whether a player can edit this outpost
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Outpost")
	void SetCanEditOutpost(AFortPlayerState* RequestingPlayer, bool bCanEdit);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Outpost")
	bool bIsOutpostOwner;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Outpost")
	bool bCanEditOutpost;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Outpost")
	int32 OutpostLevel;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortPlayerStatePvP - PvP player state
 */
UCLASS()
class FORTNITEGAME_API AFortPlayerStatePvP : public AFortPlayerState
{
	GENERATED_BODY()

public:
	AFortPlayerStatePvP(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|PvP")
	int32 PvPKills;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|PvP")
	int32 PvPDeaths;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|PvP")
	float PvPDamageDealt;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|PvP")
	float PvPDamageTaken;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
