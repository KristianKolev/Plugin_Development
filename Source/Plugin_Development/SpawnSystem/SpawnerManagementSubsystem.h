// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpawnerManagementSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API USpawnerManagementSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	USpawnerManagementSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

protected:

	
};
