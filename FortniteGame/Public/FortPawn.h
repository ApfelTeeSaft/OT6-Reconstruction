// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite-specific Pawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FortPawn.generated.h"

/**
 * Fortnite Pawn (Character)
 */
UCLASS(config=Game)
class FORTNITEGAME_API AFortPawn : public ACharacter
{
	GENERATED_BODY()

public:
	AFortPawn(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

	/**
	 * Get pawn stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Stats")
	void GetPawnStats(float& OutHealth, float& OutShield, float& OutMaxHealth, float& OutMaxShield);

	/**
	 * Get health percentage
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Stats")
	float GetHealthPercent() const;

	/**
	 * Take damage
	 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/**
	 * Handle death
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Health")
	virtual void Die(AController* Killer);

	/**
	 * Replicated health changed callback
	 */
	UFUNCTION()
	void OnRep_Health();

	/**
	 * Replicated shield changed callback
	 */
	UFUNCTION()
	void OnRep_Shield();

protected:
	/** Current health */
	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category = "Fort|Stats")
	float Health;

	/** Maximum health */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	float MaxHealth;

	/** Current shield */
	UPROPERTY(ReplicatedUsing=OnRep_Shield, BlueprintReadOnly, Category = "Fort|Stats")
	float Shield;

	/** Maximum shield */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Stats")
	float MaxShield;

	/** Is pawn dead */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|State")
	bool bIsDead;
};
