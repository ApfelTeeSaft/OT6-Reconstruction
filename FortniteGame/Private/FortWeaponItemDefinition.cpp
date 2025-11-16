// Copyright Epic Games, Inc. All Rights Reserved.
// UFortWeaponItemDefinition Implementation

#include "FortWeaponItemDefinition.h"

//////////////////////////////////////////////////////////////////////////
// UFortWeaponItemDefinition

UFortWeaponItemDefinition::UFortWeaponItemDefinition()
	: Icon(nullptr)
	, Rarity(EFortRarity::Common)
	, WeaponActorClass(nullptr)
	, WeaponCoreAnimation(EFortWeaponCoreAnimation::Rifle)
	, TriggerType(EFortWeaponTriggerType::OnPress)
	, WeaponMesh(nullptr)
	, WeaponFireMontage(nullptr)
	, WeaponFireDownsightsMontage(nullptr)
	, WeaponReloadMontage(nullptr)
	, WeaponFireCue(nullptr)
	, WeaponReloadCue(nullptr)
	, BulletShellFX(nullptr)
	, MuzzleFlashFX(nullptr)
	, ImpactFX(nullptr)
	, TracerFX(nullptr)
	, LootedWeaponsDurabilityModifier(1.0f)
{

	// Initialize default durability by rarity
	DurabilityByRarity.Add(EFortRarity::Common, 100.0f);
	DurabilityByRarity.Add(EFortRarity::Uncommon, 150.0f);
	DurabilityByRarity.Add(EFortRarity::Rare, 200.0f);
	DurabilityByRarity.Add(EFortRarity::Epic, 300.0f);
	DurabilityByRarity.Add(EFortRarity::Legendary, 400.0f);
}

const FFortWeaponStats& UFortWeaponItemDefinition::GetWeaponStats() const
{
	return WeaponStats;
}

FFortWeaponStats UFortWeaponItemDefinition::GetWeaponStatsRow() const
{

	// Return copy of weapon stats
	return WeaponStats;
}

float UFortWeaponItemDefinition::GetWeaponDurabilityByRarity(EFortRarity InRarity) const
{

	const float* Durability = DurabilityByRarity.Find(InRarity);
	if (Durability)
	{
		return (*Durability) * LootedWeaponsDurabilityModifier;
	}

	// Default durability if rarity not found
	return 100.0f * LootedWeaponsDurabilityModifier;
}

//////////////////////////////////////////////////////////////////////////
// UFortWeaponRangedItemDefinition

UFortWeaponRangedItemDefinition::UFortWeaponRangedItemDefinition()
	: ProjectileClass(nullptr)
	, bUseProjectile(false)
	, ProjectileSpeed(10000.0f)
{

	WeaponCoreAnimation = EFortWeaponCoreAnimation::Rifle;
	TriggerType = EFortWeaponTriggerType::Automatic;

	// Set default ranged weapon actor class
	WeaponActorClass = AFortWeaponRanged::StaticClass();
}

//////////////////////////////////////////////////////////////////////////
// UFortWeaponMeleeItemDefinition

UFortWeaponMeleeItemDefinition::UFortWeaponMeleeItemDefinition()
	: SwingRadius(200.0f)
	, SwingAngle(90.0f)
	, bCanHitMultiple(true)
	, MaxTargets(5)
{

	WeaponCoreAnimation = EFortWeaponCoreAnimation::Melee;
	TriggerType = EFortWeaponTriggerType::OnPress;

	// Set default melee weapon actor class
	WeaponActorClass = AFortWeaponMelee::StaticClass();
}
