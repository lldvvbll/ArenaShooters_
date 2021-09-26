// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASDragItemUserWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"

void UASDragItemUserWidget::SetItemImage(UTexture2D* InImage)
{
	if (ItemImage != nullptr)
	{
		ItemImage->SetBrushFromTexture(InImage);
	}
	else
	{
		AS_LOG_SCREEN_S(5.0f, FColor::Red);
	}
}

void UASDragItemUserWidget::SetSuitableBrush(bool bShuitable)
{
	if (BackgroundBorder != nullptr)
	{
		BackgroundBorder->SetBrushColor(bShuitable ? FLinearColor(0.5f, 0.5f, 0.5f, 0.5f) : FLinearColor(0.5f, 0.0f, 0.0f, 0.5f));
	}
}

void UASDragItemUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ItemImage = Cast<UImage>(GetWidgetFromName(TEXT("ItemImage")));
	BackgroundBorder = Cast<UBorder>(GetWidgetFromName(TEXT("BackgroundBorder")));
}
