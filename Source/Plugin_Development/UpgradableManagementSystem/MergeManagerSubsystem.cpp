// Fill out your copyright notice in the Description page of Project Settings.


#include "MergeManagerSubsystem.h"

UMergeManagerSubsystem::UMergeManagerSubsystem()
{
}

void UMergeManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMergeManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMergeManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

bool UMergeManagerSubsystem::CanMerge(TArray<int32> InComponentIds, int32 ComponentsToMerge, const TMap<FName, int32>& AvailableResources, const EMergeMode MergeMode)
{
	if (ComponentsToMerge <= 0)
	{
		return false;
	}
	
	if (InComponentIds.Num() < ComponentsToMerge)
	{
		return false;
	}

	for (int i = 0; i < ComponentsToMerge - 1; i++)
	{
		const int32 CurrentCompId = InComponentIds[i];
		const int32 NextCompId = InComponentIds[i + 1];
		if (!ComponentData.IsValidIndex(CurrentCompId) || !ComponentData.IsValidIndex(NextCompId)) return false;
		if (ComponentData[CurrentCompId].Aspect != ComponentData[NextCompId].Aspect) return false;
		if (ComponentData[CurrentCompId].Category != ComponentData[NextCompId].Category) return false;

		switch (MergeMode)
		{
		case EMergeMode::Combine:
			if (ComponentData[CurrentCompId].UpgradePathId != ComponentData[NextCompId].UpgradePathId) return false;
			break;
			
		case EMergeMode::Merge:
			if (ComponentData[CurrentCompId].UpgradePathId != ComponentData[NextCompId].UpgradePathId) return false;
			if (ComponentData[CurrentCompId].Level != ComponentData[NextCompId].Level) return false;
			break;

		case EMergeMode::Fuse:
			if (ComponentData[CurrentCompId].Level != ComponentData[NextCompId].Level) return false;
			break;

			default:
			return false;
		}
		
	}
	return true;
}

bool UMergeManagerSubsystem::HandleMergeRequest(TArray<int32> ComponentIds, int32 ComponentsToMerge, const TMap<FName, int32>& AvailableResources, EMergeMode MergeMode, TArray
                                                <int32>& OutConsumedComponentIds)
{
	bool Success = false;
	if (!CanMerge(ComponentIds, ComponentsToMerge, AvailableResources, MergeMode)) return false;

	int32 UpgradingComponentId = ComponentIds[0];
	ComponentIds.RemoveAtSwap(0);
	for ( int32 i = 0; i < ComponentsToMerge - 1; ++i)
	{
		
		OutConsumedComponentIds.Add(ComponentIds[i]);
		ComponentIds.RemoveAtSwap(i);
	}

	if (HandleUpgradeRequest(UpgradingComponentId, 1, AvailableResources))
	{
		Success = true;
		if (ComponentIds.Num() >= ComponentsToMerge)
		{
			HandleMergeRequest(ComponentIds, ComponentsToMerge, AvailableResources, MergeMode, OutConsumedComponentIds);
		}
	}

	return Success;
}

void UMergeManagerSubsystem::CleanupConsumedComponents(TArray<int32> ConsumedComponentIds)
{
	
}
