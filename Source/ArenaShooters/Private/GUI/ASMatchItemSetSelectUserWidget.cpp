// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASMatchItemSetSelectUserWidget.h"
#include "GUI/ASMatchItemSetSlotUserWidget.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "Components/ScrollBox.h"

void UASMatchItemSetSelectUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SlotsScrollBox = Cast<UScrollBox>(GetWidgetFromName(TEXT("SB_Slots")));

	if (SlotsScrollBox != nullptr)
	{
		SlotsScrollBox->ClearChildren();

		auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
		if (IsValid(GameState))
		{
			TArray<UASMatchItemSetDataAsset*> DataAssets = GameState->GetMatchItemSetDataAssets();
			for (auto& DataAsset : DataAssets)
			{
				auto MatchItemSetSlotWidget = CreateWidget<UASMatchItemSetSlotUserWidget>(this, MatchItemSetSlotWidgetClass);
				if (MatchItemSetSlotWidget != nullptr)
				{
					MatchItemSetSlotWidget->OnSelected.AddUObject(this, &UASMatchItemSetSelectUserWidget::OnSelectedSlot);

					SlotsScrollBox->AddChild(MatchItemSetSlotWidget);
					MatchItemSetSlotWidget->SetDataAsset(DataAsset);
				}
				else
				{
					AS_LOG_S(Error);
				}
			}
		}
		else
		{
			AS_LOG_S(Error);
		}

		if (SlotsScrollBox->GetChildrenCount() > 0)
		{
			if (auto SlotWidget = Cast<UASMatchItemSetSlotUserWidget>(SlotsScrollBox->GetChildAt(0)))
			{
				SlotWidget->ChangeButtonState(true);
			}
		}
	}
}

void UASMatchItemSetSelectUserWidget::OnSelectedSlot(UASMatchItemSetSlotUserWidget* SelectedSlot)
{
	if (SelectedSlot == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (SlotsScrollBox != nullptr)
	{
		FPrimaryAssetId AssetId = SelectedSlot->GetDataAssetId();
		for (auto& ItemSetSlot : SlotsScrollBox->GetAllChildren())
		{
			if (auto SlotWidget = Cast<UASMatchItemSetSlotUserWidget>(ItemSetSlot))
			{
				if (SlotWidget->GetDataAssetId() == AssetId)
				{
					AS_LOG(Warning, TEXT("UASMatchItemSetSlotUserWidget: %s"), *AssetId.ToString());
				}
				else
				{
					SlotWidget->ChangeButtonState(false);
				}
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
	}
}
