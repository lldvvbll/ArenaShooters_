// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Lobby/ASServerBrowserUserWidget.h"
#include "GUI/Lobby/ASServerSlotUserWidget.h"
#include "Components/Button.h"
#include "Components/CircularThrobber.h"
#include "Components/ScrollBox.h"
#include "ASLobbyPlayerController.h"
#include "ASGameInstance.h"

void UASServerBrowserUserWidget::SearchServer()
{
	if (RefreshButton != nullptr)
	{
		RefreshButton->SetIsEnabled(false);
	}

	if (SearchCircularThrobber != nullptr)
	{
		SearchCircularThrobber->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (ServerListScrollBox != nullptr)
	{
		ServerListScrollBox->ClearChildren();
	}

	if (auto GameInst = GetGameInstance<UASGameInstance>())
	{
		GameInst->SearchServer();
	}
}

void UASServerBrowserUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BackButton = Cast<UButton>(GetWidgetFromName(TEXT("BackButton")));
	RefreshButton = Cast<UButton>(GetWidgetFromName(TEXT("RefreshButton")));
	SearchCircularThrobber = Cast<UCircularThrobber>(GetWidgetFromName(TEXT("SearchCircularThrobber")));
	ServerListScrollBox = Cast<UScrollBox>(GetWidgetFromName(TEXT("ServerListScrollBox")));

	if (BackButton != nullptr)
	{
		BackButton->OnClicked.AddDynamic(this, &UASServerBrowserUserWidget::OnClickedBackButton);
	}

	if (RefreshButton != nullptr)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UASServerBrowserUserWidget::OnClickedRefreshButton);
	}

	if (SearchCircularThrobber != nullptr)
	{
		SearchCircularThrobber->SetVisibility(ESlateVisibility::Hidden);
	}

	if (auto GameInst = GetGameInstance<UASGameInstance>())
	{
		OnSearchSessionResultDelegateHandle = GameInst->OnSearchSessionResult.AddUObject(this, &UASServerBrowserUserWidget::OnSearchSessionResult);
	}

	SearchServer();
}

void UASServerBrowserUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (auto GameInst = GetGameInstance<UASGameInstance>())
	{
		GameInst->OnSearchSessionResult.Remove(OnSearchSessionResultDelegateHandle);
	}
}

void UASServerBrowserUserWidget::OnClickedBackButton()
{
	if (auto LobbyPlayerController = GetOwningPlayer<AASLobbyPlayerController>())
	{
		LobbyPlayerController->ShowMainMenu();
	}

	RemoveFromParent();
}

void UASServerBrowserUserWidget::OnClickedRefreshButton()
{
	SearchServer();
}

void UASServerBrowserUserWidget::OnSearchSessionResult(const TArray<FOnlineSessionSearchResult>& NewSessionResults)
{
	if (RefreshButton != nullptr)
	{
		RefreshButton->SetIsEnabled(true);
	}

	if (SearchCircularThrobber != nullptr)
	{
		SearchCircularThrobber->SetVisibility(ESlateVisibility::Hidden);
	}

	if (ServerListScrollBox != nullptr)
	{
		for (auto& Result : NewSessionResults)
		{
			if (!Result.IsValid())
				continue;

			auto ServerSlot = CreateWidget<UASServerSlotUserWidget>(ServerListScrollBox, ServerSlotWidgetClass);
			if (ServerSlot == nullptr)
				continue;

			ServerSlot->SetServerInfo(Result);

			ServerListScrollBox->AddChild(ServerSlot);
		}

		AS_LOG(Warning, TEXT("SessionResults Count: %d"), NewSessionResults.Num());
	}	
}
