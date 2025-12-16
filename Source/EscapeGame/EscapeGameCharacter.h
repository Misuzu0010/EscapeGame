// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "statemachine/StateMachineComponent.h"  // 包含枚举和组件类
#include "SprintComponent.h"                      // 包含冲刺组件
#include "InventoryComponent.h"
#include "Logging/LogMacros.h"
#include"HealthController/AttributeComponent.h"
#include "EscapeGameCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AEscapeGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CrouchAction;

	// 1. 输入动作：按 I 键
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InventoryAction;

public:
	// ... 其他输入变量 ...

	/** 声明冲刺的输入动作插槽 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SprintAction;

	

public:

	/** Constructor */
	AEscapeGameCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	


public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd(); 

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void StartCrouch();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void StopCrouch();
	
	// 注意：这里只是声明“我有个背包”，背包里具体有啥，这里不管
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStateMachineComponent* StateMachineComp;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Sprinting")
	USprintComponent* SprintComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAttributeComponent* AttributeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryComp;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf < class UUserWidget > HUDWidgetClass;

	// 2. UI 配置：我们要创建哪个 Widget？(填 WBP_InventoryMenu)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> InventoryMenuClass;

	// 3. 实例缓存：保存打开的窗口，防止重复创建
	UPROPERTY()
	class UUserWidget* InventoryMenuInstance;

	// 4. 函数声明
	UFUNCTION()
	void ToggleInventory();

	



public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


};

