#include "UpgradeDataAssetProvider.h"
#include "UpgradeDefinitionDataAsset.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "UpgradeManagerSubsystem.h"


UUpgradeDataAssetProvider::UUpgradeDataAssetProvider()
{
}


void UUpgradeDataAssetProvider::InitializeData(TMap<FName, TArray<FUpgradeDefinition>>& OutCatalog, TArray<FName>& OutResourceTypes)
{
    if (DetectedAssets.Num() == 0)
    {
        UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_00] No UpgradeDefinitionDataAssets provided"));
        return;
    }

    int32 LoadedAssets = 0;
    for (const FAssetData& AssetData : DetectedAssets)
    {
        if (AssetData.AssetClassPath != UUpgradeDefinitionDataAsset::StaticClass()->GetClassPathName())
            continue;

        UUpgradeDefinitionDataAsset* Asset = Cast<UUpgradeDefinitionDataAsset>(AssetData.GetAsset());
        if (!Asset)
        {
            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_01] Failed to load UpgradeDefinitionDataAsset '%s'"), 
                *AssetData.ObjectPath.ToString());
            continue;
        }
        ++LoadedAssets;
        
        // Fallback to asset name if no UpgradePathId is set
        FName PathId = !Asset->UpgradePathId.IsNone() ? Asset->UpgradePathId : AssetData.AssetName;
        int32 MaxLevel = (Asset->MaxLevel >= 0) ? Asset->MaxLevel : 1;

        TArray<FUpgradeDefinition>* ExistingArray = OutCatalog.Find(PathId);
        if (ExistingArray)
        {
            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADECATALOG_WARN_01] Duplicate UpgradePath '%s' found in asset '%s'. Overriding previous data."),
                   *PathId.ToString(), *AssetData.AssetName.ToString());
            ExistingArray->Reset();
        }
        
        // Ensure there is always a level-0 override
        if (!Asset->LevelOverrides.IsValidIndex(0))
        {
            UE_LOG(LogUpgradeSystem, Error,
                TEXT("[UPGRADEASSET_ERR_06] Missing level-0 override for asset '%s'. Skipping asset."),
                *AssetData.AssetName.ToString());
            continue;
        }
        if (Asset->LevelOverrides[0].UpgradeLevel != 0)
        {
            UE_LOG(LogUpgradeSystem, Error,
                TEXT("[UPGRADEASSET_ERR_07] First level override must be level 0 for asset '%s'. Skipping asset."),
                *AssetData.AssetName.ToString());
            continue;
        }

        TArray<FUpgradeDefinition>& LevelDataArray = OutCatalog.FindOrAdd(PathId);
        LevelDataArray.SetNum(MaxLevel + 1);

        int32 ProcessedLevels = 0;
        
        TMap<FName,int32> PreviousResourceCost;
        int32 PreviousTimeCost = 0;

        // Process values from level overrides into the catalog
        for (auto& LevelOverride : Asset->LevelOverrides)
        {
	if (LevelOverride.UpgradeLevel > MaxLevel)
	{
		UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEASSET_ERR_09] Invalid level range for level override in asset '%s'. Level override starts at level %d, but max level is %d"),
		*PathId.ToString(), LevelOverride.UpgradeLevel, MaxLevel);
		continue;
	}
            FUpgradeDefinition LevelData;
            for (auto& ResourcePair : LevelOverride.UpgradeResourceCosts)
            {
        
                FName ResourceName = ResourcePair.Key;
                if (ResourceName.IsNone()) continue;
                int32 OverrideValue = ResourcePair.Value;
    
                int32 ResourceIndex = AddOrFindRequiredResourceTypeIndex(ResourceName, OutResourceTypes);
                LevelData.ResourceTypeIndices.Add(ResourceIndex);
                LevelData.UpgradeCosts.Add(OverrideValue);
                // Initialize previous cost tracking with the first encountered value non-zero
                // Each resource inside the upgrade catalog should be first defined in a level override
                if (!PreviousResourceCost.Contains(ResourceName) && OverrideValue >= 0)
                {
                    PreviousResourceCost.Add(ResourceName, OverrideValue);
                }
            }
            LevelData.UpgradeSeconds = LevelOverride.UpgradeSeconds;
            LevelData.bUpgradeLocked = LevelOverride.bUpgradeLocked;
            LevelDataArray[LevelOverride.UpgradeLevel] = LevelData;
            
            // Initialize previous cost tracking with the first encountered value non-zero
            if (PreviousTimeCost <= 0 && LevelOverride.UpgradeSeconds >= 0)
            {
                PreviousTimeCost = LevelOverride.UpgradeSeconds;
            }
        }

        // Process values from resource cost scaling segments into the catalog.
        // Iterate over each resource
        for (auto& SegmentPair : Asset->CostScalingSegments)
        {
            FName ResourceName = SegmentPair.Key;
	if (!PreviousResourceCost.Contains(ResourceName))
	{
		UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEASSET_ERR_08] Invalid resource '%s' in asset '%s'. Previous resource cost not found. Each resource for an upgrade path should be first define in a level override."),
		*ResourceName.ToString(), *PathId.ToString());
		continue;
	}
            int32 PreviousSegmentEnd = 0;
            int32 ResourceIndex = AddOrFindRequiredResourceTypeIndex(ResourceName, OutResourceTypes);
            
            // Iterate over each segment within a resource
            for (auto& ResourceSegment : SegmentPair.Value.ScalingSegments)
            {
		if (PreviousSegmentEnd != ResourceSegment.StartLevel)
			{
				UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEASSET_ERR_04] Invalid segment range for resource '%s' in asset '%s'. Segment starts at level %d, but previous segment ended at level %d. There should be no gaps."),
				*ResourceName.ToString(), *PathId.ToString(), ResourceSegment.StartLevel, PreviousSegmentEnd);
				break;
		}
		if (ResourceSegment.EndLevel > MaxLevel)
				{
			UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEASSET_ERR_05] Invalid level range for resource '%s' in asset '%s'. Segment ends at level %d, but max level is %d"),
				*ResourceName.ToString(), *PathId.ToString(), ResourceSegment.EndLevel, MaxLevel);
			break;
		}

                PreviousSegmentEnd = ResourceSegment.EndLevel;
                // Iterate over the level ranges within a segment
                for (int i = ResourceSegment.StartLevel; i <= ResourceSegment.EndLevel; ++i)
                {
                    int32 CostArrayIndex = LevelDataArray[i].ResourceTypeIndices.IndexOfByKey(ResourceIndex);
                    // No cost has been added for this resource on this level. Add one now.
                    if (CostArrayIndex == INDEX_NONE)
                    {
                        int32 ResourceCost = ComputeRequirementsBySegment(&ResourceSegment, PreviousResourceCost.FindChecked(ResourceName), i);
                        LevelDataArray[i].ResourceTypeIndices.Add(ResourceIndex);
                        LevelDataArray[i].UpgradeCosts.Add(ResourceCost);
                        PreviousResourceCost.FindChecked(ResourceName) = ResourceCost;
                        
                    }
                    // The array element has been generated and has a cost associated with it - probably through the level overrides.
                    else
                    {
                        int32 CurrentResourceCost = LevelDataArray[i].UpgradeCosts[CostArrayIndex];
                        int32 ResourceCost = 0;
                        // Use scaling segment resource calculation
                        if (CurrentResourceCost < 0)
                        {
                            ResourceCost = ComputeRequirementsBySegment(&ResourceSegment, PreviousResourceCost.FindChecked(ResourceName), i);
                            PreviousResourceCost.FindChecked(ResourceName) = ResourceCost;
                        }
                        // What it would have costed for this level if we had not overriden the defined scaling method
                        else if (CurrentResourceCost == 0)
                        {
                            ResourceCost = CurrentResourceCost;
                            PreviousResourceCost.FindChecked(ResourceName) = ComputeRequirementsBySegment(&ResourceSegment, PreviousResourceCost.FindChecked(ResourceName), i);
                        }
                        // We "rebase" our cost scaling. Scaling method for the next level will use this override's value.
                        else
                        {
                            ResourceCost = CurrentResourceCost;
                            PreviousResourceCost.FindChecked(ResourceName) = CurrentResourceCost;
                        }

                        LevelDataArray[i].UpgradeCosts[CostArrayIndex] = ResourceCost;
                        ++ProcessedLevels;
                    }
                }
            }
        }
        

        // Process values from time scaling segments into the catalog.
        {
            int32 PreviousSegmentEnd = 0;
            for (auto& TimeSegments : Asset->TimeScalingSegments)
            {
		if (PreviousSegmentEnd != TimeSegments.StartLevel)
			{
				UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEASSET_ERR_11] Invalid segment range for time cost in asset '%s'. Segment starts at level %d, but previous segment ended at level %d. There should be no gaps."),
				*PathId.ToString(), TimeSegments.StartLevel, PreviousSegmentEnd);
				break;
		}
		if (TimeSegments.EndLevel > MaxLevel)
				{
			UE_LOG(LogUpgradeSystem, Error, TEXT("[UPGRADEASSET_ERR_10] Invalid level range for time costs in asset '%s'. Segment ends at level %d, but max level is %d"),
				*PathId.ToString(), TimeSegments.EndLevel, MaxLevel);
			break;
		}
                PreviousSegmentEnd = TimeSegments.EndLevel;
            
                for ( int i = TimeSegments.StartLevel; i <= TimeSegments.EndLevel; ++i )
                {                
                    int32 CurrentTimeCost = LevelDataArray[i].UpgradeSeconds;
                    int32 TimeCost = 0;
                    // Use scaling segment time calculation
                    if (CurrentTimeCost < 0)
                    {
                        TimeCost = ComputeRequirementsBySegment(&TimeSegments, PreviousTimeCost, i);
                        PreviousTimeCost = TimeCost;
                    }
                    // What it would have costed for this level if we had not overriden the defined scaling method
                    else if (CurrentTimeCost == 0)
                    {
                        TimeCost = CurrentTimeCost;
                        PreviousTimeCost = ComputeRequirementsBySegment(&TimeSegments, PreviousTimeCost, i);
                    }
                    else
                        // We "rebase" our cost scaling. Scaling method for the next level will use this override's value.
                    {
                        TimeCost = CurrentTimeCost;
                        PreviousTimeCost = CurrentTimeCost;
                    }
                    LevelDataArray[i].UpgradeSeconds = TimeCost;
                }
            }
        }
        /*
// Process values from level overrides into the catalog
for (auto& LevelOverride : Asset->LevelOverrides)
{
    FUpgradeDefinition LevelData;
    for (auto& ResourcePair : LevelOverride.UpgradeResourceCosts)
    {
        
        FName ResourceName = ResourcePair.Key;
        if (ResourceName.IsNone()) continue;
        
        int32 OverrideValue = ResourcePair.Value;
        if (OverrideValue < 0) continue;
    
        int32 ResourceIndex = AddOrFindRequiredResourceTypeIndex(ResourceName, OutResourceTypes);
        LevelData.ResourceTypeIndices.Add(ResourceIndex);
        LevelData.UpgradeCosts.Add(OverrideValue);
    }
    
    if (LevelOverride.UpgradeSeconds >= 0)
    {
        LevelData.UpgradeSeconds = LevelOverride.UpgradeSeconds;
    }
    
    LevelData.bUpgradeLocked = LevelOverride.bUpgradeLocked;
    LevelDataArray[LevelOverride.UpgradeLevel] = LevelData;
    bSlotWritten[LevelOverride.UpgradeLevel] = true;
}
*/
        /*
        for (int i = 0; i <= MaxLevel; ++i)
        {
            FUpgradeDefinition LevelData;

            // We get the cost overrides this way instead of baking them in separately in an outside loop.
            // An override could set a cost for this level to 0, but on the next level we want to resume the scaling for that resource,
            // as if it had its regular cost for this level.
            if (i == NextLevelToOverride && Asset->LevelOverrides.IsValidIndex(LevelOverridesIdx))
            {
                for (auto& Resource : Asset->LevelOverrides[LevelOverridesIdx].UpgradeResourceCosts)
                {
                    FName ResourceName = Resource.Key;
                    int32 OverrideValue = Resource.Value;
                    
                    if (OverrideValue < 0)
                    {
                        continue;
                    }
                    // What it would have costed for this level if had not overriden the defined scaling method
                    if (OverrideValue == 0)
                    {
                        const FRequirementsScalingSegment* Segment = FindSegment(Asset->CostScalingSegments.FindChecked(ResourceName).ScalingSegments, i);
                        PreviousResourceCost.FindOrAdd(ResourceName) = ComputeRequirementsBySegment(Segment, PreviousResourceCost.FindChecked(ResourceName), i);
                        
                    }
                    // We "rebase" our cost scaling. Scaling method for the next level will use this value.
                    else
                    {
                        PreviousResourceCost.FindOrAdd(ResourceName) = OverrideValue;
                    }
                    
                    int32 ResourceIndex = AddOrFindRequiredResourceTypeIndex(ResourceName, OutResourceTypes);
                    LevelData.ResourceTypeIndices.Add(ResourceIndex);
                    LevelData.UpgradeCosts.Add(OverrideValue);
                }
                LevelData.UpgradeSeconds = Asset->LevelOverrides[LevelOverridesIdx].UpgradeSeconds;
                PreviousTimeCost = Asset->LevelOverrides[LevelOverridesIdx].UpgradeSeconds;
                LevelOverridesIdx++;
                NextLevelToOverride = Asset->LevelOverrides[LevelOverridesIdx].UpgradeLevel;
            }
                
            for (const FUpgradeDefinitionAsset& LevelAsset : Asset->LevelOverrides)
            {
                
            
                if (LevelAsset.UpgradeResourceCosts.Num() == 0)
                {
                    UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_02] No resource costs defined for level %d in asset '%s'"), 
                           ProcessedLevels, *AssetData.AssetName.ToString());
                    continue;
                }

                for (const auto& Resource : LevelAsset.UpgradeResourceCosts)
                {
                    int32 ResourceIndex = AddOrFindRequiredResourceTypeIndex(Resource.Key, OutResourceTypes);
                    LevelData.ResourceTypeIndices.Add(ResourceIndex);
                    LevelData.UpgradeCosts.Add(Resource.Value);
                }
            
                LevelData.UpgradeSeconds = LevelAsset.UpgradeSeconds;
                LevelData.bUpgradeLocked = LevelAsset.bUpgradeLocked;
                LevelDataArray.Add(LevelData);
                ++ProcessedLevels;
            }
        }
        */
        if (ProcessedLevels > 0)
        {
            UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEASSET_INFO_01] Successfully processed asset '%s' with %d levels"), 
                *AssetData.AssetName.ToString(), ProcessedLevels);
        }
        else
        {
            UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_03] No valid levels processed in asset '%s'"), 
                *AssetData.AssetName.ToString());
        }
    }

	// Check if any Data Assets were actually loaded
	if (LoadedAssets == 0)
	{
		UE_LOG(LogUpgradeSystem, Warning, TEXT("[UPGRADEASSET_ERR_12] No UpgradeDefinitionDataAssets processed"));
		return;
	}

    UE_LOG(LogUpgradeSystem, Log, TEXT("[UPGRADEASSET_INFO_02] Loaded %d Data Assets (found %d PathIds)"),
        LoadedAssets, OutCatalog.Num());
}