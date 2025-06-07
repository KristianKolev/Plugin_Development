#include "UpgradableComponent.h"
#include "UpgradeManagerSubsystem.h"
#include "GameFramework/Actor.h"

UUpgradableComponent::UUpgradableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UUpgradableComponent::BeginPlay()
{
	Super::BeginPlay();

//	if (GetOwnerRole() == ROLE_Authority)
//	{
		// Register this component with the subsystem
		UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>();
		if (Subsystem)
		{
			UpgradableID = Subsystem->RegisterUpgradableComponent(this);
		}
//	}
}

void UUpgradableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (/*GetOwnerRole() == ROLE_Authority && */UpgradableID != -1)
	{
		UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>();
		if (Subsystem)
		{
			Subsystem->UnregisterUpgradableComponent(UpgradableID);
		}
	}
	Super::EndPlay(EndPlayReason);
}

// void UUpgradableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
// 	DOREPLIFETIME(UUpgradableComponent, LocalLevel);
// }

void UUpgradableComponent::Client_SetLevel_Implementation(int32 NewLevel)
{
	const int32 OldLevel = LocalLevel;
	LocalLevel = NewLevel;
	OnLevelChanged.Broadcast(OldLevel, LocalLevel);
}

void UUpgradableComponent::RequestUpgrade(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts)
{
	Server_RequestUpgrade(LevelIncrease, AvailableResourcesNames, AvailableResourceAmounts);
}

void UUpgradableComponent::ChangeActorVisualsPerUpgradeLevel(int32 Level, UStaticMeshComponent* StaticMeshComponent,
                                                             USkeletalMeshComponent* SkeletalComponent)
{
	if (StaticMeshComponent && LevelUpVisuals->StaticMeshPerLevel.Contains(Level))
	{
		StaticMeshComponent->SetStaticMesh(LevelUpVisuals->StaticMeshPerLevel[Level]);
	
	}

	if (SkeletalComponent && LevelUpVisuals->SkeletalMeshPerLevel.Contains(Level))
	{
		SkeletalComponent->SetSkeletalMesh(LevelUpVisuals->SkeletalMeshPerLevel[Level]);
	}
	
	if (LevelUpVisuals->MaterialSwapsPerLevel.Contains(Level))
	{
		const FMaterialSwapList* SwapList = LevelUpVisuals->MaterialSwapsPerLevel.Find(Level);
		for (const FMaterialSwapInfo& SwapInfo : SwapList->MaterialSwaps)
		{
			if (SwapInfo.Material == nullptr || SwapInfo.MaterialSlot < 0)
				continue;

			switch (SwapInfo.MeshForMaterialSwap)
			{
			case EMeshForMaterialSwap::StaticMesh: 
				if (StaticMeshComponent)
				{
					StaticMeshComponent->SetMaterial(
						SwapInfo.MaterialSlot, SwapInfo.Material);
				}
				break;

			case EMeshForMaterialSwap::SkeletalMesh:       
				if (SkeletalComponent)
				{
					SkeletalComponent->SetMaterial(
						SwapInfo.MaterialSlot, SwapInfo.Material);
				}
				break;

			default:
				break;
			}
		}
	}
}

bool UUpgradableComponent::Server_RequestUpgrade_Validate(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts) { return true; }

void UUpgradableComponent::Server_RequestUpgrade_Implementation(int32 LevelIncrease, const TArray<FName>& AvailableResourcesNames, const TArray<int32>& AvailableResourceAmounts)
{
	if (UUpgradeManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>())
	{
		TMap<FName, int32> AvailableResources;
		for ( int32 i = 0; i < AvailableResourcesNames.Num(); ++i)
		{			
			AvailableResources.Add(AvailableResourcesNames[i], AvailableResourceAmounts[i]);
		}
		Subsystem->HandleUpgradeRequest(UpgradableID, LevelIncrease, AvailableResources);
	}
}
