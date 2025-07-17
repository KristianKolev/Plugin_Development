// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeSubsystemBase.h"
#include "MergeManagerSubsystem.generated.h"

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
	
	bool CanMerge(int32 ComponentId);
	bool HandleMergeRequest(int32 ComponentId);

	// Change for testing push to branch from rider
};
