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

    // ==========================================
    // 5. 对话系统 (Dialogue)
    // ==========================================
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Gatekeeper_Intro);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Node_Gatekeeper_Root);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Node_Gatekeeper_KeyHint);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Node_Gatekeeper_OpenGate);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_Gatekeeper_AskKey);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_Gatekeeper_TurnInKey);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_Gatekeeper_Leave);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_NPC01_Intro);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Node_NPC01_Root);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Node_NPC01_Danger);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_NPC01_AskDanger);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_NPC01_Back);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_NPC01_Leave);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_NPC01_LeaveAfterHint);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_NPC01_QuestTest);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Node_NPC01_QuestOffer);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Node_NPC01_QuestAccepted);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Node_NPC01_QuestInProgress);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Node_NPC01_QuestComplete);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_NPC01_AcceptQuest);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_NPC01_DeclineQuest);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_NPC01_AskProgress);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Option_NPC01_TurnInQuest);

    // ==========================================
    // 6. 对话参与者与任务 (NPC / Quest)
    // ==========================================
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(NPC_Village_Gatekeeper);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(NPC_NPC01);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Gatekeeper);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Main_FindGateKey);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Objective_FindGateKey_CollectKey);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_NPC01_FindTestKey);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Objective_NPC01_CollectTestKey);

    // ==========================================
    // 7. 物品、旗标、遭遇战与奖励 (Item / Flag / Encounter / Reward)
    // ==========================================
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Key_MainGate);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Key_NPC01_TestKey);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flag_World_MainGateUnlocked);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flag_Dialogue_Gatekeeper_IntroSeen);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flag_Dialogue_NPC01_IntroSeen);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flag_Quest_NPC01_Accepted);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flag_Combat_GatekeeperIntroPlayed);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Encounter_Boss_Gatekeeper);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Puzzle_Courtyard_StatueOrder);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Reward_Item_GateKey);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Reward_Item_NPC01_TestKey);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Reward_Message_NPC01_Thanks);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Reward_Unlock_MainGate);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldEvent_MainGate_Opened);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unlock_Area_MainGate);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Player_MaxHealth);

    // ==========================================
    // 8. 人物表现预留 (Character Presentation)
    // ==========================================
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Expression_Neutral);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Expression_Happy);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Expression_Worried);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Gesture_Idle);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Gesture_Talk);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Gesture_Point);
    ESCAPEGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Focus_Player);
}
