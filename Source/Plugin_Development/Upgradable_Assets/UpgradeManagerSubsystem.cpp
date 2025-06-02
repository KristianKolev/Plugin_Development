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

bool UUpgradeManagerSubsystem::CanUpgrade(const int32 ComponentId) const
{
	return GetCurrentLevel(ComponentId) < GetMaxLevel(ComponentId);
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

int32 UUpgradeManagerSubsystem::GetResourceTypeIndex(const FName& TypeName)
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

void UUpgradeManagerSubsystem::GetNextLevelUpgradeCosts(const int32 ComponentId, TArray<int32>& ResourceCosts, TArray<FName>& Resources) const
{
	ResourceCosts.Init(-1,2);
	if (const FUpgradeLevelData* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId)))
	{
		ResourceCosts = UpgradeDefinition->UpgradeCosts;
		for (int32 ResourceIndex : UpgradeDefinition->ResourceTypeIndices)
		{
			Resources.Add(GetResourceTypeName(ResourceIndex));
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


bool UUpgradeManagerSubsystem::HandleUpgradeRequest(const int32 ComponentId, const int32 LevelIncrease)
{
	const UUpgradableComponent* Comp = GetComponentById(ComponentId);
	if (!Comp || !Comp->GetOwner()->HasAuthority()) return false;

	const int32 CurrentLevel = ComponentLevels[ComponentId];
	if (!ComponentLevels.IsValidIndex(ComponentId) || ComponentLevels[ComponentId] == -1) return false;	  // not registered
	const TArray<FUpgradeLevelData>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId);
	if (!UpgradeDefinitions || CurrentLevel >= UpgradeDefinitions->Num()-1) return false;
	
	if (!CanUpgrade(ComponentId)) return false;
	const int32 NewLevel = GetCurrentLevel(ComponentId) + LevelIncrease;   
	const FUpgradeLevelData* LevelData = &(*UpgradeDefinitions)[NewLevel];

	if (!LevelData) return false;

	// APlayerState* PS = Cast<APawn>(Comp->GetOwner())->GetPlayerState<APlayerState>();
	// if (!PS || PS->GetScore() < LevelData->UpgradeCost) return false;
	//
	//
	// PS->SetScore(PS->GetScore() - LevelData->UpgradeCost);

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
	/*const FString FilePath = FPaths::ProjectContentDir() / TEXT("Data/UpgradeLevels.json");
	FString JsonString;
	
	if (FPaths::FileExists(FilePath) && FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		InitializeProviderFromJson(JsonString);
		UE_LOG(LogTemp, Log, TEXT("UpgradeLevels.json loaded successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load UpgradeLevels.json from %s"), *FilePath);
	}*/
}

UUpgradableComponent* UUpgradeManagerSubsystem::GetComponentById(const int32 Id) const
{
	return (RegisteredComponents.IsValidIndex(Id)
			? RegisteredComponents[Id].Get()
			: nullptr);
}