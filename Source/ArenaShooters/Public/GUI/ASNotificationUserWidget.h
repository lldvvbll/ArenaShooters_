// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASNotificationUserWidget.generated.h"

class UTextBlock;

UCLASS()
class ARENASHOOTERS_API UASNotificationUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetInfo(const FString& Message, float Duration);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY()
	UTextBlock* MessageTextBlock;

	FDateTime ExpireTime;
	bool bSetDuration = false;
};
