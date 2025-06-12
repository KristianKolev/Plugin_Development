// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ResourceComponent.h"
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

	/**
 * Adds the specified amount of a resource for the given PlayerState.
 * Broadcasts OnResourceChanged after updating.
 */
	UFUNCTION(BlueprintCallable, Category="Resources")
	void AddResource(UResourceComponent* ResourceComponent, FName ResourceName, int32 Amount);

	/**
	 * Returns the current amount of the resource for the given PlayerState.
	 */
	UFUNCTION(BlueprintCallable, Category="Resources")
	int32 GetResource(const UResourceComponent* ResourceComponent, FName ResourceName) const;

	/**
	 * Attempts to spend the specified amount of resource for the given PlayerState.
	 * Returns true if the PlayerState had enough and amount was deducted.
	 * Broadcasts OnResourceChanged if successful.
	 */
	UFUNCTION(BlueprintCallable, Category="Resources")
	bool SpendResource(UResourceComponent* ResourceComponent, FName ResourceName, int32 Amount);

private:
	/** Internal map of PlayerState to its resource bucket */
	TMap<TWeakObjectPtr<UResourceComponent>, FResourceBucket> PlayerResourceMap;
	
};
