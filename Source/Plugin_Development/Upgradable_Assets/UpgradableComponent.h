// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Upgradable.h"
#include "UpgradableComponent.generated.h"


UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class PLUGIN_DEVELOPMENT_API UUpgradableComponent : public UActorComponent, public IUpgradable
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUpgradableComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Upgradable")
	int32 Level;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Upgradable")
	int32 MaxLevel;
	

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void Upgrade_Implementation();		
};
