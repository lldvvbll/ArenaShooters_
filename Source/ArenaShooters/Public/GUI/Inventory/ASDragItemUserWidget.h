// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Blueprint/UserWidget.h"
#include "ASDragItemUserWidget.generated.h"

class UImage;
class UBorder;
class UASItemUserWidget;

UCLASS()
class ARENASHOOTERS_API UASDragItemUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetItemImage(UTexture2D* InImage);
	void SetSuitableBrush(bool bShuitable);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	UImage* ItemImage;

	UPROPERTY()
	UBorder* BackgroundBorder;
};
