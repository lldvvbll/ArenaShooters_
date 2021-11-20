// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "ASServerSlotUserWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class ARENASHOOTERS_API UASServerSlotUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetServerInfo(const FOnlineSessionSearchResult& NewSearchResult);

	void SetButtonEnable(bool bIsEnable);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void JoinServer();
	
public:
	DECLARE_EVENT(UASServerSlotUserWidget, FOnButtonClickedEvent);
	FOnButtonClickedEvent OnButtonClicked;

protected:
	FOnlineSessionSearchResult SearchResult;

	UPROPERTY()
	UTextBlock* ServerNameTextBlock;

	UPROPERTY()
	UTextBlock* ServerMapTextBlock;

	UPROPERTY()
	UTextBlock* PlayersTextBlock;

	UPROPERTY()
	UTextBlock* PingTextBlock;

	UPROPERTY()
	UButton* JoinButton;

	UPROPERTY()
	UTextBlock* JoinButtonTextBlock;
};
