// Copyright Epic Games, Inc. All Rights Reserved.
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Abilities\Tasks\FortAbilityTask_CommitAccountCosts.cpp

#include "FortAbilityTask_CommitAccountCosts.h"
#include "FortPlayerController.h"
#include "FortInventory.h"

// ============================================================================
// UFortAbilityTask_CommitAccountCosts
// ============================================================================

UFortAbilityTask_CommitAccountCosts::UFortAbilityTask_CommitAccountCosts()
	: OwningAbility(nullptr)
	, AbilitySystemComponent(nullptr)
	, bIsActive(false)
	, StaminaCost(0.0f)
{
}

UFortAbilityTask_CommitAccountCosts* UFortAbilityTask_CommitAccountCosts::CommitAccountCosts(
	UFortGameplayAbility* OwningAbility,
	UFortAbilitySystemComponent* AbilitySystemComponent)
{
	UFortAbilityTask_CommitAccountCosts* Task = NewObject<UFortAbilityTask_CommitAccountCosts>();
	if (Task)
	{
		Task->OwningAbility = OwningAbility;
		Task->AbilitySystemComponent = AbilitySystemComponent;

		// Get costs from ability
		if (OwningAbility)
		{
			Task->StaminaCost = OwningAbility->EnergyCost; // Using EnergyCost as StaminaCost
		}
	}

	return Task;
}

void UFortAbilityTask_CommitAccountCosts::Activate()
{
	UE_LOG(LogTemp, Log, TEXT("UFortAbilityTask_CommitAccountCosts::Activate from effect %s"),
		OwningAbility ? *OwningAbility->GetName() : TEXT("null"));

	bIsActive = true;

	// Check if ability still exists
	if (!OwningAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortAbilityTask_CommitAccountCosts failed to consume items for ability %s and player %s but ability is gone!"),
			TEXT("unknown"), TEXT("unknown"));
		OnFailed.Broadcast(EFortAbilityCommitResult::FailedAbilityGone);
		return;
	}

	// Check stamina cost
	if (!CheckStaminaCost())
	{
		UE_LOG(LogTemp, Warning, TEXT("Ability %s has stamina cost but activator doesn't have stamina!"),
			*OwningAbility->GetName());
		OnFailed.Broadcast(EFortAbilityCommitResult::FailedStaminaCost);
		return;
	}

	// Check item costs
	if (!CheckItemCosts())
	{
		UE_LOG(LogTemp, Warning, TEXT("Ability %s has an invalid item cost!"),
			*OwningAbility->GetName());
		OnFailed.Broadcast(EFortAbilityCommitResult::FailedItemCost);
		return;
	}

	// Apply stamina cost
	ApplyStaminaCost();

	// Apply item costs
	if (!ApplyItemCosts())
	{
		// Rollback stamina if item costs fail
		if (AbilitySystemComponent && StaminaCost > 0.0f)
		{
			AbilitySystemComponent->ModifyAttributeValue(FName("Energy"), StaminaCost); // Refund
		}

		OnFailed.Broadcast(EFortAbilityCommitResult::FailedItemCost);
		return;
	}

	// Check if ability is still valid after cost application
	if (!OwningAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortAbilityTask_CommitAccountCosts consumed items for ability %s and player %s but ability is gone! Player has lost resources."),
			TEXT("unknown"), TEXT("unknown"));
	}

	// Success
	OnSuccess.Broadcast();
	bIsActive = false;
}

void UFortAbilityTask_CommitAccountCosts::Cancel()
{
	if (bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("CommitAccountCosts cannot be cancelled when active!"));
		return;
	}

	if (!OwningAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortAbilityTask_CommitAccountCosts cancelled early for ability %s and player %s but ability is gone! Player has lost resources."),
			TEXT("unknown"), TEXT("unknown"));
	}

	// Cancel logic
	bIsActive = false;
}

bool UFortAbilityTask_CommitAccountCosts::CheckStaminaCost()
{
	if (StaminaCost <= 0.0f)
	{
		return true; // No stamina cost
	}

	if (!AbilitySystemComponent)
	{
		return false;
	}

	// Check if actor has enough stamina/energy
	float CurrentEnergy = AbilitySystemComponent->GetAttributeValue(FName("Energy"));

	if (CurrentEnergy < StaminaCost)
	{
		return false;
	}

	return true;
}

void UFortAbilityTask_CommitAccountCosts::ApplyStaminaCost()
{
	if (StaminaCost > 0.0f && AbilitySystemComponent)
	{
		AbilitySystemComponent->ModifyAttributeValue(FName("Energy"), -StaminaCost);
		UE_LOG(LogTemp, Verbose, TEXT("Applied stamina cost: %.1f"), StaminaCost);
	}
}

bool UFortAbilityTask_CommitAccountCosts::CheckItemCosts()
{
	if (ItemCosts.Num() == 0)
	{
		return true; // No item costs
	}

	if (!OwningAbility)
	{
		return false;
	}

	// Get player's inventory component
	AActor* AvatarActor = OwningAbility->GetAvatarActor();
	if (!AvatarActor)
	{
		return false;
	}

	UFortInventory* Inventory = AvatarActor->FindComponentByClass<UFortInventory>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_CommitAccountCosts: No inventory component found on avatar"));
		return false;
	}

	// Check all item costs against player inventory
	for (const FFortAccountItemCost& ItemCost : ItemCosts)
	{
		if (ItemCost.ItemTemplateId.IsEmpty() || ItemCost.Quantity <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_CommitAccountCosts: Invalid item cost - TemplateId: %s, Quantity: %d"),
				*ItemCost.ItemTemplateId, ItemCost.Quantity);
			return false; // Invalid item cost
		}

		// Find items by definition name in player inventory
		FName ItemDefName = FName(*ItemCost.ItemTemplateId);
		TArray<FFortItemEntry> FoundItems = Inventory->FindItemsByDefinition(ItemDefName);

		// Calculate total count of this item type across all stacks
		int32 TotalCount = 0;
		for (const FFortItemEntry& Entry : FoundItems)
		{
			TotalCount += Entry.Count;
		}

		// Check if player has enough of this item
		if (TotalCount < ItemCost.Quantity)
		{
			UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_CommitAccountCosts: Insufficient items - Need %d of %s, have %d"),
				ItemCost.Quantity, *ItemCost.ItemTemplateId, TotalCount);
			return false;
		}
	}

	return true;
}

bool UFortAbilityTask_CommitAccountCosts::ApplyItemCosts()
{
	if (ItemCosts.Num() == 0)
	{
		return true; // No item costs
	}

	if (!OwningAbility)
	{
		return false;
	}

	// Get player's inventory component
	AActor* AvatarActor = OwningAbility->GetAvatarActor();
	if (!AvatarActor)
	{
		return false;
	}

	UFortInventory* Inventory = AvatarActor->FindComponentByClass<UFortInventory>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortAbilityTask_CommitAccountCosts: No inventory component found for item consumption"));
		FString ErrorMessage = TEXT("No inventory component");
		HandleMCPOutOfSync(ErrorMessage);
		return false;
	}

	// Consume all item costs from player inventory
	for (const FFortAccountItemCost& ItemCost : ItemCosts)
	{
		FName ItemDefName = FName(*ItemCost.ItemTemplateId);
		TArray<FFortItemEntry> FoundItems = Inventory->FindItemsByDefinition(ItemDefName);

		if (FoundItems.Num() == 0)
		{
			FString ErrorMessage = FString::Printf(TEXT("Item %s not found in inventory"), *ItemCost.ItemTemplateId);
			HandleMCPOutOfSync(ErrorMessage);
			return false;
		}

		// Consume items across stacks until we've consumed the required quantity
		int32 RemainingToConsume = ItemCost.Quantity;
		for (FFortItemEntry& Entry : FoundItems)
		{
			if (RemainingToConsume <= 0)
			{
				break;
			}

			int32 AmountToRemove = FMath::Min(RemainingToConsume, Entry.Count);
			bool bRemoveSuccess = Inventory->RemoveItem(Entry.ItemGuid, AmountToRemove);

			if (!bRemoveSuccess)
			{
				// Failed to remove items - MCP out of sync
				FString ErrorMessage = FString::Printf(TEXT("Failed to remove %d of item %s"), AmountToRemove, *ItemCost.ItemTemplateId);
				HandleMCPOutOfSync(ErrorMessage);
				return false;
			}

			RemainingToConsume -= AmountToRemove;

			UE_LOG(LogTemp, Verbose, TEXT("Consumed %d of item %s (GUID: %s)"),
				AmountToRemove, *ItemCost.ItemTemplateId, *Entry.ItemGuid.Guid.ToString());
		}

		if (RemainingToConsume > 0)
		{
			// Shouldn't happen if CheckItemCosts passed, but handle it anyway
			FString ErrorMessage = FString::Printf(TEXT("Insufficient items to consume: %s (short by %d)"), *ItemCost.ItemTemplateId, RemainingToConsume);
			HandleMCPOutOfSync(ErrorMessage);
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("Successfully consumed %d of item %s"), ItemCost.Quantity, *ItemCost.ItemTemplateId);
	}

	return true;
}

void UFortAbilityTask_CommitAccountCosts::HandleMCPOutOfSync(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Error, TEXT("UFortAbilityTask_CommitAccountCosts failed for ability %s on player %s with error %s, MCP is out of sync!"),
		OwningAbility ? *OwningAbility->GetName() : TEXT("unknown"),
		TEXT("unknown"),
		*ErrorMessage);
}

// ============================================================================
// UFortAbilityTask_WaitTargetData
// ============================================================================

UFortAbilityTask_WaitTargetData::UFortAbilityTask_WaitTargetData()
	: OwningAbility(nullptr)
	, bIsWaiting(false)
{
}

UFortAbilityTask_WaitTargetData* UFortAbilityTask_WaitTargetData::WaitTargetData(
	UFortGameplayAbility* OwningAbility,
	FName TaskInstanceName)
{
	UFortAbilityTask_WaitTargetData* Task = NewObject<UFortAbilityTask_WaitTargetData>();
	if (Task)
	{
		Task->OwningAbility = OwningAbility;
		Task->TaskInstanceName = TaskInstanceName;
	}

	return Task;
}

void UFortAbilityTask_WaitTargetData::Activate()
{
	bIsWaiting = true;
	UE_LOG(LogTemp, Log, TEXT("WaitTargetData activated: %s"), *TaskInstanceName.ToString());

	//this would wait for player input or replication
}

void UFortAbilityTask_WaitTargetData::ConfirmTargetData(const FFortAbilityTargetSelection& TargetData)
{
	if (bIsWaiting)
	{
		OnTargetDataReady.Broadcast(TargetData);
		bIsWaiting = false;
	}
}
