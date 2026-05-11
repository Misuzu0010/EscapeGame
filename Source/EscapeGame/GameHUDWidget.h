// Fill out your copyright notice in the Description page of Project Settings.
//各种UI的改变逻辑集成
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include"InventoryHotbarWidget.h"
#include "GameHUDWidget.generated.h"

/**
 * 
 */
class UProgressBar;
class UTextBlock;
class UAttributeComponent;
class UInventoryComponent;
class USprintComponent;

UCLASS()
class ESCAPEGAME_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable,Category="UI")
	void InitializeWidget(UAttributeComponent* NewAttributeComp,USprintComponent*NewSprintComponent,UInventoryComponent*NewInventoryComp);




protected:
	//变量名称必须要和UI设计器中的控件名称一致
	UPROPERTY(meta=(BindWidget))
	UProgressBar* HealthProgressBar;

	//同理，绑定血量文本，文件名也必须一致
	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;

	//绑定体力值文本
	UPROPERTY(meta=(BindWidget))
	UProgressBar* StaminaProgressBar;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* StaminaText;

	UPROPERTY(meta=(BindWidget))
	UInventoryHotbarWidget* HotbarWidget;

private:
	//回调函数，收到广播时更新血条
	UFUNCTION()
	void OnHealthUpdate(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void OnStaminaUpdate(float CurrentStamina, float MaxStamina);

	TWeakObjectPtr<UAttributeComponent>AttributeCompRef;
	
};
