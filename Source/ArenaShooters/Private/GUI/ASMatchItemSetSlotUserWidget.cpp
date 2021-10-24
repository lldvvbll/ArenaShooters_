// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASMatchItemSetSlotUserWidget.h"
#include "GUI/ASMatchItemSetSelectUserWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "DataAssets/MatchGameDataAssets/ASMatchItemSetDataAsset.h"

void UASMatchItemSetSlotUserWidget::SetDataAsset(UASMatchItemSetDataAsset* DataAsset)
{
	if (!IsValid(DataAsset))
	{
		AS_LOG_S(Error);
		return;
	}

	DataAssetId = DataAsset->GetPrimaryAssetId();

	if (ThumbnailImage != nullptr)
	{
		ThumbnailImage->SetBrushFromTexture(DataAsset->ButtonImage);
	}

	if (NameTextBlock != nullptr)
	{
		NameTextBlock->SetText(DataAsset->SetName);
	}
}

FPrimaryAssetId UASMatchItemSetSlotUserWidget::GetDataAssetId() const
{
	return DataAssetId;
}

void UASMatchItemSetSlotUserWidget::ChangeButtonState(bool bIsSelected)
{
	bSelected = bIsSelected;

	if (bSelected)
	{
		if (SelectButton != nullptr)
		{
			SelectButton->SetBackgroundColor(SelectedButtonColor);
		}
	}
	else
	{		
		if (SelectButton != nullptr)
		{
			SelectButton->SetBackgroundColor(UnselectedButtonColor);
		}
	}
}

void UASMatchItemSetSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SelectButton = Cast<UButton>(GetWidgetFromName(TEXT("Btn_Select")));
	ThumbnailImage = Cast<UImage>(GetWidgetFromName(TEXT("Img_Thumbnail")));
	NameTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("TB_Name")));

	if (SelectButton != nullptr)
	{
		SelectButton->OnClicked.AddDynamic(this, &UASMatchItemSetSlotUserWidget::OnClickedButton);
	}
}

void UASMatchItemSetSlotUserWidget::OnClickedButton()
{
	if (bSelected)
		return;

	OnSelected.Broadcast(this);

	ChangeButtonState(true);
}
