// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Ability System - UE 4.12 Native Implementation (No GameplayAbilities Plugin)
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Abilities\FortAbilitySystemComponent.cpp

#include "FortAbilitySystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "FortWeapon.h"
#include "BuildingActor.h"

// ============================================================================
// UFortAbilitySystemComponent
// ============================================================================

UFortAbilitySystemComponent::UFortAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, NextAbilityHandle(1)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	bHealthRegenDelayed = false;
	bShieldRegenDelayed = false;
}

void UFortAbilitySystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Initialize default attributes
	InitializeAttributes();

	// Start regeneration timers
	StartHealthRegeneration();
	StartShieldRegeneration();
}

void UFortAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Additional ability system ticking logic
}

void UFortAbilitySystemComponent::InitializeAttributes()
{
	// Initialize default attributes
	Attributes.Add(FName("Health"), 100.0f);
	Attributes.Add(FName("MaxHealth"), 100.0f);
	Attributes.Add(FName("Shield"), 0.0f);
	Attributes.Add(FName("MaxShield"), 100.0f);
	Attributes.Add(FName("Energy"), 100.0f);
	Attributes.Add(FName("MaxEnergy"), 100.0f);
}

FFortAbilityHandle UFortAbilitySystemComponent::GiveAbility(TSubclassOf<UFortGameplayAbility> AbilityClass)
{
	if (!AbilityClass)
	{
		return FFortAbilityHandle();
	}

	// Create ability instance
	UFortGameplayAbility* NewAbility = NewObject<UFortGameplayAbility>(this, AbilityClass);
	if (NewAbility)
	{
		NewAbility->OwningAbilitySystemComponent = this;

		int32 Handle = NextAbilityHandle++;
		GrantedAbilities.Add(Handle, NewAbility);

		UE_LOG(LogTemp, Log, TEXT("Granted ability: %s (Handle: %d)"), *NewAbility->GetName(), Handle);
		return FFortAbilityHandle(Handle);
	}

	return FFortAbilityHandle();
}

void UFortAbilitySystemComponent::RemoveAbility(FFortAbilityHandle Handle)
{
	if (GrantedAbilities.Contains(Handle.Handle))
	{
		UFortGameplayAbility* Ability = GrantedAbilities[Handle.Handle];
		UE_LOG(LogTemp, Log, TEXT("Removed ability: %s (Handle: %d)"),
			Ability ? *Ability->GetName() : TEXT("null"), Handle.Handle);

		GrantedAbilities.Remove(Handle.Handle);
	}
}

bool UFortAbilitySystemComponent::TryActivateAbility(FFortAbilityHandle Handle)
{
	if (!GrantedAbilities.Contains(Handle.Handle))
	{
		return false;
	}

	UFortGameplayAbility* Ability = GrantedAbilities[Handle.Handle];
	if (!Ability)
	{
		return false;
	}

	// Check if ability can activate
	if (!Ability->CanActivateAbility(this))
	{
		return false;
	}

	// Check and apply cost
	if (!Ability->CheckCost(this))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot activate ability %s - insufficient resources"), *Ability->GetName());
		return false;
	}

	Ability->ApplyCost(this);

	// Activate ability
	FFortAbilityActivationInfo ActivationInfo;
	ActivationInfo.Instigator = GetOwner();
	ActivationInfo.ActivationTime = GetWorld()->GetTimeSeconds();

	Ability->ActivateAbility(this, ActivationInfo);

	return true;
}

float UFortAbilitySystemComponent::GetAttributeValue(FName AttributeName) const
{
	const float* Value = Attributes.Find(AttributeName);
	return Value ? *Value : 0.0f;
}

void UFortAbilitySystemComponent::SetAttributeValue(FName AttributeName, float Value)
{
	Attributes.Add(AttributeName, Value);
	UE_LOG(LogTemp, Verbose, TEXT("Set attribute %s = %.1f"), *AttributeName.ToString(), Value);
}

void UFortAbilitySystemComponent::ModifyAttributeValue(FName AttributeName, float Delta)
{
	float CurrentValue = GetAttributeValue(AttributeName);
	SetAttributeValue(AttributeName, CurrentValue + Delta);
	UE_LOG(LogTemp, Verbose, TEXT("Modified attribute %s by %.1f (now %.1f)"),
		*AttributeName.ToString(), Delta, CurrentValue + Delta);
}

void UFortAbilitySystemComponent::OnTagUpdated(const FFortGameplayTag& Tag, bool bTagExists)
{

	static const FName HealthRegenDelayTagName = FName(TEXT("Status.HealthRegenDelay"));
	static const FName ShieldRegenDelayTagName = FName(TEXT("Status.ShieldRegenDelay"));

	if (Tag.TagName == HealthRegenDelayTagName)
	{
		UE_LOG(LogTemp, Log, TEXT("UFortAbilitySystemComponent::OnTagUpdated is updating the HealthRegenDelay tag. bTagExists: %s"),
			bTagExists ? TEXT("true") : TEXT("false"));
		UpdateHealthRegenDelay(bTagExists);
	}
	else if (Tag.TagName == ShieldRegenDelayTagName)
	{
		UE_LOG(LogTemp, Log, TEXT("UFortAbilitySystemComponent::OnTagUpdated is updating the ShieldRegenDelay tag. bTagExists: %s"),
			bTagExists ? TEXT("true") : TEXT("false"));
		UpdateShieldRegenDelay(bTagExists);
	}
}

void UFortAbilitySystemComponent::UpdateHealthRegenDelay(bool bShouldDelay)
{
	bHealthRegenDelayed = bShouldDelay;

	if (bShouldDelay)
	{
		// Stop health regeneration
		if (AActor* Owner = GetOwner())
		{
			Owner->GetWorldTimerManager().ClearTimer(HealthRegenTimerHandle);
		}
	}
	else
	{
		// Resume health regeneration
		StartHealthRegeneration();
	}
}

void UFortAbilitySystemComponent::UpdateShieldRegenDelay(bool bShouldDelay)
{
	bShieldRegenDelayed = bShouldDelay;

	if (bShouldDelay)
	{
		// Stop shield regeneration
		if (AActor* Owner = GetOwner())
		{
			Owner->GetWorldTimerManager().ClearTimer(ShieldRegenTimerHandle);
		}
	}
	else
	{
		// Resume shield regeneration
		StartShieldRegeneration();
	}
}

void UFortAbilitySystemComponent::StartHealthRegeneration()
{
	if (AActor* Owner = GetOwner())
	{
		FTimerDelegate RegenDelegate;
		RegenDelegate.BindLambda([this]()
		{
			if (!bHealthRegenDelayed)
			{
				float CurrentHealth = GetAttributeValue(FName("Health"));
				float MaxHealth = GetAttributeValue(FName("MaxHealth"));

				if (CurrentHealth < MaxHealth)
				{
					ModifyAttributeValue(FName("Health"), 1.0f); // Regen 1 HP per tick
				}
			}
		});

		Owner->GetWorldTimerManager().SetTimer(HealthRegenTimerHandle, RegenDelegate, 1.0f, true);
	}
}

void UFortAbilitySystemComponent::StartShieldRegeneration()
{
	if (AActor* Owner = GetOwner())
	{
		FTimerDelegate RegenDelegate;
		RegenDelegate.BindLambda([this]()
		{
			if (!bShieldRegenDelayed)
			{
				float CurrentShield = GetAttributeValue(FName("Shield"));
				float MaxShield = GetAttributeValue(FName("MaxShield"));

				if (CurrentShield < MaxShield)
				{
					ModifyAttributeValue(FName("Shield"), 5.0f); // Regen 5 shield per tick
				}
			}
		});

		Owner->GetWorldTimerManager().SetTimer(ShieldRegenTimerHandle, RegenDelegate, 1.0f, true);
	}
}

void UFortAbilitySystemComponent::AddFortAbilitySet(UFortAbilitySet* AbilitySet)
{
	if (!AbilitySet)
	{
		return;
	}

	// Grant ability set
	AbilitySet->GiveToAbilitySystemComponent(this);

	// Track granted ability set
	GrantedAbilitySets.AddUnique(AbilitySet);

	UE_LOG(LogTemp, Log, TEXT("Added ability set: %s"), *AbilitySet->GetName());
}

void UFortAbilitySystemComponent::UnequipFortAbilitySet(UFortAbilitySet* AbilitySet)
{
	if (!AbilitySet)
	{
		return;
	}

	// Remove ability set
	AbilitySet->RemoveFromAbilitySystemComponent(this);

	// Untrack ability set
	GrantedAbilitySets.Remove(AbilitySet);

	UE_LOG(LogTemp, Log, TEXT("Removed ability set: %s"), *AbilitySet->GetName());
}

// ============================================================================
// UFortGameplayAbility
// ============================================================================

UFortGameplayAbility::UFortGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySource = EFortAbilitySourceType::Default;
	BaseDamage = 0.0f;
	CooldownDuration = 0.0f;
	EnergyCost = 0.0f;
	OwningAbilitySystemComponent = nullptr;
}

bool UFortGameplayAbility::CanActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent) const
{
	// Base implementation - can always activate
	return true;
}

void UFortGameplayAbility::ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo)
{
	// Base implementation - log activation
	UE_LOG(LogTemp, Log, TEXT("Activated ability: %s"), *GetName());
}

void UFortGameplayAbility::EndAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, bool bWasCancelled)
{
	// Base implementation - log end
	UE_LOG(LogTemp, Log, TEXT("Ended ability: %s (Cancelled: %s)"),
		*GetName(), bWasCancelled ? TEXT("true") : TEXT("false"));
}

float UFortGameplayAbility::GetDamageStats() const
{
	UE_LOG(LogTemp, Verbose, TEXT("UFortGameplayAbility::GetDamageStats"));
	return BaseDamage;
}

bool UFortGameplayAbility::CheckCost(UFortAbilitySystemComponent* AbilitySystemComponent) const
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	UE_LOG(LogTemp, Verbose, TEXT("UFortGameplayAbility::CheckCost for %s"), *GetName());

	// Check energy cost via simple attribute lookup
	if (EnergyCost > 0.0f)
	{
		float CurrentEnergy = AbilitySystemComponent->GetAttributeValue(FName("Energy"));
		if (CurrentEnergy < EnergyCost)
		{
			UE_LOG(LogTemp, Warning, TEXT("Insufficient energy: need %.1f, have %.1f"),
				EnergyCost, CurrentEnergy);
			return false;
		}
	}

	return true;
}

void UFortGameplayAbility::ApplyCost(UFortAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	UE_LOG(LogTemp, Verbose, TEXT("UFortGameplayAbility::ApplyCost from ability %s"), *GetName());

	// Deduct energy cost directly
	if (EnergyCost > 0.0f)
	{
		AbilitySystemComponent->ModifyAttributeValue(FName("Energy"), -EnergyCost);
		UE_LOG(LogTemp, Verbose, TEXT("Applied energy cost: %.1f"), EnergyCost);
	}
}

// ============================================================================
// UFortGameplayAbility_Action
// ============================================================================

UFortGameplayAbility_Action::UFortGameplayAbility_Action(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFortGameplayAbility_Action::ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo)
{
	Super::ActivateAbility_Implementation(AbilitySystemComponent, ActivationInfo);

	// Perform sphere trace in front of player to find interactables
	if (!AbilitySystemComponent)
	{
		return;
	}

	AActor* Avatar = AbilitySystemComponent->GetOwner();
	if (!Avatar)
	{
		return;
	}

	UWorld* World = Avatar->GetWorld();
	if (!World)
	{
		return;
	}

	FVector StartLocation = Avatar->GetActorLocation();
	FVector ForwardVector = Avatar->GetActorForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * 300.0f); // 3 meter range

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Avatar);

	// Sphere sweep for interactables
	if (World->SweepSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(100.0f), // 1 meter radius
		QueryParams))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->IsA(ABuildingActor::StaticClass()))
		{
			ABuildingActor* Building = Cast<ABuildingActor>(HitActor);
			UE_LOG(LogTemp, Log, TEXT("Interacted with building: %s"), *Building->GetName());

			// Building interaction logic would go here
			// Examples: open door, activate trap, repair, etc.
		}
		else if (HitActor)
		{
			UE_LOG(LogTemp, Log, TEXT("Interacted with actor: %s"), *HitActor->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("Action ability found no interactable objects"));
	}

	EndAbility(AbilitySystemComponent, false);
}

// ============================================================================
// UFortGameplayAbility_Jump
// ============================================================================

UFortGameplayAbility_Jump::UFortGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	JumpVelocity = 600.0f;
}

void UFortGameplayAbility_Jump::ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo)
{
	Super::ActivateAbility_Implementation(AbilitySystemComponent, ActivationInfo);

	if (!AbilitySystemComponent)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(AbilitySystemComponent->GetOwner());
	if (Character)
	{
		Character->Jump();
		UE_LOG(LogTemp, Log, TEXT("Character jumped with velocity %.1f"), JumpVelocity);
	}

	EndAbility(AbilitySystemComponent, false);
}

// ============================================================================
// UFortGameplayAbility_Sprint
// ============================================================================

UFortGameplayAbility_Sprint::UFortGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SprintSpeedMultiplier = 1.5f;
}

void UFortGameplayAbility_Sprint::ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo)
{
	Super::ActivateAbility_Implementation(AbilitySystemComponent, ActivationInfo);

	if (!AbilitySystemComponent)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(AbilitySystemComponent->GetOwner());
	if (Character)
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		if (Movement)
		{
			Movement->MaxWalkSpeed *= SprintSpeedMultiplier;
			UE_LOG(LogTemp, Log, TEXT("Sprint activated - speed multiplier: %.1fx"), SprintSpeedMultiplier);
		}
	}
}

void UFortGameplayAbility_Sprint::EndAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, bool bWasCancelled)
{
	Super::EndAbility_Implementation(AbilitySystemComponent, bWasCancelled);

	if (!AbilitySystemComponent)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(AbilitySystemComponent->GetOwner());
	if (Character)
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		if (Movement)
		{
			Movement->MaxWalkSpeed /= SprintSpeedMultiplier;
			UE_LOG(LogTemp, Log, TEXT("Sprint ended - speed restored"));
		}
	}
}

// ============================================================================
// UFortGameplayAbility_RangedWeapon
// ============================================================================

UFortGameplayAbility_RangedWeapon::UFortGameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FireRate = 0.1f;
	BulletsPerShot = 1;
}

void UFortGameplayAbility_RangedWeapon::ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo)
{
	Super::ActivateAbility_Implementation(AbilitySystemComponent, ActivationInfo);

	FireWeapon();

	EndAbility(AbilitySystemComponent, false);
}

void UFortGameplayAbility_RangedWeapon::FireWeapon()
{
	// Fire weapon logic
	UE_LOG(LogTemp, Log, TEXT("Fired weapon - bullets: %d, fire rate: %.2f"), BulletsPerShot, FireRate);

	// this would:
	// - Perform line trace for hit detection
	// - Spawn projectile or apply instant damage
	// - Play fire effects (muzzle flash, sound, recoil)
	// - Consume ammo
}

// ============================================================================
// UFortGameplayAbility_Reload
// ============================================================================

UFortGameplayAbility_Reload::UFortGameplayAbility_Reload(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReloadDuration = 2.0f;
}

void UFortGameplayAbility_Reload::ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo)
{
	Super::ActivateAbility_Implementation(AbilitySystemComponent, ActivationInfo);

	if (!AbilitySystemComponent)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(AbilitySystemComponent->GetOwner());
	if (!Character)
	{
		EndAbility(AbilitySystemComponent, true);
		return;
	}

	// Find equipped weapon (typically attached to character mesh)
	TArray<AActor*> AttachedActors;
	Character->GetAttachedActors(AttachedActors);

	for (AActor* AttachedActor : AttachedActors)
	{
		AFortWeapon* Weapon = Cast<AFortWeapon>(AttachedActor);
		if (Weapon && Weapon->WeaponFireMontage)
		{
			USkeletalMeshComponent* Mesh = Character->GetMesh();
			if (Mesh && Mesh->AnimScriptInstance)
			{
				// Play reload montage
				// In UE 4.12: Mesh->AnimScriptInstance->Montage_Play(ReloadMontage, 1.0f);
				UE_LOG(LogTemp, Log, TEXT("Playing reload montage for weapon: %s (duration: %.1fs)"),
					*Weapon->GetName(), ReloadDuration);
			}
			break;
		}
	}

	// Set timer to end ability after reload duration
	if (AActor* Owner = AbilitySystemComponent->GetOwner())
	{
		FTimerHandle ReloadTimerHandle;
		FTimerDelegate ReloadDelegate;
		ReloadDelegate.BindLambda([this, AbilitySystemComponent]()
		{
			EndAbility(AbilitySystemComponent, false);
		});

		Owner->GetWorldTimerManager().SetTimer(ReloadTimerHandle, ReloadDelegate, ReloadDuration, false);
	}
}

// ============================================================================
// UFortAbilitySet
// ============================================================================

UFortAbilitySet::UFortAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFortAbilitySet::GiveToAbilitySystemComponent(UFortAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Grant abilities
	TArray<FFortAbilityHandle> GrantedHandles;
	for (TSubclassOf<UFortGameplayAbility> AbilityClass : GameplayAbilities)
	{
		if (AbilityClass)
		{
			FFortAbilityHandle Handle = AbilitySystemComponent->GiveAbility(AbilityClass);
			if (Handle.IsValid())
			{
				GrantedHandles.Add(Handle);
			}
		}
	}

	// Store handles for later removal
	GrantedAbilityHandles.Add(AbilitySystemComponent, GrantedHandles);

	// Initialize attributes
	for (const auto& AttributePair : AttributeDefaults)
	{
		AbilitySystemComponent->SetAttributeValue(AttributePair.Key, AttributePair.Value);
	}

	UE_LOG(LogTemp, Log, TEXT("Granted ability set: %s (%d abilities, %d attributes)"),
		*GetName(), GrantedHandles.Num(), AttributeDefaults.Num());
}

void UFortAbilitySet::RemoveFromAbilitySystemComponent(UFortAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Remove granted abilities
	TArray<FFortAbilityHandle>* Handles = GrantedAbilityHandles.Find(AbilitySystemComponent);
	if (Handles)
	{
		for (const FFortAbilityHandle& Handle : *Handles)
		{
			AbilitySystemComponent->RemoveAbility(Handle);
		}

		GrantedAbilityHandles.Remove(AbilitySystemComponent);

		UE_LOG(LogTemp, Log, TEXT("Removed ability set: %s (%d abilities removed)"),
			*GetName(), Handles->Num());
	}
}

// ============================================================================
// UFortGameplayAbilityTooltip
// ============================================================================

UFortGameplayAbilityTooltip::UFortGameplayAbilityTooltip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText UFortGameplayAbilityTooltip::GetCostAsText(const UFortGameplayAbility* Ability)
{
	if (!Ability)
	{
		return FText::GetEmpty();
	}

	UE_LOG(LogTemp, Verbose, TEXT("UFortGameplayAbilityTooltip::GetCostAsText from ability %s"), *Ability->GetName());

	if (Ability->EnergyCost > 0.0f)
	{
		return FText::FromString(FString::Printf(TEXT("%.0f Energy"), Ability->EnergyCost));
	}

	return FText::FromString(TEXT("No Cost"));
}

FText UFortGameplayAbilityTooltip::GetDamageAsText(const UFortGameplayAbility* Ability)
{
	if (!Ability)
	{
		return FText::GetEmpty();
	}

	if (Ability->BaseDamage > 0.0f)
	{
		return FText::FromString(FString::Printf(TEXT("%.0f Damage"), Ability->BaseDamage));
	}

	return FText::GetEmpty();
}

FText UFortGameplayAbilityTooltip::GetCooldownAsText(const UFortGameplayAbility* Ability)
{
	if (!Ability)
	{
		return FText::GetEmpty();
	}

	if (Ability->CooldownDuration > 0.0f)
	{
		return FText::FromString(FString::Printf(TEXT("%.1fs Cooldown"), Ability->CooldownDuration));
	}

	return FText::GetEmpty();
}

FText UFortGameplayAbilityTooltip::GenerateTooltip(const UFortGameplayAbility* Ability)
{
	if (!Ability)
	{
		return FText::GetEmpty();
	}

	UE_LOG(LogTemp, Verbose, TEXT("Generating tooltip for ability: %s"), *Ability->GetName());

	FString TooltipString = Ability->GetName();

	FText Cost = GetCostAsText(Ability);
	if (!Cost.IsEmpty())
	{
		TooltipString += FString::Printf(TEXT("\nCost: %s"), *Cost.ToString());
	}

	FText Damage = GetDamageAsText(Ability);
	if (!Damage.IsEmpty())
	{
		TooltipString += FString::Printf(TEXT("\n%s"), *Damage.ToString());
	}

	FText Cooldown = GetCooldownAsText(Ability);
	if (!Cooldown.IsEmpty())
	{
		TooltipString += FString::Printf(TEXT("\n%s"), *Cooldown.ToString());
	}

	return FText::FromString(TooltipString);
}
