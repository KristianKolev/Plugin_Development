// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradableComponent.h"
#include "UpgradeDataProvider.h"
#include "Subsystems/WorldSubsystem.h"
#include "UpgradeManagerSubsystem.generated.h"

class UUpgradableComponent;
class UUpgradeJsonProvider;

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	
	UUpgradeManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	void InitializeProvider();
	int32 RegisterUpgradableComponent(UUpgradableComponent* Component);
	void UnregisterUpgradableComponent(int32 ComponentId);

	bool CanUpgrade(int32 ComponentId, int32 LevelIncrease, const TMap<FName, int32>& AvailableResources) const;
	bool HandleUpgradeRequest(int32 ComponentId, int32 LevelIncrease, const TMap<FName, int32>& AvailableResources);

	/** Gets an upgradable component by its unique ID */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	UUpgradableComponent* GetComponentById(int32 Id) const;

	/** Returns the first UUpgradableComponent on TargetActor with matching Aspect. */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	UUpgradableComponent* FindComponentOnActorByAspect(AActor* TargetActor, EUpgradableAspect Aspect) const;

	/** Returns the first UUpgradableComponent on TargetActor with matching Category. */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	UUpgradableComponent* FindComponentOnActorByCategory(AActor* TargetActor, EUpgradableCategory Category) const;
	
	/**
	* Returns an array of every UUpgradableComponent in the world matching the specified filters.
	* @param Aspect The aspect to filter by.
	* @param LevelFilter If >= 0, only returns those whose current level == LevelFilter.
	*/
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Query")
	TArray<UUpgradableComponent*> GetComponentsByAspect(EUpgradableAspect Aspect, int32 LevelFilter = -1) const;

	/**
	 * Returns an array of every UUpgradableComponent in the world with UpgradePathId == PathId.
	 * If LevelFilter >= 0, only returns those whose current level == LevelFilter.
	 * @param PathId The path ID to filter by.
	 * @param LevelFilter If >= 0, only returns those whose current level == LevelFilter.
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Query")
	TArray<UUpgradableComponent*> GetComponentsByUpgradePath(FName PathId, int32 LevelFilter = -1) const;

	UFUNCTION(BlueprintCallable, Category="Upgrade System|Query")
	TArray<FUpgradeDefinition> GetUpgradeDefinitionsForPath(FName PathId) const;
	/**
		* Attempts to upgrade the component on TargetActor whose Aspect == the enum passed in.
		* Returns true if the request was sent/accepted.
		*/
	UFUNCTION(BlueprintCallable, Category="Upgrade System")
	bool RequestUpgradeForActor(AActor* TargetActor,
								EUpgradableAspect Aspect,
								int32 LevelIncrease,
								const TArray<FName>& ResourceTypes,
								const TArray<int32>& ResourceAmounts);

	/** Attempts to upgrade a component by the specified number of levels */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System")
	bool UpgradeComponent(const int32 ComponentId, const TMap<FName, int32> AvailableResources, const int32 LevelIncrease = 1) { return HandleUpgradeRequest(ComponentId, LevelIncrease, AvailableResources) ;}

	/**
	* Returns the current, client-visible level of the component on TargetActor
	* whose Aspect == the enum passed in. Returns –1 if none.
	*/
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Status")
	int32 GetUpgradeLevelForActor(AActor* TargetActor, EUpgradableAspect Aspect) const;
	
	/** Returns the current level of the specified component */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	int32 GetCurrentLevel(const int32 ComponentId) const ;

	/** Returns the next level that the component can be upgraded to */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	int32 GetNextLevel(const int32 ComponentId) const ; 

	/** Returns the maximum level achievable for the component */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	int32 GetMaxLevel(int32 ComponentId) const;

	/** Retrieves the resource costs required for the next level upgrade */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	void GetNextLevelUpgradeCosts(int32 ComponentId, TMap<FName, int32>& ResourceCosts) const;

	/** Gets the time required for the next level upgrade */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	int32 GetNextLevelUpgradeTime(const int32 ComponentId) const ;

	/** Retrieves upgrade data for a specific level */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	void GetUpgradeDataForLevel(int32 ComponentId, int32 Level, FUpgradeDefinition& LevelData) const { LevelData = *GetUpgradeDefinitionForLevel(ComponentId, Level);};
	
	/** Get existing or add new resource type, return index */
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Resources")
	int32 GetResourceTypeIndex(const FName& TypeName) const;

	/** Gets the resource type name for the specified index */
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Resources")
	FName GetResourceTypeName(int32 Index) const;

protected:

	// Catalog: Map path ID -> definition levels
	TMap<FName, TArray<FUpgradeDefinition>> UpgradeCatalog;

	// In your subsystem header:
	UPROPERTY()
	TArray<TWeakObjectPtr<UUpgradableComponent>> RegisteredComponents;
	
	// Maps component-ID → current, authoritative level.
	UPROPERTY()
	TArray<int32> ComponentLevels;		

	/** List of all available resource types in the system */
	UPROPERTY(BlueprintReadOnly, Category = "Upgrade System|Resources")
	TArray<FName> ResourceTypes;
	
	// Stack of free slots:
	UPROPERTY()
	TArray<int32> FreeIndices;

	UPROPERTY()
	FString UpgradeDataFolderPath;

	UPROPERTY()
	TObjectPtr<UUpgradeDataProvider> DataProvider;

	// Loaders
	void LoadCatalog();

	// Helpers
	void CleanupFreeIndices();

	void UpdateUpgradeLevel(const int32 ComponentId, const int32 NewLevel);
	const TArray<FUpgradeDefinition>* GetUpgradeDefinitions(FName UpgradePathId) const;
	const TArray<FUpgradeDefinition>* GetUpgradeDefinitions(int32 ComponentId) const;
	const FUpgradeDefinition* GetUpgradeDefinitionForLevel(int32 ComponentId, int32 Level) const;

};
