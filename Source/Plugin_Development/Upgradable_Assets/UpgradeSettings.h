#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UpgradeSettings.generated.h"

UENUM(BlueprintType)
enum class EUpgradeCatalogSource : uint8
{
	JsonDirectory  UMETA(DisplayName = "JSON Directory"),
	DataTable      UMETA(DisplayName = "Data Table"),
	DataAssetFolder UMETA(DisplayName = "Data Asset Folder")
};

UCLASS(config=Game, defaultconfig, meta=(DisplayName="Upgrade System Settings"))
class UUpgradeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Select catalog source
	UPROPERTY(EditAnywhere, config, Category="Catalog")
	EUpgradeCatalogSource CatalogSource = EUpgradeCatalogSource::JsonDirectory;

	// Directory path under Content/ for JSON files
	UPROPERTY(EditAnywhere, config, Category="Catalog", meta=(EditCondition="CatalogSource==EUpgradeCatalogSource::JsonDirectory"))
	FString JsonDirectory = TEXT("Data/UpgradeJSONs/");

	// DataTable assets for definitions (can list multiple)
	UPROPERTY(EditAnywhere, config, Category="Catalog", meta=(EditCondition="CatalogSource==EUpgradeCatalogSource::DataTable"))
	FString DataTableFolderPath = TEXT("/Game/Data/UpgradeDataTables");

	// Folder path for DataAssets
	UPROPERTY(EditAnywhere, config, Category="Catalog", meta=(EditCondition="CatalogSource==EUpgradeCatalogSource::DataAssetFolder"))
	FString DataAssetFolderPath = TEXT("/Game/Data/UpgradeAssets");
};