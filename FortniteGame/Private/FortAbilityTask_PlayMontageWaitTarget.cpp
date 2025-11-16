// Copyright Epic Games, Inc. All Rights Reserved.
// FortAbilityTask_PlayMontageWaitTarget Implementation

#include "FortAbilityTask_PlayMontageWaitTarget.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "FortPawn.h"

//////////////////////////////////////////////////////////////////////////
// UFortAbilityTask_PlayMontageWaitTarget

UFortAbilityTask_PlayMontageWaitTarget::UFortAbilityTask_PlayMontageWaitTarget()
	: OwningAbility(nullptr)
	, MontageToPlay(nullptr)
	, PlayRate(1.0f)
	, StartSectionName(NAME_None)
	, bMontageFinished(false)
	, bTargetReceived(false)
{
}

UFortAbilityTask_PlayMontageWaitTarget* UFortAbilityTask_PlayMontageWaitTarget::PlayMontageAndWaitForTarget(
	UFortGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	UAnimMontage* MontageToPlay,
	float PlayRate,
	FName StartSectionName)
{
	UFortAbilityTask_PlayMontageWaitTarget* Task = NewObject<UFortAbilityTask_PlayMontageWaitTarget>();
	if (Task)
	{
		Task->OwningAbility = OwningAbility;
		Task->TaskInstanceName = TaskInstanceName;
		Task->MontageToPlay = MontageToPlay;
		Task->PlayRate = PlayRate;
		Task->StartSectionName = StartSectionName;
	}

	return Task;
}

void UFortAbilityTask_PlayMontageWaitTarget::Activate()
{
	if (!OwningAbility || !MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_PlayMontageWaitTarget: Cannot activate - missing ability or montage"));
		OnCancelled.Broadcast();
		return;
	}

	// Get the pawn's anim instance
	AFortPawn* FortPawn = Cast<AFortPawn>(OwningAbility->GetAvatarActor());
	if (!FortPawn || !FortPawn->GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_PlayMontageWaitTarget: Cannot activate - invalid pawn"));
		OnCancelled.Broadcast();
		return;
	}

	UAnimInstance* AnimInstance = FortPawn->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_PlayMontageWaitTarget: Cannot activate - no anim instance"));
		OnCancelled.Broadcast();
		return;
	}

	// Play the montage
	float Duration = AnimInstance->Montage_Play(MontageToPlay, PlayRate, EMontagePlayReturnType::MontageLength, 0.0f, true);

	if (Duration > 0.0f)
	{
		// Jump to start section if specified
		if (StartSectionName != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(StartSectionName, MontageToPlay);
		}

		UE_LOG(LogTemp, Log, TEXT("UFortAbilityTask_PlayMontageWaitTarget: Playing montage %s (duration: %.2f)"),
			*MontageToPlay->GetName(), Duration);

		// Bind to montage ended delegate
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UFortAbilityTask_PlayMontageWaitTarget::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_PlayMontageWaitTarget: Failed to play montage"));
		OnCancelled.Broadcast();
	}
}

void UFortAbilityTask_PlayMontageWaitTarget::OnTargetDataReceived(const FFortAbilityTargetSelection& TargetData)
{
	bTargetReceived = true;

	UE_LOG(LogTemp, Verbose, TEXT("UFortAbilityTask_PlayMontageWaitTarget: Target data received"));

	// Broadcast target ready
	OnTargetReady.Broadcast(TargetData);

	// If montage is also finished, complete the task
	if (bMontageFinished)
	{
		OnCompleted.Broadcast();
	}
}

void UFortAbilityTask_PlayMontageWaitTarget::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bMontageFinished = true;

	if (bInterrupted)
	{
		UE_LOG(LogTemp, Verbose, TEXT("UFortAbilityTask_PlayMontageWaitTarget: Montage interrupted"));
		OnCancelled.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("UFortAbilityTask_PlayMontageWaitTarget: Montage finished"));

		// If target is also received, complete the task
		if (bTargetReceived)
		{
			OnCompleted.Broadcast();
		}
	}
}

void UFortAbilityTask_PlayMontageWaitTarget::OnMontageCancelled()
{
	UE_LOG(LogTemp, Verbose, TEXT("UFortAbilityTask_PlayMontageWaitTarget: Montage cancelled"));
	OnCancelled.Broadcast();
}

//////////////////////////////////////////////////////////////////////////
// UFortAbilityTask_SetNextMontageSectionAndWait

UFortAbilityTask_SetNextMontageSectionAndWait::UFortAbilityTask_SetNextMontageSectionAndWait()
	: OwningAbility(nullptr)
	, SectionName(NAME_None)
{
}

UFortAbilityTask_SetNextMontageSectionAndWait* UFortAbilityTask_SetNextMontageSectionAndWait::SetNextMontageSectionAndWait(
	UFortGameplayAbility* OwningAbility,
	FName SectionName)
{
	UFortAbilityTask_SetNextMontageSectionAndWait* Task = NewObject<UFortAbilityTask_SetNextMontageSectionAndWait>();
	if (Task)
	{
		Task->OwningAbility = OwningAbility;
		Task->SectionName = SectionName;
	}

	return Task;
}

void UFortAbilityTask_SetNextMontageSectionAndWait::Activate()
{
	if (!OwningAbility || SectionName == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_SetNextMontageSectionAndWait: Cannot activate - missing ability or section name"));
		OnCompleted.Broadcast();
		return;
	}

	// Get the pawn's anim instance
	AFortPawn* FortPawn = Cast<AFortPawn>(OwningAbility->GetAvatarActor());
	if (!FortPawn || !FortPawn->GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_SetNextMontageSectionAndWait: Cannot activate - invalid pawn"));
		OnCompleted.Broadcast();
		return;
	}

	UAnimInstance* AnimInstance = FortPawn->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_SetNextMontageSectionAndWait: Cannot activate - no anim instance"));
		OnCompleted.Broadcast();
		return;
	}

	// Get currently playing montage
	UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
	if (CurrentMontage)
	{
		// Set next section
		AnimInstance->Montage_SetNextSection(AnimInstance->Montage_GetCurrentSection(CurrentMontage), SectionName, CurrentMontage);

		UE_LOG(LogTemp, Log, TEXT("UFortAbilityTask_SetNextMontageSectionAndWait: Set next section to %s"), *SectionName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_SetNextMontageSectionAndWait: No montage currently playing"));
	}

	OnCompleted.Broadcast();
}

void UFortAbilityTask_SetNextMontageSectionAndWait::OnSectionEnded()
{
	OnCompleted.Broadcast();
}
