#include "EscapeGameplayTags.h"

namespace EscapeGameplayTags
{
    // 参数1：变量名  |  参数2：在编辑器里显示的实际 Tag 名字 (层级)
    UE_DEFINE_GAMEPLAY_TAG(Input_Action_LightAttack, "Input.Action.LightAttack");
    UE_DEFINE_GAMEPLAY_TAG(Input_Action_HeavyAttack, "Input.Action.HeavyAttack");
    UE_DEFINE_GAMEPLAY_TAG(Input_Action_Dodge, "Input.Action.Dodge");
    UE_DEFINE_GAMEPLAY_TAG(Input_Action_Jump, "Input.Action.Jump");
    UE_DEFINE_GAMEPLAY_TAG(Input_Action_Skill_1, "Input.Action.Skill.1");
    UE_DEFINE_GAMEPLAY_TAG(Input_Action_Skill_2, "Input.Action.Skill.2");
    UE_DEFINE_GAMEPLAY_TAG(Input_Action_UseItem, "Input.Action.UseItem");

    UE_DEFINE_GAMEPLAY_TAG(Action_State_Attacking, "Action.State.Attacking");
    UE_DEFINE_GAMEPLAY_TAG(Action_State_Dodging, "Action.State.Dodging");
    UE_DEFINE_GAMEPLAY_TAG(Action_State_Dead, "Action.State.Dead");

    UE_DEFINE_GAMEPLAY_TAG(Action_Combat_Light_1, "Action.Combat.Light.1");
    UE_DEFINE_GAMEPLAY_TAG(Action_Combat_Light_2, "Action.Combat.Light.2");
    UE_DEFINE_GAMEPLAY_TAG(Action_Combat_Light_3, "Action.Combat.Light.3");
    UE_DEFINE_GAMEPLAY_TAG(Action_Combat_Light_4, "Action.Combat.Light.4");
    UE_DEFINE_GAMEPLAY_TAG(Action_ChargedAttack_Release, "Action.ChargedAttack.Release");
    UE_DEFINE_GAMEPLAY_TAG(Action_Combat_Heavy_Charge, "Action.Combat.Heavy.Charge");
    UE_DEFINE_GAMEPLAY_TAG(Action_Combat_AirAttack, "Action.Combat.AirAttack");

    UE_DEFINE_GAMEPLAY_TAG(State_Movement_Grounded, "State.Movement.Grounded");
    UE_DEFINE_GAMEPLAY_TAG(State_Movement_Airborne, "State.Movement.Airborne");
    UE_DEFINE_GAMEPLAY_TAG(State_Status_Invincible, "State.Status.Invincible");
    UE_DEFINE_GAMEPLAY_TAG(State_Status_HyperArmor, "State.Status.HyperArmor");
    UE_DEFINE_GAMEPLAY_TAG(State_Status_Blocking, "State.Status.Blocking");
    UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Stun, "State.Debuff.Stun");
    UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Knockdown, "State.Debuff.Knockdown");
    UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Burn, "State.Debuff.Burn");

    UE_DEFINE_GAMEPLAY_TAG(Event_Montage_ComboWindow_Open, "Event.Montage.ComboWindow.Open");
    UE_DEFINE_GAMEPLAY_TAG(Event_Montage_ComboWindow_Close, "Event.Montage.ComboWindow.Close");
    UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Hit, "Event.Combat.Hit");

    UE_DEFINE_GAMEPLAY_TAG(Data_Damage_Physical, "Data.Damage.Physical");
    UE_DEFINE_GAMEPLAY_TAG(Data_Damage_Fire, "Data.Damage.Fire");
    UE_DEFINE_GAMEPLAY_TAG(Data_HitDirection_Front, "Data.HitDirection.Front");
    UE_DEFINE_GAMEPLAY_TAG(Data_HitDirection_Back, "Data.HitDirection.Back");

    UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_1, "Cooldown.Skill.1");
    UE_DEFINE_GAMEPLAY_TAG(Cooldown_Dodge, "Cooldown.Dodge");
}