// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UpgrageSubsystemBase.generated.h"

/**
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgrageSubsystemBase : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UUpgrageSubsystemBase();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;


};
