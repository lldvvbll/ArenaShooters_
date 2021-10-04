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
	AS_LOG_S(Warning);

	switch (SlotType)
	{
	case EArmorSlotType::Helmet:
		{

		}
		break;
	case EArmorSlotType::Jacket:
		{

		}
		break;
	default:
		checkNoEntry();
		break;
	}
}
