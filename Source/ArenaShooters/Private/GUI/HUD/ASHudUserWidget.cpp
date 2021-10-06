// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASHudUserWidget.h"
#include "GUI/HUD/ASInventoryStatusUserWidget.h"
#include "Components/ProgressBar.h"
#include "Controller/ASPlayerController.h"
#include "Character/ASCharacter.h"
#include "Character/ASStatusComponent.h"

void UASHudUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InventoryStatusWidget = Cast<UASInventoryStatusUserWidget>(GetWidgetFromName(TEXT("InventoryStatusWidget")));
	HealthProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("HealthProgressBar")));

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

void UASHudUserWidget::OnChangedCharacterHealth(float NewHealth) const
{
	if (HealthProgressBar != nullptr)
	{
		HealthProgressBar->SetPercent(NewHealth / MaxCharHealth);
	}
}
