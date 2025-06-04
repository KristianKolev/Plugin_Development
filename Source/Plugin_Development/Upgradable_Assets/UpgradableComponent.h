// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
//#include "Upgradable.h"
#include "UpgradeLevelData.h"
#include "UpgradableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelChangedDelegate, int32, OldLevel, int32, NewLevel);

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class PLUGIN_DEVELOPMENT_API UUpgradableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUpgradableComponent();
	
	UPROPERTY(BlueprintAssignable, Category = "Upgradable")
	FOnLevelChangedDelegate OnLevelChanged;

	// Unique identifier for this upgrade path
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgradable")
	FName UpgradePathId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgradable")
	int32 InitialLevel = 0;

	UFUNCTION(BlueprintCallable, Category="Upgradable")
	EUpgradableCategory GetUpgradableCategory() const { return Category; }
	
	UFUNCTION(BlueprintCallable, Category="Upgradable")
	EUpgradableAspect GetUpgradableAspect() const { return Aspect; }
	
	UFUNCTION(BlueprintCallable, Category="Upgradable")
	int32 GetComponentId() const { return UpgradableID; }
	
	UFUNCTION(BlueprintCallable, Category="Upgradable")
	void RequestUpgrade(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts);

	UFUNCTION(BlueprintCallable, Category="Upgradable")
	int32 GetCurrentUpgradeLevel() const { return LocalLevel; }
	
	UFUNCTION(Client, Reliable)
	void Client_SetLevel(int32 NewLevel);
	void Client_SetLevel_Implementation(int32 NewLevel);


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp Visuals")
	FUpgradeLevelDataVisuals LevelDataVisuals;
	
	UPROPERTY()
	int32 UpgradableID = -1;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Upgradable")
	int32 LocalLevel = 0;

	UPROPERTY(Blueprintable, BlueprintReadWrite, EditAnywhere, Category = "Upgradable")
	EUpgradableCategory Category = EUpgradableCategory::None;
	
	UPROPERTY(Blueprintable, BlueprintReadWrite, EditAnywhere, Category = "Upgradable")
	EUpgradableAspect Aspect = EUpgradableAspect::Level;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestUpgrade(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts);
	void Server_RequestUpgrade_Implementation(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts);
	bool Server_RequestUpgrade_Validate(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts);


	
};