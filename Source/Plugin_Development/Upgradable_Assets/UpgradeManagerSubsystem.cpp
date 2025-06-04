// Fill out your copyright notice in the Description page of Project Settings.
#include "UpgradeManagerSubsystem.h"
#include "UpgradableComponent.h"
#include "UpgradeJsonProvider.h"
#include "UpgradeSettings.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "Windows/WindowsApplication.h"


UUpgradeManagerSubsystem::UUpgradeManagerSubsystem()
{
}

void UUpgradeManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadJsonFromFile();
}

void UUpgradeManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UUpgradeManagerSubsystem::InitializeProviderFromJson(const FString& Json)
{
	Provider = NewObject<UUpgradeJsonProvider>(this);
	Provider->InitializeFromJson(Json, UpgradeCatalog, ResourceTypes);
}

UUpgradableComponent* UUpgradeManagerSubsystem::GetComponentById(const int32 Id) const
{
	return (RegisteredComponents.IsValidIndex(Id)
			? RegisteredComponents[Id].Get()
			: nullptr);
}

int32 UUpgradeManagerSubsystem::GetCurrentLevel(const int32 ComponentId) const
{
	if (ComponentLevels.IsValidIndex(ComponentId) && ComponentLevels[ComponentId] != -1)
	{
		return ComponentLevels[ComponentId];
	}
	return -1;
}
int32 UUpgradeManagerSubsystem::GetNextLevel(const int32 ComponentId) const
{
	if (ComponentLevels.IsValidIndex(ComponentId) && ComponentLevels[ComponentId] != -1)
	{
		
		return ComponentLevels[ComponentId] + 1;
	}
	return -1;
}

int32 UUpgradeManagerSubsystem::GetMaxLevel(const int32 ComponentId) const
{
	return GetUpgradeDefinitions(ComponentId)->Num()-1;
}

int32 UUpgradeManagerSubsystem::GetResourceTypeIndex(const FName& TypeName) const
{
	int32 FoundIndex = ResourceTypes.IndexOfByKey(TypeName);
	if (FoundIndex != INDEX_NONE)
	{
		return FoundIndex;
	}
	return -1;
}

FName UUpgradeManagerSubsystem::GetResourceTypeName(const int32 Index) const
{
	return ResourceTypes.IsValidIndex(Index)
			? ResourceTypes[Index]
			: NAME_None;
}

void UUpgradeManagerSubsystem::GetNextLevelUpgradeCosts(const int32 ComponentId, TMap<FName, int32>& ResourceCosts) const
{
	if (const FUpgradeLevelData* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId)))
	{
		for (int32 i = 0; i < UpgradeDefinition->ResourceTypeIndices.Num(); ++i)
		{
			ResourceCosts.Add(GetResourceTypeName(UpgradeDefinition->ResourceTypeIndices[i]), UpgradeDefinition->UpgradeCosts[i]);			
		}
	}
}

int32 UUpgradeManagerSubsystem::GetNextLevelUpgradeTime(const int32 ComponentId) const
{
	int32 SecondsForUpgrade = -1;
	if (const FUpgradeLevelData* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId)))
	{
		SecondsForUpgrade = UpgradeDefinition->UpgradeSeconds;
	}
	return SecondsForUpgrade;
}

const TArray<FUpgradeLevelData>* UUpgradeManagerSubsystem::GetUpgradeDefinitions(FName UpgradePathId) const
{
	return UpgradeCatalog.Find(UpgradePathId);
}

const TArray<FUpgradeLevelData>* UUpgradeManagerSubsystem::GetUpgradeDefinitions(int32 ComponentId) const
{
	const UUpgradableComponent* Comp = GetComponentById(ComponentId);
	if (!Comp) return nullptr;
	return UpgradeCatalog.Find(Comp->UpgradePathId);
}

const FUpgradeLevelData* UUpgradeManagerSubsystem::GetUpgradeDefinitionForLevel(int32 ComponentId, int32 Level) const
{
	const TArray<FUpgradeLevelData>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId);
	if (!UpgradeDefinitions || Level < 0 || Level > UpgradeDefinitions->Num()-1 ) return nullptr;
	return &(*UpgradeDefinitions)[Level];
}

int32 UUpgradeManagerSubsystem::RegisterUpgradableComponent(UUpgradableComponent* Component)
{
	int32 Id;
	if (FreeIndices.Num() > 0)
	{
		// Reuse the last hole
		Id = FreeIndices.Pop(/*bAllowShrinking=*/false);
		RegisteredComponents[Id] = Component;
		ComponentLevels[Id] = Component->InitialLevel;
	}
	else
	{
		// No holes—grow the array
		Id = RegisteredComponents.Add(Component);
		ComponentLevels.Add(Component->InitialLevel);
	}

	return Id;
}

void UUpgradeManagerSubsystem::UpdateUpgradeLevel(const int32 ComponentId, const int32 NewLevel)
{
	if (UUpgradableComponent* Comp = GetComponentById(ComponentId))
	{
		ComponentLevels[ComponentId] = NewLevel;
		Comp->Client_SetLevel(NewLevel);
	}
}

void UUpgradeManagerSubsystem::UnregisterUpgradableComponent(const int32 ComponentId)
{
	if (RegisteredComponents.IsValidIndex(ComponentId))
	{
		RegisteredComponents[ComponentId].Reset();    // Clear the weak ptr
		FreeIndices.Add(ComponentId);                // Remember this slot as a hole
		ComponentLevels[ComponentId] = -1;           // Mark as unused 
	}
}

void UUpgradeManagerSubsystem::LoadCatalog()
{
}

bool UUpgradeManagerSubsystem::CanUpgrade(const int32 ComponentId, const int32 LevelIncrease, const TMap<FName, int32>& AvailableResources) const
{
	bool Success = false;
	
	if (!GetComponentById(ComponentId)) return false;
	if (LevelIncrease <= 0) return false;
	// Trying to upgrade to a level higher than the max level
	if (GetCurrentLevel(ComponentId) + LevelIncrease > GetMaxLevel(ComponentId)) return false;

	if (const TArray<FUpgradeLevelData>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId))
	{
		const FUpgradeLevelData* LevelData = &(*UpgradeDefinitions)[GetCurrentLevel(ComponentId)];
		// Future implementation idea: Add upgrade queue to chain multiple upgrades
		if (LevelData->bUpgrading) return false;
		TMap<FName, int32> TotalResourceCosts;
		// Iterate over all levels if trying to upgrade several levels at once
		for (int32 i = GetCurrentLevel(ComponentId); i < GetCurrentLevel(ComponentId) + LevelIncrease; ++i)
		{
			LevelData = &(*UpgradeDefinitions)[i];
			if (LevelData->bUpgradeLocked) return false;
			// Add up the required resource cost for each resource for this level
			for (int32 j = 0; j < LevelData->ResourceTypeIndices.Num(); ++j)
			{
				const FName ResourceType = GetResourceTypeName(LevelData->ResourceTypeIndices[j]);
				// no resource of the required type was provided
				if (!AvailableResources.Find(ResourceType)) return false;
				
				TotalResourceCosts.FindOrAdd(ResourceType) += LevelData->UpgradeCosts[j];
			}
		}
		TArray<FName> RequiredResources;
		TotalResourceCosts.GetKeys(RequiredResources);
		for (FName ResourceType : RequiredResources)
		{
			// not enough resources of the required type
			if (TotalResourceCosts[ResourceType] > AvailableResources[ResourceType]) return false;
		}
		
		Success = true;
	}
	return Success;
}

bool UUpgradeManagerSubsystem::HandleUpgradeRequest(const int32 ComponentId, const int32 LevelIncrease, const TMap<FName, int32>& AvailableResources)
{
	const UUpgradableComponent* Comp = GetComponentById(ComponentId);
	if (!Comp || !Comp->GetOwner()->HasAuthority()) return false;

	const int32 CurrentLevel = ComponentLevels[ComponentId];
	if (!ComponentLevels.IsValidIndex(ComponentId) || ComponentLevels[ComponentId] == -1) return false;	  // not registered
	const TArray<FUpgradeLevelData>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId);
	if (!UpgradeDefinitions || CurrentLevel >= UpgradeDefinitions->Num()-1) return false;
	
	if (!CanUpgrade(ComponentId, LevelIncrease, AvailableResources)) return false;
	const int32 NewLevel = GetCurrentLevel(ComponentId) + LevelIncrease;   
	
	UpdateUpgradeLevel(ComponentId, NewLevel);
	return true;
}

void UUpgradeManagerSubsystem::LoadJsonFromFile()
{

    const UUpgradeSettings* Settings = GetDefault<UUpgradeSettings>();
    const FString Dir = FPaths::ProjectContentDir() / Settings->JsonDirectory;
    TArray<FString> Files;
    IFileManager::Get().FindFilesRecursive(Files, *Dir, TEXT("*.json"), true, false);
    UpgradeCatalog.Empty();

	for (const FString& FilePath : Files)
    {
	    InitializeProviderFromJson(FilePath);
    }
}

