// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ResourceDefinition.generated.h"

/**
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API UResourceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique identifier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource System")
	FName ResourceName;

	/** Designer-assigned icon for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource System")
	UTexture2D* Icon;

	/** Short description or tooltip */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource System", meta=(MultiLine=true))
	FText Description;
};

UENUM(BlueprintType)
enum class EResourceType : uint8
{
	None = 0,
	Food,
	Water,
	Wood,
	Stone,
	Iron,
	Gold,
	Moonstone,
	Energy,
	Power
};