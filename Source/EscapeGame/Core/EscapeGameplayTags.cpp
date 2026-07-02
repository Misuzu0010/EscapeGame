#include "Core/EscapeGameplayTags.h"

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

    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Gatekeeper_Intro, "Dialogue.Gatekeeper.Intro");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Node_Gatekeeper_Root, "Dialogue.Node.Gatekeeper.Root");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Node_Gatekeeper_KeyHint, "Dialogue.Node.Gatekeeper.KeyHint");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Node_Gatekeeper_OpenGate, "Dialogue.Node.Gatekeeper.OpenGate");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_Gatekeeper_AskKey, "Dialogue.Option.Gatekeeper.AskKey");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_Gatekeeper_TurnInKey, "Dialogue.Option.Gatekeeper.TurnInKey");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_Gatekeeper_Leave, "Dialogue.Option.Gatekeeper.Leave");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_NPC01_Intro, "Dialogue.NPC01.Intro");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Node_NPC01_Root, "Dialogue.Node.NPC01.Root");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Node_NPC01_Danger, "Dialogue.Node.NPC01.Danger");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_NPC01_AskDanger, "Dialogue.Option.NPC01.AskDanger");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_NPC01_Back, "Dialogue.Option.NPC01.Back");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_NPC01_Leave, "Dialogue.Option.NPC01.Leave");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_NPC01_LeaveAfterHint, "Dialogue.Option.NPC01.LeaveAfterHint");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_NPC01_QuestTest, "Dialogue.NPC01.QuestTest");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Node_NPC01_QuestOffer, "Dialogue.Node.NPC01.QuestOffer");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Node_NPC01_QuestAccepted, "Dialogue.Node.NPC01.QuestAccepted");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Node_NPC01_QuestInProgress, "Dialogue.Node.NPC01.QuestInProgress");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Node_NPC01_QuestComplete, "Dialogue.Node.NPC01.QuestComplete");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_NPC01_AcceptQuest, "Dialogue.Option.NPC01.AcceptQuest");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_NPC01_DeclineQuest, "Dialogue.Option.NPC01.DeclineQuest");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_NPC01_AskProgress, "Dialogue.Option.NPC01.AskProgress");
    UE_DEFINE_GAMEPLAY_TAG(Dialogue_Option_NPC01_TurnInQuest, "Dialogue.Option.NPC01.TurnInQuest");

    UE_DEFINE_GAMEPLAY_TAG(NPC_Village_Gatekeeper, "NPC.Village.Gatekeeper");
    UE_DEFINE_GAMEPLAY_TAG(NPC_NPC01, "NPC.NPC01");
    UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Gatekeeper, "Enemy.Boss.Gatekeeper");
    UE_DEFINE_GAMEPLAY_TAG(Quest_Main_FindGateKey, "Quest.Main.FindGateKey");
    UE_DEFINE_GAMEPLAY_TAG(Quest_Objective_FindGateKey_CollectKey, "Quest.Objective.FindGateKey.CollectKey");
    UE_DEFINE_GAMEPLAY_TAG(Quest_NPC01_FindTestKey, "Quest.NPC01.FindTestKey");
    UE_DEFINE_GAMEPLAY_TAG(Quest_Objective_NPC01_CollectTestKey, "Quest.Objective.NPC01.CollectTestKey");

    UE_DEFINE_GAMEPLAY_TAG(Item_Key_MainGate, "Item.Key.MainGate");
    UE_DEFINE_GAMEPLAY_TAG(Item_Key_NPC01_TestKey, "Item.Key.NPC01.TestKey");
    UE_DEFINE_GAMEPLAY_TAG(Flag_World_MainGateUnlocked, "Flag.World.MainGateUnlocked");
    UE_DEFINE_GAMEPLAY_TAG(Flag_Dialogue_Gatekeeper_IntroSeen, "Flag.Dialogue.Gatekeeper.IntroSeen");
    UE_DEFINE_GAMEPLAY_TAG(Flag_Dialogue_NPC01_IntroSeen, "Flag.Dialogue.NPC01.IntroSeen");
    UE_DEFINE_GAMEPLAY_TAG(Flag_Quest_NPC01_Accepted, "Flag.Quest.NPC01.Accepted");
    UE_DEFINE_GAMEPLAY_TAG(Flag_Combat_GatekeeperIntroPlayed, "Flag.Combat.GatekeeperIntroPlayed");
    UE_DEFINE_GAMEPLAY_TAG(Encounter_Boss_Gatekeeper, "Encounter.Boss.Gatekeeper");
    UE_DEFINE_GAMEPLAY_TAG(Puzzle_Courtyard_StatueOrder, "Puzzle.Courtyard.StatueOrder");
    UE_DEFINE_GAMEPLAY_TAG(Reward_Item_GateKey, "Reward.Item.GateKey");
    UE_DEFINE_GAMEPLAY_TAG(Reward_Item_NPC01_TestKey, "Reward.Item.NPC01.TestKey");
    UE_DEFINE_GAMEPLAY_TAG(Reward_Message_NPC01_Thanks, "Reward.Message.NPC01.Thanks");
    UE_DEFINE_GAMEPLAY_TAG(Reward_Unlock_MainGate, "Reward.Unlock.MainGate");
    UE_DEFINE_GAMEPLAY_TAG(WorldEvent_MainGate_Opened, "WorldEvent.MainGate.Opened");
    UE_DEFINE_GAMEPLAY_TAG(Unlock_Area_MainGate, "Unlock.Area.MainGate");
    UE_DEFINE_GAMEPLAY_TAG(Attribute_Player_MaxHealth, "Attribute.Player.MaxHealth");

    UE_DEFINE_GAMEPLAY_TAG(Character_Expression_Neutral, "Character.Expression.Neutral");
    UE_DEFINE_GAMEPLAY_TAG(Character_Expression_Happy, "Character.Expression.Happy");
    UE_DEFINE_GAMEPLAY_TAG(Character_Expression_Worried, "Character.Expression.Worried");
    UE_DEFINE_GAMEPLAY_TAG(Character_Gesture_Idle, "Character.Gesture.Idle");
    UE_DEFINE_GAMEPLAY_TAG(Character_Gesture_Talk, "Character.Gesture.Talk");
    UE_DEFINE_GAMEPLAY_TAG(Character_Gesture_Point, "Character.Gesture.Point");
    UE_DEFINE_GAMEPLAY_TAG(Character_Focus_Player, "Character.Focus.Player");
}
