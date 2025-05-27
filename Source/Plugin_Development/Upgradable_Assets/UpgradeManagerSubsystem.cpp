// Fill out your copyright notice in the Description page of Project Settings.
#include "UpgradeManagerSubsystem.h"
#include "UpgradableComponent.h"
#include "UpgradeJsonProvider.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"


UUpgradeManagerSubsystem::UUpgradeManagerSubsystem()
{
}

void UUpgradeManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUpgradeManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}



void UUpgradeManagerSubsystem::InitializeProviderFromJson(const FString& Json)
{
	Provider = NewObject<UUpgradeJsonProvider>(this);
	Provider->InitializeFromJson(Json);
}

bool UUpgradeManagerSubsystem::CanUpgrade(const UUpgradableComponent* Component) const
{
	return Provider && Component->GetUpgradeLevel_Implementation() < Provider->GetMaxLevels() - 1;
}

void UUpgradeManagerSubsystem::HandleUpgradeRequest(UUpgradableComponent* Component) const
{
	if (!Component || !Component->GetOwner()->HasAuthority()) return;

	if (!CanUpgrade(Component)) return;

	const int32 NextLevel = Component->GetUpgradeLevel_Implementation() + 1;
	const FUpgradeLevelData* LevelData = Provider->GetLevelData(NextLevel);

	if (!LevelData) return;

	APlayerState* PS = Cast<APawn>(Component->GetOwner())->GetPlayerState<APlayerState>();
	if (!PS || PS->GetScore() < LevelData->UpgradeCost) return;

	
	PS->SetScore(PS->GetScore() - LevelData->UpgradeCost);
	Component->ApplyUpgradeInternal();
}
