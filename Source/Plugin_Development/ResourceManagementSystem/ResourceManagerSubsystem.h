// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ResourceDefinition.h"
#include "Subsystems/WorldSubsystem.h"
#include "ResourceSystemComponent.h"
#include "ResourceManagerSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FResourceBucket
{
	GENERATED_BODY()

	/** Map of resource identifier to current amount */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Resources")
	TMap<FName, int32> Resources;
};


UCLASS()
class PLUGIN_DEVELOPMENT_API UResourceManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	UResourceManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	/** Register a resource component (called in component's BeginPlay) */
	void RegisterComponent(UResourceSystemComponent* Comp);

	/** Unregister a resource component (called in component's EndPlay) */
	void UnregisterComponent(UResourceSystemComponent* Comp);
	/**
 * Adds the specified amount of a resource for the given PlayerState.
 * Broadcasts OnResourceChanged after updating.
 */
	UFUNCTION(BlueprintCallable, Category="Resources System")
	void AddResource(UResourceSystemComponent* ResourceComponent, FName ResourceName, int32 Amount);

	/**
	 * Returns the current amount of the resource for the given PlayerState.
	 */
	UFUNCTION(BlueprintCallable, Category="Resources System")
	int32 GetResource(const UResourceSystemComponent* ResourceComponent, FName ResourceName) const;

	/** Returns all resources for the given component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Resources System")
	void GetAllResources(const UResourceSystemComponent* Comp, TMap<FName, int32>& OutAvailableResources) const;

	/**
	 * Attempts to spend the specified amount of resource for the given PlayerState.
	 * Returns true if the PlayerState had enough and amount was deducted.
	 * Broadcasts OnResourceChanged if successful.
	 */
	UFUNCTION(BlueprintCallable, Category="Resources System")
	bool SpendResource(UResourceSystemComponent* ResourceComponent, FName ResourceName, int32 Amount);

	/** Returns nullptr if this name isn’t defined */
	UFUNCTION(BlueprintPure, Category="Resources System")
	UResourceDefinition* GetDefinition(FName ResourceName) const;

private:
	/** Internal map of PlayerState to its resource bucket */
	TMap<TWeakObjectPtr<UResourceSystemComponent>, FResourceBucket> ComponentResourceMap;

	/** Map of resource-name → its design-time data asset */
	UPROPERTY()
	TMap<FName, UResourceDefinition*> Definitions;
	
};
