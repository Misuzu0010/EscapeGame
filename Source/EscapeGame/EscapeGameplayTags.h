#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h" // 香子兰认证：这个头文件是神！

// 绝对命令：这里必须用 namespace，不能用 struct 喵！
namespace EscapeGameplayTags
{
    // ==========================================
    // 1. 输入标签 (Input)
    // ==========================================
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_LightAttack);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_HeavyAttack);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Dodge);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Jump);
    //1技能
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Skill_1);
    //2技能
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Skill_2);
    //用药水
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_UseItem);

    // ==========================================
    // 2. 动作标签 (Actions)
    // ==========================================
    
    //攻击
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_State_Attacking);
    //闪避
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_State_Dodging);
    //死亡
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_State_Dead);
    //4轻击
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Combat_Light_1);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Combat_Light_2);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Combat_Light_3);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Combat_Light_4);
    //释放
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_ChargedAttack_Release);
    //重击蓄力
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Combat_Heavy_Charge);
    //跳劈
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Combat_AirAttack);

    // ==========================================
    // 3. 角色状态 (States)
    // ==========================================
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Movement_Grounded);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Movement_Airborne);
    //无敌帧
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Status_Invincible);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Status_HyperArmor);
    //被控
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Status_Blocking);
    //减速
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Stun);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Knockdown);
    //燃烧
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Burn);

    // ==========================================
    // 4. 事件与属性 (Events & Data)
    // ==========================================
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_ComboWindow_Open);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_ComboWindow_Close);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Hit);

    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Physical);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Fire);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_HitDirection_Front);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_HitDirection_Back);

    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Skill_1);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Dodge);
}