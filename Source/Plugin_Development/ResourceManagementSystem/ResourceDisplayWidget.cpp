#include "ResourceDisplayWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "ResourceSystemComponent.h"
#include "ResourceManagerSubsystem.h"

void UResourceDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();

}

void UResourceDisplayWidget::BindResourceComponent(UResourceSystemComponent* InComponent)
{
    if (!InComponent) return;
    
    ResourceComp = InComponent;
    // Subscribe to change notifications
    ResourceComp->OnResourceChanged.AddDynamic(this, &UResourceDisplayWidget::HandleResourceChanged);
    // Initial population
    BuildResourceEntries();
}

void UResourceDisplayWidget::BuildResourceEntries()
{
    if (!ResourceComp || !ResourceListBox)
    {
        return;
    }

    ResourceListBox->ClearChildren();
    ResourceEntries.Empty();

    TMap<FName, int32> AllResources;
    ResourceComp->GetAllResources(AllResources);

    for (auto& Pair : AllResources)
    {
        UTextBlock* Entry = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (Entry)
        {
            FString EntryText = FString::Printf(TEXT("%s: %d"), *Pair.Key.ToString(), Pair.Value);
            Entry->SetText(FText::FromString(EntryText));
            ResourceListBox->AddChild(Entry);
            ResourceEntries.Add(Pair.Key, Entry);
        }
    }
}

UTextBlock* UResourceDisplayWidget::EnsureEntryExists(FName ResourceName, UVerticalBox* ContainerToQuery, TMap<FName, TWeakObjectPtr<UTextBlock>>& EntriesMap)
{
    if (auto Found = EntriesMap.Find(ResourceName))
    {
        if (UTextBlock* Existing = Found->Get()) return Existing;
    }
    
    if (!ContainerToQuery) return nullptr;
    
    UTextBlock* Entry = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    
    if (Entry)
    {
        ContainerToQuery->AddChild(Entry);
        EntriesMap.Add(ResourceName, Entry);
    }
    return Entry;
}

void UResourceDisplayWidget::HandleResourceChanged(FName ResourceName, int32 NewAmount, int32 DeltaAmount)
{
    if (UTextBlock* Entry = EnsureEntryExists(ResourceName, ResourceListBox, ResourceEntries))
    {
        Entry->SetText(FText::FromString(FString::Printf(TEXT("%s: %d"), *ResourceName.ToString(), NewAmount)));
    }
    
    if (UTextBlock* Entry = EnsureEntryExists(ResourceName, ChangedListBox, ResourceChangeEntries))
    {
        Entry->SetText(FText::FromString(FString::Printf(TEXT("%s: %d"), *ResourceName.ToString(), DeltaAmount)));
    }
}
