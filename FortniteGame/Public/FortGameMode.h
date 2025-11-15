// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Game Mode - Server-side game management
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\FortGameMode.cpp

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FortEnums.h"
#include "FortGameMode.generated.h"

/**
 * AFortGameMode - Base game mode for Fortnite
 */
UCLASS()
class FORTNITEGAME_API AFortGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AFortGameMode(const FObjectInitializer& ObjectInitializer);

	//~ Begin AGameMode Interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void StartPlay() override;
	virtual bool ReadyToStartMatch_Implementation() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	//~ End AGameMode Interface

	/**
	 * Return to main menu on host
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|GameMode")
	virtual void ReturnToMainMenuHost();

	/**
	 * Called when save world record completes for main menu
	 */
	virtual void OnSaveWorldRecordCompleteForMainMenu(int32 SaveResult);

	/**
	 * Perform logout analytics for player
	 */
	virtual void DoLogoutAnalytics(APlayerController* Player, const FString& AccountID);

	/**
	 * Handle score to XP conversion updates
	 */
	virtual void HandleScoreToXPUpdate();

	/**
	 * Say command implementation for game mode
	 */
	UFUNCTION(Exec)
	virtual void Say(const FString& Message);

	/**
	 * Team say command implementation
	 */
	UFUNCTION(Exec)
	virtual void TeamSay(const FString& Message);

protected:
	/** Match state tracking */
	UPROPERTY(Replicated)
	bool bMatchHasStarted;

	UPROPERTY(Replicated)
	float MatchStartTime;

	UPROPERTY(Replicated)
	float MatchDuration;

	/** Player tracking */
	UPROPERTY()
	TArray<APlayerController*> ConnectedPlayers;

	/** Analytics data */
	UPROPERTY()
	TMap<FString, float> PlayerSessionData;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortGameModeEmptyDedicated - Empty dedicated server mode
 * Used for dedicated servers without active gameplay
 */
UCLASS()
class FORTNITEGAME_API AFortGameModeEmptyDedicated : public AFortGameMode
{
	GENERATED_BODY()

public:
	AFortGameModeEmptyDedicated(const FObjectInitializer& ObjectInitializer);

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
};

/**
 * AFortGameModeFrontEnd - Frontend/menu game mode
 * Used for main menu and lobby
 */
UCLASS()
class FORTNITEGAME_API AFortGameModeFrontEnd : public AFortGameMode
{
	GENERATED_BODY()

public:
	AFortGameModeFrontEnd(const FObjectInitializer& ObjectInitializer);

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
