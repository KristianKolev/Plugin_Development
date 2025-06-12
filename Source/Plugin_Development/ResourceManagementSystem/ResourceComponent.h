// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceComponent.generated.h"

class UResourceManagerSubsystem;
class APlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResourceChanged, FName, ResourceName, int32, NewAmount, int32, DeltaAmount);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PLUGIN_DEVELOPMENT_API UResourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UResourceComponent();

	/** Adds Amount of ResourceName to the owner player's bucket */
	UFUNCTION(BlueprintCallable, Category="Resources")
	void AddResource(FName ResourceName, int32 Amount);

	/** Spends Amount if available; returns true on success */
	UFUNCTION(BlueprintCallable, Category="Resources")
	bool SpendResource(FName ResourceName, int32 Amount);

	/** Returns the current amount of ResourceName for the owner */
	UFUNCTION(BlueprintPure, Category="Resources")
	int32 GetResource(FName ResourceName) const;

	/** Event fired when this player's resource changes */
	UPROPERTY(BlueprintAssignable, Category="Resources")
	FOnResourceChanged OnResourceChanged;
	
	UFUNCTION(BlueprintNativeEvent, Category="Resources")
	void HandleResourceChanged(FName ResourceName, int32 NewAmount, int32 AmountChange);

protected:
	virtual void BeginPlay() override;

private:
	
	/** Cached pointer to the WorldSubsystem */
	UResourceManagerSubsystem* GetWorldSubsystem() const;

	/** Handler for subsystem's broadcast, filters by owner */
	
};
