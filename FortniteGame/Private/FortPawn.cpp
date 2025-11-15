// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Pawn Implementation

#include "FortPawn.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"

AFortPawn::AFortPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Health(100.0f)
	, MaxHealth(100.0f)
	, Shield(0.0f)
	, MaxShield(100.0f)
	, bIsDead(false)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AFortPawn::BeginPlay()
{
	Super::BeginPlay();

	// Initialize health to max
	if (Role == ROLE_Authority)
	{
		Health = MaxHealth;
	}

	UE_LOG(LogNet, Log, TEXT("FortPawn: BeginPlay for %s (Health: %.0f/%.0f, Shield: %.0f/%.0f)"),
		*GetName(), Health, MaxHealth, Shield, MaxShield);
}

void AFortPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFortPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate health and shield
	DOREPLIFETIME(AFortPawn, Health);
	DOREPLIFETIME(AFortPawn, MaxHealth);
	DOREPLIFETIME(AFortPawn, Shield);
	DOREPLIFETIME(AFortPawn, MaxShield);
	DOREPLIFETIME(AFortPawn, bIsDead);
}

void AFortPawn::GetPawnStats(float& OutHealth, float& OutShield, float& OutMaxHealth, float& OutMaxShield)
{

	OutHealth = Health;
	OutShield = Shield;
	OutMaxHealth = MaxHealth;
	OutMaxShield = MaxShield;
}

float AFortPawn::GetHealthPercent() const
{

	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return Health / MaxHealth;
}

float AFortPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Only process damage on server
	if (Role != ROLE_Authority)
	{
		return 0.0f;
	}

	if (bIsDead || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	float ActualDamage = DamageAmount;

	// Shield absorbs damage first
	if (Shield > 0.0f)
	{
		float ShieldDamage = FMath::Min(Shield, ActualDamage);
		Shield -= ShieldDamage;
		ActualDamage -= ShieldDamage;

		UE_LOG(LogNet, Log, TEXT("FortPawn: %s took %.0f shield damage (Shield: %.0f/%.0f)"),
			*GetName(), ShieldDamage, Shield, MaxShield);
	}

	// Remaining damage goes to health
	if (ActualDamage > 0.0f)
	{
		Health -= ActualDamage;

		UE_LOG(LogNet, Log, TEXT("FortPawn: %s took %.0f health damage (Health: %.0f/%.0f)"),
			*GetName(), ActualDamage, Health, MaxHealth);
	}

	// Check for death
	if (Health <= 0.0f)
	{
		Health = 0.0f;
		Die(EventInstigator);
	}

	return DamageAmount;
}

void AFortPawn::Die(AController* Killer)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	UE_LOG(LogNet, Log, TEXT("FortPawn: %s died"), *GetName());

	// Ragdoll physics
	if (GetMesh())
	{
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Notify controller
	if (Controller)
	{
		Controller->UnPossess();
	}
}

void AFortPawn::OnRep_Health()
{

	UE_LOG(LogNet, Verbose, TEXT("FortPawn: OnRep_Health - Health is now %.0f/%.0f"),
		Health, MaxHealth);

	// Update UI, play effects, etc.
}

void AFortPawn::OnRep_Shield()
{

	UE_LOG(LogNet, Verbose, TEXT("FortPawn: OnRep_Shield - Shield is now %.0f/%.0f"),
		Shield, MaxShield);

	// Update UI, play effects, etc.
}
