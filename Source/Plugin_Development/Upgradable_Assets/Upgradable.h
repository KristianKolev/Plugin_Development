// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Upgradable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUpgradable : public UInterface
{
	GENERATED_BODY()
};

class PLUGIN_DEVELOPMENT_API IUpgradable
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintNativeEvent, Category = "Upgradable")
	void Upgrade();
};
