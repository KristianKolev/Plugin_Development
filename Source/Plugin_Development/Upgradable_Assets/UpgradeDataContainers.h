#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "UpgradeDataContainers.generated.h"

USTRUCT(BlueprintType)	//This will be useful for the resource management system. The upgrade system doesnt need to know resource descriptions
struct PLUGIN_DEVELOPMENT_API FResourceType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description {"Info"};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name {"Stuff"};
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