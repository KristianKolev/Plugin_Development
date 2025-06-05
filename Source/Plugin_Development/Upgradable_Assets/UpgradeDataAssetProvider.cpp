#include "UpgradeDataAssetProvider.h"

#include "UpgradeDataTableProvider.h"
#include "AssetRegistry/AssetRegistryModule.h"


UUpgradeDataAssetProvider::UUpgradeDataAssetProvider()
{
}

void UUpgradeDataAssetProvider::InitializeData(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& OutCatalog, TArray<FName>& OutResourceTypes)
{
	    // 2) Query AssetRegistry for all DataTable assets under that path
    FAssetRegistryModule& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetList;
    AR.Get().GetAssetsByPath(FName(*FilePath), AssetList, /*bRecursive=*/true);

    int32 LoadedTables = 0;
    for (const FAssetData& AD : AssetList)
    {
        // Only care about DataTable assets
        if (AD.AssetClass != UDataTable::StaticClass()->GetFName())
            continue;

        // 3) Load the UDataTable
        UDataTable* Table = Cast<UDataTable>(AD.GetAsset());
        if (!Table)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpgradeDataTableProvider: Failed to load DataTable '%s'"), *AD.ObjectPath.ToString());
            continue;
        }
        ++LoadedTables;

        // 4) Iterate every row in this DataTable
        for (auto& Pair : Table->GetRowMap())
        {
            FName RowName = Pair.Key;  // e.g. “Orc.Unit” or whatever
            // We assume each row’s struct is FUpgradeDefinition, which might look like:
            // struct FUpgradeDefinition { FName UpgradePath; TArray<FUpgradeLevelData> Levels; /* etc. */ };
            // So cast Value to FUpgradeDefinition*
            FUpgradeLevelData* Def = reinterpret_cast<FUpgradeLevelData*>(Pair.Value);
            if (!Def)
            {
                UE_LOG(LogTemp, Warning, TEXT("UpgradeDataTableProvider: Invalid row '%s' in DataTable '%s'"),
                       *RowName.ToString(), *Table->GetName());
                continue;
            }

            // 5) Determine which PathId to use:
            //    If the struct has an explicit UpgradePath (non‐None), use that; otherwise fallback to RowName.
			FName PathId = FName(*Table->GetName());

            // 6) Insert or append the Levels array (already containing FUpgradeLevelData)
            TArray<FUpgradeLevelData>& LevelArray = OutCatalog.FindOrAdd(PathId);
            for (const FUpgradeLevelData& LevelData : Def->Levels)
            {
                // If you need to “intern” resource‐type strings here, do it now:
                //    If LevelData.ResourceTypeIndices actually stored raw FNames or FString,
                //    you would push that TypeName into OutResourceTypes (if missing), then store its index.
                //    But assuming DataTable rows already used indices that match the same ResourceTypes table
                //    you can simply copy the data. If not, you must do something like:
                //
                //    for (int32 i = 0; i < LevelData.RawResourceTypeNames.Num(); ++i)
                //    {
                //       FName RName = LevelData.RawResourceTypeNames[i];
                //       int32 Found = OutResourceTypes.IndexOfByKey(RName);
                //       if (Found == INDEX_NONE) Found = OutResourceTypes.Add(RName);
                //       // Build a new FUpgradeLevelData with ResourceTypeIndices.Add(Found) and so on...
                //    }
                //
                // In this example, we assume Levels already contain valid ResourceTypeIndices compatible
                // with OutResourceTypes (e.g. somebody pre‐filled them in Blueprints or via a shared CSV).
                LevelArray.Add(LevelData);

                // If you do need to merge in brand‐new resource names from the DataTable row,
                // you can do so by iterating over LevelData.ResourceTypeIndices → grabbing a name, checking
                // if it’s in OutResourceTypes, etc.  For brevity, this demo assumes the DataTable’s
                // ResourceTypeIndices are already correct.
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UpgradeDataTableProvider: Loaded %d DataTables from '%s' (found %d PathIds)"),
           LoadedTables, *DTFolder, OutCatalog.Num());
	
	/*const UUpgradeSettings* Settings = GetDefault<UUpgradeSettings>();
	UpgradeCatalog.Empty();

	FAssetRegistryModule& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> Assets;
	AR.Get().GetAssetsByPath(FName(*Settings->DataAssetFolderPath), Assets, true);

	for (auto& AD : Assets)
	{
		UUpgradeDefinitionAsset* Asset = Cast<UUpgradeDefinitionAsset>(AD.GetAsset());
		if (!Asset)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load UpgradeDefinitionAsset at %s"), *AD.ObjectPath.ToString());
			continue;
		}
		// Determine path ID: use Asset->OverrideUpgradePath if set, else asset name
		FName PathId = !Asset->OverrideUpgradePath.IsNone() ? Asset->OverrideUpgradePath : AD.AssetName;
		UpgradeCatalog.Add(PathId, Asset->Definition.Levels);
	}
	UE_LOG(LogTemp, Log, TEXT("Loaded %d DataAsset catalogs"), UpgradeCatalog.Num());*/
}


int32 UUpgradeDataAssetProvider::AddRequiredResourceType(const FName& ResourceType, TArray<FName>& ResourceTypes)
{
	return -1;
}

