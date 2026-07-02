// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Dialogue/EscapeDialogueTypes.h"
#include "EscapeDialogueSaveGame.generated.h"

// 对话与任务系统的存档对象。
// 这里只保存需要跨地图、跨启动保留的数据；当前正在打开的 UI 或临时会话不应该放进来。
UCLASS()
class ESCAPEGAME_API UEscapeDialogueSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 存档结构版本。
	// 以后字段升级时可以用它做兼容处理。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	int32 SaveVersion = 1;

	// 保存时玩家所在地图名。
	// 读档时可用于判断是否需要切回对应地图。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FName LastMapName;

	// 所有任务的存档快照。
	// 读档后会还原到 QuestRuntimeMap。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	TArray<FQuestSaveRecord> SavedQuestStates;

	// 所有对话的存档快照。
	// 读档后会还原已读节点、已选选项和当前节点。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	TArray<FDialogueSaveRecord> SavedDialogueStates;

	// 全局旗标存档快照。
	// 用于恢复门、机关、Boss 状态、剧情分支等世界状态。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	TArray<FGlobalFlagSaveRecord> SavedGlobalFlags;

	// 世界事件存档快照。
	// 用于恢复一次性事件或可重复事件的触发次数。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	TArray<FWorldEventSaveRecord> SavedWorldEvents;

	// 保存发生的时间。
	// 主要用于存档列表显示和调试。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FDateTime SaveTimestamp;
};
