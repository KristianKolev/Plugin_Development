// Fill out your copyright notice in the Description page of Project Settings.
#include "UpgradeManagerSubsystem.h"
#include "UpgradableComponent.h"
#include "UpgradeDataAssetProvider.h"
#include "UpgradeDataTableProvider.h"
#include "UpgradeJsonProvider.h"
#include "UpgradeSettings.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Windows/WindowsApplication.h"


UUpgradeManagerSubsystem::UUpgradeManagerSubsystem()
{
}

void UUpgradeManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUpgradeManagerSubsystem::Deinitialize()
{
	DataProvider = nullptr;
	Super::Deinitialize();
}

void UUpgradeManagerSubsystem::InitializeProvider()
{
	const UUpgradeSettings* Settings = GetDefault<UUpgradeSettings>();
	switch (Settings->CatalogSource)
	{
	case EUpgradeCatalogSource::JsonFolder:
		DataProvider = NewObject<UUpgradeJsonProvider>(this);
		UpgradeDataFolderPath = FPaths::ProjectContentDir() / Settings->JsonFolderPath;
		break;
	case EUpgradeCatalogSource::DataTableFolder:
		DataProvider = NewObject<UUpgradeDataTableProvider>(this);
		UpgradeDataFolderPath = Settings->DataTableFolderPath;
		break;
	case EUpgradeCatalogSource::DataAssetFolder:
		DataProvider = NewObject<UUpgradeDataAssetProvider>(this);
		UpgradeDataFolderPath = Settings->DataAssetFolderPath;
		break;
	default:
		DataProvider = nullptr;
		UpgradeDataFolderPath = TEXT("INVALID PATH");
		UE_LOG(LogTemp, Warning, TEXT("[UPGRADECATALOG_ERR_01] Invalid catalog source"));
		break;
	}
}

void UUpgradeManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	InitializeProvider();
	LoadCatalog();
}

void UUpgradeManagerSubsystem::LoadCatalog()
{
	if (!DataProvider)	return;
	UpgradeCatalog.Empty();
	DataProvider->InitializeData(UpgradeDataFolderPath, UpgradeCatalog, ResourceTypes);
}

int32 UUpgradeManagerSubsystem::RegisterUpgradableComponent(UUpgradableComponent* Component)
{
	int32 Id;
	if (FreeComponentIndices.Num() > 0)
	{
		// Reuse the last hole
		Id = FreeComponentIndices.Pop(/*bAllowShrinking=*/false);
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
		if (IsUpgradeTimerActive(ComponentId))
		{
			CancelUpgrade(ComponentId);
		}
		RegisteredComponents[ComponentId].Reset();    // Clear the weak ptr
		FreeComponentIndices.Add(ComponentId);                // Remember this slot as a hole
		ComponentLevels[ComponentId] = -1;           // Mark as unused 
	}
}

float UUpgradeManagerSubsystem::GetUpgradeTimerDuration(int32 ComponentId, int32 LevelIncrease) const
{
	float UpgradeTime = 0.0f;
	for ( int32 i = 0; i < LevelIncrease; ++i )
	{
		const FUpgradeDefinition* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId) + i);
		if (UpgradeDefinition)
		{
			UpgradeTime += UpgradeDefinition->UpgradeSeconds;
		}
	}
	return UpgradeTime;
}

float UUpgradeManagerSubsystem::StartUpgradeTimer(int32 ComponentId, float TimerDuration)
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("OnUpgradeTimerFinished"), ComponentId);
	FTimerHandle& TimerHandle = UpgradeInProgressData.FindOrAdd(ComponentId).UpgradeTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, TimerDuration, false);
	
	if (UUpgradableComponent* Comp = GetComponentById(ComponentId))
	{
		Comp->OnUpgradeStarted.Broadcast(TimerDuration);
	}
	return TimerDuration;
}

void UUpgradeManagerSubsystem::StopUpgradeTimer(int32 ComponentId)
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		GetWorld()->GetTimerManager().ClearTimer(Handle);
	}
}

void UUpgradeManagerSubsystem::CancelUpgrade(int32 ComponentId)
{
	UUpgradableComponent* Comp = GetComponentById(ComponentId);
	if (Comp && IsUpgradeTimerActive(ComponentId))
	{
		StopUpgradeTimer(ComponentId);
		UpgradeInProgressData.Remove(ComponentId);
		Comp->OnUpgradeCanceled.Broadcast(GetCurrentLevel(ComponentId));
	}
}

void UUpgradeManagerSubsystem::OnUpgradeTimerFinished(int32 ComponentId)
{
	StopUpgradeTimer(ComponentId);

	if (!GetComponentById(ComponentId)) return;

	const int32 NewLevel = GetCurrentLevel(ComponentId) + UpgradeInProgressData[ComponentId].RequestedLevelIncrease;   
	UpgradeInProgressData.Remove(ComponentId);
	
	UpdateUpgradeLevel(ComponentId, NewLevel);
}

float UUpgradeManagerSubsystem::UpdateUpgradeTimer(int32 ComponentId, float DeltaTime)
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		float TimeRemaining = TimerManager.GetTimerRemaining(Handle);
		float NewTimeRemaining = FMath::Max(0.f, FMath::FloorToInt(TimeRemaining + DeltaTime));
		
		if (UUpgradableComponent* Comp = GetComponentById(ComponentId))
		{
			Comp->OnTimeToUpgradeChanged.Broadcast(DeltaTime);
		}
		if (NewTimeRemaining > 0.f)
		{
			StopUpgradeTimer(ComponentId);
			StartUpgradeTimer(ComponentId, NewTimeRemaining);
			return NewTimeRemaining;
		}
		else
		{
			OnUpgradeTimerFinished(ComponentId);
			return -1.f;
		}

	}
	return -1.f;
}

float UUpgradeManagerSubsystem::GetUpgradeTimeRemaining(int32 ComponentId) const
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		const FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		return GetWorld()->GetTimerManager().GetTimerRemaining(Handle);
	}
	return -1.f;
}

int32 UUpgradeManagerSubsystem::GetRequestedLevelIncrease(int32 ComponentId) const
{
	if ( UpgradeInProgressData.Contains(ComponentId))
	{
		return UpgradeInProgressData[ComponentId].RequestedLevelIncrease;
	}
	return -1;
}


UUpgradableComponent* UUpgradeManagerSubsystem::GetComponentById(const int32 Id) const
{
	return (RegisteredComponents.IsValidIndex(Id)
			? RegisteredComponents[Id].Get()
			: nullptr);
}

UUpgradableComponent* UUpgradeManagerSubsystem::FindComponentOnActorByAspect(AActor* TargetActor, EUpgradableAspect Aspect) const
{
	if (!TargetActor)
		return nullptr;

	TArray<UUpgradableComponent*> Comps;
	TargetActor->GetComponents<UUpgradableComponent>(Comps);
	for (UUpgradableComponent* Comp : Comps)
	{
		if (Comp && Comp->GetUpgradableAspect() == Aspect)
		{
			return Comp;
		}
	}
	return nullptr;
}

UUpgradableComponent* UUpgradeManagerSubsystem::FindComponentOnActorByCategory(AActor* TargetActor,
                                                                               EUpgradableCategory Category) const
{
	if (!TargetActor)
		return nullptr;

	TArray<UUpgradableComponent*> Comps;
	TargetActor->GetComponents<UUpgradableComponent>(Comps);
	for (UUpgradableComponent* Comp : Comps)
	{
		if (Comp && Comp->GetUpgradableCategory() == Category)
		{
			return Comp;
		}
	}
	return nullptr;
}

bool UUpgradeManagerSubsystem::RequestUpgradeForActor(AActor* TargetActor,
													  EUpgradableAspect Aspect,
													  int32 LevelIncrease,
													  const TArray<FName>& ResourceTypesArray,
													  const TArray<int32>& ResourceAmounts)
{
	if (!TargetActor)
		return false;

	UUpgradableComponent* Comp = FindComponentOnActorByAspect(TargetActor, Aspect);
	if (!Comp)
		return false;

	Comp->RequestUpgrade(LevelIncrease, ResourceTypesArray, ResourceAmounts);
	return true;
}

TArray<UUpgradableComponent*> UUpgradeManagerSubsystem::GetComponentsByAspect(EUpgradableAspect Aspect,
	int32 LevelFilter) const
{
	TArray<UUpgradableComponent*> Result;

	for (int32 Id = 0; Id < RegisteredComponents.Num(); ++Id)
	{
		if (!RegisteredComponents[Id].IsValid())
			continue;

		UUpgradableComponent* Comp = RegisteredComponents[Id].Get();
		if (!Comp)
			continue;

		if (Comp->GetUpgradableAspect() != Aspect)
			continue;

		// If a level filter is specified, ensure it matches
		if (LevelFilter >= 0)
		{
			if (!ComponentLevels.IsValidIndex(Id) || ComponentLevels[Id] != LevelFilter)
				continue;
		}

		Result.Add(Comp);
	}

	return Result;
}

TArray<UUpgradableComponent*> UUpgradeManagerSubsystem::GetComponentsByUpgradePath(FName PathId,
	int32 LevelFilter) const
{
	TArray<UUpgradableComponent*> Result;

	for (int32 Id = 0; Id < RegisteredComponents.Num(); ++Id)
	{
		if (!RegisteredComponents[Id].IsValid())
			continue;

		UUpgradableComponent* Comp = RegisteredComponents[Id].Get();
		if (!Comp)
			continue;

		if (Comp->UpgradePathId != PathId)
			continue;

		// If a level filter is specified, ensure it matches
		if (LevelFilter >= 0)
		{
			if (!ComponentLevels.IsValidIndex(Id) || ComponentLevels[Id] != LevelFilter)
				continue;
		}

		Result.Add(Comp);
	}

	return Result;
}

TArray<FUpgradeDefinition> UUpgradeManagerSubsystem::GetUpgradeDefinitionsForPath(FName PathId) const
{
	TArray<FUpgradeDefinition> Result;
	const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(PathId);
	if (UpgradeDefinitions)
	{
		Result = *UpgradeDefinitions;
	}
	return Result;
}

int32 UUpgradeManagerSubsystem::GetUpgradeLevelForActor(AActor* TargetActor, EUpgradableAspect Aspect) const
{
	if (!TargetActor) return -1;

	UUpgradableComponent* Comp = FindComponentOnActorByAspect(TargetActor, Aspect);
	if (!Comp) return -1;
	return GetCurrentLevel(Comp->GetComponentId());
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
	if (const FUpgradeDefinition* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId)))
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
	if (const FUpgradeDefinition* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId)))
	{
		SecondsForUpgrade = UpgradeDefinition->UpgradeSeconds;
	}
	return SecondsForUpgrade;
}


const TArray<FUpgradeDefinition>* UUpgradeManagerSubsystem::GetUpgradeDefinitions(FName UpgradePathId) const
{
	return UpgradeCatalog.Find(UpgradePathId);
}

const TArray<FUpgradeDefinition>* UUpgradeManagerSubsystem::GetUpgradeDefinitions(int32 ComponentId) const
{
	const UUpgradableComponent* Comp = GetComponentById(ComponentId);
	if (!Comp) return nullptr;
	return UpgradeCatalog.Find(Comp->UpgradePathId);
}

const FUpgradeDefinition* UUpgradeManagerSubsystem::GetUpgradeDefinitionForLevel(int32 ComponentId, int32 Level) const
{
	const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId);
	if (!UpgradeDefinitions || Level < 0 || Level > UpgradeDefinitions->Num()-1 ) return nullptr;
	return &(*UpgradeDefinitions)[Level];
}




bool UUpgradeManagerSubsystem::CanUpgrade(const int32 ComponentId, const int32 LevelIncrease, const TMap<FName, int32>& AvailableResources) const
{
	bool Success = false;
	
	if (!GetComponentById(ComponentId)) return false;
	// Future implementation idea: Add upgrade queue to chain multiple upgrades
	if (IsUpgradeTimerActive(ComponentId)) return false;
	
	if (LevelIncrease <= 0) return false;
	// Trying to upgrade to a level higher than the max level
	if (GetCurrentLevel(ComponentId) + LevelIncrease > GetMaxLevel(ComponentId)) return false;

	if (const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId))
	{
		const FUpgradeDefinition* LevelData = &(*UpgradeDefinitions)[GetCurrentLevel(ComponentId)];
		
		
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
	if (!CanUpgrade(ComponentId, LevelIncrease, AvailableResources)) return false;
	RequestedLevelIncreases.Add(ComponentId, LevelIncrease);
	const float UpgradeDuration = GetUpgradeTimerDuration(ComponentId, LevelIncrease);
	UpgradeInProgressData.FindOrAdd(ComponentId).TotalUpgradeTime = UpgradeDuration;
	UpgradeInProgressData.FindOrAdd(ComponentId).RequestedLevelIncrease = LevelIncrease;
	StartUpgradeTimer(ComponentId, UpgradeDuration);

	return true;
}
