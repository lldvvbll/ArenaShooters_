// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASGameMenuUserWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Controller/ASPlayerController.h"
#include "Character/ASCharacter.h"
#include "ASGameInstance.h"

void UASGameMenuUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameMenuWidgetSwitcher = Cast<UWidgetSwitcher>(GetWidgetFromName(TEXT("GameMenuWidgetSwitcher")));
	ContinueButton = Cast<UButton>(GetWidgetFromName(TEXT("ContinueButton")));
	KeySettingsButton = Cast<UButton>(GetWidgetFromName(TEXT("KeySettingsButton")));
	KeySettingsBackButton = Cast<UButton>(GetWidgetFromName(TEXT("KeySettingsBackButton")));
	GoToMainMenuButton = Cast<UButton>(GetWidgetFromName(TEXT("GoToMainMenuButton")));
	QuitGameButton = Cast<UButton>(GetWidgetFromName(TEXT("QuitGameButton")));

	if (ContinueButton != nullptr)
	{
		ContinueButton->OnClicked.AddDynamic(this, &UASGameMenuUserWidget::OnClickedContinueButton);
	}

	if (KeySettingsButton != nullptr)
	{
		KeySettingsButton->OnClicked.AddDynamic(this, &UASGameMenuUserWidget::OnClickedKeySettingsButton);
	}
	
	if (KeySettingsBackButton != nullptr)
	{
		KeySettingsBackButton->OnClicked.AddDynamic(this, &UASGameMenuUserWidget::OnClickedKeySettingsBackButton);
	}

	if (GoToMainMenuButton != nullptr)
	{
		GoToMainMenuButton->OnClicked.AddDynamic(this, &UASGameMenuUserWidget::OnClickedGoToMainMenuButton);
	}

	if (QuitGameButton != nullptr)
	{
		QuitGameButton->OnClicked.AddDynamic(this, &UASGameMenuUserWidget::OnClickedQuitGameButton);
	}
	
	FOnInputAction EscAction;
	EscAction.BindDynamic(this, &UASGameMenuUserWidget::OnEscKeyPressed);
	ListenForInputAction(FName(TEXT("ShowGameMenu")), EInputEvent::IE_Pressed, true, EscAction);

	OnConstructed.Broadcast(this);
}

void UASGameMenuUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	OnDestructed.Broadcast(this);
}

void UASGameMenuUserWidget::OnClickedContinueButton()
{
	RemoveFromParent();
}

void UASGameMenuUserWidget::OnClickedKeySettingsButton()
{
	if (ensure(GameMenuWidgetSwitcher != nullptr))
	{
		GameMenuWidgetSwitcher->SetActiveWidgetIndex(EWidgetIndex::KeySettings);
	}
}

void UASGameMenuUserWidget::OnClickedKeySettingsBackButton()
{
	if (ensure(GameMenuWidgetSwitcher != nullptr))
	{
		GameMenuWidgetSwitcher->SetActiveWidgetIndex(EWidgetIndex::GameMenu);
	}
}

void UASGameMenuUserWidget::OnClickedGoToMainMenuButton()
{
	auto PC = GetOwningPlayer<AASPlayerController>();
	if (ensure(IsValid(PC)))
	{
		PC->GoToMainMenu();
	}
}

void UASGameMenuUserWidget::OnClickedQuitGameButton()
{
	GetOwningPlayer()->ConsoleCommand(TEXT("quit"));
}

void UASGameMenuUserWidget::OnEscKeyPressed()
{
	if (ensure(GameMenuWidgetSwitcher != nullptr))
	{
		switch (GameMenuWidgetSwitcher->GetActiveWidgetIndex())
		{
		case EWidgetIndex::GameMenu:
			RemoveFromParent();
			break;
		case EWidgetIndex::KeySettings:
			OnClickedKeySettingsBackButton();
			break;
		default:
			checkNoEntry();
			break;
		}
	}
}
