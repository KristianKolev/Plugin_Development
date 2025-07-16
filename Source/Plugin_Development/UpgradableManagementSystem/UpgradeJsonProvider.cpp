#include "UpgradeJsonProvider.h"
#include "AssetRegistry/AssetData.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UpgradeManagerSubsystem.h"
#include "UpgradeSettings.h"

UUpgradeJsonProvider::UUpgradeJsonProvider()
{
}

void UUpgradeJsonProvider::InitializeData(TMap<FName, TArray<FUpgradeDefinition>> &OutCatalog,
	TArray<FName> &OutResourceTypes)
{
	if (DetectedFiles.Num() == 0)
	{
		UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_00] No JSON files provided to provider"));
		return;
	}

	int32 LoadedFiles = 0;
	for (const FString &File : DetectedFiles)
	{
		FString JsonString;
		if (!FFileHelper::LoadFileToString(JsonString, *File))
		{
			UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_01] Failed to read JSON file: %s"), *File);
			continue;
		}

		TSharedPtr<FJsonObject> Root;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_02] Invalid JSON in file: %s"), *File);
			continue;
		}

		FName PathId = FName(*Root->GetStringField(TEXT("UpgradePathId")));
		if (PathId.IsNone())
		{
			PathId = FName(*FPaths::GetBaseFilename(File));
			UE_LOG(LogUpgradeSystem, Verbose, TEXT("[UPGRADEJSON_INFO_02] Using filename '%s' as UpgradePathId for file: %s"), *PathId.ToString(), *File);
		}

		int32 MaxLevel = 1;
		Root->TryGetNumberField(TEXT("MaxLevel"), MaxLevel);

		TArray<FUpgradeDefinition> *ExistingArray = OutCatalog.Find(PathId);
		if (ExistingArray)
		{
			UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADECATALOG_WARN_01] Duplicate UpgradePath '%s' found in JSON file '%s'. Overriding previous data."),
			       *PathId.ToString(), *FPaths::GetCleanFilename(File));
			ExistingArray->Reset();
		}

		TArray<FUpgradeDefinition> &LevelDataArray = OutCatalog.FindOrAdd(PathId);
		LevelDataArray.SetNum(MaxLevel + 1);

		int32 ProcessedLevels = 0;
		TMap<FName, int32> PreviousResourceCost;
		int32 PreviousTimeCost = 0;

		const TArray<TSharedPtr<FJsonValue>> *LevelOverrides;
		if (!Root->TryGetArrayField(TEXT("LevelOverrides"), LevelOverrides) || LevelOverrides->Num() == 0)
		{
			UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEJSON_ERR_03] JSON file %s missing 'LevelOverrides' array"), *File);
			continue;
		}

		const TSharedPtr<FJsonObject> *FirstOverrideObj;
		if (!(*LevelOverrides)[0]->TryGetObject(FirstOverrideObj) || (*FirstOverrideObj)->GetIntegerField(TEXT("UpgradeLevel")) != 0)
		{
			UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEJSON_ERR_04] First level override must be level 0 in file '%s'"), *File);
			continue;
		}

		for (const TSharedPtr<FJsonValue> &OverrideVal : *LevelOverrides)
		{
			const TSharedPtr<FJsonObject> *OverrideObj;
			if (!OverrideVal->TryGetObject(OverrideObj))
			{
				continue;
			}

			int32 UpgradeLevel = (*OverrideObj)->GetIntegerField(TEXT("UpgradeLevel"));
			if (UpgradeLevel > MaxLevel)
			{
				UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEJSON_ERR_05] Invalid level range for level override in file '%s'. Level override starts at level %d, but max level is %d"),
				       *File, UpgradeLevel, MaxLevel);
				continue;
			}

			FUpgradeDefinition LevelData;
			const TSharedPtr<FJsonObject> *ResCostsObj;
			if ((*OverrideObj)->TryGetObjectField(TEXT("UpgradeResourceCosts"), ResCostsObj))
			{
				for (const auto &Pair : (*ResCostsObj)->Values)
				{
					FName ResourceName(*Pair.Key);
					int32 OverrideValue = Pair.Value->AsNumber();
					int32 ResourceIndex = AddOrFindRequiredResourceTypeIndex(ResourceName, OutResourceTypes);
					LevelData.ResourceTypeIndices.Add(ResourceIndex);
					LevelData.UpgradeCosts.Add(OverrideValue);
					if (!PreviousResourceCost.Contains(ResourceName) && OverrideValue >= 0)
					{
						PreviousResourceCost.Add(ResourceName, OverrideValue);
					}
				}
			}

			LevelData.UpgradeSeconds = (*OverrideObj)->GetIntegerField(TEXT("UpgradeSeconds"));
			LevelData.bUpgradeLocked = (*OverrideObj)->GetBoolField(TEXT("bUpgradeLocked"));
			LevelDataArray[UpgradeLevel] = LevelData;
			ProcessedLevels++;

			if (PreviousTimeCost <= 0 && LevelData.UpgradeSeconds >= 0)
			{
				PreviousTimeCost = LevelData.UpgradeSeconds;
			}
		}

		const TSharedPtr<FJsonObject> *CostSegmentsObj;
		if (Root->TryGetObjectField(TEXT("CostScalingSegments"), CostSegmentsObj))
		{
			for (const auto &ResourcePair : (*CostSegmentsObj)->Values)
			{
				FName ResourceName(*ResourcePair.Key);
				if (!PreviousResourceCost.Contains(ResourceName))
				{
					UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEJSON_ERR_06] Invalid resource '%s' in file '%s'. Previous resource cost not found."), *ResourceName.ToString(), *File);
					continue;
				}

				const TSharedPtr<FJsonObject> *ResObj;
				if (!ResourcePair.Value->TryGetObject(ResObj))
				{
					continue;
				}

				const TArray<TSharedPtr<FJsonValue>> *SegmentsArray;
				if (!(*ResObj)->TryGetArrayField(TEXT("ScalingSegments"), SegmentsArray))
				{
					continue;
				}

				int32 PreviousSegmentEnd = 0;
				int32 ResourceIndex = AddOrFindRequiredResourceTypeIndex(ResourceName, OutResourceTypes);

				for (const TSharedPtr<FJsonValue> &SegVal : *SegmentsArray)
				{
					const TSharedPtr<FJsonObject> *SegObj;
					if (!SegVal->TryGetObject(SegObj))
					{
						continue;
					}

					FRequirementsScalingSegment Segment;
					ParseScalingSegment(*SegObj, Segment);

					if ((PreviousSegmentEnd + 1) != Segment.StartLevel)
					{
						UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEJSON_ERR_07] Invalid segment range for resource '%s' in file '%s'. Segment starts at level %d, but previous segment ended at level %d."),
						       *ResourceName.ToString(), *File, Segment.StartLevel, PreviousSegmentEnd);
						break;
					}
					if (Segment.EndLevel > MaxLevel)
					{
						UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEJSON_ERR_08] Invalid level range for resource '%s' in file '%s'. Segment ends at level %d, but max level is %d"),
						       *ResourceName.ToString(), *File, Segment.EndLevel, MaxLevel);
						break;
					}

					for (int32 i = Segment.StartLevel; i <= Segment.EndLevel; ++i)
					{
						int32 CostArrayIndex = LevelDataArray[i].ResourceTypeIndices.IndexOfByKey(ResourceIndex);
						if (CostArrayIndex == INDEX_NONE)
						{
							int32 ResourceCost = ComputeRequirementsBySegment(&Segment, PreviousResourceCost.FindChecked(ResourceName), i);
							LevelDataArray[i].ResourceTypeIndices.Add(ResourceIndex);
							LevelDataArray[i].UpgradeCosts.Add(ResourceCost);
							PreviousResourceCost.FindChecked(ResourceName) = ResourceCost;
						}
						else
						{
							int32 CurrentResourceCost = LevelDataArray[i].UpgradeCosts[CostArrayIndex];
							int32 ResourceCost = 0;
							if (CurrentResourceCost < 0)
							{
								ResourceCost = ComputeRequirementsBySegment(&Segment, PreviousResourceCost.FindChecked(ResourceName), i);
								PreviousResourceCost.FindChecked(ResourceName) = ResourceCost;
							}
							else if (CurrentResourceCost == 0)
							{
								ResourceCost = CurrentResourceCost;
								PreviousResourceCost.FindChecked(ResourceName) = ComputeRequirementsBySegment(&Segment, PreviousResourceCost.FindChecked(ResourceName), i);
							}
							else
							{
								ResourceCost = CurrentResourceCost;
								PreviousResourceCost.FindChecked(ResourceName) = CurrentResourceCost;
							}
							LevelDataArray[i].UpgradeCosts[CostArrayIndex] = ResourceCost;
						}
					}
					PreviousSegmentEnd = Segment.EndLevel;
				}
			}
		}

		const TArray<TSharedPtr<FJsonValue>> *TimeSegmentsArray;
		if (Root->TryGetArrayField(TEXT("TimeScalingSegments"), TimeSegmentsArray))
		{
			int32 PreviousSegmentEnd = 0;
			for (const TSharedPtr<FJsonValue> &SegVal : *TimeSegmentsArray)
			{
				const TSharedPtr<FJsonObject> *SegObj;
				if (!SegVal->TryGetObject(SegObj))
				{
					continue;
				}

				FRequirementsScalingSegment Segment;
				ParseScalingSegment(*SegObj, Segment);

				if (PreviousSegmentEnd != Segment.StartLevel)
				{
					UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEJSON_ERR_09] Invalid segment range for time cost in file '%s'. Segment starts at level %d, but previous segment ended at level %d."),
					       *File, Segment.StartLevel, PreviousSegmentEnd);
					break;
				}
				if (Segment.EndLevel > MaxLevel)
				{
					UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEJSON_ERR_10] Invalid level range for time costs in file '%s'. Segment ends at level %d, but max level is %d"),
					       *File, Segment.EndLevel, MaxLevel);
					break;
				}

				for (int32 i = Segment.StartLevel; i <= Segment.EndLevel; ++i)
				{
					int32 CurrentTimeCost = LevelDataArray[i].UpgradeSeconds;
					int32 TimeCost = 0;
					if (CurrentTimeCost < 0)
					{
						TimeCost = ComputeRequirementsBySegment(&Segment, PreviousTimeCost, i);
						PreviousTimeCost = TimeCost;
					}
					else if (CurrentTimeCost == 0)
					{
						TimeCost = CurrentTimeCost;
						PreviousTimeCost = ComputeRequirementsBySegment(&Segment, PreviousTimeCost, i);
					}
					else
					{
						TimeCost = CurrentTimeCost;
						PreviousTimeCost = CurrentTimeCost;
					}
					LevelDataArray[i].UpgradeSeconds = TimeCost;
				}
				PreviousSegmentEnd = Segment.EndLevel;
			}
		}

		if (ProcessedLevels > 0)
		{
			UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEJSON_INFO_03] Successfully processed file '%s' with %d levels"), *FPaths::GetCleanFilename(File), ProcessedLevels);
			LoadedFiles++;
		}
		else
		{
			UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_11] No valid levels processed in file '%s'"), *FPaths::GetCleanFilename(File));
		}
	}

	if (LoadedFiles == 0)
	{
		UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEJSON_ERR_12] No JSON files processed"));
		return;
	}

UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEJSON_INFO_04] Loaded %d JSON files (found %d PathIds)"), LoadedFiles, OutCatalog.Num());
}

bool UUpgradeJsonProvider::ParseScalingSegment(const TSharedPtr<FJsonObject>& JsonObject, FRequirementsScalingSegment& OutSegment) const
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	OutSegment.StartLevel = JsonObject->GetIntegerField(TEXT("StartLevel"));
	OutSegment.EndLevel = JsonObject->GetIntegerField(TEXT("EndLevel"));

	FString ModeStr = JsonObject->GetStringField(TEXT("ScalingMode"));
	UEnum* EnumPtr = StaticEnum<ECostScalingMode>();
	OutSegment.ScalingMode = EnumPtr ? static_cast<ECostScalingMode>(EnumPtr->GetValueByNameString(ModeStr)) : ECostScalingMode::HardCoded;

	switch (OutSegment.ScalingMode)
	{
	case ECostScalingMode::Constant:
		JsonObject->TryGetNumberField(TEXT("ConstantCost"), OutSegment.ConstantCost);
		break;
	case ECostScalingMode::Linear:
		JsonObject->TryGetNumberField(TEXT("LinearSlope"), OutSegment.LinearSlope);
		break;
	case ECostScalingMode::Exponential:
		JsonObject->TryGetNumberField(TEXT("ExpRate"), OutSegment.ExpRate);
		break;
	case ECostScalingMode::Polynomial:
		JsonObject->TryGetNumberField(TEXT("PolyCoeff"), OutSegment.PolyCoeff);
		JsonObject->TryGetNumberField(TEXT("PolyPower"), OutSegment.PolyPower);
		JsonObject->TryGetNumberField(TEXT("PolyOffset"), OutSegment.PolyOffset);
		break;
	case ECostScalingMode::Custom:
		{
			FString CustomName;
			if (JsonObject->TryGetStringField(TEXT("CustomFunctionName"), CustomName))
			{
				OutSegment.CustomFunctionName = FName(*CustomName);
			}
		}
		break;
	default:
		break;
	}

	return true;
}
