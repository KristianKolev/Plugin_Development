// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UpgradeManagerSubsystem.generated.h"

class UUpgradableComponent;
class UUpgradeJsonProvider;

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	UUpgradeManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
public:
	void InitializeProviderFromJson(const FString& Json);
	void HandleUpgradeRequest(UUpgradableComponent* Component) const;
	bool CanUpgrade(const UUpgradableComponent* Component) const;

protected:
	UPROPERTY()
	TObjectPtr<UUpgradeJsonProvider> Provider;
};
