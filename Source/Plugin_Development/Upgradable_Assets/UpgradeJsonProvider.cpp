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
						EResourceType ResourceType = EResourceType::None;
						int32 Amount;
						FString TypeString;
												
						if ((*ResourceObj)->TryGetStringField(TEXT("type"), TypeString))
						{
							UEnum* EnumPtr = StaticEnum<EResourceType>();
							if (EnumPtr)
							{
								int64 EnumValue = EnumPtr->GetValueByName(FName(*TypeString));
								if (EnumValue != INDEX_NONE)
								{
									ResourceType = static_cast<EResourceType>(EnumValue);
									UE_LOG(LogTemp, Log, TEXT("Successfully converted '%s' to enum value: %s"), 
										*TypeString, 
										*EnumPtr->GetNameStringByValue(static_cast<int64>(ResourceType)));
										Data.ResourceTypes.Add(ResourceType);
								}
								else
								{
									UE_LOG(LogTemp, Warning, TEXT("Failed to convert '%s' to EResourceType enum. Value not found in enum."), 
										*TypeString);
								}
							}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("Failed to get EResourceType enum definition"));
							}
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("Failed to get 'type' field from JSON"));
						}
						if((*ResourceObj)->TryGetNumberField(TEXT("amount"), Amount))
						{
							Data.UpgradeCosts.Add(Amount);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("No 'amount' value found in level data"));
						}
					}
				}
			}
			
			// Add parsing for single upgradeSeconds value
			int32 UpgradeSeconds;
			if ((*LevelObj)->TryGetNumberField(TEXT("upgradeSeconds"), UpgradeSeconds))
			{
				Data.UpgradeSeconds = UpgradeSeconds;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No 'upgradeSeconds' value found in level data"));
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