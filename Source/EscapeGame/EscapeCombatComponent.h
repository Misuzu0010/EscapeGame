// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include"GameplayTagContainer.h"
#include "EscapeCombatComponent.generated.h"
class UCharacterAnimData;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPEGAME_API UEscapeCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEscapeCombatComponent();


    // ==========================================
    // 配置项 (Configuration)
    // ==========================================
    // 核心数据库：请在蓝图里把你的 DataAsset 拖进去！
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCharacterAnimData> CharacterAnimData;

    // ==========================================
    // 接口 (Interface)
    // ==========================================
    // 尝试执行某个动作 (查表播放)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TryPlayActionByTag(FGameplayTag ActionTag);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    class ACharacter* GetOwnerCharacter() const;

		
};
