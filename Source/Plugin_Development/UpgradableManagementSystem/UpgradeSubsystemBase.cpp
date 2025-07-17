// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeSubsystemBase.h"
#include "UpgradableComponent.h"
#include "UpgradeDataProvider.h"
#include "UpgradeSettings.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogUpgradeSystem);

UUpgradeSubsystemBase::UUpgradeSubsystemBase()
{
}

void UUpgradeSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUpgradeSubsystemBase::Deinitialize()
{
	Super::Deinitialize();
}

void UUpgradeSubsystemBase::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	const TArray<UUpgradeDataProvider*> DataProviders = InitializeProviders();
	LoadCatalog(DataProviders);
}

TArray<UUpgradeDataProvider*> UUpgradeSubsystemBase::InitializeProviders()
{
	const UUpgradeSettings* Settings = GetDefault<UUpgradeSettings>();
	FString UpgradeDataFolderPath = Settings->UpgradeDataFolderPath;
	
	UUpgradeDataProvider* Scanner = NewObject<UUpgradeDataProvider>(this);
	return Scanner->Scan(UpgradeDataFolderPath);
}

void UUpgradeSubsystemBase::LoadCatalog(TArray<UUpgradeDataProvider*> DataProviders)
{
	if (DataProviders.Num() == 0)	return;

	UpgradeCatalog.Empty();
	ResourceTypes.Empty();

	for (UUpgradeDataProvider* Provider : DataProviders)
	{
		if (!Provider) continue;
		Provider->InitializeData(UpgradeCatalog, ResourceTypes);
	}
	UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEMGR_INFO_03] Loaded Upgrade Catalog from %d provider(s)"), DataProviders.Num());

}

int32 UUpgradeSubsystemBase::RegisterUpgradableComponent(UUpgradableComponent* Component)
{
	int32 Id;
	FUpgradableComponentData Entry;
	
	if (Component)
	{
		Entry.Component = Component;
		Entry.Owner = Component->GetOwner();
		Entry.UpgradePathId = Component->UpgradePathId;
		Entry.Aspect = Component->GetUpgradableAspect();
		Entry.Category = Component->GetUpgradableCategory();
		Entry.Level = Component->InitialLevel;
	}
	
	
	if (FreeComponentIndices.Num() > 0)
	{
		Id = FreeComponentIndices.Pop(/*bAllowShrinking=*/false);
		ComponentData[Id] = Entry;
	}
	else
	{
		Id = ComponentData.Add(Entry);
	}
	
	if (UE_LOG_ACTIVE(LogUpgradeSystem, Verbose))
	{
		UE_LOG(LogUpgradeSystem, Verbose, TEXT("[UPGRADEMGR_INFO_04] Registered component ID %d at level %d. Total components %d"), Id, Entry.Level, ComponentData.Num() - FreeComponentIndices.Num());
	}
	
	return Id;
}

void UUpgradeSubsystemBase::UnregisterUpgradableComponent(int32 ComponentId)
	{
	if (ComponentData.IsValidIndex(ComponentId))
	{
		if (IsUpgradeTimerActive(ComponentId))
		{
			CancelUpgrade(ComponentId);
		}
		ComponentData[ComponentId] = FUpgradableComponentData();
		FreeComponentIndices.Add(ComponentId);
	}
	
	if (UE_LOG_ACTIVE(LogUpgradeSystem, Verbose))
	{
	UE_LOG(LogUpgradeSystem, Verbose, TEXT("[UPGRADEMGR_INFO_05] Unregistered component ID %d. Total components %d"), ComponentId, ComponentData.Num() - FreeComponentIndices.Num());
	}
}

bool UUpgradeSubsystemBase::CanUpgrade(const int32 ComponentId, const int32 LevelIncrease,
                                       const TMap<FName, int32>& AvailableResources) const
{
	UE_LOG(LogUpgradeSystem, Verbose, TEXT("[UPGRADEMGR_INFO_01] Checking upgrade eligibility for component %d (increase %d)"), ComponentId, LevelIncrease);

	bool Success = false;

	if (!GetComponentById(ComponentId))
	{
		UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEMGR_ERR_00] Component %d not registered"), ComponentId);
		return false;
	}
	// Future implementation idea: Add upgrade queue to chain multiple upgrades
	if (IsUpgradeTimerActive(ComponentId))
	{
		UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEMGR_ERR_01] Component %d already upgrading"), ComponentId);
		return false;
	}

	if (LevelIncrease <= 0)
	{
		UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEMGR_ERR_02] Invalid level increase %d for component %d"), LevelIncrease, ComponentId);
		return false;
	}
	// Trying to upgrade to a level higher than the max level
	if (GetCurrentLevel(ComponentId) + LevelIncrease > GetMaxLevel(ComponentId))
	{
		UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEMGR_ERR_03] Requested level exceeds max for component %d"), ComponentId);
		return false;
	}

	if (const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId))
	{
		const FUpgradeDefinition* LevelData = nullptr;

		TMap<FName, int32> TotalResourceCosts;
		// Iterate over all levels if trying to upgrade several levels at once
		for (int32 i = GetNextLevel(ComponentId); i <= GetCurrentLevel(ComponentId) + LevelIncrease; ++i)
		{
			LevelData = &(*UpgradeDefinitions)[i];
			if (LevelData->bUpgradeLocked)
			{
				UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEMGR_ERR_04] Level %d locked for component %d"), i, ComponentId);
				return false;
			}
			FName ResourceType ;
			// Add up the required resource cost for each resource for this level
			for (int32 j = 0; j < LevelData->ResourceTypeIndices.Num(); ++j)
			{
				ResourceType = GetResourceTypeName(LevelData->ResourceTypeIndices[j]);
				// no resource of the required type was provided
				if (!AvailableResources.Contains(ResourceType))
				{
				UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEMGR_ERR_05] Missing resource '%s' for component %d"), *ResourceType.ToString(), ComponentId);
				return false;
				}
				TotalResourceCosts.FindOrAdd(ResourceType) += LevelData->UpgradeCosts[j];
			}
		}
		TArray<FName> RequiredResources;
		TotalResourceCosts.GetKeys(RequiredResources);
		for (FName ResourceType : RequiredResources)
		{
			// not enough resources of the required type
			if (TotalResourceCosts[ResourceType] > AvailableResources.FindRef(ResourceType))
			{
		       UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEMGR_ERR_06] Insufficient '%s' for component %d"), *ResourceType.ToString(), ComponentId);
		       return false;
			}
		}

		Success = true;
		UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEMGR_INFO_02] Component %d can upgrade by %d levels"), ComponentId, LevelIncrease);
	}
	return Success;
}

bool UUpgradeSubsystemBase::HandleUpgradeRequest(const int32 ComponentId, const int32 LevelIncrease,
	const TMap<FName, int32>& AvailableResources)
{
	if (!CanUpgrade(ComponentId, LevelIncrease, AvailableResources)) return false;
	const float UpgradeDuration = GetUpgradeTimerDuration(ComponentId, LevelIncrease);
	const TMap<FName, int32> TotalResourceCosts = GetUpgradeTotalResourceCost(ComponentId, LevelIncrease);

	if (UpgradeDuration > 0.f)
	{
		UpgradeInProgressData.FindOrAdd(ComponentId).TotalUpgradeTime = UpgradeDuration;
		UpgradeInProgressData.FindOrAdd(ComponentId).RequestedLevelIncrease = LevelIncrease;
		UpgradeInProgressData.FindOrAdd(ComponentId).UpgradeResourceCost = TotalResourceCosts;
		StartUpgradeTimer(ComponentId, UpgradeDuration);
	}
	else
	{
		UpdateUpgradeLevel(ComponentId, GetCurrentLevel(ComponentId) + LevelIncrease);
	}

	return true;
}

void UUpgradeSubsystemBase::UpdateUpgradeLevel(const int32 ComponentId, const int32 NewLevel)
{
	if (UUpgradableComponent* Comp = GetComponentById(ComponentId))
	{
		ComponentData[ComponentId].Level = NewLevel;
		Comp->Client_SetLevel(NewLevel);
	}
}

bool UUpgradeSubsystemBase::RequestUpgradeForActor(AActor* TargetActor, const EUpgradableAspect Aspect, const int32 LevelIncrease,
	const TArray<FName>& ResourceTypesArray, const TArray<int32>& ResourceAmounts) const
{
	if (!TargetActor)
		return false;

	UUpgradableComponent* Comp = FindComponentOnActorByAspect(TargetActor, Aspect);
	if (!Comp)
		return false;

	Comp->RequestUpgrade(LevelIncrease, ResourceTypesArray, ResourceAmounts);
	return true;
}

UUpgradableComponent* UUpgradeSubsystemBase::GetComponentById(const int32 Id) const
{
	return (ComponentData.IsValidIndex(Id) && ComponentData[Id].Component.IsValid())
	? ComponentData[Id].Component.Get() : nullptr;
}

UUpgradableComponent* UUpgradeSubsystemBase::FindComponentOnActorByAspect(AActor* TargetActor, const EUpgradableAspect Aspect) const
{
	if (!TargetActor) return nullptr;
	// We prefer doing this, rather than using GetComponent on the TargetActor, since the Actor might have a long list of components.
	// In most real world use cases, this would be more performant.
	for (const FUpgradableComponentData& Entry : ComponentData)
	{
		if (Entry.Aspect != Aspect) continue;
		if (!Entry.Component.IsValid())	continue;
		if (Entry.Owner.Get() != TargetActor) continue;
		
		return Entry.Component.Get();
	}
	return nullptr;
}

UUpgradableComponent* UUpgradeSubsystemBase::FindComponentOnActorByCategory(AActor* TargetActor,
	const EUpgradableCategory Category) const
{
	if (!TargetActor) return nullptr;
	// We prefer doing this, rather than using GetComponent on the TargetActor, since the Actor might have a long list of components.
	// In most real world use cases, this would be more performant.
	for (const FUpgradableComponentData& Entry : ComponentData)
	{
		if (Entry.Category != Category) continue;
		if (!Entry.Component.IsValid())	continue;
		if (Entry.Owner.Get() != TargetActor) continue;
		
		return Entry.Component.Get();
	}
	
	return nullptr;
}

TArray<UUpgradableComponent*> UUpgradeSubsystemBase::GetComponentsByAspect(const EUpgradableAspect Aspect, const int32 LevelFilter) const
{
	TArray<UUpgradableComponent*> Result;
	
	for (const FUpgradableComponentData& Entry : ComponentData)
	{
		if (Entry.Aspect != Aspect)	continue;
		if (!Entry.Component.IsValid())	continue;
		// If a level filter is specified, ensure it matches
		if (LevelFilter >= 0 && Entry.Level != LevelFilter)	continue;
		
		Result.Add(Entry.Component.Get());
	}

	return Result;
}

TArray<UUpgradableComponent*> UUpgradeSubsystemBase::GetComponentsByUpgradePath(const FName PathId, const int32 LevelFilter) const
{
	TArray<UUpgradableComponent*> Result;

	for (const FUpgradableComponentData& Entry : ComponentData)
	{
		if (Entry.UpgradePathId != PathId) continue;
		if (!Entry.Component.IsValid())	continue;
		// If a level filter is specified, ensure it matches
		if (LevelFilter >= 0 && Entry.Level != LevelFilter)	continue;
		
		Result.Add(Entry.Component.Get());
	}

	return Result;
}

TArray<UUpgradableComponent*> UUpgradeSubsystemBase::GetComponentsByActor(AActor* TargetActor)
{
	TArray<UUpgradableComponent*> Results;
	if (!TargetActor) return Results;
	// We prefer doing this, rather than using GetComponent on the TargetActor, since the Actor might have a long list of components.
	// In most real world use cases, this would be more performant.
	for (const FUpgradableComponentData& Entry : ComponentData)
	{
		if (!Entry.Component.IsValid())	continue;
		if (Entry.Owner.Get() != TargetActor) continue;
		
		Results.Add(Entry.Component.Get());
	}
	
	return Results;
}

TArray<FUpgradeDefinition> UUpgradeSubsystemBase::GetUpgradeDefinitionsForPath(const FName PathId) const
{
	TArray<FUpgradeDefinition> Result;
	const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(PathId);
	if (UpgradeDefinitions)
	{
		Result = *UpgradeDefinitions;
	}
	return Result;
}

int32 UUpgradeSubsystemBase::GetCurrentLevel(const int32 ComponentId) const
{
	if (ComponentData.IsValidIndex(ComponentId) && ComponentData[ComponentId].Level != -1)
	{
		return ComponentData[ComponentId].Level;
	}
	return -1;
}

int32 UUpgradeSubsystemBase::GetUpgradeLevelForActor(AActor* TargetActor, const EUpgradableAspect Aspect) const
{
	if (!TargetActor) return -1;

	const UUpgradableComponent* Comp = FindComponentOnActorByAspect(TargetActor, Aspect);
	if (!Comp) return -1;
	return GetCurrentLevel(Comp->GetComponentId());
}

int32 UUpgradeSubsystemBase::GetNextLevel(const int32 ComponentId) const
{
	if (ComponentData.IsValidIndex(ComponentId) && ComponentData[ComponentId].Level != -1)
	{
		return ComponentData[ComponentId].Level + 1;
	}
	return -1;
}

int32 UUpgradeSubsystemBase::GetMaxLevel(const int32 ComponentId) const
{
	return GetUpgradeDefinitions(ComponentId)->Num()-1;
}

int32 UUpgradeSubsystemBase::GetInProgressLevelIncrease(const int32 ComponentId) const
{
	if ( UpgradeInProgressData.Contains(ComponentId))
	{
		return UpgradeInProgressData[ComponentId].RequestedLevelIncrease;
	}
	return -1;
}

void UUpgradeSubsystemBase::CancelUpgrade(int32 ComponentId)
{
	UUpgradableComponent* Comp = GetComponentById(ComponentId);
	if (Comp && IsUpgradeTimerActive(ComponentId))
	{
		StopUpgradeTimer(ComponentId);
		UpgradeInProgressData.Remove(ComponentId);
		Comp->Client_OnUpgradeCanceled(GetCurrentLevel(ComponentId));
	}
}

int32 UUpgradeSubsystemBase::GetNextLevelUpgradeTime(const int32 ComponentId) const
{
	int32 SecondsForUpgrade = -1;
	if (const FUpgradeDefinition* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId)))
	{
		SecondsForUpgrade = UpgradeDefinition->UpgradeSeconds;
	}
	return SecondsForUpgrade;
}

float UUpgradeSubsystemBase::UpdateUpgradeTimer(const int32 ComponentId, const float DeltaTime)
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		const FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		const FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		const float TimeRemaining = TimerManager.GetTimerRemaining(Handle);
		const float NewTimeRemaining = FMath::Max(0.f, FMath::FloorToInt(TimeRemaining + DeltaTime));
		
		if (UUpgradableComponent* Comp = GetComponentById(ComponentId))
		{
			Comp->Client_OnTimeToUpgradeChanged(DeltaTime);
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

float UUpgradeSubsystemBase::GetUpgradeTimeRemaining(const int32 ComponentId) const
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		const FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		return GetWorld()->GetTimerManager().GetTimerRemaining(Handle);
	}
	return -1.f;
}

float UUpgradeSubsystemBase::GetInProgressTotalUpgradeTime(const int32 ComponentId) const
{
	if (UpgradeInProgressData.Contains(ComponentId))
	{
		return UpgradeInProgressData[ComponentId].TotalUpgradeTime;
	}
	return -1.f;
}

int32 UUpgradeSubsystemBase::GetResourceTypeIndex(const FName& TypeName) const
{
	const int32 FoundIndex = ResourceTypes.IndexOfByKey(TypeName);
	if (FoundIndex != INDEX_NONE)
	{
		return FoundIndex;
	}
	return -1;
}

FName UUpgradeSubsystemBase::GetResourceTypeName(const int32 Index) const
{
	return ResourceTypes.IsValidIndex(Index)
		? ResourceTypes[Index]
		: NAME_None;
}

void UUpgradeSubsystemBase::GetNextLevelUpgradeCosts(int32 ComponentId, TMap<FName, int32>& ResourceCosts) const
{
	if (const FUpgradeDefinition* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId)))
	{
		for (int32 i = 0; i < UpgradeDefinition->ResourceTypeIndices.Num(); ++i)
		{
			ResourceCosts.Add(GetResourceTypeName(UpgradeDefinition->ResourceTypeIndices[i]), UpgradeDefinition->UpgradeCosts[i]);			
		}
	}
}

TMap<FName, int32> UUpgradeSubsystemBase::GetUpgradeTotalResourceCost(int32 ComponentId, int32 LevelIncrease) const
{
	TMap<FName, int32> TotalResourceCosts;

	if (const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId))
	{
		const FUpgradeDefinition* LevelData = nullptr;

		for (int32 i = GetNextLevel(ComponentId); i <= GetCurrentLevel(ComponentId) + LevelIncrease; ++i)
		{
			LevelData = &(*UpgradeDefinitions)[i];

			for (int32 j = 0; j < LevelData->ResourceTypeIndices.Num(); ++j)
			{
				const FName ResourceType = GetResourceTypeName(LevelData->ResourceTypeIndices[j]);
				TotalResourceCosts.FindOrAdd(ResourceType) += LevelData->UpgradeCosts[j];
			}
		}
	}
	return TotalResourceCosts;
}

TMap<FName, int32> UUpgradeSubsystemBase::GetInProgressTotalResourceCost(int32 ComponentId) const
{
	if (UpgradeInProgressData.Contains(ComponentId))
	{
		return UpgradeInProgressData[ComponentId].UpgradeResourceCost;
	}
	return TMap<FName, int32>();
}



float UUpgradeSubsystemBase::GetUpgradeTimerDuration(int32 ComponentId, int32 LevelIncrease) const
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

float UUpgradeSubsystemBase::StartUpgradeTimer(int32 ComponentId, float TimerDuration)
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("OnUpgradeTimerFinished"), ComponentId);
	FTimerHandle& TimerHandle = UpgradeInProgressData.FindOrAdd(ComponentId).UpgradeTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, TimerDuration, false);
	
	if (UUpgradableComponent* Comp = GetComponentById(ComponentId))
	{
		Comp->Client_OnUpgradeStarted(TimerDuration);
	}
	return TimerDuration;
}

void UUpgradeSubsystemBase::StopUpgradeTimer(int32 ComponentId)
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		GetWorld()->GetTimerManager().ClearTimer(Handle);
	}
}

void UUpgradeSubsystemBase::OnUpgradeTimerFinished(int32 ComponentId)
{
	StopUpgradeTimer(ComponentId);

	if (!GetComponentById(ComponentId)) return;

	const int32 NewLevel = GetCurrentLevel(ComponentId) + UpgradeInProgressData[ComponentId].RequestedLevelIncrease;   
	UpgradeInProgressData.Remove(ComponentId);
	
	UpdateUpgradeLevel(ComponentId, NewLevel);
}

const TArray<FUpgradeDefinition>* UUpgradeSubsystemBase::GetUpgradeDefinitions(FName UpgradePathId) const
{
	return UpgradeCatalog.Find(UpgradePathId);
}

const TArray<FUpgradeDefinition>* UUpgradeSubsystemBase::GetUpgradeDefinitions(int32 ComponentId) const
{
	if (!ComponentData.IsValidIndex(ComponentId)) return nullptr;
	
	return UpgradeCatalog.Find(ComponentData[ComponentId].UpgradePathId);
}

const FUpgradeDefinition* UUpgradeSubsystemBase::GetUpgradeDefinitionForLevel(int32 ComponentId, int32 Level) const
{
	const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId);
	if (!UpgradeDefinitions || Level < 0 || Level > UpgradeDefinitions->Num()-1 ) return nullptr;
	return &(*UpgradeDefinitions)[Level];
}
