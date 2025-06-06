#include "UpgradeDataTableProvider.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"


UUpgradeDataTableProvider::UUpgradeDataTableProvider()
{
}

void UUpgradeDataTableProvider::InitializeData(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& OutCatalog, TArray<FName>& OutResourceTypes)
{
    FAssetRegistryModule& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetList;
    AR.Get().GetAssetsByPath(FName(*FilePath), AssetList, /*bRecursive=*/true);

    int32 LoadedTables = 0;
    for (const FAssetData& AD : AssetList)
    {
        if (AD.AssetClassPath != UDataTable::StaticClass()->GetClassPathName())
            continue;

        UDataTable* Table = Cast<UDataTable>(AD.GetAsset());
        if (!Table)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpgradeDataTableProvider: Failed to load DataTable '%s'"), *AD.ObjectPath.ToString());
            continue;
        }
        ++LoadedTables;

        FName PathId = FName(*Table->GetName());
        TArray<FUpgradeLevelData>& LevelArray = OutCatalog.FindOrAdd(PathId);

        for (auto& Pair : Table->GetRowMap())
        {
            FUpgradeLevelDataAsset* AssetData = reinterpret_cast<FUpgradeLevelDataAsset*>(Pair.Value);
            if (!AssetData)
            {
                UE_LOG(LogTemp, Warning, TEXT("Invalid row '%s' in DataTable %s"), *Pair.Key.ToString(), *Table->GetName());
                continue;
            }

            FUpgradeLevelData LevelData;
            
            // Convert the TMap of resource costs to parallel arrays
            TArray<int32> TypeIndices;
            TArray<int32> CostsArr;
            
            for (const auto& ResourcePair : AssetData->UpgradeResourceCosts)
            {
                // Add the resource type and get its index
                int32 TypeIdx = AddOrFindRequiredResourceTypeIndex(ResourcePair.Key, OutResourceTypes);
                
                LevelData.ResourceTypeIndices.Add(TypeIdx);
                LevelData.UpgradeCosts.Add(ResourcePair.Value);
            }

            // Set the converted data
            LevelData.UpgradeSeconds = AssetData->UpgradeSeconds;
            LevelData.bUpgradeLocked = AssetData->bUpgradeLocked;
            
            LevelArray.Add(LevelData);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UpgradeDataTableProvider: Loaded %d DataTables from '%s' (found %d PathIds)"),
           LoadedTables, *FilePath, OutCatalog.Num());
}
