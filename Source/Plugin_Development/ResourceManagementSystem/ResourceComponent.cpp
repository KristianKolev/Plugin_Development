#include "ResourceComponent.h"
#include "ResourceManagerSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Controller.h"

UResourceComponent::UResourceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UResourceComponent::BeginPlay()
{
    Super::BeginPlay();
    
        OnResourceChanged.AddDynamic(this, &UResourceComponent::HandleResourceChanged);
}

UResourceManagerSubsystem* UResourceComponent::GetWorldSubsystem() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetSubsystem<UResourceManagerSubsystem>();
    }
    return nullptr;
}

void UResourceComponent::AddResource(FName ResourceName, int32 Amount)
{
    if (UResourceManagerSubsystem* Sub = GetWorldSubsystem())
    {
            Sub->AddResource(this, ResourceName, Amount);
    }
}

bool UResourceComponent::SpendResource(FName ResourceName, int32 Amount)
{
    if (UResourceManagerSubsystem* Sub = GetWorldSubsystem())
    {
            return Sub->SpendResource(this, ResourceName, Amount);
    }
    return false;
}

int32 UResourceComponent::GetResource(FName ResourceName) const
{
    if (UResourceManagerSubsystem* Sub = GetWorldSubsystem())
    {
            return Sub->GetResource(this, ResourceName);
    }
    return 0;
}

void UResourceComponent::HandleResourceChanged_Implementation(FName ResourceName, int32 NewAmount, int32 AmountChange)
{
    
}
