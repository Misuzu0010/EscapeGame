// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapeGameplayTags.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"

FEscapeGameplayTags FEscapeGameplayTags::GameplayTags;

void FEscapeGameplayTags::InitializeNativeTags()
{
	GameplayTags.AddAllTags(UGameplayTagsManager::Get());
}

void FEscapeGameplayTags::AddAllTags(UGameplayTagsManager& Manager) 
{
	// 辅助宏，为了让你少打点字 (懒人必备)
#define ADD_TAG(TagName, TagString, TagComment) \
        TagName = Manager.AddNativeGameplayTag(FName(TagString), FString(TEXT(TagComment)));

    // ---------------------------------------------------------
    // 输入
    ADD_TAG(Input_Action_LightAttack, "Input.Action.LightAttack", "输入: 轻攻击");
    ADD_TAG(Input_Action_HeavyAttack, "Input.Action.HeavyAttack", "输入: 重攻击");
    ADD_TAG(Input_Action_Dodge, "Input.Action.Dodge", "输入: 闪避");
    ADD_TAG(Input_Action_Jump, "Input.Action.Jump", "输入: 跳跃");

    // ---------------------------------------------------------
    // 动作状态 (Action States)
    ADD_TAG(Action_State_Attacking, "Action.State.Attacking", "状态: 正在攻击中");
    ADD_TAG(Action_State_Dodging, "Action.State.Dodging", "状态: 正在闪避中");
    ADD_TAG(Action_State_Dead, "Action.State.Dead", "状态: 死亡");

    // 具体招式
    ADD_TAG(Action_Combat_Light_1, "Action.Combat.Light.1", "招式: 轻攻击第一段");
    ADD_TAG(Action_Combat_Light_2, "Action.Combat.Light.2", "招式: 轻攻击第二段");
    ADD_TAG(Action_Combat_Light_3, "Action.Combat.Light.3", "招式: 轻攻击第三段");
    ADD_TAG(Action_Combat_Heavy_Charge, "Action.Combat.Heavy.Charge", "招式: 蓄力重击");

    // ---------------------------------------------------------
    // 状态 (Status)
    ADD_TAG(State_Status_Invincible, "State.Status.Invincible", "Buff: 无敌帧");
    ADD_TAG(State_Status_HyperArmor, "State.Status.HyperArmor", "Buff: 霸体(不可打断)");

    ADD_TAG(State_Debuff_Stun, "State.Debuff.Stun", "Debuff: 眩晕/硬直");
    ADD_TAG(State_Debuff_Knockdown, "State.Debuff.Knockdown", "Debuff: 倒地");

    // ---------------------------------------------------------
    // 事件 (Events)
    ADD_TAG(Event_Montage_ComboWindow_Open, "Event.Montage.ComboWindow.Open", "通知: 连击窗口开启");
    ADD_TAG(Event_Montage_ComboWindow_Close, "Event.Montage.ComboWindow.Close", "通知: 连击窗口关闭");

    // ---------------------------------------------------------
    // 数据 (Data)
    ADD_TAG(Data_HitDirection_Front, "Data.HitDirection.Front", "判定: 前方受击");
    ADD_TAG(Data_HitDirection_Back, "Data.HitDirection.Back", "判定: 后方受击");

#undef ADD_TAG
}