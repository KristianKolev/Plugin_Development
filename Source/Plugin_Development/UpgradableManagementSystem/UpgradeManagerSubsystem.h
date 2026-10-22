// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradableComponent.h"
#include "UpgradeDataProvider.h"
#include "Subsystems/WorldSubsystem.h"
#include "Logging/LogMacros.h"
#include "UpgradeSubsystemBase.h"
#include "UpgradeManagerSubsystem.generated.h"


class UUpgradableComponent;
class UUpgradeJsonProvider;

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeManagerSubsystem : public UUpgradeSubsystemBase
{
	GENERATED_BODY()
	
public:
	
	UUpgradeManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

};
