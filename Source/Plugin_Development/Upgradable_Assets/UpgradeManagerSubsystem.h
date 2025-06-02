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
	bool HandleUpgradeRequest(int32 ComponentId, int32 LevelIncrease);
	void LoadJsonFromFile();

	bool CanUpgrade(int32 ComponentId) const;
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade|Status")
	UUpgradableComponent* GetComponentById(int32 Id) const;
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade|Status")
	int32 GetCurrentLevel(const int32 ComponentId) const ;

	UFUNCTION(BlueprintCallable, Category = "Upgrade|Status")
	int32 GetNextLevel(const int32 ComponentId) const ; 

	UFUNCTION(BlueprintCallable, Category = "Upgrade|Status")
	void GetNextLevelUpgradeCosts(int32 ComponentId, TArray<int32>& ResourceCosts, TArray<FName>& Resources) const;

	UFUNCTION(BlueprintCallable, Category = "Upgrade|Status")
	int32 GetNextLevelUpgradeTime(const int32 ComponentId) const ;

	UFUNCTION(BlueprintCallable, Category = "Upgrade|Status")
	int32 GetMaxLevel(int32 ComponentId) const;
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade|Status")
	bool UpgradeComponent(const int32 ComponentId, const int32 LevelIncrease = 1) { return HandleUpgradeRequest(ComponentId, LevelIncrease) ;}

	/** Get existing or add new resource type, return index */
	UFUNCTION(BlueprintCallable, Category="Upgrade|Resources")
	int32 GetResourceTypeIndex(const FName& TypeName);

	UFUNCTION(BlueprintCallable, Category="Upgrade|Resources")
	FName GetResourceTypeName(int32 Index) const;
	
	int32 RegisterUpgradableComponent(UUpgradableComponent* Component);
	void UpdateUpgradeLevel(const int32 ComponentId, const int32 NewLevel);
	void UnregisterUpgradableComponent(int32 ComponentId);

protected:

	// Catalog: Map path ID -> definition levels
	TMap<FName, TArray<FUpgradeLevelData>> UpgradeCatalog;
	
	// Maps component-ID → current, authoritative level.
	UPROPERTY()
	TArray<int32> ComponentLevels;		

	// Dynamic resource type interning
	/** Shared list of resource type names */
	UPROPERTY(BlueprintReadOnly, Category = "Upgrade|Resources")
	TArray<FName> ResourceTypes;
	
	// In your subsystem header:
	UPROPERTY()
	TArray<TWeakObjectPtr<UUpgradableComponent>> RegisteredComponents;

	// Stack of free slots:
	UPROPERTY()
	TArray<int32> FreeIndices;

	UPROPERTY()
	TObjectPtr<UUpgradeJsonProvider> Provider;



	// Loaders
	void LoadCatalog();
	void LoadCatalogFromJsonDirectory();
	void LoadCatalogFromDataTables();
	void LoadCatalogFromDataAssetFolder();

	const TArray<FUpgradeLevelData>* GetUpgradeDefinitions(FName UpgradePathId) const;
	const TArray<FUpgradeLevelData>* GetUpgradeDefinitions(int32 ComponentId) const;
	const FUpgradeLevelData* GetUpgradeDefinitionForLevel(int32 ComponentId, int32 Level) const;
	// Helpers
	void CleanupFreeIndices();
};
