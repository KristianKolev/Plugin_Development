#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UpgradeSettings.generated.h"

UENUM(BlueprintType)
enum class EUpgradeCatalogSource : uint8
{
	JsonFolder  UMETA(DisplayName = "JSON Folder"),
	DataTableFolder      UMETA(DisplayName = "Data Table Folder"),
        DataAssetFolder UMETA(DisplayName = "Data Asset Folder")
};

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
       FString UpgradeLockedField = TEXT("UpgradeLocked");
};

UCLASS(config=Game, defaultconfig, meta=(DisplayName="Upgrade System Settings"))
class UUpgradeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Select catalog source
	UPROPERTY(EditAnywhere, config, Category="Upgrade Catalog")
	EUpgradeCatalogSource CatalogSource = EUpgradeCatalogSource::JsonFolder;

	// Directory path under Content/ for JSON files
	UPROPERTY(EditAnywhere, config, Category="Upgrade Catalog", meta=(EditCondition="CatalogSource==EUpgradeCatalogSource::JsonFolder"))
	FString JsonFolderPath = TEXT("Data/UpgradeJSONs");

	// Directory path for DataTables: include /Game/...
	UPROPERTY(EditAnywhere, config, Category="Upgrade Catalog", meta=(EditCondition="CatalogSource==EUpgradeCatalogSource::DataTableFolder"))
	FString DataTableFolderPath = TEXT("/Game/Data/UpgradeDataTables");

       // Directory path for DataAssets: include /Game/...
       UPROPERTY(EditAnywhere, config, Category="Upgrade Catalog", meta=(EditCondition="CatalogSource==EUpgradeCatalogSource::DataAssetFolder"))
       FString DataAssetFolderPath = TEXT("/Game/Data/UpgradeDataAssets");

       // Custom JSON field names if your files use different keys
       UPROPERTY(EditAnywhere, config, Category="Upgrade Catalog", meta=(EditCondition="CatalogSource==EUpgradeCatalogSource::JsonFolder"))
       FUpgradeJsonFieldNames JsonFieldNames;
};