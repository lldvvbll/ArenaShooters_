// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASCrossHairUserWidget.h"
#include "Components/Image.h"
#include "Character/ASCharacter.h"
#include "Character/ASInventoryComponent.h"
#include "Item/ASWeapon.h"

void UASCrossHairUserWidget::OnChangeSelectedWeapon(const TWeakObjectPtr<UASWeapon>& OldWeapon, const TWeakObjectPtr<UASWeapon>& NewWeapon)
{
	ESlateVisibility BarVisibility = ESlateVisibility::Visible;

	if (NewWeapon.IsValid())
	{
		constexpr float ReduceRate = 4000.0f * 0.15f;

		MinBarOffset = FMath::Tan(FMath::DegreesToRadians(NewWeapon->GetMinBulletSpread())) * ReduceRate;
		MaxBarOffset = FMath::Tan(FMath::DegreesToRadians(NewWeapon->GetMaxBulletSpread())) * ReduceRate;
		OffsetPerShot = FMath::Tan(FMath::DegreesToRadians(NewWeapon->GetBulletSpreadAmountPerShot())) * ReduceRate;
		RecoverySpeed = FMath::Tan(FMath::DegreesToRadians(NewWeapon->GetBulletSpreadRecoverySpeed())) * ReduceRate;
	}
	else
	{
		MinBarOffset = TNumericLimits<float>::Max();
		MaxBarOffset = TNumericLimits<float>::Max();
		OffsetPerShot = TNumericLimits<float>::Max();
		RecoverySpeed = 0.0f;

		BarVisibility = ESlateVisibility::Hidden;
	}

	CurrentBarOffset = MinBarOffset;

	if (TopBar != nullptr)
	{
		TopBar->SetVisibility(BarVisibility);
	}
	if (BottomBar != nullptr)
	{
		BottomBar->SetVisibility(BarVisibility);
	}
	if (LeftBar != nullptr)
	{
		LeftBar->SetVisibility(BarVisibility);
	}
	if (RightBar != nullptr)
	{
		RightBar->SetVisibility(BarVisibility);
	}
}

void UASCrossHairUserWidget::SpreadBar()
{
	CurrentBarOffset = FMath::Clamp(CurrentBarOffset + OffsetPerShot, MinBarOffset, MaxBarOffset);
}

void UASCrossHairUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TopBar = Cast<UImage>(GetWidgetFromName(TEXT("TopBar")));
	BottomBar = Cast<UImage>(GetWidgetFromName(TEXT("BottomBar")));
	LeftBar = Cast<UImage>(GetWidgetFromName(TEXT("LeftBar")));
	RightBar = Cast<UImage>(GetWidgetFromName(TEXT("RightBar")));

	if (auto Char = GetOwningPlayerPawn<AASCharacter>())
	{
		if (UASInventoryComponent* InvenComp = Char->GetInventoryComponent())
		{
			InvenComp->OnChangedSelectedWeapon.AddUObject(this, &UASCrossHairUserWidget::OnChangeSelectedWeapon);

			OnChangeSelectedWeapon(nullptr, InvenComp->GetSelectedWeapon());
		}

		Char->OnPlayShootMontage.AddUObject(this, &UASCrossHairUserWidget::SpreadBar);
	}
}

void UASCrossHairUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	CurrentBarOffset = FMath::FInterpConstantTo(CurrentBarOffset, MinBarOffset, InDeltaTime, RecoverySpeed);

	if (TopBar != nullptr)
	{
		TopBar->SetRenderTranslation(FVector2D(0.0f, -CurrentBarOffset));
	}
	if (BottomBar != nullptr)
	{
		BottomBar->SetRenderTranslation(FVector2D(0.0f, CurrentBarOffset));
	}
	if (LeftBar != nullptr)
	{
		LeftBar->SetRenderTranslation(FVector2D(-CurrentBarOffset, 0.0f));
	}
	if (RightBar != nullptr)
	{
		RightBar->SetRenderTranslation(FVector2D(CurrentBarOffset, 0.0f));
	}
}
