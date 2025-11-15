// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Game Enumerations
// Extracted from IDA pseudo-C dumps - Complete enum definitions

#pragma once

#include "CoreMinimal.h"

/**
 * EFortItemType - All item types in Fortnite
 */
UENUM(BlueprintType)
enum class EFortItemType : uint8
{
	WorldItem = 0,
	Ammo = 1,
	Badge = 2,
	BackpackPickup = 3,
	BuildingPiece = 4,
	CharacterPart = 5,
	Deco = 6,
	EditTool = 7,
	Food = 8,
	Gadget = 9,
	HomebaseGadget = 10,
	HeroAbility = 11,
	MissionItem = 12,
	Trap = 13,
	Weapon = 14,
	WeaponMelee = 15,
	WeaponRanged = 16,
	WeaponHarvest = 17,
	WorldResource = 18,
	Outpost = 19,
	AccountItem = 20,
	AccountResource = 21,
	Alteration = 22,
	CardPack = 23,
	CharacterCosmetic = 24,
	Currency = 25,
	Hero = 26,
	MyFortBuilding = 27,
	Schematic = 28,
	Ingredient = 29,
	Trait = 30,
	Worker = 31,
	Token = 32,
	HeroSpecialization = 33,
	Quest = 34,
	Emote = 35,
	Stack = 36,
	Profile = 37,
	Max_None = 38,
	EFortItemType_MAX = 39
};

/**
 * EFortBuildingType - Building piece types
 */
UENUM(BlueprintType)
enum class EFortBuildingType : uint8
{
	Wall = 0,
	Floor = 1,
	Corner = 2,
	Deco = 3,
	Prop = 4,
	Stairs = 5,
	Roof = 6,
	Pillar = 7,
	SpawnedItem = 8,
	Container = 9,
	Trap = 10,
	GenericCenterCellActor = 11,
	None = 12,
	EFortBuildingType_MAX = 13
};

/**
 * EFortTeam - Team assignments for players and AI
 */
UENUM(BlueprintType)
enum class EFortTeam : uint8
{
	HumanCampaign = 0,		// PvE team
	Monster = 1,			// AI enemies
	HumanPvP_Team1 = 2,
	HumanPvP_Team2 = 3,
	HumanPvP_Team3 = 4,
	HumanPvP_Team4 = 5,
	HumanPvP_Team5 = 6,
	HumanPvP_Team6 = 7,
	HumanPvP_Team7 = 8,
	HumanPvP_Team8 = 9,
	HumanPvP_Team9 = 10,
	HumanPvP_Team10 = 11,
	Spectator = 12,
	MAX = 13,
	EFortTeam_MAX = 14
};

/**
 * EFortCustomGender - Character gender options
 */
UENUM(BlueprintType)
enum class EFortCustomGender : uint8
{
	Male = 0,
	Female = 1,
	Both = 2,
	EFortCustomGender_MAX = 3
};

/**
 * EFortCombatFactors - AI combat decision factors
 */
UENUM(BlueprintType)
enum class EFortCombatFactors : uint8
{
	PlayerDamageThreat = 0,
	ObjectiveDamageThreat = 1,
	ObjectivePathCost = 2,
	PlayerPathCost = 3,
	PlayerMovement = 4,
	TrapsEffective = 5,
	PlayerWander = 6,
	NearbyEnemyPresence = 7,
	OffensiveResources = 8,
	DefensiveResources = 9,
	Boredom = 10,
	ArtilleryVulnerability = 11,
	Max_None = 12,
	EFortCombatFactors_MAX = 13
};

/**
 * EFortCompletionResult - Mission/match completion status
 */
UENUM(BlueprintType)
enum class EFortCompletionResult : uint8
{
	Win = 0,
	Loss = 1,
	Draw = 2,
	Undefined = 3,
	EFortCompletionResult_MAX = 4
};

/**
 * EFortEncounterSpawnLocationPlacementMode - Spawn location placement strategies
 */
UENUM(BlueprintType)
enum class EFortEncounterSpawnLocationPlacementMode : uint8
{
	Directional = 0,		// Spawn from specific direction
	Ring = 1,				// Spawn in ring around target
	Volume = 2,				// Spawn within volume
	Custom = 3,				// Custom placement logic
	Max_None = 4,
	EFortEncounterSpawnLocationPlacementMode_MAX = 5
};

/**
 * EFortBuildingInitializationReason - Reason a building actor was created
 */
UENUM(BlueprintType)
enum class EFortBuildingInitializationReason : uint8
{
	StaticallyPlaced = 0,		// Placed in editor
	Spawned = 1,				// Dynamically spawned
	Replaced = 2,				// Replaced existing building
	LoadedFromSave = 3,			// Loaded from saved game
	DynamicBuilderPlaced = 4,	// Placed by dynamic builder
	PlacementTool = 5,			// Placed by player tool
	TrapTool = 6,				// Placed as trap
	None = 7,
	EFortBuildingInitializationReason_MAX = 8
};

/**
 * EFortMontageInputType - Animation montage input types
 */
UENUM(BlueprintType)
enum class EFortMontageInputType : uint8
{
	WindowClickOrHold = 0,
	WindowHoldOnly = 1,
	InstantClick = 2,
	EFortMontageInputType_MAX = 3
};

/**
 * EFortObjectLibrary - Object library categories for data assets
 */
UENUM(BlueprintType)
enum class EFortObjectLibrary : uint8
{
	AlterationData = 0,
	CharacterCosmeticData = 1,
	DecoData = 2,
	CardPackData = 3,
	WorldItems = 4,
	HeroSpecializationData = 5,
	QuestData = 6,
	OutpostData = 7,
	Max_None = 8,
	EFortObjectLibrary_MAX = 9
};

/**
 * EFortAIUtility - AI utility types
 */
UENUM(BlueprintType)
enum class EFortAIUtility : uint8
{
	Assassin = 0,
	MAX = 1,
	EFortAIUtility_MAX = 2
};

/**
 * EFortAIDirectorFactor - Factors influencing AI director decisions
 */
UENUM(BlueprintType)
enum class EFortAIDirectorFactor : uint8
{
	PlayerProgress = 0,
	TimeElapsed = 1,
	CombatIntensity = 2,
	EFortAIDirectorFactor_MAX = 3
};

/**
 * EFortCostInfoTypes - Cost information types for abilities/items
 */
UENUM(BlueprintType)
enum class EFortCostInfoTypes : uint8
{
	None = 0,
	Ability = 1,
	Conversion = 2,
	EFortCostInfoTypes_MAX = 3
};

/**
 * EFortReplenishmentType - Resource replenishment types
 */
UENUM(BlueprintType)
enum class EFortReplenishmentType : uint8
{
	Gradual = 0,
	Instant = 1,
	EFortReplenishmentType_MAX = 2
};

/**
 * EFortTileEdgeType - Grid tile edge types for building placement
 */
UENUM(BlueprintType)
enum class EFortTileEdgeType : uint8
{
	Undefined = 0,
	Outer_1 = 1,
	Transition_2 = 2,
	Inner_3 = 3,
	Border_4 = 4,
	BorderTransitionSingle_5 = 5,
	BorderTransitionDouble_6 = 6,
	MAX = 7,
	EFortTileEdgeType_MAX = 8
};

/**
 * EFortAbilitySourceType - Source of ability activation
 */
UENUM(BlueprintType)
enum class EFortAbilitySourceType : uint8
{
	Unknown = 0,
	Weapon = 1,
	Gadget = 2,
	Pawn = 3,
	Building = 4,
	HomeBase = 5,
	EFortAbilitySourceType_MAX = 6
};

/**
 * EFortEncounterPacingMode - AI encounter pacing strategies
 */
UENUM(BlueprintType)
enum class EFortEncounterPacingMode : uint8
{
	SpawnPointsPercentageCurve = 0,
	IntensityCurve = 1,
	Burst = 2,
	EFortEncounterPacingMode_MAX = 3
};

/**
 * EFortBangType - UI notification/badge types ("!" indicators)
 */
UENUM(BlueprintType)
enum class EFortBangType : uint8
{
	Invalid = 0,
	Custom = 1,
	PlayTab = 2,
	HeroesTab = 3,
	VaultTab = 4,
	StoreTab = 5,
	FriendsButton = 6,
	PartyInviteButton = 7,
	DailyRewardsButton = 8,
	QuestsButton = 9,
	EFortBangType_MAX = 10
};

/**
 * EFortOptionGenerationResult - Result of generating UI options
 */
UENUM(BlueprintType)
enum class EFortOptionGenerationResult : uint8
{
	NoOptionsGenerated = 0,
	NewOptionsGenerated = 1,
	ExistingOptionsGenerated = 2,
	EFortOptionGenerationResult_MAX = 3
};

/**
 * EFortQuickBars - Quick bar slot categories
 */
UENUM(BlueprintType)
enum class EFortQuickBars : uint8
{
	Primary = 0,
	Secondary = 1,
	Max_None = 2,
	EFortQuickBars_MAX = 3
};

/**
 * EFortRequestedGameplayAction - Player-requested gameplay state changes
 */
UENUM(BlueprintType)
enum class EFortRequestedGameplayAction : uint8
{
	ContinuePlaying = 0,
	StartPlaying = 1,
	StopPlaying = 2,
	EnterZone = 3,
	EFortRequestedGameplayAction_MAX = 4
};

/**
 * EFortPvPGameResult - PvP match result
 */
UENUM(BlueprintType)
enum class EFortPvPGameResult : uint8
{
	Win = 0,
	Loss = 1,
	Draw = 2,
	EFortPvPGameResult_MAX = 3
};

/**
 * EFortTemplateAccess - Template access permissions
 */
UENUM(BlueprintType)
enum class EFortTemplateAccess : uint8
{
	Normal = 0,
	Trusted = 1,
	Private = 2,
	EFortTemplateAccess_MAX = 3
};

/**
 * EFortBuildingPersistentState - Building persistence state
 */
UENUM(BlueprintType)
enum class EFortBuildingPersistentState : uint8
{
	Default = 0,
	New = 1,
	Constructed = 2,
	EFortBuildingPersistentState_MAX = 3
};

/**
 * EFortAIDirectorEvent - Events triggering AI director behavior
 */
UENUM(BlueprintType)
enum class EFortAIDirectorEvent : uint8
{
	PlayerAIEnemies = 0,
	ObjectiveTakingDamage = 1,
	PlayerUnderAttack = 2,
	EFortAIDirectorEvent_MAX = 3
};

/**
 * EFortCustomPartType - Character customization part types
 */
UENUM(BlueprintType)
enum class EFortCustomPartType : uint8
{
	Head = 0,
	Body = 1,
	Accessory = 2,
	EFortCustomPartType_MAX = 3
};

/**
 * EFortReplicatedStat - Statistics replicated to clients
 */
UENUM(BlueprintType)
enum class EFortReplicatedStat : uint8
{
	MonsterKills = 0,
	PlayerKills = 1,
	Headshots = 2,
	Assists = 3,
	Deaths = 4,
	Resources = 5,
	BuildingsBuilt = 6,
	BuildingsEdited = 7,
	BuildingsDestroyed = 8,
	EFortReplicatedStat_MAX = 9
};

/**
 * EFortObjectiveStatus - Current status of mission objectives
 */
UENUM(BlueprintType)
enum class EFortObjectiveStatus : uint8
{
	Created = 0,
	InProgress = 1,
	Succeeded = 2,
	Failed = 3,
	NeutralCompletion = 4,
	EFortObjectiveStatus_MAX = 5
};

/**
 * EFortEncounterDirection - Direction for encounter spawns
 */
UENUM(BlueprintType)
enum class EFortEncounterDirection : uint8
{
	North = 0,
	NorthEast = 1,
	East = 2,
	SouthEast = 3,
	South = 4,
	SouthWest = 5,
	West = 6,
	NorthWest = 7,
	EFortEncounterDirection_MAX = 8
};

/**
 * EFortMovementUrgency - AI movement urgency level
 */
UENUM(BlueprintType)
enum class EFortMovementUrgency : uint8
{
	None = 0,
	Low = 1,
	Medium = 2,
	High = 3,
	EFortMovementUrgency_MAX = 4
};

/**
 * EFortAreaFlag - Area flags for zone management
 */
UENUM(BlueprintType)
enum class EFortAreaFlag : uint8
{
	Objective = 0,
	Spawn = 1,
	NoBuilding = 2,
	SafeZone = 3,
	EFortAreaFlag_MAX = 4
};

/**
 * EFortFeedbackAddressee - Who should receive feedback
 */
UENUM(BlueprintType)
enum class EFortFeedbackAddressee : uint8
{
	Instigator = 0,
	Recipient = 1,
	All = 2,
	EFortFeedbackAddressee_MAX = 3
};

/**
 * EFortFeedbackContext - Context for gameplay feedback
 */
UENUM(BlueprintType)
enum class EFortFeedbackContext : uint8
{
	DamageTaken = 0,
	DamageDealt = 1,
	EnemyEliminated = 2,
	ItemPickup = 3,
	BuildingComplete = 4,
	EFortFeedbackContext_MAX = 5
};

/**
 * EFortEncounterUtilitiesMode - Encounter utilities behavior mode
 */
UENUM(BlueprintType)
enum class EFortEncounterUtilitiesMode : uint8
{
	LockedTarget = 0,
	BestTarget = 1,
	EFortEncounterUtilitiesMode_MAX = 2
};

/**
 * EFortDayPhase - Day/night cycle phases
 */
UENUM(BlueprintType)
enum class EFortDayPhase : uint8
{
	Morning = 0,
	Day = 1,
	Evening = 2,
	Night = 3,
	EFortDayPhase_MAX = 4
};
