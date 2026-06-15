// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameHUDWidget.h"
#include"Components/ProgressBar.h"
#include "Inventory/InventoryComponent.h"
#include"Components/TextBlock.h"
#include "Character/Components/SprintComponent.h"
#include "Character/Components/AttributeComponent.h"

void UGameHUDWidget::InitializeWidget(UAttributeComponent* NewAttributeComp,USprintComponent*NewSprintComponent,UInventoryComponent*NewInventoryComp) 
{
	UE_LOG(LogTemp, Warning, TEXT("HUD诊断：InitializeWidget 被调用。Attribute=%s, Sprint=%s, Inventory=%s"),
		NewAttributeComp ? *NewAttributeComp->GetName() : TEXT("None"),
		NewSprintComponent ? *NewSprintComponent->GetName() : TEXT("None"),
		NewInventoryComp ? *NewInventoryComp->GetName() : TEXT("None"));

	UE_LOG(LogTemp, Warning, TEXT("HUD诊断：BindWidget 状态 HealthBar=%s, HealthText=%s, StaminaBar=%s, StaminaText=%s, Hotbar=%s"),
		HealthProgressBar ? TEXT("OK") : TEXT("None"),
		HealthText ? TEXT("OK") : TEXT("None"),
		StaminaProgressBar ? TEXT("OK") : TEXT("None"),
		StaminaText ? TEXT("OK") : TEXT("None"),
		HotbarWidget ? TEXT("OK") : TEXT("None"));

	if (NewAttributeComp) 
	{
		AttributeCompRef = NewAttributeComp;

		//绑定委托
		NewAttributeComp->OnHealthChanged.AddDynamic(this, &UGameHUDWidget::OnHealthUpdate);


		float Current = NewAttributeComp->GetCurrentHealthPercent() * NewAttributeComp->MaxHealth;
		OnHealthUpdate(Current, NewAttributeComp->MaxHealth);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HUD诊断：AttributeComponent 为空，血条不会更新。"));
	}

	if (NewSprintComponent) 
	{
		NewSprintComponent->OnStaminaChanged.AddDynamic(this, &UGameHUDWidget::OnStaminaUpdate);
		float CurrentStamina = NewSprintComponent->GetCurrentStaminaPercent() * NewSprintComponent->MaxStamina;

		OnStaminaUpdate(CurrentStamina, NewSprintComponent->MaxStamina);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HUD诊断：SprintComponent 为空，体力条不会更新。"));
	}

	if (HotbarWidget && AttributeCompRef.IsValid()) 
	{
		HotbarWidget->InitializeHotbar(NewInventoryComp);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HUD诊断：快捷栏未初始化。HotbarWidget=%s, AttributeRefValid=%s"),
			HotbarWidget ? TEXT("OK") : TEXT("None"),
			AttributeCompRef.IsValid() ? TEXT("true") : TEXT("false"));
	}
}

void UGameHUDWidget::OnHealthUpdate(float CurrentHealth, float MaxHealth) 
{
	if (HealthProgressBar) 
	{
		HealthProgressBar->SetPercent(MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0);
	}

	if (HealthText) 
	{
		FString HealthStr = FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth);
		HealthText->SetText(FText::FromString(HealthStr));
	}
}

void UGameHUDWidget::OnStaminaUpdate(float CurrentStamina, float MaxStamina) 
{
	if (StaminaProgressBar) 
	{
		StaminaProgressBar->SetPercent(MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0);
	}

	if (StaminaText) 
	{
		FString StaminaStr = FString::Printf(TEXT("%.0f / %.0f"), CurrentStamina, MaxStamina);
		StaminaText->SetText(FText::FromString(StaminaStr));
	}
}
