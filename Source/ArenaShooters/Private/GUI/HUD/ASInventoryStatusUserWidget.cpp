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
	InventoryAmmoTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("InventoryAmmoTextBlock")));

	if (auto Ctrlr = GetOwningPlayer<AASPlayerController>())
	{
		if (auto Char = Ctrlr->GetPawn<AASCharacter>())
		{
			InventoryComp = Char->GetInventoryComponent();
			if (InventoryComp != nullptr)
			{
				InventoryComp->OnChangedSelectedWeapon.AddUObject(this, &UASInventoryStatusUserWidget::OnChangedSelectedWeapon);
				InventoryComp->OnInsertArmor.AddUObject(this, &UASInventoryStatusUserWidget::OnInsertArmor);
				InventoryComp->OnChangedInventoryAmmoCount.AddUObject(this, &UASInventoryStatusUserWidget::OnChangedInventoryAmmoCount);

				TWeakObjectPtr<UASWeapon> NewWeapon = InventoryComp->GetSelectedWeapon();

				OnChangedSelectedWeapon(nullptr, NewWeapon);
				OnInsertArmor(EArmorSlotType::Helmet, nullptr);
				OnInsertArmor(EArmorSlotType::Jacket, nullptr);

				if (NewWeapon.IsValid())
				{
					OnChangedInventoryAmmoCount(InventoryComp->GetAmmoCountInInventory(NewWeapon->GetAmmoType()));
				}
				else
				{
					OnChangedInventoryAmmoCount(0);
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
	else
	{
		AS_LOG_S(Error);
	}
}

void UASInventoryStatusUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (auto Ctrlr = GetOwningPlayer<AASPlayerController>())
	{
		if (auto Char = Ctrlr->GetPawn<AASCharacter>())
		{
			InventoryComp = Char->GetInventoryComponent();
			if (InventoryComp != nullptr)
			{
				InventoryComp->OnChangedSelectedWeapon.RemoveAll(this);
				InventoryComp->OnInsertArmor.RemoveAll(this);
				InventoryComp->OnChangedInventoryAmmoCount.RemoveAll(this);
			}
		}
	}
}

void UASInventoryStatusUserWidget::OnChangedSelectedWeapon(const TWeakObjectPtr<UASWeapon>& OldWeapon, const TWeakObjectPtr<UASWeapon>& NewWeapon)
{
	if (OldWeapon.IsValid())
	{
		OldWeapon->OnFireModeChanged.RemoveAll(this);
		OldWeapon->OnCurrentAmmoCountChanged.RemoveAll(this);
	}

	if (NewWeapon.IsValid())
	{
		NewWeapon->OnFireModeChanged.AddUObject(this, &UASInventoryStatusUserWidget::OnChangedFireMode);
		NewWeapon->OnCurrentAmmoCountChanged.AddUObject(this, &UASInventoryStatusUserWidget::OnChangedCurrentAmmoCount);
		
		if (FireModeTextBlock != nullptr)
		{
			FireModeTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		OnChangedFireMode(NewWeapon->GetFireMode());
		OnChangedCurrentAmmoCount(NewWeapon->GetCurrentAmmoCount());
	}
	else
	{
		if (FireModeTextBlock != nullptr)
		{
			FireModeTextBlock->SetVisibility(ESlateVisibility::Hidden);
		}

		OnChangedCurrentAmmoCount(0);
	}
}

void UASInventoryStatusUserWidget::OnInsertArmor(EArmorSlotType SlotType, UASArmor* Armor)
{
	switch (SlotType)
	{
	case EArmorSlotType::Helmet:
		{
			if (Armor != nullptr)
			{
				Armor->OnChangedDurability.RemoveAll(this);
			}

			BindProgressBarToArmor(HelmetProgressBar, SlotType);
		}		
		break;
	case EArmorSlotType::Jacket:
		{
			if (Armor != nullptr)
			{
				Armor->OnChangedDurability.RemoveAll(this);
			}

			BindProgressBarToArmor(JacketProgressBar, SlotType);
		}
		break;
	default:
		checkNoEntry();
		break;
	}
}

void UASInventoryStatusUserWidget::BindProgressBarToArmor(UProgressBar* ProgressBar, EArmorSlotType SlotType)
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

void UASInventoryStatusUserWidget::OnChangedFireMode(EFireMode NewFireMode)
{
	if (FireModeTextBlock == nullptr)
		return;

	FText FireModeText;
	switch (NewFireMode)
	{
	case EFireMode::SemiAuto:
		FireModeText = FText::FromString(TEXT("SemiAuto"));
		break;
	case EFireMode::FullAuto:
		FireModeText = FText::FromString(TEXT("FullAuto"));
		break;
	default:
		checkNoEntry();
		break;
	}

	FireModeTextBlock->SetText(FireModeText);
}

void UASInventoryStatusUserWidget::OnChangedCurrentAmmoCount(int32 NewCount)
{
	if (CurrentAmmoTextBlock == nullptr)
		return;

	CurrentAmmoTextBlock->SetText(FText::FromString(FString::FromInt(NewCount)));
}

void UASInventoryStatusUserWidget::OnChangedInventoryAmmoCount(int32 NewCount)
{
	if (InventoryAmmoTextBlock == nullptr)
		return;

	InventoryAmmoTextBlock->SetText(FText::FromString(FString::FromInt(NewCount)));
}
