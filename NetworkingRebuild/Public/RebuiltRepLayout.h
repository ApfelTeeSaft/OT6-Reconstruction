// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/RepLayout.h"
#include "RebuiltRepLayout.generated.h"

/**
 * Property change detection and replication layout
 */

/** Property replication command */
USTRUCT()
struct FRebuiltRepCommand
{
	GENERATED_BODY()

	/** Property to replicate */
	UPROPERTY()
	UProperty* Property;

	/** Offset in the object */
	UPROPERTY()
	int32 Offset;

	/** Size of the property */
	UPROPERTY()
	int32 ElementSize;

	/** Array index (-1 for non-array) */
	UPROPERTY()
	int32 ArrayIndex;

	/** Replication condition */
	UPROPERTY()
	ELifetimeCondition Condition;

	/** OnRep function name (if any) */
	UPROPERTY()
	FName RepNotifyFuncName;

	FRebuiltRepCommand()
		: Property(nullptr)
		, Offset(0)
		, ElementSize(0)
		, ArrayIndex(-1)
		, Condition(COND_None)
		, RepNotifyFuncName(NAME_None)
	{
	}
};

/**
 * Shadow state for property change detection
 * Stores last replicated values to detect changes
 */
struct FRebuiltRepState
{
	/** Buffer containing shadow state */
	TArray<uint8> ShadowData;

	/** History of replicated properties */
	TArray<int32> HistoryStart;
	TArray<int32> HistoryEnd;

	/** Number of times replicated */
	int32 NumReplicationFrames;

	/** Shared change tracker */
	TSharedPtr<struct FRepChangedPropertyTracker> RepChangedPropertyTracker;

	FRebuiltRepState()
		: NumReplicationFrames(0)
	{
	}

	/** Copy data to shadow state */
	void CopyDataToShadowState(const uint8* Source, int32 Size)
	{
		ShadowData.SetNumUninitialized(Size);
		FMemory::Memcpy(ShadowData.GetData(), Source, Size);
	}

	/** Compare current data with shadow state */
	bool HasDataChanged(const uint8* Current, int32 Size) const
	{
		if (ShadowData.Num() != Size)
		{
			return true;
		}

		return FMemory::Memcmp(ShadowData.GetData(), Current, Size) != 0;
	}
};

/**
 * Replication layout for a class
 * Manages property change detection and serialization
 */
class NETWORKINGREBUILD_API FRebuiltRepLayout
{
public:
	FRebuiltRepLayout();
	~FRebuiltRepLayout();

	/**
	 * Initialize layout for a class
	 * Scans class for replicated properties and builds command list
	 */
	void InitFromClass(UClass* InClass);

	/**
	 * Build shadow offsets for change detection
	 * Creates shadow state buffers for storing last replicated values
	 */
	void BuildShadowOffsets(const UClass* InClass);

	/**
	 * Compare properties and detect changes
	 */
	bool CompareProperties(FRebuiltRepState* RepState, const void* Data) const;

	/**
	 * Send properties over network
	 * Serializes changed properties into bit stream
	 */
	void SendProperties(FRebuiltRepState* RepState, const void* Data, UNetConnection* Connection, FBitWriter& Writer) const;

	/**
	 * Receive properties from network
	 * Deserializes properties from bit stream and calls OnRep functions
	 */
	bool ReceiveProperties(UObject* Object, FBitReader& Reader) const;

	/**
	 * Initialize shadow state
	 * Creates initial shadow state for an object
	 */
	void InitShadowData(FRebuiltRepState* RepState, const void* Data) const;

	/**
	 * Update shadow state after replication
	 * Copies current values to shadow state
	 */
	void UpdateShadowData(FRebuiltRepState* RepState, const void* Data) const;

	/** Get the class this layout is for */
	UClass* GetOwner() const { return Owner; }

	/** Get total number of properties */
	int32 GetNumProperties() const { return Commands.Num(); }

protected:
	/**
	 * Build replication commands for a class
	 * Scans all replicated properties and creates commands
	 */
	void BuildCommands(UClass* InClass);

	/**
	 * Compare a single property
	 * Handles different property types (primitives, structs, arrays, etc.)
	 */
	bool CompareProperty(const FRebuiltRepCommand& Cmd, const void* Data, const void* ShadowData) const;

	/**
	 * Serialize a property
	 * Write property value to bit stream
	 */
	void SerializeProperty(const FRebuiltRepCommand& Cmd, const void* Data, FBitWriter& Writer) const;

	/**
	 * Deserialize a property
	 * Read property value from bit stream
	 */
	void DeserializeProperty(const FRebuiltRepCommand& Cmd, void* Data, FBitReader& Reader) const;

	/**
	 * Call OnRep function if needed
	 * Invokes replication notification callback
	 */
	void CallRepNotifyFunc(UObject* Object, const FRebuiltRepCommand& Cmd, const void* OldData) const;

private:
	/** Class this layout is for */
	UClass* Owner;

	/** List of replication commands */
	TArray<FRebuiltRepCommand> Commands;

	/** Total size of replicated data */
	int32 TotalSize;

	/** Shadow state offset */
	int32 ShadowDataOffset;
};

/**
 * Manager for replication layouts
 * Caches layouts per class
 */
class NETWORKINGREBUILD_API FRebuiltRepLayoutManager
{
public:
	/**
	 * Get or create layout for a class
	 */
	TSharedPtr<FRebuiltRepLayout> GetLayout(UClass* InClass);

private:
	/** Cache of layouts by class */
	TMap<UClass*, TSharedPtr<FRebuiltRepLayout>> LayoutCache;
};
