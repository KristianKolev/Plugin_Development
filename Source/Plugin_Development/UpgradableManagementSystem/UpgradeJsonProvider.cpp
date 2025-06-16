#include "UpgradeJsonProvider.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UpgradeSettings.h"
#include "AssetRegistry/AssetData.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UpgradeManagerSubsystem.h"

UUpgradeJsonProvider::UUpgradeJsonProvider()
{
}

void UUpgradeJsonProvider::InitializeData(TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog,
                                          TArray<FName>& OutResourceTypes)
{
   const FUpgradeJsonFieldNames& Fields = GetDefault<UUpgradeSettings>()->JsonFieldNames;

    if (DetectedFiles.Num() == 0)
    {
        UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_00] No JSON files provided to provider"));
        return;
    }

    for (const FString& File : DetectedFiles)
    {
        FString JsonString;
        if (!FFileHelper::LoadFileToString(JsonString, *File))
        {
            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_01] Failed to read JSON file: %s"), *File);
            return;
        }
        TSharedPtr<FJsonObject> Root;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
        {
            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_02] Invalid JSON in file: %s"), *File);
            return;
        }
        // Determine UpgradePathId from JSON field or fallback to filename
        FString PathIdStr;
        FName PathId;
        if (Root->TryGetStringField(*Fields.UpgradePathField, PathIdStr) && !PathIdStr.IsEmpty())
		{
			PathId = FName(*PathIdStr);
			UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEJSON_INFO_01] Using UpgradePath '%s' from JSON"), *PathIdStr);
		}
    	
	    else
	    {
	        PathId = FName(*FPaths::GetBaseFilename(File));
	        UE_LOG(LogUpgradeSystem, Verbose, TEXT("[UPGRADEJSON_INFO_02] Using filename '%s' as UpgradePathId for file: %s"), *PathId.ToString(), *File);
	    }
    	
	    TArray<FUpgradeDefinition>* ExistingArray = OutCatalog.Find(PathId);
    		
	    if (ExistingArray)
	    {
			UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADECATALOG_WARN_01] Duplicate UpgradePath '%s' found in JSON file '%s'. Overriding previous data."),
			       *PathId.ToString(), *FPaths::GetCleanFilename(File));
			ExistingArray->Reset();
	    }

	    TArray<FUpgradeDefinition>& LevelDataArray = OutCatalog.FindOrAdd(PathId);

		// Extract levels array
		const TArray<TSharedPtr<FJsonValue>>* Levels;
		if (!Root->TryGetArrayField(*Fields.LevelsField, Levels))
		{
		   UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_03] JSON file %s missing '%s' array."), *File, *Fields.LevelsField);
		   return;
		}

		for (int32 LvlIndex = 0; LvlIndex < Levels->Num(); ++LvlIndex)
		{
			const TSharedPtr<FJsonValue>& LvlVal = (*Levels)[LvlIndex];
			const TSharedPtr<FJsonObject>* LvlObj;
			if (!LvlVal->TryGetObject(LvlObj))
			{
	            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_04] Level %d in %s is not an object."), LvlIndex, *File);
				continue;
			}

			FUpgradeDefinition LevelData;

			// Parse resources array
			const TArray<TSharedPtr<FJsonValue>>* Resources;
			if (!(*LvlObj)->TryGetArrayField(*Fields.ResourcesField, Resources))
			{
				UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_05] Level %d in %s missing '%s' array."), LvlIndex, *File, *Fields.ResourcesField);
				continue;
			}

			for (int32 ResIndex = 0; ResIndex < Resources->Num(); ++ResIndex)
			{
				const TSharedPtr<FJsonObject>* ResObj;
				if (!(*Resources)[ResIndex]->TryGetObject(ResObj))
				{
	                UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_06] Resource entry %d in level %d of %s is not an object."), ResIndex, LvlIndex, *File);
					continue;
				}
				
				// Type string
	            FString TypeStr = (*ResObj)->GetStringField(*Fields.ResourceTypeField);
				FName TypeName(*TypeStr);
				int32 TypeIdx = AddOrFindRequiredResourceTypeIndex(TypeName, OutResourceTypes);
				LevelData.ResourceTypeIndices.Add(TypeIdx);
		        int32 Amount = (*ResObj)->GetIntegerField(*Fields.ResourceAmountField);
				LevelData.UpgradeCosts.Add(Amount);
			}

			if (LevelData.ResourceTypeIndices.Num() == 0)
			{
	            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_07] No valid resources for level %d in %s"), LvlIndex, *File);
				continue;
			}
			int32 UpgradeSeconds = 0;
			if (!(*LvlObj)->TryGetNumberField(*Fields.UpgradeSecondsField, UpgradeSeconds))
			{
				UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_08] Level %d in %s missing or invalid '%s' field."), LvlIndex, *File, *Fields.UpgradeSecondsField);
	        }
			LevelData.UpgradeSeconds = UpgradeSeconds;

			bool bLocked = false;
			if (!(*LvlObj)->TryGetBoolField(*Fields.UpgradeLockedField, bLocked))
			{
				UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_09] Level %d in %s missing or invalid '%s' field."), LvlIndex, *File, *Fields.UpgradeLockedField);
			}
			LevelData.bUpgradeLocked = bLocked;
			
			LevelDataArray.Add(LevelData);
			UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEJSON_INFO_03] Successfully parsed level %d data"), LvlIndex);
		}
    	UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEJSON_INFO_04] Parsed %d upgrade levels."), LevelDataArray.Num());
	}
}
