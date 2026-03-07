#include "EscapeCombatComponent.h"
#include "CharacterAnimData.h"      // 必须引用，不然看不懂 FActionDefinition
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"  // 引用 Montage 类


UEscapeCombatComponent::UEscapeCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 战斗组件通常不需要每帧 Tick，省性能
}

void UEscapeCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    // 可以在这里检查一下有没有漏填 DataAsset，给个警告
    if (!CharacterAnimData)
    {
        UE_LOG(LogTemp, Error, TEXT("杂鱼主人！你的 %s 忘了在蓝图里配置 CharacterAnimData！动作没法播啦！"), *GetName());
    }
    
}

void UEscapeCombatComponent::TryPlayActionByTag(FGameplayTag ActionTag)
{
    // 1. 安全检查
    if (!CharacterAnimData) return;
    ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar) return;

    // 2. 去 DataAsset 里查表
    // ActionMap 是我们在 DataAsset 里定义的那个 TMap
    const FActionDefinition* ActionDef = CharacterAnimData->ActionMap.Find(ActionTag);

    // 3. 如果找到了配置
    if (ActionDef)
    {
        // [进阶预留位]: 这里以后可以判断 体力够不够？是否在冷却中？
        

        // 4. 加载资源 (同步加载)
        // 因为我们在 DataAsset 里用的是 TSoftObjectPtr，所以必须 LoadSynchronous() 才能变成真正的 UAnimMontage*
        UAnimMontage* MontageToPlay = ActionDef->Montage.LoadSynchronous();

        // 5. 播放动画！
        if (MontageToPlay)
        {
            // PlayAnimMontage 会返回动画总时长，如果返回 0 说明播放失败
            float Duration = OwnerChar->PlayAnimMontage(MontageToPlay, ActionDef->PlayRate);

            if (Duration > 0.f)
            {
                // [调试信息] 方便你看到到底播了啥
                UE_LOG(LogTemp, Log, TEXT("成功播放动作: %s (Tag: %s)"), *MontageToPlay->GetName(), *ActionTag.ToString());

                // [进阶预留位]: 这里以后要加上 CurrentActiveTags.AddTag(ActionTag) 来锁状态
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("动作 Tag [%s] 找到了配置，但是 Montage 资源是空的！"), *ActionTag.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DataAsset 里找不到 Tag [%s] 对应的动作！是不是忘了配表？"), *ActionTag.ToString());
    }
}

ACharacter* UEscapeCombatComponent::GetOwnerCharacter() const
{
    return Cast<ACharacter>(GetOwner());
}