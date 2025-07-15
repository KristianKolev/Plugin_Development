
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeDataContainers.h"
#include "UpgradeDefinitionDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FCostSegmentsContainer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRequirementsScalingSegment> ScalingSegments;
};

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeDefinitionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Provide path ID for this upgrade definition (if None, will use asset name)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	FName UpgradePathId = NAME_None;
	// Should match the last Scaling Segments EndLevel
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	int32 MaxLevel = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TMap<FName, FCostSegmentsContainer> CostScalingSegments;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TArray<FRequirementsScalingSegment> TimeScalingSegments;
	
	// Override costs and bools for desired upgrade levels. UpgradeLevel variable shouldn't be higher than MaxLevel
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TArray<FUpgradeDefinitionAsset> LevelOverrides;
};