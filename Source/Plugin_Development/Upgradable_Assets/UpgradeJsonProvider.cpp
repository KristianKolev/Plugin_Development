#include "UpgradeJsonProvider.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UUpgradeJsonProvider::InitializeFromJson(const FString& Json)
{
	// Parse Json -> LevelDataArray (simplified for brevity)
}

const FUpgradeLevelData* UUpgradeJsonProvider::GetLevelData(int32 Level) const
{
	return LevelDataArray.IsValidIndex(Level) ? &LevelDataArray[Level] : nullptr;
}

int32 UUpgradeJsonProvider::GetMaxLevels() const
{
	return LevelDataArray.Num();
}