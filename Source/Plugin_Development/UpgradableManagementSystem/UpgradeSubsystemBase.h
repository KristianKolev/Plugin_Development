// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataContainers.h"
#include "Subsystems/WorldSubsystem.h"
#include "UpgradeSubsystemBase.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogUpgradeSystem, Log, All);

class UUpgradableComponent;
class AActor;
class UUpgradeDataProvider;

USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FUpgradableComponentData
{
	GENERATED_BODY()
	
	UPROPERTY()
	FName UpgradePathId = NAME_None;
		
	UPROPERTY()
	int32 Level = -1;
	
	UPROPERTY()
	TWeakObjectPtr<UUpgradableComponent> Component = nullptr;

	UPROPERTY()
	TWeakObjectPtr<AActor> Owner = nullptr;

	UPROPERTY()
	EUpgradableAspect Aspect = EUpgradableAspect::None;

	UPROPERTY()
	EUpgradableCategory Category = EUpgradableCategory::None;

	// only compare the four “key” fields:
	bool operator==(FUpgradableComponentData const& Other) const
	{
		return UpgradePathId == Other.UpgradePathId
			&& Level         == Other.Level
			&& Aspect        == Other.Aspect
			&& Category      == Other.Category;
	}

	bool operator!=(FUpgradableComponentData const& Other) const
	{
		return !(*this == Other);
	}

};
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

	// START section upgrade handling
	
	bool CanUpgrade(int32 ComponentId, int32 LevelIncrease, const TMap<FName, int32>& AvailableResources) const;
	bool HandleUpgradeRequest(const int32 ComponentId, const int32 LevelIncrease, const TMap<FName, int32>& AvailableResources);
	void UpdateUpgradeLevel(const int32 ComponentId, const int32 NewLevel);

	/**
		* Attempts to upgrade the component on TargetActor whose Aspect == the enum passed in.
		* Returns true if the request was sent/accepted.
		*/
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Upgrade")
	bool RequestUpgradeForActor(AActor* TargetActor,
								EUpgradableAspect Aspect,
								int32 LevelIncrease,
								const TArray<FName>& ResourceTypesArray,
								const TArray<int32>& ResourceAmounts) const;

	/** Attempts to upgrade a component by the specified number of levels */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Upgrade")
	bool UpgradeComponent(const int32 ComponentId, const TMap<FName, int32> AvailableResources, const int32 LevelIncrease = 1) { return HandleUpgradeRequest(ComponentId, LevelIncrease, AvailableResources) ;}

	// END section upgrade handling
	
	// START section getters that return UUpgradableComponent
	
	/** Gets an upgradable component by its unique ID */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Query")
	UUpgradableComponent* GetComponentById(const int32 Id) const;
	
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

	UFUNCTION(BlueprintCallable, Category="Upgrade System|Query")
	TArray<UUpgradableComponent*> GetComponentsByActor (AActor* TargetActor) ;
	
	UFUNCTION(BlueprintCallable, Category="Upgrade System|Query")
	FUpgradableComponentData GetComponentDataByID(const int32 ComponentId) const
	{ return ComponentData.IsValidIndex(ComponentId) ? ComponentData[ComponentId] : FUpgradableComponentData(); };

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
	
	// Is an upgrade in progress on the component?
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Timer")
	bool IsUpgradeTimerActive(int32 ComponentId) const {return UpgradeInProgressData.Contains(ComponentId);}

	/** Gets the time required for the next level upgrade */
	UFUNCTION(BlueprintCallable, Category = "Upgrade System|Timer")
	int32 GetNextLevelUpgradeTime(const int32 ComponentId) const ;

	/**
	 * Updates the upgrade timer for the specified component by the specified amount of time.
	 
	 * @param DeltaTime - Increase or reduce the timer by this number of seconds.
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

	/** Cached data for each registered component indexed by component ID */
	UPROPERTY()
	TArray<FUpgradableComponentData> ComponentData;

	// Maps each component ID to the data for their pending upgrade.
	UPROPERTY()
	TMap<int32, FUpgradeInProgressData> UpgradeInProgressData;

	/** List of all available resource types in the system as encountered in the catalog.*/
	UPROPERTY(BlueprintReadOnly, Category = "Upgrade System|Resources")
	TArray<FName> ResourceTypes;

	/* Stack of free slots to be assigned to new components.
	 * Used to avoid re-allocating memory for new components when de-/registering.
	 */
	UPROPERTY()
	TArray<int32> FreeComponentIndices;

	// Loaders
	TArray<UUpgradeDataProvider*> InitializeProviders();
	void LoadCatalog(TArray<UUpgradeDataProvider*> DataProviders);
	
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
