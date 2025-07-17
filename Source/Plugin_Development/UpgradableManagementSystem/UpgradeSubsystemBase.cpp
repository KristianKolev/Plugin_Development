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

int32 UUpgradeSubsystemBase::RegisterUpgradableComponent(UUpgradableComponent* Component)
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
		// No holes, grow the array
		Id = RegisteredComponents.Add(Component);
		ComponentLevels.Add(Component->InitialLevel);
	}
	if (UE_LOG_ACTIVE(LogUpgradeSystem, Verbose))
	{
		UE_LOG(LogUpgradeSystem, Verbose, TEXT("[UPGRADEMGR_INFO_04] Registered component ID %d at level %d. Total components %d"), Id, Component->InitialLevel, RegisteredComponents.Num()-FreeComponentIndices.Num());
	}
	return Id;
}

void UUpgradeSubsystemBase::UnregisterUpgradableComponent(int32 ComponentId)
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

	if (UE_LOG_ACTIVE(LogUpgradeSystem, Verbose))
	{
		UE_LOG(LogUpgradeSystem, Verbose, TEXT("[UPGRADEMGR_INFO_05] Unregistered component ID %d. Total components %d"), ComponentId, RegisteredComponents.Num()-FreeComponentIndices.Num());
	}
}

bool UUpgradeSubsystemBase::CanUpgrade(int32 ComponentId, int32 LevelIncrease,
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

bool UUpgradeSubsystemBase::HandleUpgradeRequest(int32 ComponentId, int32 LevelIncrease,
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

TArray<UUpgradeDataProvider*> UUpgradeSubsystemBase::InitializeProviders()
{
	const UUpgradeSettings* Settings = GetDefault<UUpgradeSettings>();
	FString UpgradeDataFolderPath = Settings->UpgradeDataFolderPath;
	
	UUpgradeDataProvider* Scanner = NewObject<UUpgradeDataProvider>(this);
	return Scanner->Scan(UpgradeDataFolderPath);
}

void UUpgradeSubsystemBase::UpdateUpgradeLevel(const int32 ComponentId, const int32 NewLevel)
{
	if (UUpgradableComponent* Comp = GetComponentById(ComponentId))
	{
		ComponentLevels[ComponentId] = NewLevel;
		Comp->Client_SetLevel(NewLevel);
	}
}

bool UUpgradeSubsystemBase::RequestUpgradeForActor(AActor* TargetActor, EUpgradableAspect Aspect, int32 LevelIncrease,
	const TArray<FName>& ResourceTypesArray, const TArray<int32>& ResourceAmounts)
{
	if (!TargetActor)
		return false;

	UUpgradableComponent* Comp = FindComponentOnActorByAspect(TargetActor, Aspect);
	if (!Comp)
		return false;

	Comp->RequestUpgrade(LevelIncrease, ResourceTypesArray, ResourceAmounts);
	return true;
}

UUpgradableComponent* UUpgradeSubsystemBase::GetComponentById(int32 Id) const
{
	return (RegisteredComponents.IsValidIndex(Id)
		? RegisteredComponents[Id].Get()
		: nullptr);
}

UUpgradableComponent* UUpgradeSubsystemBase::FindComponentOnActorByAspect(AActor* TargetActor,
	EUpgradableAspect Aspect) const
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

UUpgradableComponent* UUpgradeSubsystemBase::FindComponentOnActorByCategory(AActor* TargetActor,
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

TArray<UUpgradableComponent*> UUpgradeSubsystemBase::GetComponentsByAspect(EUpgradableAspect Aspect,
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

TArray<UUpgradableComponent*> UUpgradeSubsystemBase::GetComponentsByUpgradePath(FName PathId, int32 LevelFilter) const
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

TArray<FUpgradeDefinition> UUpgradeSubsystemBase::GetUpgradeDefinitionsForPath(FName PathId) const
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
	if (ComponentLevels.IsValidIndex(ComponentId) && ComponentLevels[ComponentId] != -1)
	{
		return ComponentLevels[ComponentId];
	}
	return -1;
}

int32 UUpgradeSubsystemBase::GetUpgradeLevelForActor(AActor* TargetActor, EUpgradableAspect Aspect) const
{
	if (!TargetActor) return -1;

	UUpgradableComponent* Comp = FindComponentOnActorByAspect(TargetActor, Aspect);
	if (!Comp) return -1;
	return GetCurrentLevel(Comp->GetComponentId());
}

int32 UUpgradeSubsystemBase::GetNextLevel(const int32 ComponentId) const
{
	if (ComponentLevels.IsValidIndex(ComponentId) && ComponentLevels[ComponentId] != -1)
	{
		
		return ComponentLevels[ComponentId] + 1;
	}
	return -1;
}

int32 UUpgradeSubsystemBase::GetMaxLevel(int32 ComponentId) const
{
	return GetUpgradeDefinitions(ComponentId)->Num()-1;
}

int32 UUpgradeSubsystemBase::GetInProgressLevelIncrease(int32 ComponentId) const
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

float UUpgradeSubsystemBase::UpdateUpgradeTimer(int32 ComponentId, float DeltaTime)
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		float TimeRemaining = TimerManager.GetTimerRemaining(Handle);
		float NewTimeRemaining = FMath::Max(0.f, FMath::FloorToInt(TimeRemaining + DeltaTime));
		
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

float UUpgradeSubsystemBase::GetUpgradeTimeRemaining(int32 ComponentId) const
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		const FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		return GetWorld()->GetTimerManager().GetTimerRemaining(Handle);
	}
	return -1.f;
}

float UUpgradeSubsystemBase::GetInProgressTotalUpgradeTime(int32 ComponentId) const
{
	if (UpgradeInProgressData.Contains(ComponentId))
	{
		return UpgradeInProgressData[ComponentId].TotalUpgradeTime;
	}
	return -1.f;
}

int32 UUpgradeSubsystemBase::GetResourceTypeIndex(const FName& TypeName) const
{
	int32 FoundIndex = ResourceTypes.IndexOfByKey(TypeName);
	if (FoundIndex != INDEX_NONE)
	{
		return FoundIndex;
	}
	return -1;
}

FName UUpgradeSubsystemBase::GetResourceTypeName(int32 Index) const
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
	const UUpgradableComponent* Comp = GetComponentById(ComponentId);
	if (!Comp) return nullptr;
	return UpgradeCatalog.Find(Comp->UpgradePathId);
}

const FUpgradeDefinition* UUpgradeSubsystemBase::GetUpgradeDefinitionForLevel(int32 ComponentId, int32 Level) const
{
	const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId);
	if (!UpgradeDefinitions || Level < 0 || Level > UpgradeDefinitions->Num()-1 ) return nullptr;
	return &(*UpgradeDefinitions)[Level];
}
