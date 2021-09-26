// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Lobby/ASMainMenuUserWidget.h"
#include "GUI/Lobby/ASServerBrowserUserWidget.h"
#include "Components/Button.h"
#include "ASLobbyPlayerController.h"

void UASMainMenuUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FindServerButton = Cast<UButton>(GetWidgetFromName(TEXT("FindServerButton")));
	ExitButton = Cast<UButton>(GetWidgetFromName(TEXT("ExitButton")));

	if (FindServerButton != nullptr)
	{
		FindServerButton->OnClicked.AddDynamic(this, &UASMainMenuUserWidget::OnClickedFindServerButton);
	}

	if (ExitButton != nullptr)
	{
		ExitButton->OnClicked.AddDynamic(this, &UASMainMenuUserWidget::OnClickedExitButton);
	}
}

void UASMainMenuUserWidget::OnClickedFindServerButton()
{
	SetVisibility(ESlateVisibility::Hidden);

	if (auto LobbyPlayerController = GetOwningPlayer<AASLobbyPlayerController>())
	{
		LobbyPlayerController->ShowServerBrowser();
	}
}

void UASMainMenuUserWidget::OnClickedExitButton()
{
	GetOwningPlayer()->ConsoleCommand("quit");
}
