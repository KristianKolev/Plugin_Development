



#pragma once

#include "CoreMinimal.h"
#include "UpgradeDataProvider.h"
#include "UpgradeJsonProvider.generated.h"

UCLASS()
class PLUGIN_DEVELOPMENT_API UUpgradeJsonProvider : public UUpgradeDataProvider
{
	GENERATED_BODY()
public:
	UUpgradeJsonProvider();

	/**
	 * Initializes an upgrade catalog and resource type list by parsing data from a JSON file.
	 *
	 * The method reads a JSON file, extracts upgrade levels and associated data, and populates the input
	 * upgrade catalog with the parsed information. If a valid upgrade path identifier is not provided
	 * in the JSON, the base filename of the file is used instead. The method also validates the JSON
	 * structure and handles missing or malformed fields by logging warnings and halting further processing.
	 *
	 * @param FilePath The path to the JSON file containing upgrade data.
	 * @param InUpgradeCatalog A map where each key represents an upgrade path ID, and each value contains
	 *        an array of upgrade level data corresponding to that path.
	 * @param InResourceTypes A list of all resource types identified during parsing.
	 */
	void InitializeFromJson(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& InUpgradeCatalog, TArray<FName>& InResourceTypes);

	virtual void InitializeData(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& OutCatalog, TArray<FName>& OutResourceTypes) override;
};