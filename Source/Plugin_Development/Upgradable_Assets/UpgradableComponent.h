// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Upgradable.h"
#include "UpgradeLevelData.h"
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

	// IUpgradableInterface 
	virtual int32 GetCurrentUpgradeLevel_Implementation() const override { return CurrentLevel; }

	virtual EUpgradableCategory GetUpgradableCategory_Implementation() const override { return Category; }

	virtual EUpgradableAspect GetUpgradableAspect_Implementation() const override { return Aspect; }

	virtual int32 GetComponentId_Implementation() const override { return UpgradableID; }

	virtual void RequestUpgrade_Implementation(int32 LevelIncrease) override;

	virtual bool CanUpgrade_Implementation() const override;

	// IUpgradableInterface 
	
	void ApplyUpgradeInternal(int32 NewLevel);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp Visuals")
	FUpgradeLevelDataVisuals LevelDataVisuals;

	// Unique identifier for this upgrade path
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgradable")
	FName UpgradePathId;
	
	UPROPERTY()
	int32 UpgradableID = -1;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentLevel)
	int32 CurrentLevel = 0;

	UPROPERTY(Blueprintable, BlueprintReadWrite, EditAnywhere, Category = "Upgradable")
	EUpgradableCategory Category = EUpgradableCategory::None;
	
	UPROPERTY(Blueprintable, BlueprintReadWrite, EditAnywhere, Category = "Upgradable")
	EUpgradableAspect Aspect = EUpgradableAspect::Level;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnRep_CurrentLevel(int32 OldLevel);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestUpgrade(int32 LevelIncrease);
	void Server_RequestUpgrade_Implementation(int32 LevelIncrease);
	bool Server_RequestUpgrade_Validate(int32 LevelIncrease);

};