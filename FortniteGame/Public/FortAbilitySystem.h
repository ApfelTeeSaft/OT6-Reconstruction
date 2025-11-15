// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Ability System - UE 4.12 Native Implementation (No GameplayAbilities Plugin)
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Abilities\FortAbilitySystemComponent.cpp

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "FortEnums.h"
#include "FortAbilitySystem.generated.h"

/**
 * Simple tag structure for UE 4.12 (replaces GameplayTag)
 */
USTRUCT(BlueprintType)
struct FFortGameplayTag
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Tag")
	FName TagName;

	FFortGameplayTag() : TagName(NAME_None) {}
	FFortGameplayTag(FName InTagName) : TagName(InTagName) {}

	bool IsValid() const { return TagName != NAME_None; }
	bool operator==(const FFortGameplayTag& Other) const { return TagName == Other.TagName; }
};

/**
 * Simple ability activation info (replaces FGameplayAbilityActivationInfo)
 */
USTRUCT(BlueprintType)
struct FFortAbilityActivationInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Fort|Ability")
	AActor* Instigator;

	UPROPERTY(BlueprintReadOnly, Category = "Fort|Ability")
	float ActivationTime;

	FFortAbilityActivationInfo() : Instigator(nullptr), ActivationTime(0.f) {}
};

/**
 * Simple ability handle (replaces FGameplayAbilitySpecHandle)
 */
USTRUCT(BlueprintType)
struct FFortAbilityHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Handle;

	FFortAbilityHandle() : Handle(0) {}
	FFortAbilityHandle(int32 InHandle) : Handle(InHandle) {}

	bool IsValid() const { return Handle > 0; }
	bool operator==(const FFortAbilityHandle& Other) const { return Handle == Other.Handle; }
};

/**
 * UFortAbilitySystemComponent - Fortnite's ability system component
 * UE 4.12 Native implementation (no GameplayAbilities plugin required)
 */
UCLASS()
class FORTNITEGAME_API UFortAbilitySystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFortAbilitySystemComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin UActorComponent Interface
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

	/**
	 * Grant an ability to this component
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Abilities")
	FFortAbilityHandle GiveAbility(TSubclassOf<class UFortGameplayAbility> AbilityClass);

	/**
	 * Remove an ability from this component
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Abilities")
	void RemoveAbility(FFortAbilityHandle Handle);

	/**
	 * Try to activate an ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Abilities")
	bool TryActivateAbility(FFortAbilityHandle Handle);

	/**
	 * Called when tag is updated
	 */
	virtual void OnTagUpdated(const FFortGameplayTag& Tag, bool bTagExists);

	/**
	 * Handle health regeneration delay
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Abilities")
	void UpdateHealthRegenDelay(bool bShouldDelay);

	/**
	 * Handle shield regeneration delay
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Abilities")
	void UpdateShieldRegenDelay(bool bShouldDelay);

	/**
	 * Grant ability set to component
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Abilities")
	void AddFortAbilitySet(class UFortAbilitySet* AbilitySet);

	/**
	 * Remove ability set from component
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Abilities")
	void UnequipFortAbilitySet(class UFortAbilitySet* AbilitySet);

	/**
	 * Attribute accessors (replaces AttributeSet system)
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Attributes")
	float GetAttributeValue(FName AttributeName) const;

	UFUNCTION(BlueprintCallable, Category = "Fort|Attributes")
	void SetAttributeValue(FName AttributeName, float Value);

	UFUNCTION(BlueprintCallable, Category = "Fort|Attributes")
	void ModifyAttributeValue(FName AttributeName, float Delta);

	/** Health regen state */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Abilities")
	bool bHealthRegenDelayed;

	/** Shield regen state */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Abilities")
	bool bShieldRegenDelayed;

	/** Ability sets granted to this component */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Abilities")
	TArray<class UFortAbilitySet*> GrantedAbilitySets;

	/** Active tags on this component */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Abilities")
	TArray<FFortGameplayTag> ActiveTags;

	/** Attributes (replaces AttributeSet) */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Attributes")
	TMap<FName, float> Attributes;

protected:
	/** Regeneration timers */
	FTimerHandle HealthRegenTimerHandle;
	FTimerHandle ShieldRegenTimerHandle;

	/** Granted abilities */
	UPROPERTY()
	TMap<int32, class UFortGameplayAbility*> GrantedAbilities;

	int32 NextAbilityHandle;

	/** Start health regeneration */
	void StartHealthRegeneration();

	/** Start shield regeneration */
	void StartShieldRegeneration();

	/** Initialize default attributes */
	void InitializeAttributes();
};

/**
 * UFortGameplayAbility - Base Fort gameplay ability
 * UE 4.12 Native implementation (no GameplayAbilities plugin required)
 */
UCLASS(Abstract, Blueprintable)
class FORTNITEGAME_API UFortGameplayAbility : public UObject
{
	GENERATED_BODY()

public:
	UFortGameplayAbility(const FObjectInitializer& ObjectInitializer);

	/**
	 * Can this ability activate?
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Fort|Ability")
	bool CanActivateAbility(UFortAbilitySystemComponent* AbilitySystemComponent) const;
	virtual bool CanActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent) const;

	/**
	 * Activate the ability
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Fort|Ability")
	void ActivateAbility(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo);
	virtual void ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo);

	/**
	 * End the ability
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Fort|Ability")
	void EndAbility(UFortAbilitySystemComponent* AbilitySystemComponent, bool bWasCancelled);
	virtual void EndAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, bool bWasCancelled);

	/**
	 * Get damage stats for this ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Ability")
	virtual float GetDamageStats() const;

	/**
	 * Check if ability cost can be paid
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Ability")
	virtual bool CheckCost(UFortAbilitySystemComponent* AbilitySystemComponent) const;

	/**
	 * Apply ability cost
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Ability")
	virtual void ApplyCost(UFortAbilitySystemComponent* AbilitySystemComponent);

	/** Ability configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Ability")
	EFortAbilitySourceType AbilitySource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Ability")
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Ability")
	float CooldownDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Ability")
	float EnergyCost;

	/** Owning ability system component */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Ability")
	UFortAbilitySystemComponent* OwningAbilitySystemComponent;
};

/**
 * UFortGameplayAbility_Action - Action ability (interact, etc.)
 */
UCLASS()
class FORTNITEGAME_API UFortGameplayAbility_Action : public UFortGameplayAbility
{
	GENERATED_BODY()

public:
	UFortGameplayAbility_Action(const FObjectInitializer& ObjectInitializer);

	virtual void ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo) override;
};

/**
 * UFortGameplayAbility_Jump - Jump ability
 */
UCLASS()
class FORTNITEGAME_API UFortGameplayAbility_Jump : public UFortGameplayAbility
{
	GENERATED_BODY()

public:
	UFortGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer);

	virtual void ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Jump")
	float JumpVelocity;
};

/**
 * UFortGameplayAbility_Sprint - Sprint ability
 */
UCLASS()
class FORTNITEGAME_API UFortGameplayAbility_Sprint : public UFortGameplayAbility
{
	GENERATED_BODY()

public:
	UFortGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer);

	virtual void ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo) override;
	virtual void EndAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, bool bWasCancelled) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Sprint")
	float SprintSpeedMultiplier;
};

/**
 * UFortGameplayAbility_RangedWeapon - Ranged weapon firing ability
 */
UCLASS()
class FORTNITEGAME_API UFortGameplayAbility_RangedWeapon : public UFortGameplayAbility
{
	GENERATED_BODY()

public:
	UFortGameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer);

	virtual void ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo) override;

	/** Fire weapon */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	virtual void FireWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Weapon")
	int32 BulletsPerShot;
};

/**
 * UFortGameplayAbility_Reload - Weapon reload ability
 */
UCLASS()
class FORTNITEGAME_API UFortGameplayAbility_Reload : public UFortGameplayAbility
{
	GENERATED_BODY()

public:
	UFortGameplayAbility_Reload(const FObjectInitializer& ObjectInitializer);

	virtual void ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Reload")
	float ReloadDuration;
};

/**
 * UFortAbilitySet - Collection of abilities to grant
 */
UCLASS(Blueprintable)
class FORTNITEGAME_API UFortAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:
	UFortAbilitySet(const FObjectInitializer& ObjectInitializer);

	/**
	 * Grant all abilities in this set to a component
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AbilitySet")
	void GiveToAbilitySystemComponent(UFortAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Remove all abilities in this set from a component
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AbilitySet")
	void RemoveFromAbilitySystemComponent(UFortAbilitySystemComponent* AbilitySystemComponent);

	/** Abilities to grant */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|AbilitySet")
	TArray<TSubclassOf<UFortGameplayAbility>> GameplayAbilities;

	/** Attributes to initialize */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|AbilitySet")
	TMap<FName, float> AttributeDefaults;

protected:
	/** Stored handle mappings for removal */
	UPROPERTY()
	TMap<UFortAbilitySystemComponent*, TArray<FFortAbilityHandle>> GrantedAbilityHandles;
};

/**
 * UFortGameplayAbilityTooltip - Tooltip generator for abilities
 */
UCLASS()
class FORTNITEGAME_API UFortGameplayAbilityTooltip : public UObject
{
	GENERATED_BODY()

public:
	UFortGameplayAbilityTooltip(const FObjectInitializer& ObjectInitializer);

	/**
	 * Get cost as formatted text
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Tooltip")
	static FText GetCostAsText(const UFortGameplayAbility* Ability);

	/**
	 * Get damage as formatted text
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Tooltip")
	static FText GetDamageAsText(const UFortGameplayAbility* Ability);

	/**
	 * Get cooldown as formatted text
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Tooltip")
	static FText GetCooldownAsText(const UFortGameplayAbility* Ability);

	/**
	 * Generate full tooltip for ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Tooltip")
	static FText GenerateTooltip(const UFortGameplayAbility* Ability);
};
