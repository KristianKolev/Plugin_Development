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
	LoadJsonFromFile();
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
	return Provider && Component->GetCurrentUpgradeLevel_Implementation() < Provider->GetMaxLevel() - 1;
}

TArray<int32> UUpgradeManagerSubsystem::GetNextLevelUpgradeCosts(const UUpgradableComponent* Component) const
{
	TArray<int32> Costs;
	int32 NextLevel = Component->GetCurrentUpgradeLevel_Implementation() + 1;
	if (Provider && Provider->GetLevelData(NextLevel))
	{
		Costs = Provider->GetLevelData(NextLevel)->UpgradeCosts;
	}
	return Costs;
}

int32 UUpgradeManagerSubsystem::GetMaxLevel() const
{
	return Provider->GetMaxLevel();
}

int32 UUpgradeManagerSubsystem::RegisterUpgradableComponent(UUpgradableComponent* Component)
{
	int32 Id;
	if (FreeIndices.Num() > 0)
	{
		// Reuse the last hole
		Id = FreeIndices.Pop(/*bAllowShrinking=*/false);
		RegisteredComponents[Id] = Component;
	}
	else
	{
		// No holes—grow the array
		Id = RegisteredComponents.Add(Component);
	}
	return Id;
}

void UUpgradeManagerSubsystem::UpdateUpgradeLevel(const int32 ComponentId, const int32 NewLevel) const
{
	UUpgradableComponent* Comp = GetComponentById(ComponentId);
	Comp->ApplyUpgradeInternal(NewLevel);
}

void UUpgradeManagerSubsystem::UnregisterUpgradableComponent(const int32 ComponentId)
{
	if (RegisteredComponents.IsValidIndex(ComponentId))
	{
		RegisteredComponents[ComponentId].Reset();    // Clear the weak ptr
		FreeIndices.Add(ComponentId);                // Remember this slot as a hole
	}
}

void UUpgradeManagerSubsystem::HandleUpgradeRequest(const int32 ComponentId, const int32 LevelIncrease) const
{
    const UUpgradableComponent* Comp = GetComponentById(ComponentId);
    if (!Comp || !Comp->GetOwner()->HasAuthority()) return;

    if (!CanUpgrade(Comp)) return;

    const int32 NewLevel = Comp->GetCurrentUpgradeLevel_Implementation() + LevelIncrease;
    const FUpgradeLevelData* LevelData = Provider->GetLevelData(NewLevel);

    if (!LevelData) return;

    // APlayerState* PS = Cast<APawn>(Comp->GetOwner())->GetPlayerState<APlayerState>();
    // if (!PS || PS->GetScore() < LevelData->UpgradeCost) return;
    //
    //
    // PS->SetScore(PS->GetScore() - LevelData->UpgradeCost);

	UpdateUpgradeLevel(ComponentId, NewLevel);
}

void UUpgradeManagerSubsystem::LoadJsonFromFile()
{
	const FString FilePath = FPaths::ProjectContentDir() / TEXT("Data/UpgradeLevels.json");
	FString JsonString;

	if (FPaths::FileExists(FilePath) && FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		InitializeProviderFromJson(JsonString);
		UE_LOG(LogTemp, Log, TEXT("UpgradeLevels.json loaded successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load UpgradeLevels.json from %s"), *FilePath);
	}
}

UUpgradableComponent* UUpgradeManagerSubsystem::GetComponentById(const int32 Id) const
{
	return (RegisteredComponents.IsValidIndex(Id)
			? RegisteredComponents[Id].Get()
			: nullptr);
}