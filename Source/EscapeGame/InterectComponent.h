// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InterectComponent.generated.h"
struct FInputActionValue;
class UInventoryComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPEGAME_API UInterectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInterectComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	// 这是一个可以在蓝图里设置的 Widget 类（比如 WBP_Inventory）
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> InventoryMenuClass;

	// --- 2. 补上：运行时的缓存变量 ---

	// 存那个打开的 UI 实例
	UPROPERTY()
	UUserWidget* InventoryMenuInstance;

	// 存兄弟组件（背包组件），因为 UI 初始化需要它
	UPROPERTY()
	class UInventoryComponent* InventoryComp;


	// 4. 函数声明
	UFUNCTION()
	void ToggleInventory();

	// 2. 声明回调函数 (按下 E 时执行的逻辑)
	UFUNCTION()
	void OnInteract(const FInputActionValue& Value);


		
};
