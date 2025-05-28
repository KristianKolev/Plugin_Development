#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "UpgradeLevelData.generated.h"

USTRUCT(BlueprintType)	//Maybe unnecessary and replaced by enum
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
struct PLUGIN_DEVELOPMENT_API FUpgradeLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<EResourceType> ResourceTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> UpgradeCosts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UpgradeSeconds = 0;
};

USTRUCT(BlueprintType)
struct PLUGIN_DEVELOPMENT_API FUpgradeLevelDataVisuals
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