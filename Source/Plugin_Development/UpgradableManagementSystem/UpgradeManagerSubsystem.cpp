// Fill out your copyright notice in the Description page of Project Settings.
#include "UpgradeManagerSubsystem.h"
#include "UpgradableComponent.h"
#include "UpgradeDataAssetProvider.h"
#include "UpgradeDataTableProvider.h"
#include "UpgradeJsonProvider.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/DataTable.h"
#include "UpgradeDefinitionDataAsset.h"
#include "UpgradeSettings.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../CustomLogging.h"
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
	Super::Deinitialize();
}

TArray<UUpgradeDataProvider*> UUpgradeManagerSubsystem::InitializeProviders()
{
	const UUpgradeSettings* Settings = GetDefault<UUpgradeSettings>();
	FString UpgradeDataFolderPath = Settings->UpgradeDataFolderPath;
	
	UUpgradeDataProvider* Scanner = NewObject<UUpgradeDataProvider>(this);
	return Scanner->Scan(UpgradeDataFolderPath);
}

void UUpgradeManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	const TArray<UUpgradeDataProvider*> DataProviders = InitializeProviders();
	LoadCatalog(DataProviders);
}

void UUpgradeManagerSubsystem::LoadCatalog(TArray<UUpgradeDataProvider*> DataProviders)
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

bool UUpgradeManagerSubsystem::CanUpgrade(const int32 ComponentId, const int32 LevelIncrease, const TMap<FName, int32>& AvailableResources) const
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

bool UUpgradeManagerSubsystem::HandleUpgradeRequest(const int32 ComponentId, const int32 LevelIncrease, const TMap<FName, int32>& AvailableResources)
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
