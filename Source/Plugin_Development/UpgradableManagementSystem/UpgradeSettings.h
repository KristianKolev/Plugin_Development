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
};