// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

UCLASS()
class PLUGIN_DEVELOPMENT_API ASpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UBoxComponent* SpawningBox = nullptr;
};
