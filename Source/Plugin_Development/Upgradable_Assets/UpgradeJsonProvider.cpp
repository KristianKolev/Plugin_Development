#include "UpgradeJsonProvider.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UUpgradeJsonProvider::InitializeFromJson(const FString& Json)
{
	
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	TSharedPtr<FJsonObject> RootObject;

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse upgrade JSON."));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* Levels;
	if (!RootObject->TryGetArrayField(TEXT("levels"), Levels))
	{
		UE_LOG(LogTemp, Error, TEXT("JSON does not contain a 'levels' array."));
		return;
	}

	LevelDataArray.Empty();

	for (const TSharedPtr<FJsonValue>& Entry : *Levels)
	{
		const TSharedPtr<FJsonObject>* LevelObj;
		if (Entry->TryGetObject(LevelObj))
		{
			FUpgradeLevelData Data;
			
			const TArray<TSharedPtr<FJsonValue>>* ResourceArray;
			if ((*LevelObj)->TryGetArrayField(TEXT("resources"), ResourceArray))
			{
				for (const TSharedPtr<FJsonValue>& ResourceVal : *ResourceArray)
				{
					const TSharedPtr<FJsonObject>* ResourceObj;
					if (ResourceVal->TryGetObject(ResourceObj))
					{
						FResourceType ResourceType;
						int32 Amount;
						(*ResourceObj)->TryGetStringField(TEXT("type"), ResourceType.Name);
						(*ResourceObj)->TryGetNumberField(TEXT("amount"), Amount);
						Data.ResourceTypes.Add(ResourceType);
						Data.UpgradeCosts.Add(Amount);
					}
				}
			}
			LevelDataArray.Add(Data);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Parsed %d upgrade levels."), LevelDataArray.Num());

}

const FUpgradeLevelData* UUpgradeJsonProvider::GetLevelData(int32 Level) const
{
	return LevelDataArray.IsValidIndex(Level) ? &LevelDataArray[Level] : nullptr;
}

int32 UUpgradeJsonProvider::GetMaxLevel() const
{
	return LevelDataArray.Num();
}