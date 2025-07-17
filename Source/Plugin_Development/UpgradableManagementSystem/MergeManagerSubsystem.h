// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeSubsystemBase.h"
#include "MergeManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMergeMode : uint8
{
	None,
	// Consolidates components of different levels
	Combine,
	// Consolidates wholly identical components 
	Merge,
	// Consolidates components of different upgrade paths
	Fuse
};
/**
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API UMergeManagerSubsystem : public UUpgradeSubsystemBase
{
	GENERATED_BODY()

public:

	UMergeManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// Expects that InComponents.Num() == ComponentsToMerge
	bool CanMerge(TArray<int32> InComponentIds, int32 ComponentsToMerge = 2, const TMap<FName, int32>& AvailableResources, EMergeMode MergeMode = {});
	bool HandleMergeRequest(TArray<int32> ComponentIds, int32 ComponentsToMerge = 2, const TMap<FName, int32>& AvailableResources, EMergeMode MergeMode = {}, TArray
	                        <int32>& OutConsumedComponentIds);
	void CleanupConsumedComponents(TArray<int32> ConsumedComponentIds);

	// Change for testing push to branch from rider
};
