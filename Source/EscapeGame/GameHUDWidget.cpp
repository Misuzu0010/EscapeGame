// Fill out your copyright notice in the Description page of Project Settings.


#include "GameHUDWidget.h"
#include"Components/ProgressBar.h"
#include"InventoryComponent.h"
#include"Components/TextBlock.h"
#include"SprintComponent.h"
#include"HealthController/AttributeComponent.h"

void UGameHUDWidget::InitializeWidget(UAttributeComponent* NewAttributeComp,USprintComponent*NewSprintComponent,UInventoryComponent*NewInventoryComp) 
{
	if (NewAttributeComp) 
	{
		AttributeCompRef = NewAttributeComp;

		//绑定委托
		NewAttributeComp->OnHealthChanged.AddDynamic(this, &UGameHUDWidget::OnHealthUpdate);


		float Current = NewAttributeComp->GetCurrentHealthPercent() * NewAttributeComp->MaxHealth;
		OnHealthUpdate(Current, NewAttributeComp->MaxHealth);
	}
	if (NewSprintComponent) 
	{
		NewSprintComponent->OnStaminaChanged.AddDynamic(this, &UGameHUDWidget::OnStaminaUpdate);
		float CurrentStamina = NewSprintComponent->GetCurrentStaminaPercent() * NewSprintComponent->MaxStamina;

		OnStaminaUpdate(CurrentStamina, NewSprintComponent->MaxStamina);
	}

	if (HotbarWidget && AttributeCompRef.IsValid()) 
	{
		HotbarWidget->InitializeHotbar(NewInventoryComp);
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