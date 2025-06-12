// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ResourceSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "ResourceDisplayWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UResourceComponent;

/**
 * 
 */
UCLASS()
class PLUGIN_DEVELOPMENT_API UResourceDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Bind this widget to a ResourceComponent to display its data */
	UFUNCTION(BlueprintCallable, Category="Resource System")
	void BindResourceComponent(UResourceSystemComponent* InComponent);

protected:
	/** Vertical box in UMG to hold per-resource text entries */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UVerticalBox* ResourceListBox;

	/** Container for spent resource notifications */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UVerticalBox* ChangedListBox;

private:

	/** Map of resource name → its TextBlock for quick updates */
	TMap<FName, TWeakObjectPtr<UTextBlock>> ResourceEntries;
	
	/** Map of resource name → its TextBlock for spent values */
	TMap<FName, TWeakObjectPtr<UTextBlock>> ResourceChangeEntries;
	
	/** The component providing resource data */
	UPROPERTY()
	UResourceSystemComponent* ResourceComp;

	/** Handler for resource change events */
	UFUNCTION()
	void HandleResourceChanged(FName ResourceName, int32 NewAmount, int32 DeltaAmount);

	/** Build the initial list of resources and cache the TextBlocks */
	void BuildResourceEntries();

	/** Ensure there is a TextBlock for ResourceName (create if missing) */
	UTextBlock* EnsureEntryExists(FName ResourceName, UVerticalBox* ContainerToQuery, TMap<FName, TWeakObjectPtr<UTextBlock>>& EntriesMap);

};
