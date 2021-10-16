// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Lobby/ASMainMenuUserWidget.h"
#include "GUI/Lobby/ASServerBrowserUserWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Controller/ASLobbyPlayerController.h"

void UASMainMenuUserWidget::ShowMainMenu()
{
	if (MainMenuWidgetSwitcher != nullptr)
	{
		MainMenuWidgetSwitcher->SetActiveWidgetIndex(EWidgetIndex::MainMenu);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASMainMenuUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bFirstFindServer = true;

	MainMenuWidgetSwitcher = Cast<UWidgetSwitcher>(GetWidgetFromName(TEXT("MainMenuWidgetSwitcher")));
	FindServerButton = Cast<UButton>(GetWidgetFromName(TEXT("FindServerButton")));
	ServerBrowserWidget = Cast<UASServerBrowserUserWidget>(GetWidgetFromName(TEXT("ServerBrowserWidget")));
	KeySettingsButton = Cast<UButton>(GetWidgetFromName(TEXT("KeySettingsButton")));
	KeySettingsBackButton = Cast<UButton>(GetWidgetFromName(TEXT("KeySettingsBackButton")));
	ExitButton = Cast<UButton>(GetWidgetFromName(TEXT("ExitButton")));

	if (FindServerButton != nullptr)
	{
		FindServerButton->OnClicked.AddDynamic(this, &UASMainMenuUserWidget::OnClickedFindServerButton);
	}

	if (ServerBrowserWidget != nullptr)
	{
		ServerBrowserWidget->SetMainMenuWidget(this);
	}

	if (KeySettingsButton != nullptr)
	{
		KeySettingsButton->OnClicked.AddDynamic(this, &UASMainMenuUserWidget::OnClickedKeySettingsButton);
	}

	if (KeySettingsBackButton != nullptr)
	{
		KeySettingsBackButton->OnClicked.AddDynamic(this, &UASMainMenuUserWidget::OnClickedKeySettingsBackButton);
	}

	if (ExitButton != nullptr)
	{
		ExitButton->OnClicked.AddDynamic(this, &UASMainMenuUserWidget::OnClickedExitButton);
	}

	FOnInputAction EscAction;
	EscAction.BindDynamic(this, &UASMainMenuUserWidget::OnEscKeyPressed);
	ListenForInputAction(FName(TEXT("ShowGameMenu")), EInputEvent::IE_Pressed, true, EscAction);
}

void UASMainMenuUserWidget::OnClickedFindServerButton()
{
	if (MainMenuWidgetSwitcher != nullptr)
	{
		MainMenuWidgetSwitcher->SetActiveWidgetIndex(EWidgetIndex::FindServer);

		if (bFirstFindServer)
		{
			if (ServerBrowserWidget != nullptr)
			{
				bFirstFindServer = false;

				ServerBrowserWidget->SearchServer();
			}
			else
			{
				AS_LOG_S(Error);
			}
		}		
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASMainMenuUserWidget::OnClickedKeySettingsButton()
{
	if (MainMenuWidgetSwitcher != nullptr)
	{
		MainMenuWidgetSwitcher->SetActiveWidgetIndex(EWidgetIndex::KeySettings);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASMainMenuUserWidget::OnClickedKeySettingsBackButton()
{
	ShowMainMenu();
}

void UASMainMenuUserWidget::OnClickedExitButton()
{
	GetOwningPlayer()->ConsoleCommand("quit");
}

void UASMainMenuUserWidget::OnEscKeyPressed()
{
	if (MainMenuWidgetSwitcher != nullptr)
	{
		switch (MainMenuWidgetSwitcher->GetActiveWidgetIndex())
		{
		case EWidgetIndex::MainMenu:
			//RemoveFromParent();
			break;
		case EWidgetIndex::FindServer:
		case EWidgetIndex::KeySettings:
			ShowMainMenu();
			break;
		default:
			checkNoEntry();
			break;
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}
