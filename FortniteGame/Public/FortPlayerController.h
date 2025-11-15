// Copyright Epic Games, Inc. All Rights Reserved.
// Reconstructed from Fortnite UE 4.12 IDA dumps
// Fortnite-specific PlayerController

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FortPlayerController.generated.h"

/**
 * Fortnite Player Controller
 */
UCLASS(config=Game)
class FORTNITEGAME_API AFortPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFortPlayerController(const FObjectInitializer& ObjectInitializer);

	//~ Begin APlayerController Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~ End APlayerController Interface

	/**
	 * Get player stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Player")
	void GetPlayerStats(float& Health, float& Shield, float& MaxHealth, float& MaxShield);

	/**
	 * Server RPC: Perform gameplay ability
	 */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Fort|Abilities")
	void ServerActivateAbility(int32 AbilityIndex);

	/**
	 * Client RPC: Notify ability activation
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Fort|Abilities")
	void ClientNotifyAbilityActivated(int32 AbilityIndex);

	/**
	 * Handle player death
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Player")
	void HandlePlayerDeath();

	/**
	 * Respawn player
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Player")
	void RequestRespawn();

protected:
	/** Current ability being used */
	UPROPERTY(Replicated)
	int32 CurrentAbilityIndex;

	/** Player unique ID */
	UPROPERTY(Replicated)
	FString PawnUniqueID;

	/** Is player in combat */
	UPROPERTY(Replicated)
	bool bInCombat;
};
