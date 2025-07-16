// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradableComponent.h"
#include "UpgradeDataProvider.h"
#include "Subsystems/WorldSubsystem.h"
#include "Logging/LogMacros.h"
#include "UpgrageSubsystemBase.h"
#include "UpgradeManagerSubsystem.generated.h"


class UUpgradableComponent;
class UUpgradeJsonProvider;

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeManagerSubsystem : public UUpgrageSubsystemBase
{
	GENERATED_BODY()
	
public:
	
	UUpgradeManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	TArray<UUpgradeDataProvider*> InitializeProviders();

	bool CanUpgrade(int32 ComponentId, int32 LevelIncrease, const TMap<FName, int32>& AvailableResources) const;
	bool HandleUpgradeRequest(int32 ComponentId, int32 LevelIncrease, const TMap<FName, int32>& AvailableResources);
	

	/**
		* Attempts to upgrade the component on TargetActor whose Aspect == the enum passed in.
		* Returns true if the request was sent/accepted.
		*/
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Upgrade")
	bool RequestUpgradeForActor(AActor* TargetActor,
								EUpgradableAspect Aspect,
								int32 LevelIncrease,
								const TArray<FName>& ResourceTypesArray,
								const TArray<int32>& ResourceAmounts);

	/** Attempts to upgrade a component by the specified number of levels */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Upgrade")
	bool UpgradeComponent(const int32 ComponentId, const TMap<FName, int32> AvailableResources, const int32 LevelIncrease = 1) { return HandleUpgradeRequest(ComponentId, LevelIncrease, AvailableResources) ;}


protected:

	// Loaders
	void LoadCatalog(TArray<UUpgradeDataProvider*> DataProviders);
	
};
