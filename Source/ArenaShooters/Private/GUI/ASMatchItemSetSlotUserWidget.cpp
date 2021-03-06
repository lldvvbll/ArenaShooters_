// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASMatchItemSetSlotUserWidget.h"
#include "GUI/ASMatchItemSetSelectUserWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "DataAssets/ItemDataAssets/ASItemSetDataAsset.h"
#include "Controller/ASPlayerState.h"

void UASMatchItemSetSlotUserWidget::SetDataAsset(UASItemSetDataAsset* DataAsset)
{
	if (!ensure(DataAsset))
		return;

	ItemSetDataAsset = DataAsset;

	if (ThumbnailImage != nullptr)
	{
		ThumbnailImage->SetBrushFromTexture(DataAsset->ButtonImage);
	}

	if (NameTextBlock != nullptr)
	{
		NameTextBlock->SetText(DataAsset->SetName);
	}
}

FPrimaryAssetId UASMatchItemSetSlotUserWidget::GetItemSetDataAssetId() const
{
	return (ItemSetDataAsset != nullptr) ? ItemSetDataAsset->GetPrimaryAssetId() : FPrimaryAssetId();
}

void UASMatchItemSetSlotUserWidget::ChangeButtonState(bool bIsSelected)
{
	if (bSelected == bIsSelected)
		return;

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

	auto PlayerState = GetOwningPlayer()->GetPlayerState<AASPlayerState>();
	if (ensure(IsValid(PlayerState)))
	{
		PlayerState->ServerSetItemSetDataAsset(ItemSetDataAsset);
	}
}
