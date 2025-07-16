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
	bool CanMerge(int32 ComponentId);
	bool HandleMergeRequest(int32 ComponentId);
};
