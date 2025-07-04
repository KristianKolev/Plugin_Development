// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceSystemComponent.generated.h"

class UResourceManagerSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResourceChanged, FName, ResourceName, int32, NewAmount, int32, DeltaAmount);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PLUGIN_DEVELOPMENT_API UResourceSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UResourceSystemComponent();
	
	/** Adds Amount of ResourceName to the owner player's bucket */
	UFUNCTION(BlueprintCallable, Category="Resource System")
	void AddResource(FName ResourceName, int32 Amount);

	/** Spends Amount if available*/
	UFUNCTION(BlueprintCallable, Category="Resource System")
	void SpendResource(FName ResourceName, int32 Amount);

	/** Returns the current amount of ResourceName for the owner */
	UFUNCTION(BlueprintPure, Category="Resource System")
	int32 GetResource(FName ResourceName) const;

	UFUNCTION(BlueprintPure, Category="Resource System")
	void GetAllResources(TMap<FName, int32>& OutAvailableResources) const;

	/** Event fired when this player's resource changes - when adding or spending */
	UPROPERTY(BlueprintAssignable, Category="Resource System")
	FOnResourceChanged OnResourceChanged;

	/** Server RPC to forward client requests */
	UFUNCTION(Server, Reliable)
	void Server_AddResource(FName ResourceName, int32 Amount);

	UFUNCTION(Server, Reliable)
	void Server_SpendResource(FName ResourceName, int32 Amount);

	/** Client RPCs */
	UFUNCTION(Client, Reliable)
	void Client_UpdateResource(FName ResourceName, int32 NewAmount, int32 DeltaAmount);
	
	UFUNCTION(BlueprintNativeEvent, Category="Resource System")
	void HandleResourceChanged(FName ResourceName, int32 NewAmount, int32 AmountChange);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	TMap<FName, int32> LocalResources;
	
	/** Cached pointer to the WorldSubsystem */
	UResourceManagerSubsystem* GetResourceSubsystem() const;

};
