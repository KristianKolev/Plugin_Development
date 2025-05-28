// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UpgradeLevelData.h"
#include "Upgradable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UUpgradable : public UInterface
{
	GENERATED_BODY()
};

class PLUGIN_DEVELOPMENT_API IUpgradable
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgradable")
	int32 GetCurrentUpgradeLevel() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgradable")
	bool CanUpgrade() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgradable")
	void RequestUpgrade(int32 LevelIncrease);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgradable")
	EUpgradableCategory GetUpgradableCategory() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgradable")
	EUpgradableAspect GetUpgradableAspect() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Upgradable")
	int32 GetComponentId() const;
};
