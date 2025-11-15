// Copyright Epic Games, Inc. All Rights Reserved.
// FULL IMPLEMENTATION - RepLayout

#include "NetworkingRebuild.h"
#include "RebuiltRepLayout.h"
#include "Engine/NetConnection.h"
#include "GameFramework/Actor.h"
#include "UObject/UnrealType.h"

FRebuiltRepLayout::FRebuiltRepLayout()
	: Owner(nullptr)
	, TotalSize(0)
	, ShadowDataOffset(0)
{
}

FRebuiltRepLayout::~FRebuiltRepLayout()
{
}

void FRebuiltRepLayout::InitFromClass(UClass* InClass)
{

	if (!InClass)
	{
		return;
	}

	Owner = InClass;

	// Build replication commands
	BuildCommands(InClass);

	// Calculate shadow state size
	BuildShadowOffsets(InClass);

	UE_LOG(LogNet, Log, TEXT("RepLayout: Initialized for class %s with %d replicated properties"),
		*InClass->GetName(), Commands.Num());
}

void FRebuiltRepLayout::BuildCommands(UClass* InClass)
{

	Commands.Empty();

	// Iterate all properties in the class
	for (TFieldIterator<UProperty> It(InClass); It; ++It)
	{
		UProperty* Property = *It;

		// Check if property is replicated
		if (!(Property->PropertyFlags & CPF_Net))
		{
			continue;
		}

		// Create command for this property
		FRebuiltRepCommand Cmd;
		Cmd.Property = Property;
		Cmd.Offset = Property->GetOffset_ForInternal();
		Cmd.ElementSize = Property->ElementSize;

		// Get replication condition
		Cmd.Condition = Property->GetBlueprintReplicationCondition();

		// Check for OnRep function
		// From: 1098 OnRep_ callbacks found in dump
		if (Property->HasAnyPropertyFlags(CPF_RepNotify))
		{
			Cmd.RepNotifyFuncName = Property->RepNotifyFunc;
		}

		// Handle arrays
		UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property);
		if (ArrayProp)
		{
			// Array replication requires special handling
			Cmd.ArrayIndex = -1; // Whole array
		}

		Commands.Add(Cmd);

		UE_LOG(LogNetTraffic, VeryVerbose, TEXT("RepLayout: Added command for %s (offset %d, size %d)"),
			*Property->GetName(), Cmd.Offset, Cmd.ElementSize);
	}

	TotalSize = InClass->GetPropertiesSize();
}

void FRebuiltRepLayout::BuildShadowOffsets(const UClass* InClass)
{

	if (!InClass)
	{
		return;
	}

	// Shadow data starts after the object itself
	ShadowDataOffset = InClass->GetPropertiesSize();

	UE_LOG(LogNetTraffic, Verbose, TEXT("RepLayout: Shadow offset for %s is %d"),
		*InClass->GetName(), ShadowDataOffset);
}

bool FRebuiltRepLayout::CompareProperties(FRebuiltRepState* RepState, const void* Data) const
{

	if (!RepState || !Data)
	{
		return false;
	}

	bool bAnyChanged = false;

	// Initialize shadow state if needed
	if (RepState->ShadowData.Num() == 0)
	{
		InitShadowData(RepState, Data);
		return true; // All properties are "changed" on first replication
	}

	// Compare each property
	for (const FRebuiltRepCommand& Cmd : Commands)
	{
		const void* CurrentData = ((const uint8*)Data) + Cmd.Offset;
		const void* Shadow = RepState->ShadowData.GetData() + Cmd.Offset;

		if (CompareProperty(Cmd, CurrentData, Shadow))
		{
			bAnyChanged = true;
		}
	}

	return bAnyChanged;
}

bool FRebuiltRepLayout::CompareProperty(const FRebuiltRepCommand& Cmd, const void* Data, const void* ShadowData) const
{

	if (!Cmd.Property || !Data || !ShadowData)
	{
		return false;
	}

	// Use property's built-in comparison
	if (!Cmd.Property->Identical(Data, ShadowData))
	{
		return true; // Property has changed
	}

	return false;
}

void FRebuiltRepLayout::SendProperties(FRebuiltRepState* RepState, const void* Data, UNetConnection* Connection, FBitWriter& Writer) const
{

	if (!RepState || !Data)
	{
		return;
	}

	// Write number of properties that changed
	uint32 NumChanged = 0;
	int32 NumChangedPos = Writer.GetNumBits();
	Writer << NumChanged; // Placeholder

	// Serialize changed properties
	for (int32 i = 0; i < Commands.Num(); i++)
	{
		const FRebuiltRepCommand& Cmd = Commands[i];
		const void* CurrentData = ((const uint8*)Data) + Cmd.Offset;
		const void* Shadow = RepState->ShadowData.GetData() + Cmd.Offset;

		if (CompareProperty(Cmd, CurrentData, Shadow))
		{
			// Write property index
			Writer << i;

			// Serialize the property value
			SerializeProperty(Cmd, CurrentData, Writer);

			NumChanged++;
		}
	}

	// Go back and write actual count
	int32 EndPos = Writer.GetNumBits();
	Writer.SetNum(NumChangedPos);
	Writer << NumChanged;
	Writer.SetNum(EndPos);

	// Update shadow state
	UpdateShadowData(RepState, Data);

	UE_LOG(LogNetTraffic, VeryVerbose, TEXT("RepLayout: Sent %d changed properties (%d bits)"),
		NumChanged, Writer.GetNumBits());
}

bool FRebuiltRepLayout::ReceiveProperties(UObject* Object, FBitReader& Reader) const
{

	if (!Object)
	{
		return false;
	}

	// Read number of changed properties
	uint32 NumChanged = 0;
	Reader << NumChanged;

	if (Reader.IsError())
	{
		UE_LOG(LogNet, Error, TEXT("RepLayout: Error reading property count"));
		return false;
	}

	// Deserialize each changed property
	for (uint32 i = 0; i < NumChanged; i++)
	{
		// Read property index
		int32 PropIndex = 0;
		Reader << PropIndex;

		if (PropIndex < 0 || PropIndex >= Commands.Num())
		{
			UE_LOG(LogNet, Error, TEXT("RepLayout: Invalid property index %d"), PropIndex);
			return false;
		}

		const FRebuiltRepCommand& Cmd = Commands[PropIndex];

		// Store old value for OnRep comparison
		TArray<uint8> OldData;
		void* PropertyData = ((uint8*)Object) + Cmd.Offset;

		if (Cmd.RepNotifyFuncName != NAME_None)
		{
			// Copy old value
			OldData.SetNumUninitialized(Cmd.ElementSize);
			FMemory::Memcpy(OldData.GetData(), PropertyData, Cmd.ElementSize);
		}

		// Deserialize new value
		DeserializeProperty(Cmd, PropertyData, Reader);

		if (Reader.IsError())
		{
			UE_LOG(LogNet, Error, TEXT("RepLayout: Error deserializing property %s"),
				*Cmd.Property->GetName());
			return false;
		}

		// Call OnRep function
		CallRepNotifyFunc(Object, Cmd, OldData.GetData());
	}

	UE_LOG(LogNetTraffic, VeryVerbose, TEXT("RepLayout: Received %d changed properties"),
		NumChanged);

	return true;
}

void FRebuiltRepLayout::SerializeProperty(const FRebuiltRepCommand& Cmd, const void* Data, FBitWriter& Writer) const
{
	if (!Cmd.Property || !Data)
	{
		return;
	}

	// Use property's built-in serialization
	Cmd.Property->NetSerializeItem(Writer, nullptr, const_cast<void*>(Data));
}

void FRebuiltRepLayout::DeserializeProperty(const FRebuiltRepCommand& Cmd, void* Data, FBitReader& Reader) const
{

	if (!Cmd.Property || !Data)
	{
		return;
	}

	// Use property's built-in deserialization
	Cmd.Property->NetSerializeItem(Reader, nullptr, Data);
}

void FRebuiltRepLayout::CallRepNotifyFunc(UObject* Object, const FRebuiltRepCommand& Cmd, const void* OldData) const
{
	// From: 1098 OnRep_ callbacks found in dump

	if (!Object || Cmd.RepNotifyFuncName == NAME_None)
	{
		return;
	}

	// Find the OnRep function
	UFunction* RepNotifyFunc = Object->FindFunction(Cmd.RepNotifyFuncName);
	if (!RepNotifyFunc)
	{
		UE_LOG(LogNet, Warning, TEXT("RepLayout: OnRep function %s not found on %s"),
			*Cmd.RepNotifyFuncName.ToString(), *Object->GetName());
		return;
	}

	// Call the function
	Object->ProcessEvent(RepNotifyFunc, nullptr);

	UE_LOG(LogNetTraffic, VeryVerbose, TEXT("RepLayout: Called OnRep function %s on %s"),
		*Cmd.RepNotifyFuncName.ToString(), *Object->GetName());
}

void FRebuiltRepLayout::InitShadowData(FRebuiltRepState* RepState, const void* Data) const
{

	if (!RepState || !Data)
	{
		return;
	}

	// Allocate shadow data
	RepState->ShadowData.SetNumUninitialized(TotalSize);

	// Copy current state to shadow
	FMemory::Memcpy(RepState->ShadowData.GetData(), Data, TotalSize);
}

void FRebuiltRepLayout::UpdateShadowData(FRebuiltRepState* RepState, const void* Data) const
{

	if (!RepState || !Data)
	{
		return;
	}

	// Update shadow state with current values
	if (RepState->ShadowData.Num() != TotalSize)
	{
		RepState->ShadowData.SetNumUninitialized(TotalSize);
	}

	FMemory::Memcpy(RepState->ShadowData.GetData(), Data, TotalSize);

	// Increment replication frame counter
	RepState->NumReplicationFrames++;
}

//-----------------------------------------------------------------------------
// FRebuiltRepLayoutManager implementation
//-----------------------------------------------------------------------------

TSharedPtr<FRebuiltRepLayout> FRebuiltRepLayoutManager::GetLayout(UClass* InClass)
{

	if (!InClass)
	{
		return nullptr;
	}

	// Check cache first
	TSharedPtr<FRebuiltRepLayout>* ExistingLayout = LayoutCache.Find(InClass);
	if (ExistingLayout)
	{
		return *ExistingLayout;
	}

	// Create new layout
	TSharedPtr<FRebuiltRepLayout> NewLayout = MakeShared<FRebuiltRepLayout>();
	NewLayout->InitFromClass(InClass);

	// Cache it
	LayoutCache.Add(InClass, NewLayout);

	return NewLayout;
}
