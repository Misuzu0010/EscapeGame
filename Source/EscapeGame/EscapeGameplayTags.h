// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UGameplayTagsManager;
/**
 * 
 */

struct ESCAPEGAME_API FEscapeGameplayTags
{
public:
    static const FEscapeGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeTags();
    // === 声明标签变量 ===

  // ==========================================
    // 1. 输入标签 (Input) - 玩家想要干什么
    // ==========================================
    FGameplayTag Input_Action_LightAttack;
    FGameplayTag Input_Action_HeavyAttack;
    FGameplayTag Input_Action_Dodge;
    FGameplayTag Input_Action_Jump;
    FGameplayTag Input_Action_Skill_1;
    FGameplayTag Input_Action_Skill_2;
    FGameplayTag Input_Action_UseItem;

    // ==========================================
    // 2. 动作标签 (Actions) - 正在执行什么
    // ==========================================
    FGameplayTag Action_State_Attacking;    // 只要在攻击中，就有这个标签
    FGameplayTag Action_State_Dodging;      // 正在闪避
    FGameplayTag Action_State_Dead;         // 死了

    // 具体的招式 (用于 DataAsset 查表)
    FGameplayTag Action_Combat_Light_1;
    FGameplayTag Action_Combat_Light_2;
    FGameplayTag Action_Combat_Light_3;
    FGameplayTag Action_Combat_Heavy_Charge;
    FGameplayTag Action_Combat_AirAttack;

    // ==========================================
    // 3. 角色状态 (States) - Buff/Debuff/环境
    // ==========================================
    // 移动状态
    FGameplayTag State_Movement_Grounded;
    FGameplayTag State_Movement_Airborne;

    // 正面状态
    FGameplayTag State_Status_Invincible;   // 无敌 (翻滚帧)
    FGameplayTag State_Status_HyperArmor;   // 霸体 (不会被打断)
    FGameplayTag State_Status_Blocking;     // 格挡中

    // 负面状态 (Debuff)
    FGameplayTag State_Debuff_Stun;         // 眩晕 (硬直)
    FGameplayTag State_Debuff_Knockdown;    // 击倒
    FGameplayTag State_Debuff_Burn;         // 燃烧

    // ==========================================
    // 4. 事件与属性 (Events & Data)
    // ==========================================
    // 动画通知事件 (用于 AnimNotify)
    FGameplayTag Event_Montage_ComboWindow_Open;
    FGameplayTag Event_Montage_ComboWindow_Close;
    FGameplayTag Event_Combat_Hit;          // 造成了伤害

    // 属性 (用于计算伤害)
    FGameplayTag Data_Damage_Physical;
    FGameplayTag Data_Damage_Fire;
    FGameplayTag Data_HitDirection_Front;
    FGameplayTag Data_HitDirection_Back;

    // 冷却时间 (Cooldowns)
    FGameplayTag Cooldown_Skill_1;
    FGameplayTag Cooldown_Dodge;

    


protected:
    // 注册函数
    void AddAllTags(UGameplayTagsManager& Manager);


private:
    static FEscapeGameplayTags GameplayTags;
};