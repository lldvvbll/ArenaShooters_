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

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void JoinServer();
	
private:
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
};
