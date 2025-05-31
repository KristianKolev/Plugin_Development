#include "UpgradeJsonProvider.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UUpgradeJsonProvider::InitializeFromJson(const FString& FilePath, TMap<FName, TArray<FUpgradeLevelData>>& InUpgradeCatalog, TArray<FName>& InResourceTypes)
{
	
		FString JsonString;
		if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_01] Failed to read JSON file: %s"), *FilePath);
			return;
		}
		TSharedPtr<FJsonObject> Root;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_02] Invalid JSON in file: %s"), *FilePath);
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
			PathId = FName(*FPaths::GetBaseFilename(FilePath));
			UE_LOG(LogTemp, Verbose, TEXT("[UPGRADEJSON_INFO_02] Using filename '%s' as UpgradePathId for file: %s"), *PathId.ToString(), *FilePath);
		}
		TArray<FUpgradeLevelData>& Arr = InUpgradeCatalog.FindOrAdd(PathId);

		// Extract levels array
		const TArray<TSharedPtr<FJsonValue>>* Levels;
		if (!Root->TryGetArrayField(TEXT("levels"), Levels))
		{
			UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_03] JSON file %s missing 'levels' array."), *FilePath);
			return;
		}

		for (int32 lvlIndex = 0; lvlIndex < Levels->Num(); ++lvlIndex)
		{
			const TSharedPtr<FJsonValue>& Val = (*Levels)[lvlIndex];
			const TSharedPtr<FJsonObject>* Obj;
			if (!Val->TryGetObject(Obj))
			{
				UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_04] Level %d in %s is not an object."), lvlIndex, *FilePath);
				continue;
			}

			FUpgradeLevelData Data;

			// Parse resources array
			const TArray<TSharedPtr<FJsonValue>>* Resources;
			if (!(*Obj)->TryGetArrayField(TEXT("resources"), Resources))
			{
				UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_05] Level %d in %s missing 'resources' array."), lvlIndex, *FilePath);
				continue;
			}

			// Temporary storage
			TArray<int32> TypeIndices;
			TArray<int32> CostsArr;

			for (int32 resIndex = 0; resIndex < Resources->Num(); ++resIndex)
			{
				const TSharedPtr<FJsonObject>* RObj;
				if (!(*Resources)[resIndex]->TryGetObject(RObj))
				{
					UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_06] Resource entry %d in level %d of %s is not an object."), resIndex, lvlIndex, *FilePath);
					continue;
				}
				// Type string
				FString TypeStr = (*RObj)->GetStringField(TEXT("type"));
				FName TypeName(*TypeStr);
				int32 TypeIdx = AddRequiredResourceType(TypeName, InResourceTypes);
				TypeIndices.Add(TypeIdx);

				// Amount
				int32 Amount = (*RObj)->GetIntegerField(TEXT("amount"));
				CostsArr.Add(Amount);
			}

			if (TypeIndices.Num() == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_07] No valid resources for level %d in %s"), lvlIndex, *FilePath);
				continue;
			}
			Data.ResourceTypeIndices = MoveTemp(TypeIndices);
			Data.UpgradeCosts		= MoveTemp(CostsArr);

			// Parse upgradeSeconds (single value)
			int32 SecondsVal = 0;
			if (!(*Obj)->TryGetNumberField(TEXT("upgradeSeconds"), SecondsVal))
			{
				UE_LOG(LogTemp, Warning, TEXT("[UPGRADEJSON_ERR_08] Level %d in %s missing or invalid 'upgradeSeconds' field."), lvlIndex, *FilePath);
			}
			Data.UpgradeSeconds = SecondsVal;

			Arr.Add(Data);
			UE_LOG(LogTemp, Log, TEXT("[UPGRADEJSON_INFO_03] Successfully parsed level %d data"), lvlIndex);
			LevelDataArray = Arr;
		}
	UE_LOG(LogTemp, Log, TEXT("[UPGRADEJSON_INFO_04] Parsed %d upgrade levels."), LevelDataArray.Num());

}

const FUpgradeLevelData* UUpgradeJsonProvider::GetLevelData(int32 Level) const
{
	return LevelDataArray.IsValidIndex(Level) ? &LevelDataArray[Level] : nullptr;
}

int32 UUpgradeJsonProvider::GetMaxLevel() const
{
	return LevelDataArray.Num();
}

int32 UUpgradeJsonProvider::AddRequiredResourceType(FName& ResourceType, TArray<FName>& ResourceTypes)
{
    int32 FoundIndex = ResourceTypes.IndexOfByKey(ResourceType);
    if (FoundIndex != INDEX_NONE)
    {
        return FoundIndex;
    }
    int32 NewIndex = ResourceTypes.Num();
    ResourceTypes.Add(ResourceType);
    return NewIndex;
}
