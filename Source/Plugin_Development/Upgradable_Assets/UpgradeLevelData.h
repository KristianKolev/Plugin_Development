

#pragma once

#include "CoreMinimal.h"
#include "UpgradeLevelData.generated.h"

USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FResourceType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description {"Info"};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name {"Stuff"};
};


USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FUpgradeLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FResourceType> ResourceTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> UpgradeCosts;
};