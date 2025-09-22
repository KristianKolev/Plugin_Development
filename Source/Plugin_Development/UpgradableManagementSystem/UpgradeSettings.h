#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UpgradeSettings.generated.h"


USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FUpgradeJsonFieldNames
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString UpgradePathField = TEXT("UpgradePath");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString LevelsField = TEXT("Levels");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString ResourcesField = TEXT("Resources");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString ResourceTypeField = TEXT("Type");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString ResourceAmountField = TEXT("Amount");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString UpgradeSecondsField = TEXT("UpgradeSeconds");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString UpgradeLockedField = TEXT("bUpgradeLocked");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString UpgradePathIdField = TEXT("UpgradePathId");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString MaxLevelField = TEXT("MaxLevel");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString LevelOverridesField = TEXT("LevelOverrides");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString UpgradeLevelField = TEXT("UpgradeLevel");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString UpgradeResourceCostsField = TEXT("UpgradeResourceCosts");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString CostScalingSegmentsField = TEXT("CostScalingSegments");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString ScalingSegmentsField = TEXT("ScalingSegments");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString TimeScalingSegmentsField = TEXT("TimeScalingSegments");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString StartLevelField = TEXT("StartLevel");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString EndLevelField = TEXT("EndLevel");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString ScalingModeField = TEXT("ScalingMode");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString ConstantCostField = TEXT("ConstantCost");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString LinearSlopeField = TEXT("LinearSlope");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString ExpRateField = TEXT("ExpRate");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString PolyCoeffField = TEXT("PolyCoeff");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString PolyPowerField = TEXT("PolyPower");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString PolyOffsetField = TEXT("PolyOffset");

	UPROPERTY(EditAnywhere, Config, Category="Field Names")
	FString CustomFunctionNameField = TEXT("CustomFunctionName");
};

UCLASS(config=Game, defaultconfig, meta=(DisplayName="Upgrade System Settings"))
class UUpgradeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	public:
	// Folder containing upgrade definition files of any supported type
	// For assets this should be an "/Game/..." path. JSON files will be loaded from the corresponding directory on disk.
	UPROPERTY(EditAnywhere, config, Category="Upgrade Catalog")
	FString UpgradeDataFolderPath = TEXT("/Game/Data/UpgradeDefinitions");

	// Custom JSON field names if your files use different keys
	UPROPERTY(EditAnywhere, config, Category="Upgrade Catalog")
	FUpgradeJsonFieldNames JsonFieldNames;
};
