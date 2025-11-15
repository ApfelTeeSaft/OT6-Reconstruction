# Fortnite Online Test 6.5 Alpha - Decompilation Documentation

A comprehensive decompilation and documentation of Fortnite Online Test 6.5 Alpha (2016) from 32-bit binary memory dumps. This repository serves as **code documentation** for historical preservation and educational analysis of early Fortnite development.

[![UE Version](https://img.shields.io/badge/Unreal%20Engine-4.12-blue)](https://www.unrealengine.com/)
[![Game Version](https://img.shields.io/badge/Fortnite-OT%206.5%20Alpha-orange)](/)
[![Status](https://img.shields.io/badge/Status-Documentation-yellow)](/)

## ⚠️ Important Notice

**This is NOT production-ready code.** This repository contains:
- Decompiled C++ code from 32-bit binary memory dumps
- Reconstructed headers and implementations based on IDA Pro analysis
- Documentation of early Fortnite (2016) architecture and systems
- Educational material for understanding UE 4.12 game development patterns

**This code will NOT compile or run without significant work.** It exists purely for documentation and research purposes.

## 📋 Table of Contents

- [Overview](#overview)
- [Decompilation Methodology](#decompilation-methodology)
- [Architecture Documentation](#architecture-documentation)
- [System Breakdown](#system-breakdown)
- [Code Statistics](#code-statistics)
- [Research Notes](#research-notes)
- [Historical Context](#historical-context)

## 🎮 Overview

**Fortnite Online Test 6.5 Alpha** (2016) represents an early build of Fortnite's PvE mode (Save the World) before the Battle Royale mode existed. This decompilation documents the core gameplay systems as they existed during closed alpha testing.

### Source Material
- **Binary**: 32-bit Windows executable (.exe)
- **Engine**: Unreal Engine 4.12
- **Decompilation Tools**: IDA Pro, Hex-Rays decompiler
- **Analysis Period**: 2025
- **Original Build Date**: ~2016

### What This Repository Contains
```
✓ Reconstructed class hierarchies
✓ Function signatures and logic flows
✓ Network replication patterns
✓ Enum definitions and data structures
✓ Debug strings and log references
✓ Game system architecture documentation

✗ Original Epic Games source code
✗ Compilable project files
✗ Game assets or content
✗ Executable binaries
```

## 🔬 Decompilation Methodology

### Analysis Process

1. **Binary Extraction**
   - Loaded 32-bit .exe into IDA Pro 9
   - Identified Unreal Engine 4.12 patterns and vtables
   - Located RTTI (Runtime Type Information) structures

2. **Symbol Recovery**
   - Extracted class names from UObject type system
   - Recovered function names from debug strings
   - Identified properties via reflection metadata

3. **Code Reconstruction**
   - Converted IDA pseudo-C to valid C++ syntax
   - Inferred parameter types from usage patterns
   - Reconstructed header files from class definitions

4. **Validation**
   - Cross-referenced with UE 4.12 documentation
   - Verified network replication patterns
   - Confirmed function signatures via debug logs

### Debug String Examples

Original debug strings found in binary:
```
"UNetConnection::SendRawBunch. ChIndex: %d. Bits: %d. PacketId: %d"
"UNetConnection::Tick: Connection TIMED OUT. Closing connection..."
"AFortPlayerState::InitializeHero. FortPC: %s, PlayerState: %s, HeroId: %s"
"Building Actor Health: %.2f. %.2f%% of %.2f."
```

These strings were instrumental in reconstructing function logic and variable names.

## 🏗️ Architecture Documentation

### Core Systems

#### 1. Building System (`BuildingActor.h/cpp`)
```
ABuildingActor (Base Class)
├── ABuildingSMActor (Static Mesh Buildings)
│   ├── ABuildingSMActorWall
│   ├── ABuildingSMActorFloor
│   ├── ABuildingSMActorStairs
│   ├── ABuildingSMActorRoof
│   ├── ABuildingSMActorPillar
│   ├── ABuildingSMActorTrap
│   ├── ABuildingSMActorContainer
│   └── ABuildingSMActorSpawnedItem
└── AStrategicBuildingActor (Mission Objectives)
```

**Key Features Documented:**
- Grid-based placement system
- Material types (Wood/Stone/Metal) with health multipliers
- Building editing and repair systems
- Resource spawning on destruction
- LOD and visual effects integration

#### 2. AI Director (`FortAIDirector.h/cpp`)

**Decompiled Systems:**
- `UFortAIEncounterInfo` - Encounter state management
- `UFortAISpawnGroup` - Enemy group compositions
- `UFortIntensityCurveSequenceProgression` - Difficulty curves
- `AFortAIDirector` - Master AI controller

**Documented Patterns:**
```cpp
// Intensity calculation (reconstructed from assembly)
float DesiredIntensity = SampleIntensityCurve(EncounterTime);
float CurrentIntensity = CalculateFromActiveAI();
float IntensityError = DesiredIntensity - CurrentIntensity;

// Spawn decision based on error
if (IntensityError > Threshold) {
    SpawnMoreEnemies();
}
```

#### 3. Networking Subsystem (`NetworkingRebuild/`)

**Custom Implementation Details:**
- **RepLayout System** - Property change detection via shadow state
- **PackageMapClient** - Object GUID assignment and resolution
- **Custom Channels** - Actor, Control, Voice data transmission
- **Bandwidth Optimization** - Delta compression and property filtering

**Network Architecture:**
```
URebuiltNetDriver
├── URebuiltNetConnection (per client)
│   ├── URebuiltActorChannel (per replicated actor)
│   ├── URebuiltControlChannel (connection control)
│   └── URebuiltVoiceChannel (voice chat)
└── URebuiltPackageMapClient (GUID management)
```

#### 4. Ability System (`FortAbilitySystem.h/cpp`)

**Notable:** This is a **native UE 4.12 implementation**, not Epic's GameplayAbilities plugin (which came later).

```cpp
// Documented ability execution flow
UFortAbilitySystemComponent::TryActivateAbility()
├── CheckCost() // Energy/resource validation
├── ApplyCost() // Deduct resources
├── ActivateAbility() // Execute ability logic
└── CallRepNotifyFunc() // Network notification
```

## 📊 Code Statistics

### Functions Decompiled

| System | Functions | Lines | Confidence |
|--------|-----------|-------|------------|
| Building System | 47 | ~2,100 | HIGH |
| AI Director | 38 | ~1,800 | MEDIUM-HIGH |
| Networking | 62 | ~2,500 | HIGH |
| Ability System | 31 | ~1,400 | MEDIUM |
| Weapons | 28 | ~1,200 | HIGH |
| Inventory | 24 | ~1,000 | MEDIUM |
| **Total** | **230+** | **~10,000** | - |

### Network Patterns Found

```cpp
// RPC Distribution (from function name analysis)
Server RPCs:     28 functions (ServerActivateAbility, ServerAddItemInternal, etc.)
Client RPCs:     59 functions (ClientNotifyAbilityActivated, etc.)
Multicast RPCs:  27 functions (NetMulticast_*, broadcast events)
OnRep Callbacks: 1,098 functions (property replication notifications)
```

### Enumeration Coverage

- **EFortItemType**: 39 item types documented
- **EFortBuildingType**: 13 building types
- **EFortTeam**: 14 team assignments
- **EFortDayPhase**: 4 time-of-day phases
- **40+ additional enums** fully reconstructed

## 🔍 Research Notes

### Binary Analysis Artifacts

#### String References
- **145,000+ debug strings** extracted
- **1,098 OnRep_** function callbacks identified
- **600+ log categories** documented

#### Memory Layout
```
.text section:   ~45 MB (executable code)
.rdata section:  ~12 MB (read-only data, strings)
.data section:   ~8 MB (initialized data)
RTTI metadata:   ~2 MB (class information)
```

## 🎯 System Breakdown

### Building System Deep Dive

#### Health Calculation Formula (Decompiled)
```cpp
// From ABuildingSMActor::DetermineHealthMax()
float BaseHealth = GetMaterialBaseHealth(); // Wood: 200, Stone: 400, Metal: 600

// Level scaling
AFortGameStateZone* GameState = GetGameState();
int32 RecommendedLevel = GameState->GetZoneDifficultyInfo().RecommendedPlayerLevel;
float LevelModifier = 1.0f + (RecommendedLevel * HealthModifierPerLevel);

return BaseHealth * LevelModifier;
```

#### Resource Spawning (Documented Pattern)
```cpp
// From ABuildingSMActor::AttemptSpawnResources()
// Resources awarded on destruction: ~60% of build cost

WallCost = 10;
FloorCost = 10;
StairsCost = 10;

WallReturn = 6;  // 60% return rate
FloorReturn = 5;
StairsReturn = 7;
```

### AI Director Analysis

#### Intensity Curve System
```cpp
// Decompiled from UFortAIEncounterInfo::UpdateEncounterIntensity()

// Sample intensity from curve table at current encounter time
float EncounterTime = GetWorld()->GetTimeSeconds() - EncounterStartTime;
UCurveTable* IntensityCurve = GetIntensityCurveForLevel(CurrentLevel);
float DesiredIntensity = IntensityCurve->Eval(EncounterTime); // 0.0 to 1.0

// Calculate current intensity from active AI
int32 ActiveAI = CountActiveEnemies();
float CurrentIntensity = ActiveAI / MaxPossibleAI;

// Error correction drives spawning
if (DesiredIntensity > CurrentIntensity + 0.1f) {
    SpawnAdditionalWave();
}
```

#### Mutator System (Reconstructed)
```cpp
// From AFortAIDirector::SpawnAIGroupWithMutator()

if (MutatorName == "Husky") {
    HealthMultiplier = 2.0f;
    DamageMultiplier = 1.5f;
    SpeedMultiplier = 0.8f;
}
else if (MutatorName == "Mini") {
    HealthMultiplier = 0.5f;
    DamageMultiplier = 0.75f;
    SpeedMultiplier = 1.5f;
}
// ... etc for Elemental, Frenzied, Chrome
```

### Networking Internals

#### RepLayout Property Comparison
```cpp
// From FRebuiltRepLayout::CompareProperties()

// Shadow state comparison (how UE4 detects "dirty" properties)
for (const FRebuiltRepCommand& Cmd : Commands) {
    void* CurrentData = ObjectBase + Cmd.Offset;
    void* ShadowData = RepState->ShadowData + Cmd.Offset;
    
    if (!Cmd.Property->Identical(CurrentData, ShadowData)) {
        // Property changed, queue for replication
        MarkPropertyDirty(Cmd);
    }
}
```

#### NetGUID Assignment Pattern
```cpp
// From URebuiltPackageMapClient::AssignNewNetGUID()

FNetworkGUID NewGUID;
NewGUID.Value = NextNetGUID++; // Incremental GUID assignment

// Dynamic vs Static flag
if (Actor->IsNetStartupActor()) {
    NewGUID.bIsDynamic = false; // Placed in level
} else {
    NewGUID.bIsDynamic = true;  // Runtime spawned
}
```

## 📚 Historical Context

### Fortnite Development Timeline

- **2011**: Initial Fortnite announcement (as "Fortnite")
- **2014**: Announced as free-to-play co-op game
- **2015**: Closed Alpha testing begins
- **2016**: Online Test 6.5 Alpha (this build)
- **2017**: Save the World Early Access (July)
- **2017**: Battle Royale mode released (September)
- **2018**: Battle Royale dominates, StW development slows

### Technical Evolution

**UE 4.12 (2016) → UE 4.26+ (2020)**
- Custom ability system → GameplayAbilities plugin
- Manual replication → Enhanced GAS replication
- Custom networking → Standard NetDriver improvements
- 32-bit builds → 64-bit only

### Alpha vs Release Differences

| Feature | OT 6.5 Alpha (2016) | Release (2017+) |
|---------|---------------------|-----------------|
| Building Grid | Fixed grid | Smoother placement |
| AI Director | Intensity curves | More sophisticated |
| Abilities | Native system | GameplayAbilities |
| Networking | Custom | Optimized standard |
| Team Size | 4 players | 4 players (StW) |

## ⚖️ Legal & Ethical Considerations

### What This Is
- **Academic research** - Documentation of software archaeology
- **Historical preservation** - Recording early game development practices
- **Educational resource** - Learning UE4 networking and game systems

### What This Is NOT
- Not a leak of Epic Games source code
- Not intended for commercial use
- Not a functional game or mod
- Not encouraging piracy or ToS violations

### Usage Guidelines
✅ **Allowed:**
- Academic research and citation
- Learning game development patterns
- Historical documentation and archival
- Non-commercial educational use

❌ **Not Allowed:**
- Commercial use of any kind
- Claiming as original work
- Redistributing Epic Games assets

## 🤝 Contributing

This is primarily a **documentation project**. Contributions welcome for:

- **Improved documentation** - Clarifying complex systems
- **Cross-referencing** - Linking to UE4 documentation
- **Error corrections** - Fixing decompilation mistakes
- **Additional analysis** - Deeper dives into specific systems

**Not accepting:**
- Requests to make code compilable
- Asset files or game content

## 📖 Citation

If you use this research in academic work:

```bibtex
@misc{fortnite_ot65_decompilation,
  author = {ApfelTeeSaft},
  title = {Fortnite Online Test 6.5 Alpha - Decompilation Documentation},
  year = {2026},
  publisher = {GitHub},
  url = {https://github.com/ApfelTeeSaft/OT6-Reconstruction}
}
```

## 📜 License & Disclaimer

### Code Documentation License
The **documentation, analysis, and commentary** in this repository are provided under MIT License for educational purposes.

### Original Code Copyright
All reconstructed Fortnite code is **©2016 Epic Games, Inc.** This repository contains NO original Epic Games source code - only decompiled/reconstructed documentation.

### Trademarks
- Fortnite® is a registered trademark of Epic Games, Inc.
- Unreal® Engine is a registered trademark of Epic Games, Inc.
- All trademarks belong to their respective owners.

### Disclaimer
```
This project is for EDUCATIONAL and RESEARCH purposes only.
Not affiliated with, endorsed by, or connected to Epic Games.
Do not use this for commercial purposes or to violate Epic's Terms of Service.
The author is not responsible for misuse of this information.
```

## 🙏 Acknowledgments

- **Epic Games** - For creating Fortnite and Unreal Engine
- **IDA Pro / Hex-Rays** - Decompilation tools
- **UE4 Community** - Documentation and reverse engineering knowledge
- **Fortnite Alpha Testers** - Original community from 2016

*Last Updated: 2022 | Fortnite OT 6.5 Alpha (2016) | Unreal Engine 4.12*

**Remember:** This is historical documentation of a 2016 alpha build, not production code. Use responsibly for learning and research only.