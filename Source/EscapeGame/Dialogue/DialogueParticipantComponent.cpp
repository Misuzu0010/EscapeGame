// Fill out your copyright notice in the Description page of Project Settings.

#include "Dialogue/DialogueParticipantComponent.h"

UDialogueParticipantComponent::UDialogueParticipantComponent()
{
	// 对话参与者组件只保存配置，不需要每帧更新。
	// 触发逻辑应由交互、重叠、AI 或 Subsystem 主动调用。
	PrimaryComponentTick.bCanEverTick = false;
}
