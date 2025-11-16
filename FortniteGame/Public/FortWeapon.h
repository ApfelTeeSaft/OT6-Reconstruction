// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Weapon Base Classes
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Weapons\FortWeapon.cpp

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FortEnums.h"
#include "FortWeaponData.h"
#include "FortWeapon.generated.h"

class UFortWeaponItemDefinition;
class USkeletalMeshComponent;

/**
 * EFortWeaponTriggerType - Weapon firing trigger modes
 */
UENUM(BlueprintType)
enum class EFortWeaponTriggerType : uint8
{
	OnPress = 0,              // Fire on button press
	Automatic = 1,            // Continuous automatic fire
	OnRelease = 2,            // Fire on button release
	OnPressAndRelease = 3,    // Fire on both press and release
	EFortWeaponTriggerType_MAX = 4
};

/**
 * EFortWeaponCoreAnimation - Weapon animation types
 */
UENUM(BlueprintType)
enum class EFortWeaponCoreAnimation : uint8
{
	Melee = 0,
	Pistol = 1,
	Shotgun = 2,
	PaperBlueprint = 3,
	Rifle = 4,
	MeleeOneHand = 5,
	MachinePistol = 6,
	RocketLauncher = 7,
	GrenadeLauncher = 8,
	GoingCommando = 9,
	AssaultRifle = 10,
	TacticalShotgun = 11,
	SniperRifle = 12,
	TrapPlacement = 13,
	ShoulderLauncher = 14,
	AbilityDecoTool = 15,
	MAX = 16,
	EFortWeaponCoreAnimation_MAX = 17
};

// Note: Weapon stats structures are now defined in FortWeaponData.h
// FFortWeaponStats and FFortReplicatedWeaponData contain comprehensive weapon properties

/**
 * AFortWeapon - Base weapon class
 */
UCLASS(Abstract)
class FORTNITEGAME_API AFortWeapon : public AActor
{
	GENERATED_BODY()

public:
	AFortWeapon(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

	/**
	 * Called when pawn montage is started
	 */
	UFUNCTION()
	virtual void OnPawnMontageStarted(class UAnimMontage* Montage);

	/**
	 * Play weapon fire visual FX
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Fort|Weapon")
	void OnPlayWeaponFireFX();

	/**
	 * Stop weapon fire visual FX
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Fort|Weapon")
	void OnStopWeaponFireFX();

	/**
	 * Play weapon fire FX (client-side visuals)
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void PlayWeaponFireFX();

	/**
	 * Stop weapon fire FX
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void StopWeaponFireFX();

	/**
	 * Play impact effects
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Fort|Weapon|Effects")
	void OnPlayImpactFX(const FHitResult& Hit);

	/**
	 * Play reload effects
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Fort|Weapon|Effects")
	void OnPlayReloadFX();

	/**
	 * Use weapon durability
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void UseWeaponDurability(float DurabilityCost);

	/**
	 * Check if weapon durability is destroyed
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	bool IsWeaponDurabilityDestroyed() const;

	/**
	 * Get bullet shell FX template
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon|Effects")
	virtual class UParticleSystem* GetBulletShellFXTemplate() const;

	/**
	 * Should spawn bullet shell FX
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon|Effects")
	virtual bool ShouldSpawnBulletShellFX() const;

	/**
	 * Called when targeting state changes
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Fort|Weapon")
	void OnSetTargeting(bool bIsTargeting);

	/**
	 * Called when charge starts
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Fort|Weapon")
	void OnStartCharge();

	/**
	 * Called when weapon is equipped
	 */
	UFUNCTION()
	virtual void OnWeaponEquipped();

	/**
	 * Attempt to fire weapon
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual bool TryFire();

	/**
	 * Start weapon fire
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void StartFire();

	/**
	 * Stop weapon fire
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void StopFire();

	/**
	 * Reload weapon
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void Reload();

	/**
	 * Check if weapon can fire
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual bool CanFire() const;

	/**
	 * Weapon mesh component
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	USkeletalMeshComponent* WeaponMesh0;

	/**
	 * Weapon item definition
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Weapon")
	UFortWeaponItemDefinition* WeaponItemDefinition;

	/** Weapon configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	EFortWeaponTriggerType TriggerType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	EFortWeaponCoreAnimation WeaponCoreAnimation;

	/**
	 * Weapon statistics (using comprehensive FFortWeaponStats from FortWeaponData.h)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	FFortWeaponStats WeaponStats;

	/**
	 * Replicated weapon data
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReplicatedWeaponData, Category = "Fort|Weapon")
	FFortReplicatedWeaponData ReplicatedWeaponData;

	/**
	 * Item entry GUID
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Weapon")
	FGuid ItemEntryGuid;

	/**
	 * At minimum reticle spread
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Weapon")
	bool bAtMinimumReticleSpread;

	/**
	 * Update local ammo count
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Weapon")
	bool bUpdateLocalAmmoCount;

	/** Weapon state */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Weapon")
	int32 AmmoCount;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Weapon")
	int32 MagazineAmmoCount;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Weapon")
	bool bIsReloading;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Weapon")
	bool bIsFiring;

	/** Weapon durability */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Weapon")
	float CurrentDurability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	float MaxDurability;

	/** Owning pawn */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Weapon")
	APawn* OwnerPawn;

	/** Montages */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Animation")
	class UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Animation")
	class UAnimMontage* WeaponFireDownsightsMontage;

	/** Sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Audio")
	class USoundBase* WeaponFireCue;

protected:
	/**
	 * Replication callback for weapon data
	 */
	UFUNCTION()
	virtual void OnRep_ReplicatedWeaponData();

	/** Timers */
	FTimerHandle FireTimerHandle;
	FTimerHandle ReloadTimerHandle;

	/** Internal fire logic */
	virtual void FireWeaponInternal();

	/** Complete reload */
	virtual void CompleteReload();

	/** Is targeting */
	bool bIsTargeting;

	/** Is charging */
	bool bIsCharging;

	/**
	 * Charge percent
	 */
	float ChargePercent;
};

/**
 * AFortWeaponRanged - Ranged weapon class (guns, bows, etc.)
 */
UCLASS()
class FORTNITEGAME_API AFortWeaponRanged : public AFortWeapon
{
	GENERATED_BODY()

public:
	AFortWeaponRanged(const FObjectInitializer& ObjectInitializer);

	//~ Begin AFortWeapon Interface
	virtual bool TryFire() override;
	virtual void FireWeaponInternal() override;
	//~ End AFortWeapon Interface

	/**
	 * Perform hitscan trace
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual bool PerformHitscan(FHitResult& OutHit);

	/**
	 * Apply weapon damage to hit target
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void ApplyWeaponDamage(const FHitResult& Hit);

	/** Ranged weapon properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	float BulletSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	int32 BulletsPerShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	bool bUseProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	TSubclassOf<class AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	float ProjectileSpeed;

protected:
	/**
	 * Spawn projectile
	 */
	virtual class AActor* SpawnProjectile(const FVector& Direction);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortWeaponMelee - Melee weapon class
 */
UCLASS()
class FORTNITEGAME_API AFortWeaponMelee : public AFortWeapon
{
	GENERATED_BODY()

public:
	AFortWeaponMelee(const FObjectInitializer& ObjectInitializer);

	//~ Begin AFortWeapon Interface
	virtual bool TryFire() override;
	virtual void FireWeaponInternal() override;
	//~ End AFortWeapon Interface

	/**
	 * Perform melee swing trace
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual bool PerformMeleeSwing(TArray<FHitResult>& OutHits);

	/**
	 * Apply melee damage
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void ApplyMeleeDamage(const TArray<FHitResult>& Hits);

	/** Melee weapon properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	float SwingRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	float SwingAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	bool bCanHitMultiple;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	int32 MaxTargets;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AFortWeaponHarvest - Harvest tool (pickaxe)
 */
UCLASS()
class FORTNITEGAME_API AFortWeaponHarvest : public AFortWeaponMelee
{
	GENERATED_BODY()

public:
	AFortWeaponHarvest(const FObjectInitializer& ObjectInitializer);

	//~ Begin AFortWeapon Interface
	virtual void FireWeaponInternal() override;
	//~ End AFortWeapon Interface

	/**
	 * Harvest resources from target
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void HarvestResources(AActor* Target);

	/** Harvest properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Harvest")
	float HarvestDamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Harvest")
	float ResourceGatherMultiplier;
};
