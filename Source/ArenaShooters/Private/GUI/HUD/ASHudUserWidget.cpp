// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASHudUserWidget.h"
#include "GUI/HUD/ASInventoryStatusUserWidget.h"
#include "Components/ProgressBar.h"

void UASHudUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InventoryStatusWidget = Cast<UASInventoryStatusUserWidget>(GetWidgetFromName(TEXT("InventoryStatusWidget")));
	HealthProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("HealthProgressBar")));
}