#include "UpgradeDataTableProvider.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"

UUpgradeDataTableProvider::UUpgradeDataTableProvider()
{
}

void UUpgradeDataTableProvider::InitializeData(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& OutCatalog, TArray<FName>& OutResourceTypes)
{
    
    FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetList;
    RegistryModule.Get().GetAssetsByPath(FName(*FilePath), AssetList, /*bRecursive=*/true);

    int32 LoadedTables = 0;
    for (const FAssetData& AssetData : AssetList)
    {
        
        if (AssetData.AssetClassPath != UDataTable::StaticClass()->GetClassPathName())
            continue;

        UDataTable* Table = Cast<UDataTable>(AssetData.GetAsset());
        if (!Table)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpgradeDataTableProvider: Failed to load DataTable '%s'"), *AssetData.ObjectPath.ToString());
            continue;
        }
        ++LoadedTables;

        FName PathId = FName(*Table->GetName());
        TArray<FUpgradeLevelData>& LevelArray = OutCatalog.FindOrAdd(PathId);

        for (auto& Pair : Table->GetRowMap())
        {
            FUpgradeLevelDataAsset* LevelAsset = reinterpret_cast<FUpgradeLevelDataAsset*>(Pair.Value);
            if (!LevelAsset)
            {
                UE_LOG(LogTemp, Warning, TEXT("Invalid row '%s' in DataTable %s"), *Pair.Key.ToString(), *Table->GetName());
                continue;
            }

            FUpgradeLevelData LevelData;
            for (const auto& ResourcePair : LevelAsset->UpgradeResourceCosts)
            {
                // Add the resource type and get its index
                int32 TypeIdx = AddOrFindRequiredResourceTypeIndex(ResourcePair.Key, OutResourceTypes);
                LevelData.ResourceTypeIndices.Add(TypeIdx);
                LevelData.UpgradeCosts.Add(ResourcePair.Value);
            }

            // Set the converted data
            LevelData.UpgradeSeconds = LevelAsset->UpgradeSeconds;
            LevelData.bUpgradeLocked = LevelAsset->bUpgradeLocked;
            
            LevelArray.Add(LevelData);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UpgradeDataTableProvider: Loaded %d DataTables from '%s' (found %d PathIds)"),
           LoadedTables, *FilePath, OutCatalog.Num());
}
