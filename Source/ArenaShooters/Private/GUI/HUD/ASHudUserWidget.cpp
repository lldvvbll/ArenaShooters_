// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASHudUserWidget.h"
#include "GUI/HUD/ASInventoryStatusUserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Controller/ASPlayerController.h"
#include "Character/ASCharacter.h"
#include "Character/ASStatusComponent.h"
#include "GameMode/ASMatchGameStateBase.h"

void UASHudUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InventoryStatusWidget = Cast<UASInventoryStatusUserWidget>(GetWidgetFromName(TEXT("InventoryStatusWidget")));
	FinishCountDownBorder = Cast<UBorder>(GetWidgetFromName(TEXT("FinishCountDownBorder")));
	FinishCountDownTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("FinishCountDownTextBlock")));	
	HealthProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("HealthProgressBar")));

	bSetMatchFinishTime = false;
	MatchFinishTime = FDateTime::MaxValue();

	if (FinishCountDownBorder != nullptr)
	{
		FinishCountDownBorder->SetVisibility(ESlateVisibility::Hidden);
	}

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (IsValid(GameState))
	{
		GameState->OnSetMatchFinishTime.AddUObject(this, &UASHudUserWidget::OnSetMatchFinishTime);
	}
	else
	{
		AS_LOG_S(Error);
	}

	if (auto ASPlayerController = GetOwningPlayer<AASPlayerController>())
	{
		if (auto Char = ASPlayerController->GetPawn<AASCharacter>())
		{
			if (auto StatusComp = Char->GetStatusComponent())
			{
				StatusComp->OnChangeCurrentHealth.AddUObject(this, &UASHudUserWidget::OnChangedCharacterHealth);

				MaxCharHealth = StatusComp->GetMaxHealth();
				OnChangedCharacterHealth(StatusComp->GetCurrentHealth());
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASHudUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bSetMatchFinishTime)
	{
		if (FinishCountDownTextBlock != nullptr)
		{
			FTimespan RemainTime = MatchFinishTime - FDateTime::Now();

			int32 RemainMinutes = RemainTime.GetMinutes();
			if (RemainMinutes < 0)
			{
				RemainMinutes = 0;
			}

			int32 RemainSeconds = RemainTime.GetSeconds();
			if (RemainSeconds < 0)
			{
				RemainSeconds = 0;
			}

			FinishCountDownTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d : %d"), RemainMinutes, RemainSeconds)));
		}
	}
}

void UASHudUserWidget::OnSetMatchFinishTime(float Time)
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (IsValid(GameState))
	{
		bSetMatchFinishTime = true;

		float DeltaTime = Time - GameState->GetServerWorldTimeSeconds();
		MatchFinishTime = FDateTime::Now() + FTimespan::FromSeconds(DeltaTime);

		if (FinishCountDownBorder != nullptr)
		{
			FinishCountDownBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASHudUserWidget::OnChangedCharacterHealth(float NewHealth) const
{
	if (HealthProgressBar != nullptr)
	{
		HealthProgressBar->SetPercent(NewHealth / MaxCharHealth);
	}
}
