// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgrageSubsystemBase.h"
#include "UpgradableComponent.h"

DEFINE_LOG_CATEGORY(LogUpgradeSystem);

UUpgrageSubsystemBase::UUpgrageSubsystemBase()
{
}

void UUpgrageSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUpgrageSubsystemBase::Deinitialize()
{
	Super::Deinitialize();
}

void UUpgrageSubsystemBase::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

int32 UUpgrageSubsystemBase::RegisterUpgradableComponent(UUpgradableComponent* Component)
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

void UUpgrageSubsystemBase::UnregisterUpgradableComponent(int32 ComponentId)
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

void UUpgrageSubsystemBase::UpdateUpgradeLevel(const int32 ComponentId, const int32 NewLevel)
{
	if (UUpgradableComponent* Comp = GetComponentById(ComponentId))
	{
		ComponentLevels[ComponentId] = NewLevel;
		Comp->Client_SetLevel(NewLevel);
	}
}

UUpgradableComponent* UUpgrageSubsystemBase::GetComponentById(int32 Id) const
{
	return (RegisteredComponents.IsValidIndex(Id)
		? RegisteredComponents[Id].Get()
		: nullptr);
}

UUpgradableComponent* UUpgrageSubsystemBase::FindComponentOnActorByAspect(AActor* TargetActor,
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

UUpgradableComponent* UUpgrageSubsystemBase::FindComponentOnActorByCategory(AActor* TargetActor,
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

TArray<UUpgradableComponent*> UUpgrageSubsystemBase::GetComponentsByAspect(EUpgradableAspect Aspect,
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

TArray<UUpgradableComponent*> UUpgrageSubsystemBase::GetComponentsByUpgradePath(FName PathId, int32 LevelFilter) const
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

TArray<FUpgradeDefinition> UUpgrageSubsystemBase::GetUpgradeDefinitionsForPath(FName PathId) const
{
	TArray<FUpgradeDefinition> Result;
	const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(PathId);
	if (UpgradeDefinitions)
	{
		Result = *UpgradeDefinitions;
	}
	return Result;
}

int32 UUpgrageSubsystemBase::GetCurrentLevel(const int32 ComponentId) const
{
	if (ComponentLevels.IsValidIndex(ComponentId) && ComponentLevels[ComponentId] != -1)
	{
		return ComponentLevels[ComponentId];
	}
	return -1;
}

int32 UUpgrageSubsystemBase::GetUpgradeLevelForActor(AActor* TargetActor, EUpgradableAspect Aspect) const
{
	if (!TargetActor) return -1;

	UUpgradableComponent* Comp = FindComponentOnActorByAspect(TargetActor, Aspect);
	if (!Comp) return -1;
	return GetCurrentLevel(Comp->GetComponentId());
}

int32 UUpgrageSubsystemBase::GetNextLevel(const int32 ComponentId) const
{
	if (ComponentLevels.IsValidIndex(ComponentId) && ComponentLevels[ComponentId] != -1)
	{
		
		return ComponentLevels[ComponentId] + 1;
	}
	return -1;
}

int32 UUpgrageSubsystemBase::GetMaxLevel(int32 ComponentId) const
{
	return GetUpgradeDefinitions(ComponentId)->Num()-1;
}

int32 UUpgrageSubsystemBase::GetInProgressLevelIncrease(int32 ComponentId) const
{
	if ( UpgradeInProgressData.Contains(ComponentId))
	{
		return UpgradeInProgressData[ComponentId].RequestedLevelIncrease;
	}
	return -1;
}

void UUpgrageSubsystemBase::CancelUpgrade(int32 ComponentId)
{
	UUpgradableComponent* Comp = GetComponentById(ComponentId);
	if (Comp && IsUpgradeTimerActive(ComponentId))
	{
		StopUpgradeTimer(ComponentId);
		UpgradeInProgressData.Remove(ComponentId);
		Comp->Client_OnUpgradeCanceled(GetCurrentLevel(ComponentId));
	}
}

int32 UUpgrageSubsystemBase::GetNextLevelUpgradeTime(const int32 ComponentId) const
{
	int32 SecondsForUpgrade = -1;
	if (const FUpgradeDefinition* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId)))
	{
		SecondsForUpgrade = UpgradeDefinition->UpgradeSeconds;
	}
	return SecondsForUpgrade;
}

float UUpgrageSubsystemBase::UpdateUpgradeTimer(int32 ComponentId, float DeltaTime)
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

float UUpgrageSubsystemBase::GetUpgradeTimeRemaining(int32 ComponentId) const
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		const FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		return GetWorld()->GetTimerManager().GetTimerRemaining(Handle);
	}
	return -1.f;
}

float UUpgrageSubsystemBase::GetInProgressTotalUpgradeTime(int32 ComponentId) const
{
	if (UpgradeInProgressData.Contains(ComponentId))
	{
		return UpgradeInProgressData[ComponentId].TotalUpgradeTime;
	}
	return -1.f;
}

int32 UUpgrageSubsystemBase::GetResourceTypeIndex(const FName& TypeName) const
{
	int32 FoundIndex = ResourceTypes.IndexOfByKey(TypeName);
	if (FoundIndex != INDEX_NONE)
	{
		return FoundIndex;
	}
	return -1;
}

FName UUpgrageSubsystemBase::GetResourceTypeName(int32 Index) const
{
	return ResourceTypes.IsValidIndex(Index)
		? ResourceTypes[Index]
		: NAME_None;
}

void UUpgrageSubsystemBase::GetNextLevelUpgradeCosts(int32 ComponentId, TMap<FName, int32>& ResourceCosts) const
{
	if (const FUpgradeDefinition* UpgradeDefinition = GetUpgradeDefinitionForLevel(ComponentId, GetNextLevel(ComponentId)))
	{
		for (int32 i = 0; i < UpgradeDefinition->ResourceTypeIndices.Num(); ++i)
		{
			ResourceCosts.Add(GetResourceTypeName(UpgradeDefinition->ResourceTypeIndices[i]), UpgradeDefinition->UpgradeCosts[i]);			
		}
	}
}

TMap<FName, int32> UUpgrageSubsystemBase::GetUpgradeTotalResourceCost(int32 ComponentId, int32 LevelIncrease) const
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

TMap<FName, int32> UUpgrageSubsystemBase::GetInProgressTotalResourceCost(int32 ComponentId) const
{
	if (UpgradeInProgressData.Contains(ComponentId))
	{
		return UpgradeInProgressData[ComponentId].UpgradeResourceCost;
	}
	return TMap<FName, int32>();
}

float UUpgrageSubsystemBase::GetUpgradeTimerDuration(int32 ComponentId, int32 LevelIncrease) const
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

float UUpgrageSubsystemBase::StartUpgradeTimer(int32 ComponentId, float TimerDuration)
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

void UUpgrageSubsystemBase::StopUpgradeTimer(int32 ComponentId)
{
	if (UpgradeInProgressData.Find(ComponentId))
	{
		FTimerHandle& Handle = UpgradeInProgressData[ComponentId].UpgradeTimerHandle;
		GetWorld()->GetTimerManager().ClearTimer(Handle);
	}
}

void UUpgrageSubsystemBase::OnUpgradeTimerFinished(int32 ComponentId)
{
	StopUpgradeTimer(ComponentId);

	if (!GetComponentById(ComponentId)) return;

	const int32 NewLevel = GetCurrentLevel(ComponentId) + UpgradeInProgressData[ComponentId].RequestedLevelIncrease;   
	UpgradeInProgressData.Remove(ComponentId);
	
	UpdateUpgradeLevel(ComponentId, NewLevel);
}

const TArray<FUpgradeDefinition>* UUpgrageSubsystemBase::GetUpgradeDefinitions(FName UpgradePathId) const
{
	return UpgradeCatalog.Find(UpgradePathId);
}

const TArray<FUpgradeDefinition>* UUpgrageSubsystemBase::GetUpgradeDefinitions(int32 ComponentId) const
{
	const UUpgradableComponent* Comp = GetComponentById(ComponentId);
	if (!Comp) return nullptr;
	return UpgradeCatalog.Find(Comp->UpgradePathId);
}

const FUpgradeDefinition* UUpgrageSubsystemBase::GetUpgradeDefinitionForLevel(int32 ComponentId, int32 Level) const
{
	const TArray<FUpgradeDefinition>* UpgradeDefinitions = GetUpgradeDefinitions(ComponentId);
	if (!UpgradeDefinitions || Level < 0 || Level > UpgradeDefinitions->Num()-1 ) return nullptr;
	return &(*UpgradeDefinitions)[Level];
}
