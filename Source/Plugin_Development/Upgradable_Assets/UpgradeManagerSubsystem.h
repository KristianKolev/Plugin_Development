// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradableComponent.h"
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
	void LoadJsonFromFile();
	bool CanUpgrade(const UUpgradableComponent* Component) const;

	UFUNCTION(BlueprintCallable, Category = "Upgrade Status")
	int32 GetCurrentLevel(const UUpgradableComponent* Component) const { return Component->GetCurrentUpgradeLevel_Implementation(); }

	UFUNCTION(BlueprintCallable, Category = "Upgrade Status")
	TArray<int32> GetNextLevelUpgradeCosts(const UUpgradableComponent* Component) const ;

	UFUNCTION(BlueprintCallable, Category = "Upgrade Status")
	int32 GetMaxLevel() const ;
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade Status")
	void UpgradeComponent(UUpgradableComponent* Component) const { HandleUpgradeRequest(Component) ;}

protected:
	UPROPERTY()
	TObjectPtr<UUpgradeJsonProvider> Provider;
};
