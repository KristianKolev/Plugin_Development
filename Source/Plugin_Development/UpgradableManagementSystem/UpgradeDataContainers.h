#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "UpgradeDataContainers.generated.h"

USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FUpgradeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> ResourceTypeIndices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> UpgradeCosts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UpgradeSeconds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUpgrading = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUpgradeLocked = false;
};

USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FUpgradeDefinitionAsset : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UpgradeLevel = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, int32> UpgradeResourceCosts;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UpgradeSeconds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUpgradeLocked = false;
};

USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FLevelUpVisuals
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, UStaticMesh*> StaticMeshPerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, UMaterialInstance*> MaterialInstancePerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, UNiagaraSystem*> NiagaraSystemPerLevel;
};

UENUM(BlueprintType)
enum class EUpgradableCategory : uint8
{
	None = 0,
	Unit,
	Building,
	Equipment
};

UENUM(BlueprintType)
enum class EUpgradableAspect : uint8
{
	None = 0,
	Level,
	Tier,
	Rank,
	Star
};

USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FUpgradeInProgressData
{
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<FName, int32> UpgradeResourceCost = {};
	
	// Maps each component ID to its upgrade timer.
	UPROPERTY()
	FTimerHandle UpgradeTimerHandle;

	UPROPERTY()
	float TotalUpgradeTime = 0.0f;

	// Maps each component ID to the level increase requested by the client.
	UPROPERTY()
	int32 RequestedLevelIncrease = 1;
};

UENUM(BlueprintType)
enum class ECostScalingMode : uint8
{
	HardCoded,
	Constant,
	Linear,
	Exponential,
	Polynomial,
	Custom
};

USTRUCT(BlueprintType)
struct FRequirementsScalingSegment
{
	GENERATED_BODY()

	/** Inclusive start level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StartLevel = 0;

	/** Exclusive end level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 EndLevel = 0;

	/** Scaling mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECostScalingMode ScalingMode = ECostScalingMode::HardCoded;

	/** Only for Constant mode: exact cost every level in [Start,End) */
	UPROPERTY(EditAnywhere, meta=(EditCondition="ScalingMode==ECostScalingMode::Constant"))
	int32 ConstantCost = 0;

	/** Linear parameters (Cost = Prev + Slope) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ScalingMode==ECostScalingMode::Linear"))
	float LinearSlope = 10.f;

	/** Exponential parameters (Cost = Prev × Rate) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ScalingMode==ECostScalingMode::Exponential"))
	float ExpRate = 1.2f;

	/** Polynomial parameters (Cost = Coeff × Level^Power + Offset) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ScalingMode==ECostScalingMode::Polynomial"))
	float PolyCoeff = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ScalingMode==ECostScalingMode::Polynomial"))
	float PolyPower = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ScalingMode==ECostScalingMode::Polynomial"))
	float PolyOffset = 0.f;
	
	/** Blueprint hook for custom formulas */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ScalingMode==ECostScalingMode::Custom"))
	FName CustomFunctionName;
};

USTRUCT(BlueprintType)
struct FCostsScalingSegment : public FRequirementsScalingSegment
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FTimeScalingSegment : public FRequirementsScalingSegment
{
	GENERATED_BODY()
};