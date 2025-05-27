

#pragma once

#include "CoreMinimal.h"
#include "UpgradeLevelData.generated.h"

USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FResourceType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;
};


USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FUpgradeLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FResourceType ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UpgradeCost;


};