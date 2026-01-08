#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.generated.h"

class UItemDefinition; // 指向道具逻辑/资源的DataAsset

UENUM(BlueprintType)
enum class EItemType : uint8
{
    // 工具类：用来解谜、触发机关
    Key        UMETA(DisplayName = "Key"),           // 钥匙
    Tool       UMETA(DisplayName = "Tool"),          // 一般工具（手电筒、撬棍等）

    // 消耗品：回血、增益、临时道具
    Consumable UMETA(DisplayName = "Consumable"),    // 药水、食物
    BuffItem   UMETA(DisplayName = "Buff Item"),     // 暂时增强能力（加速度、夜视等）

    // 装备类：武器/防具/护符
    Weapon     UMETA(DisplayName = "Weapon"),        // 武器（近战或远程）
    Armor      UMETA(DisplayName = "Armor"),         // 防具
    Accessory  UMETA(DisplayName = "Accessory"),    // 饰品/护符（增强属性）

    // 任务道具：剧情或解谜必需品
    QuestItem  UMETA(DisplayName = "Quest Item")     // 关键任务物品
};

USTRUCT(BlueprintType)
struct FItemText
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Item Text")
    FText Name;         // 道具名字

    UPROPERTY(EditAnywhere, Category = "Item Text")
    FText Description;  // 道具描述
};

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    // 道具类型
    UPROPERTY(EditAnywhere, Category = "Item Data")
    EItemType ItemType;

    // 道具唯一ID
    UPROPERTY(EditAnywhere, Category = "Item Data")
    FName ID;

    // 道具名字/描述
    UPROPERTY(EditAnywhere, Category = "Item Data")
    FItemText ItemText;

    // 道具图标
    UPROPERTY(EditAnywhere, Category = "Item Data")
    UTexture2D* Icon=nullptr;

    // 道具数量（默认1）
    UPROPERTY(EditAnywhere, Category = "Item Data")
    int32 DefaultCount = 1;

    // 道具逻辑/行为DataAsset
    UPROPERTY(EditAnywhere, Category = "Item Data")
    TObjectPtr<UItemDefinition> ItemLogic;

    // 回复多少血？
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Usage", meta = (EditCondition = "ItemType==EItemType::Consumable"))
    float RestoreHealthAmount=0;

    // 增加多少移动速度？
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Usage", meta = (EditCondition = "ItemType==EItemType::Consumable"))
    float SpeedBoostAmount=0;

    // 增加多少移动速度？
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Usage", meta = (EditCondition = "ItemType==EItemType::Consumable"))
    float DamageBoostAmount = 0;

    // 持续多长时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Usage", meta = (EditCondition = "ItemType==EItemType::Consumable"))
    float DamageBoostTime = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Item Representation")
    TObjectPtr<UStaticMesh> WorldMesh;

};
// --- 必须补上这个：背包里的堆叠数据 ---
USTRUCT(BlueprintType)
struct FItemStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemData ItemData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count=0;
};