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
