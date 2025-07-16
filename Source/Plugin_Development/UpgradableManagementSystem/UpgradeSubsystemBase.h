// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataContainers.h"
#include "Subsystems/WorldSubsystem.h"
#include "UpgradeSubsystemBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUpgradeSystem, Log, All);

class UUpgradableComponent;
/**
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeSubsystemBase : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UUpgradeSubsystemBase();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	int32 RegisterUpgradableComponent(UUpgradableComponent* Component);
	void UnregisterUpgradableComponent(int32 ComponentId);

	void UpdateUpgradeLevel(const int32 ComponentId, const int32 NewLevel);

	// START section getters that return UUpgradableComponent
	
	/** Gets an upgradable component by its unique ID */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Query")
	UUpgradableComponent* GetComponentById(int32 Id) const;
	
	/** Returns the first UUpgradableComponent on TargetActor with matching Aspect. */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Query")
	UUpgradableComponent* FindComponentOnActorByAspect(AActor* TargetActor, EUpgradableAspect Aspect) const;

	/** Returns the first UUpgradableComponent on TargetActor with matching Category. */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Query")
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

	// END section getters that return UUpgradableComponent

	// START section getters that return FUpgradeDefinition
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Query")
	TArray<FUpgradeDefinition> GetUpgradeDefinitionsForPath(FName PathId) const;

	/** Retrieves upgrade data for a specific level */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Query")
	void GetUpgradeDataForLevel(int32 ComponentId, int32 Level, FUpgradeDefinition& LevelData) const { LevelData = *GetUpgradeDefinitionForLevel(ComponentId, Level);};

	// END section getters that return FUpgradeDefinition
	
	// START section getters that return level status
	
	/** Returns the current level of the specified component */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	int32 GetCurrentLevel(const int32 ComponentId) const ;

	/**
	* Returns the current, client-visible level of the component on TargetActor
	* whose Aspect == the enum passed in. Returns –1 if none.
	*/
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Status")
	int32 GetUpgradeLevelForActor(AActor* TargetActor, EUpgradableAspect Aspect) const;
	
	/** Returns the next level that the component can be upgraded to */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	int32 GetNextLevel(const int32 ComponentId) const ;
	
	/** Returns the maximum level achievable for the component */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	int32 GetMaxLevel(int32 ComponentId) const;

	/**
	* @return - -1 if no upgrade is in progress
	*/
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Status")
	int32 GetInProgressLevelIncrease(int32 ComponentId) const ;
	
	// END section getters that return level status

	// START section upgrade timers
	
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Timer")
	void CancelUpgrade(int32 ComponentId);
	
	// Is an upgrade in progress on the component
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Timer")
	bool IsUpgradeTimerActive(int32 ComponentId) const {return UpgradeInProgressData.Contains(ComponentId);}

	/** Gets the time required for the next level upgrade */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Timer")
	int32 GetNextLevelUpgradeTime(const int32 ComponentId) const ;

	/**
	 * Updates the upgrade timer for the specified component by the specified amount of time.
	 
	 * @param DeltaTime - Increase or reduce the timer by this amount of seconds.
	 * @return - The remaining time until completion or -1.f if the upgrade was completed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Timer")
	float UpdateUpgradeTimer(int32 ComponentId, float DeltaTime);
	
	/**
	 * @return - -1 if no upgrade is in progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Timer")
	float GetUpgradeTimeRemaining(int32 ComponentId) const;

	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Timer")
	float GetInProgressTotalUpgradeTime(int32 ComponentId) const;
	// END section upgrade timers

	// START section getters that return resource info

	/** Get the index from the encountered resources array for the specified resource type from the */
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Resources")
	int32 GetResourceTypeIndex(const FName& TypeName) const;

	/** Gets the resource type name for the specified index of the encountered resources array */
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Resources")
	FName GetResourceTypeName(int32 Index) const;
	
	/** Retrieves the resource costs required for the next level upgrade */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Resources")
	void GetNextLevelUpgradeCosts(int32 ComponentId, TMap<FName, int32>& ResourceCosts) const;

	/** Retrieves the resource costs required for the given levels upgrade */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Resources")
	TMap<FName, int32> GetUpgradeTotalResourceCost (int32 ComponentId, int32 LevelIncrease) const;
	
	// Not implemented yet. Use GetNextLevelUpgradeCosts
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Resources")
	TMap<FName, int32> GetInProgressTotalResourceCost(int32 ComponentId) const;
	
	// END section getters that return resource info
	
protected:

	// Catalog of each Upgrade Path and its corresponding level progression.
	TMap<FName, TArray<FUpgradeDefinition>> UpgradeCatalog;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<UUpgradableComponent>> RegisteredComponents;
	
	// Maps each component ID to the data for their pending upgrade.
	UPROPERTY()
	TMap<int32, FUpgradeInProgressData> UpgradeInProgressData;

	/** List of all available resource types in the system as encountered in the catalog.*/
	UPROPERTY(BlueprintReadOnly, Category = "Upgrade System|Resources")
	TArray<FName> ResourceTypes;
	
	// Stores the current level of each component ID.
	// TODO make this into a struct of component data. Cache them locally in the manager when the component registers itself. To be used for quick lookup instead of looping over all components in query functions.
	UPROPERTY()
	TArray<int32> ComponentLevels;
	
	/* Stack of free slots to be assigned to new components.
	* Used to avoid re-allocating memory for new components when de-/registering.
	*/
	UPROPERTY()
	TArray<int32> FreeComponentIndices;
	
	// Upgrade Timer functions
	float GetUpgradeTimerDuration(int32 ComponentId, int32 LevelIncrease) const;
	float StartUpgradeTimer(int32 ComponentId, float TimerDuration);

	void StopUpgradeTimer(int32 ComponentId);
	
	UFUNCTION()
	void OnUpgradeTimerFinished(int32 ComponentId);

	const TArray<FUpgradeDefinition>* GetUpgradeDefinitions(FName UpgradePathId) const;
	const TArray<FUpgradeDefinition>* GetUpgradeDefinitions(int32 ComponentId) const;
	const FUpgradeDefinition* GetUpgradeDefinitionForLevel(int32 ComponentId, int32 Level) const;

	// Helpers
	void CleanupFreeIndices();
};
