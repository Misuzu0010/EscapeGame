// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/Components/StateMachineComponent.h"  // 包含枚举和组件类
#include "Character/Components/SprintComponent.h"                      // 包含冲刺组件
#include "Inventory/InventoryComponent.h"
#include "Logging/LogMacros.h"
#include "Interaction/InterectComponent.h"
#include "Character/Components/AttributeComponent.h"
#include "Interfaces/EscapeCombatAttacker.h"
#include "Interfaces/EscapeCombatDamageable.h"
#include "EscapeGameCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UEscapeCombatComponent;
class UWindSimulationComponent;
class UWeaponDefinition;
class UMeshComponent;
class UStaticMeshComponent;
class UInputAction;
class UClothLODControllerComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AEscapeGameCharacter : public ACharacter, public IEscapeCombatDamageable, public IEscapeCombatAttacker
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/*JumpInput Action */
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

	// 下蹲输入动作 ctrl
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CrouchAction;

	// 1. 输入动作：按 I 键
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InventoryAction;

	// 2. 捡起物品/交互键：按 E 键
	UPROPERTY(EditAnywhere,Category="Input")
	UInputAction* InteractAction;

	// 切换视角
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ToggleCameraAction;

	UPROPERTY(EditAnywhere,  Category = "Input")
	UInputAction* AttackAction;

	// 使用物品
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UseItemAction;

	// 丢弃物品
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DropItemAction;

	// 装备物品
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* EquipItemAction;

	// 取消装备物品
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UnequipItemAction;

	//esc暂停游戏
	UPROPERTY(EditAnywhere,Category="Input")
	UInputAction* PauseAction;
	
	

public:
	// ... 其他输入变量 ...
	int32 CurrentSelectedSlotIndex = 0;

	/** 声明冲刺的输入动作插槽 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SprintAction;

	/** Constructor */
	AEscapeGameCharacter();	



	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void Input_UseItem(const FInputActionValue& Value);
	
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
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	bool EquipWeapon(UWeaponDefinition* WeaponDef);

	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void UnequipWeapon();
	
	UWeaponDefinition* GetEquippedWeaponDef() const
	{
		return CurrentWeaponDefinition;
	}

	UMeshComponent* GetEquippedWeaponMesh() const
	{
		return EquippedWeaponMesh;
	}
	
	virtual FCombatDamageResult ApplyDamage_Implementation(const FCombatDamageContext& DamageContext) override;
	
	virtual float GetBaseDamage_Implementation() const override;
	virtual int32 GetCurrentComboCount_Implementation() const override;
	virtual void NotifyHitConfirmed_Implementation(AActor* HitTarget, const FHitResult& HitResult) override;
	
	

	// 注意：这里只是声明“我有个背包”，背包里具体有啥，这里不管
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStateMachineComponent* StateMachineComp;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Sprinting")
	USprintComponent* SprintComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAttributeComponent* AttributeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Component")
	UInterectComponent* InteractComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	UEscapeCombatComponent* EscapeCombatComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UWindSimulationComponent> WindSimulationComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UClothLODControllerComponent> ClothLODComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf < class UUserWidget > HUDWidgetClass;


	UFUNCTION()
	void ToggleCameraMode();

	UPROPERTY()
	bool bIsFirstPerson = false;
	

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> EquippedWeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWeaponDefinition> DefaultWeaponDefinition;

	UPROPERTY(Transient)
	TObjectPtr<UWeaponDefinition> CurrentWeaponDefinition = nullptr;



};
