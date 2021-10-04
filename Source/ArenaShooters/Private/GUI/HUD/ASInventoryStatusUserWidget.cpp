// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASInventoryStatusUserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Controller/ASPlayerController.h"
#include "Character/ASCharacter.h"
#include "Character/ASInventoryComponent.h"
#include "Item/ASWeapon.h"
#include "Item/ASArmor.h"

void UASInventoryStatusUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HelmetProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("HelmetProgressBar")));
	JacketProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("JacketProgressBar")));
	FireModeTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("FireModeTextBlock")));
	CurrentAmmoTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("CurrentAmmoTextBlock")));
	TotalAmmoTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("TotalAmmoTextBlock")));

	if (auto Ctrlr = GetOwningPlayer<AASPlayerController>())
	{
		if (auto Char = Ctrlr->GetPawn<AASCharacter>())
		{
			InventoryComp = Char->GetInventoryComponent();
			if (InventoryComp != nullptr)
			{
				InventoryComp->OnChangedSelectedWeapon.AddUObject(this, &UASInventoryStatusUserWidget::OnChangedSelectedWeapon);
				InventoryComp->OnInsertArmor.AddUObject(this, &UASInventoryStatusUserWidget::OnInsertArmor);

				OnChangedSelectedWeapon(nullptr, InventoryComp->GetSelectedWeapon());
				OnInsertArmor(EArmorSlotType::Helmet, nullptr);
				OnInsertArmor(EArmorSlotType::Jacket, nullptr);
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASInventoryStatusUserWidget::OnChangedSelectedWeapon(const TWeakObjectPtr<UASWeapon>& OldWeapon, const TWeakObjectPtr<UASWeapon>& NewWeapon)
{
	AS_LOG_S(Warning);
}

void UASInventoryStatusUserWidget::OnInsertArmor(EArmorSlotType SlotType, UASArmor* Armor)
{
	switch (SlotType)
	{
	case EArmorSlotType::Helmet:
		{
			if (Armor != nullptr)
			{
				Armor->OnChangedDurability.Remove(OnChangedHelmetDurabilityDelegateHandle);
			}

			BindProgressBarToArmor(HelmetProgressBar, SlotType, OnChangedHelmetDurabilityDelegateHandle);
		}		
		break;
	case EArmorSlotType::Jacket:
		{
			if (Armor != nullptr)
			{
				Armor->OnChangedDurability.Remove(OnChangedJacketDurabilityDelegateHandle);
			}

			BindProgressBarToArmor(JacketProgressBar, SlotType, OnChangedJacketDurabilityDelegateHandle);
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void UASInventoryStatusUserWidget::BindProgressBarToArmor(UProgressBar* ProgressBar, EArmorSlotType SlotType,
	FDelegateHandle& InOnChangedArmorDurabilityDelegateHandle)
{
	if (InventoryComp == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (ProgressBar == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	ItemPtrBoolPair ItemPair = InventoryComp->FindItemFromArmorSlot(SlotType);
	if (!ItemPair.Value)
	{
		AS_LOG_S(Error);
		return;
	}

	if (UASArmor* Armor = Cast<UASArmor>(ItemPair.Key))
	{
		ProgressBar->SetVisibility(ESlateVisibility::HitTestInvisible);

		int32 SlotTypeInt = static_cast<int32>(SlotType);
		InOnChangedArmorDurabilityDelegateHandle = 
			Armor->OnChangedDurability.AddUObject(this, &UASInventoryStatusUserWidget::OnChangedArmorDurability, SlotTypeInt);

		OnChangedArmorDurability(Armor->GetCurrentDurability(), Armor->GetMaxDurability(), SlotTypeInt);
	}
	else
	{
		ProgressBar->SetVisibility(ESlateVisibility::Hidden);
		ProgressBar->SetPercent(0.0f);
	}
}

void UASInventoryStatusUserWidget::OnChangedArmorDurability(float Durability, float MaxDurability, int32 SlotTypeInt)
{
	auto SlotType = static_cast<EArmorSlotType>(SlotTypeInt);

	switch (SlotType)
	{
	case EArmorSlotType::Helmet:
		if (HelmetProgressBar != nullptr)
		{
			HelmetProgressBar->SetPercent(Durability / MaxDurability);
		}
		else
		{
			AS_LOG_S(Error);
		}
		break;
	case EArmorSlotType::Jacket:
		if (JacketProgressBar != nullptr)
		{
			JacketProgressBar->SetPercent(Durability / MaxDurability);
		}
		else
		{
			AS_LOG_S(Error);
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}
