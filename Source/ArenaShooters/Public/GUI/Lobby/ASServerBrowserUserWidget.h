// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASServerBrowserUserWidget.generated.h"

class UButton;
class UCircularThrobber;
class UScrollBox;
class UASServerSlotUserWidget;
class UASMainMenuUserWidget;
class USizeBox;

UCLASS()
class ARENASHOOTERS_API UASServerBrowserUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SearchServer();
	void SetMainMenuWidget(UASMainMenuUserWidget* InMainMenuWidget);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnClickedBackButton();

	UFUNCTION()
	void OnClickedRefreshButton();

	void OnSearchSessionResult(const TArray<FOnlineSessionSearchResult>& NewSessionResults);

	void OnJoinButtonClicked();

protected:
	UPROPERTY()
	UASMainMenuUserWidget* MainMenuWidget;

	UPROPERTY()
	UButton* BackButton;

	UPROPERTY()
	UButton* RefreshButton;

	UPROPERTY()
	UCircularThrobber* SearchCircularThrobber;

	UPROPERTY()
	UScrollBox* ServerListScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UASServerSlotUserWidget> ServerSlotWidgetClass;

	FDelegateHandle OnSearchSessionResultDelegateHandle;

	UPROPERTY()
	USizeBox* ServerNotFoundSizeBox;
};
