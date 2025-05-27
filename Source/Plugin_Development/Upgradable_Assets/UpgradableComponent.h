// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Upgradable.h"
#include "UpgradableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelChangedDelegate, int32, OldLevel, int32, NewLevel);

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class PLUGIN_DEVELOPMENT_API UUpgradableComponent : public UActorComponent, public IUpgradable
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUpgradableComponent();
	


	UPROPERTY(BlueprintAssignable, Category = "Upgradable")
	FOnLevelChangedDelegate OnLevelChanged;

	virtual int32 GetUpgradeLevel_Implementation() const override { return CurrentLevel; }

	virtual void RequestUpgrade_Implementation() override;
	
	void ApplyUpgradeInternal();

private:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_CurrentLevel();

	UPROPERTY(ReplicatedUsing=OnRep_CurrentLevel)
	int32 CurrentLevel = 0;

	virtual bool CanUpgrade_Implementation() const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestUpgrade();
	void Server_RequestUpgrade_Implementation();
	bool Server_RequestUpgrade_Validate();

};	