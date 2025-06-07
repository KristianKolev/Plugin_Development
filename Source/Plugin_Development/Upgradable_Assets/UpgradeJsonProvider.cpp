#include "UpgradeJsonProvider.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UUpgradeJsonProvider::UUpgradeJsonProvider()
{
}

void UUpgradeJsonProvider::InitializeData(const FString& FolderPath, TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog,
                                          TArray<FName>& OutResourceTypes)
{
	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *FolderPath, TEXT("*.json"), true, false);

	for (const FString& File : Files)
	{
		FString JsonString;
		if (!FFileHelper::LoadFileToString(JsonString, *File))
		{
			UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_01] Failed to read JSON file: %s"), *FolderPath);
			return;
		}
		TSharedPtr<FJsonObject> Root;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_02] Invalid JSON in file: %s"), *FolderPath);
			return;
		}
		// Determine UpgradePathId from JSON field or fallback to filename
		FString PathIdStr;
		FName PathId;
		if (Root->TryGetStringField(TEXT("UpgradePath"), PathIdStr) && !PathIdStr.IsEmpty())
		{
			PathId = FName(*PathIdStr);
			UE_LOG(LogTemp, Log, TEXT("[UPGRADEJSON_INFO_01] Using UpgradePath '%s' from JSON"), *PathIdStr);
		}
		else
		{
			PathId = FName(*FPaths::GetBaseFilename(FolderPath));
			UE_LOG(LogTemp, Verbose, TEXT("[UPGRADEJSON_INFO_02] Using filename '%s' as UpgradePathId for file: %s"), *PathId.ToString(), *FolderPath);
		}
		TArray<FUpgradeDefinition>& LevelDataArray = OutCatalog.FindOrAdd(PathId);

		// Extract levels array
		const TArray<TSharedPtr<FJsonValue>>* Levels;
		if (!Root->TryGetArrayField(TEXT("Levels"), Levels))
		{
			UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_03] JSON file %s missing 'Levels' array."), *FolderPath);
			return;
		}

		for (int32 LvlIndex = 0; LvlIndex < Levels->Num(); ++LvlIndex)
		{
			const TSharedPtr<FJsonValue>& LvlVal = (*Levels)[LvlIndex];
			const TSharedPtr<FJsonObject>* LvlObj;
			if (!LvlVal->TryGetObject(LvlObj))
			{
				UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_04] Level %d in %s is not an object."), LvlIndex, *FolderPath);
				continue;
			}

			FUpgradeDefinition LevelData;

			// Parse resources array
			const TArray<TSharedPtr<FJsonValue>>* Resources;
			if (!(*LvlObj)->TryGetArrayField(TEXT("Resources"), Resources))
			{
				UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_05] Level %d in %s missing 'Resources' array."), LvlIndex, *FolderPath);
				continue;
			}

			for (int32 ResIndex = 0; ResIndex < Resources->Num(); ++ResIndex)
			{
				const TSharedPtr<FJsonObject>* ResObj;
				if (!(*Resources)[ResIndex]->TryGetObject(ResObj))
				{
					UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_06] Resource entry %d in level %d of %s is not an object."), ResIndex, LvlIndex, *FolderPath);
					continue;
				}
				// Type string
				FString TypeStr = (*ResObj)->GetStringField(TEXT("Type"));
				FName TypeName(*TypeStr);
				int32 TypeIdx = AddOrFindRequiredResourceTypeIndex(TypeName, OutResourceTypes);
				LevelData.ResourceTypeIndices.Add(TypeIdx);

				int32 Amount = (*ResObj)->GetIntegerField(TEXT("Amount"));
				LevelData.UpgradeCosts.Add(Amount);
			}

			if (LevelData.ResourceTypeIndices.Num() == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_07] No valid resources for level %d in %s"), LvlIndex, *FolderPath);
				continue;
			}
			
			int32 UpgradeSeconds = 0;
			if (!(*LvlObj)->TryGetNumberField(TEXT("UpgradeSeconds"), UpgradeSeconds))
			{
				UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_08] Level %d in %s missing or invalid 'UpgradeSeconds' field."), LvlIndex, *FolderPath);
			}
			LevelData.UpgradeSeconds = UpgradeSeconds;

			bool bLocked = false;
			if (!(*LvlObj)->TryGetBoolField(TEXT("UpgradeLocked"), bLocked))
			{
				UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_09] Level %d in %s missing or invalid 'UpgradeLocked' field."), LvlIndex, *FolderPath);
			}
			LevelData.bUpgradeLocked = bLocked;
			
			LevelDataArray.Add(LevelData);
			UE_LOG(LogTemp, Log, TEXT("[UPGRADEJSON_INFO_03] Successfully parsed level %d data"), LvlIndex);
		}
		UE_LOG(LogTemp, Log, TEXT("[UPGRADEJSON_INFO_04] Parsed %d upgrade levels."), LevelDataArray.Num());
	}
	

}
