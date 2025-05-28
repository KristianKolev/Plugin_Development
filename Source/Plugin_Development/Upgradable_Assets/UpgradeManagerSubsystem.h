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
	void HandleUpgradeRequest(int32 ComponentId, int32 LevelIncrease) const;
	void LoadJsonFromFile();

	bool CanUpgrade(const UUpgradableComponent* Component) const;
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade Status")
	UUpgradableComponent* GetComponentById(int32 Id) const;
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade Status")
	int32 GetCurrentLevel(const UUpgradableComponent* Component) const { return Component->GetCurrentUpgradeLevel_Implementation(); }

	UFUNCTION(BlueprintCallable, Category = "Upgrade Status")
	TArray<int32> GetNextLevelUpgradeCosts(const UUpgradableComponent* Component) const ;

	UFUNCTION(BlueprintCallable, Category = "Upgrade Status")
	int32 GetMaxLevel() const ;
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade Status")
	void UpgradeComponent(const int32 ComponentId, const int32 LevelIncrease = 1) const { HandleUpgradeRequest(ComponentId, LevelIncrease) ;}

	int32 RegisterUpgradableComponent(UUpgradableComponent* Component);
	void UpdateUpgradeLevel(int32 ComponentId, int32 NewLevel) const;
	void UnregisterUpgradableComponent(int32 ComponentId);

protected:


	// In your subsystem header:
	UPROPERTY()
	TArray<TWeakObjectPtr<UUpgradableComponent>> RegisteredComponents;

	// Stack of free slots:
	UPROPERTY()
	TArray<int32> FreeIndices;

	UPROPERTY()
	TObjectPtr<UUpgradeJsonProvider> Provider;
};
