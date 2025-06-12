// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeTimerDisplay.h"
#include "MightyraiderFunctionLibrary.h"
#include "UpgradeManagerSubsystem.h"


void UUpgradeTimerDisplay::NativeConstruct()
{
	Super::NativeConstruct();
	if (TrackedComponent)
	{
		TrackedComponent->OnUpgradeStarted.AddDynamic(this, &UUpgradeTimerDisplay::HandleUpgradeStarted);
		TrackedComponent->OnUpgradeCanceled.AddDynamic(this, &UUpgradeTimerDisplay::HandleUpgradeCanceled);
		TrackedComponent->OnLevelChanged.AddDynamic(this, &UUpgradeTimerDisplay::HandleLevelChanged);
		TrackedComponent->OnTimeToUpgradeChanged.AddDynamic(this, &UUpgradeTimerDisplay::HandleTimeToUpgradeChanged);
		
	}
}

void UUpgradeTimerDisplay::BindUpgradableComponent(int32 ComponentId)
{
	if (!TrackedComponent) return;
	
	if (UUpgradableComponent* Comp = GetWorld()->GetSubsystem<UUpgradeManagerSubsystem>()->GetComponentById(ComponentId))
	{
		TrackedComponent = Comp;
		TrackedComponent->OnUpgradeStarted.AddDynamic(this, &UUpgradeTimerDisplay::HandleUpgradeStarted);
		TrackedComponent->OnUpgradeCanceled.AddDynamic(this, &UUpgradeTimerDisplay::HandleUpgradeCanceled);
		TrackedComponent->OnLevelChanged.AddDynamic(this, &UUpgradeTimerDisplay::HandleLevelChanged);
		TrackedComponent->OnTimeToUpgradeChanged.AddDynamic(this, &UUpgradeTimerDisplay::HandleTimeToUpgradeChanged);
	}
}

void UUpgradeTimerDisplay::HandleUpgradeStarted_Implementation(float SecondsUntilCompleted)
{
	
	if (CountdownHandle.IsValid())
	{
		StopCountdownTimer();
		RemainingTime = SecondsUntilCompleted;
	}
	else
	{
		TotalTime = SecondsUntilCompleted;
		RemainingTime = SecondsUntilCompleted;
	}
	
	if (RemainingTime < 60.f)
	{
		CountdownText->SetText(FText::AsNumber(RemainingTime));
	}
	else
	{
		FTimespan Timespan = FTimespan::FromSeconds(RemainingTime);
		CountdownText->SetText(FText::AsTimespan(Timespan));
	}
	
	StartCountdownTimer();
}

void UUpgradeTimerDisplay::HandleUpgradeCanceled_Implementation(int32 CurrentLevel)
{
	StopCountdownTimer();
	TotalTime = -1.f;
	RemainingTime = -1.f;
	LevelText->SetText(FText::AsNumber(CurrentLevel));
}

void UUpgradeTimerDisplay::HandleTimeToUpgradeChanged_Implementation(float DeltaTime)
{
	const int32 Sign = FMath::Sign(DeltaTime);
	const TCHAR SignChar = (Sign >= 0 ? TEXT('+') : TEXT('-'));
	
	const float AbsDelta = FMath::Abs(DeltaTime);
	
	const FString Formatted = FString::Printf(TEXT("%c%.0f"), SignChar, AbsDelta);
	
	CountdownChangedText->SetText(FText::FromString(Formatted));
	RemainingTime += DeltaTime;

	
	if (RemainingTime > 0.f)
	{
		const float ElapsedTime = TotalTime - RemainingTime; 
		UpgradeProgress->SetPercent(ElapsedTime / TotalTime);
	}
	else
	{
		CountdownText->SetText(FText::AsNumber(0.f));
		UpgradeProgress->SetPercent(1.f);
	}
}

void UUpgradeTimerDisplay::HandleLevelChanged_Implementation(int32 OldLevel, int32 NewLevel)
{
	StopCountdownTimer();
	CountdownText->SetText(FText::AsNumber(0.f));
	LevelText->SetText(FText::AsNumber(NewLevel));
	UpgradeProgress->SetPercent(0.f);
	TotalTime = -1.f;
	RemainingTime = -1.f;

	
}

void UUpgradeTimerDisplay::StartCountdownTimer()
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("UpdateCountdownText"));
	if (RemainingTime > 10.f)
	{
		GetWorld()->GetTimerManager().SetTimer(CountdownHandle, TimerDelegate, 1.f, true);
		CountdownTimerRate = 1.f;
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(CountdownHandle, TimerDelegate, 0.1f, true);
		CountdownTimerRate = 0.1f;
	}
	
}

void UUpgradeTimerDisplay::UpdateCountdownText()
{

	RemainingTime -= CountdownTimerRate;
	const float ElapsedTime = TotalTime - RemainingTime; 
	UpgradeProgress->SetPercent(ElapsedTime / TotalTime);
	
	if (RemainingTime <= 60.f)
	{
		CountdownText->SetText(FText::AsNumber(RemainingTime));
		if (RemainingTime < 10.f && (CountdownTimerRate - 0.1f > KINDA_SMALL_NUMBER) )
		{
			StopCountdownTimer();
			StartCountdownTimer();
		}
	}
	else
	{
		FTimespan Timespan = FTimespan::FromSeconds(RemainingTime);
		CountdownText->SetText(FText::AsTimespan(Timespan));
	}
	
	
	
}

void UUpgradeTimerDisplay::StopCountdownTimer()
{
	if (CountdownHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownHandle);
	}
}


