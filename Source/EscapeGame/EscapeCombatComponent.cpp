#include "EscapeCombatComponent.h"
#include "CharacterAnimData.h"      // 必须引用，不然看不懂 FCombatActionDefinition
#include "GameFramework/Character.h"
#include"HealthController/AttributeComponent.h"
#include"statemachine/StateMachineComponent.h"
#include "EscapeCombatType.h"
#include "WeaponDefinition.h"
#include "Components/MeshComponent.h"
#include "Interface/EscapeCombatAttacker.h"
#include "SprintComponent.h"
#include "EscapeGameplayTags.h"
#include "Animation/AnimMontage.h"  // 引用 Montage 类
#include "DrawDebugHelpers.h"


UEscapeCombatComponent::UEscapeCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 战斗组件通常不需要每帧 Tick，省性能

}
void UEscapeCombatComponent::BeginPlay()
{
    Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CombatWindow=0.55f;
        AttributeComp = OwnerCharacter->FindComponentByClass<UAttributeComponent>();
		SprintComp = OwnerCharacter->FindComponentByClass<USprintComponent>();
        StateMachineComp = OwnerCharacter->FindComponentByClass<UStateMachineComponent>();

            // 2. 缓存动画实例并绑定核心委托
        if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
        {
            CachedAnimInstance = Mesh->GetAnimInstance();
            if (CachedAnimInstance)
            {
                // 动态绑定动画结束事件，这是动作游戏状态机的核心防卡死机制！
                CachedAnimInstance->OnMontageEnded.AddDynamic(this, &UEscapeCombatComponent::HandleAttackMontageEnded);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("香子兰警告：主人！战斗组件没有挂载到 Character 身上哦！"));
        }
        if (!SprintComp)
        {
            UE_LOG(LogTemp, Error, TEXT("香子兰警告：主人！这个角色身上没有冲刺组件，闪避要报错啦！"));
        }
        if (!AttributeComp)
        {
            UE_LOG(LogTemp, Error, TEXT("香子兰警告：主人！这个角色身上没有HealthLifeBar组件，闪避要报错啦！"));
        }
    }
    // 可以在这里检查一下有没有漏填 DataAsset，给个警告
    if (!CharacterAnimData)
    {
        UE_LOG(LogTemp, Error, TEXT("杂鱼主人！你的 %s 忘了在蓝图里配置 CharacterAnimData！动作没法播啦！"), *GetName());
    }

}

void UEscapeCombatComponent::TryPlayActionByTag(FGameplayTag ActionTag)
{
    TryPlayActionByTagInternal(ActionTag);
}

bool UEscapeCombatComponent::TryPlayActionByTagInternal(FGameplayTag ActionTag)
{
    ACharacter* OwnerChar = GetOwnerCharacter();
    FString FailReason;
    if (!CanStartCombatAction(ActionTag, &FailReason))
    {
        UE_LOG(LogTemp, Warning, TEXT("无法启动战斗动作 [%s]：%s"), *ActionTag.ToString(), *FailReason);
        return false;
    }
    if (!CharacterAnimData)
    {
        UE_LOG(LogTemp, Warning, TEXT("无法执行动作 [%s]：CharacterAnimData 未配置！"), *ActionTag.ToString());
        return false;
    }

    if (!OwnerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("无法执行动作 [%s]：没有找到拥有者 Character！"), *ActionTag.ToString());
        return false;
    }

    // 2. 去 DataAsset 里查表
    // ActionMap 是我们在 DataAsset 里定义的那个 TMap
    const FCombatActionDefinition* ActionDef = CharacterAnimData->CombatActionMap.Find(ActionTag);

    // 3. 如果找到了配置
    if (!ActionDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("DataAsset 里找不到 Tag [%s] 对应的动作！是不是忘了配表？"), *ActionTag.ToString());
        return false;
    }

    // [进阶预留位]: 这里以后可以判断 体力够不够？是否在冷却中？
	float CurrentStamina = SprintComp ? SprintComp->GetCurrentStamina() : 0.f;

    if (CurrentStamina < 10.0f)
    {
		const float MaxStamina = SprintComp ? SprintComp->MaxStamina : 0.f;
		UE_LOG(LogTemp, Warning,
			TEXT("体力不足，无法执行动作 [%s]！SprintComp=%s, CurrentStamina=%.2f, MaxStamina=%.2f, bStaminaDrained=%s, bSprintRequested=%s, bIsActurallySprinting=%s"),
			*ActionTag.ToString(),
			*GetNameSafe(SprintComp),
			CurrentStamina,
			MaxStamina,
			(SprintComp && SprintComp->bStaminaDrained) ? TEXT("true") : TEXT("false"),
			(SprintComp && SprintComp->bSprintRequested) ? TEXT("true") : TEXT("false"),
			(SprintComp && SprintComp->bIsActurallySprinting) ? TEXT("true") : TEXT("false"));
        return false;
    }


    // 4. 加载资源 (同步加载)
    // 因为我们在 DataAsset 里用的是 TSoftObjectPtr，所以必须 LoadSynchronous() 才能变成真正的 UAnimMontage*
    UAnimMontage* MontageToPlay = ActionDef->Montage.LoadSynchronous();
    if (!MontageToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("动作 Tag [%s] 找到了配置，但是 Montage 资源是空的！"), *ActionTag.ToString());
        return false;
    }

    // 5. 播放动画！
    // PlayAnimMontage 会返回动画总时长，如果返回 0 说明播放失败
    const float Duration = OwnerChar->PlayAnimMontage(MontageToPlay, ActionDef->PlayRate);
    if (Duration <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("动作 Montage [%s] 播放失败！Tag: %s"), *MontageToPlay->GetName(), *ActionTag.ToString());
        return false;
    }

    // 只有 Montage 真正播放成功后，才提交当前动作状态。
    RuntimeState.BeginAction(ActionTag, MontageToPlay);
    StateMachineComp->SetState(ECharacterState::Attacking);

    UE_LOG(LogTemp, Log, TEXT("当前体力：%f"), SprintComp->GetCurrentStamina());
    UE_LOG(LogTemp, Log, TEXT("成功播放动作: %s (Tag: %s)"), *MontageToPlay->GetName(), *ActionTag.ToString());

    return true;
}

ACharacter* UEscapeCombatComponent::GetOwnerCharacter() const
{
    return Cast<ACharacter>(GetOwner());
}

void UEscapeCombatComponent::BroadcastComboChange(int32 NewCount)
{
    RuntimeState.ComboCount = NewCount;
    if (OnComboCountChanged.IsBound())
    {
        OnComboCountChanged.Broadcast(RuntimeState.ComboCount);
    }
}
/*
	以下两个函数是用于处理蓄力攻击的，蓝图 Enhanced Input 的 Started/Triggered 引脚调用 BeginOrUpdateChargedAttack，Completed/Canceled 引脚调用 ReleaseChargedAttack
*/
void UEscapeCombatComponent::BeginOrUpdateChargedAttack()
{
    if (TryPlayActionByTagInternal(EscapeGameplayTags::Action_Combat_Heavy_Charge))
    {
        RuntimeState.RemoveCombatTag(EscapeGameplayTags::Action_State_Attacking);
        RuntimeState.bHasSavedComboInput = false;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("蓄力动作播放失败，未进入蓄力状态。"));
    }
}
void UEscapeCombatComponent::ReleaseChargedAttack()
{
    // 【换锁】：检查身上是否有 蓄力中 的 Tag。如果没有，说明根本没在蓄力，无视松开按键的指令。
    if (!RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_Combat_Heavy_Charge))
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[EscapeCombat] ReleaseChargedAttack skipped: not in charge state."));
        return;
    }

    // 【状态切换】：撕掉蓄力的标签，贴上释放重击的标签！
    RuntimeState.RemoveCombatTag(EscapeGameplayTags::Action_Combat_Heavy_Charge);
    RuntimeState.AddActionTag(EscapeGameplayTags::Action_ChargedAttack_Release);

    ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar || !CharacterAnimData || !RuntimeState.CurrentActionTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] ReleaseChargedAttack failed: Owner=%s AnimData=%s ActionTagValid=%s"),
            *GetNameSafe(OwnerChar),
            *GetNameSafe(CharacterAnimData),
            RuntimeState.CurrentActionTag.IsValid() ? TEXT("true") : TEXT("false"));
        return;
    }

    UAnimInstance* AnimInstance = OwnerChar->GetMesh() ? OwnerChar->GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] ReleaseChargedAttack failed: AnimInstance is null."));
        return;
    }

    const FCombatActionDefinition* ActionDef = CharacterAnimData->CombatActionMap.Find(RuntimeState.CurrentActionTag);
    if (ActionDef)
    {
        UAnimMontage* CurrentMontage = ActionDef->Montage.Get();
        if (CurrentMontage && AnimInstance->Montage_IsPlaying(CurrentMontage))
        {
            // 跳转到 Montage 里名为 "Attack" 的 Section
            AnimInstance->Montage_JumpToSection(FName("Attack"), CurrentMontage);
            UE_LOG(LogTemp, Log, TEXT("香子兰汇报：主人松手了，蓄力结束，正在狠狠地斩向敌人！"));
        }
        else
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("[EscapeCombat] ReleaseChargedAttack: montage not playing or invalid. ActionTag=%s"),
                *RuntimeState.CurrentActionTag.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] ReleaseChargedAttack failed: no ActionDef for ActionTag=%s"),
            *RuntimeState.CurrentActionTag.ToString());
    }
}
void UEscapeCombatComponent::CheckCombo()
{
    // 【换锁】：用 Tag 替换 Enum！如果没有 Attacking 标签，说明被打断了，直接拦截！
    if (!RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_State_Attacking))
    {
        UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：检测到幽灵 Notify 或未处于攻击状态，已成功拦截连击判定！"));
        return;
    }

    if (!CharacterAnimData || !RuntimeState.CurrentActionTag.IsValid())
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[EscapeCombat] CheckCombo skipped: AnimData=%s ActionTagValid=%s"),
            *GetNameSafe(CharacterAnimData),
            RuntimeState.CurrentActionTag.IsValid() ? TEXT("true") : TEXT("false"));
        return;
    }
    const FCombatActionDefinition* ActionDef = CharacterAnimData->CombatActionMap.Find(RuntimeState.CurrentActionTag);
    if (!ActionDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] CheckCombo failed: no ActionDef for ActionTag=%s"),
            *RuntimeState.CurrentActionTag.ToString());
        return;
    }

    if (RuntimeState.CombatBufferedInput == ECombatBufferedInput::Light)
    {
        RuntimeState.CombatBufferedInput = ECombatBufferedInput::None;
        CommitNextLightCombo();
        return;
    }

    if (RuntimeState.bHasSavedComboInput)
    {
        RuntimeState.bHasSavedComboInput = false;

        FGameplayTag NextTag = ActionDef->NextComboTag;
        if (NextTag.IsValid() && CharacterAnimData->CombatActionMap.Contains(NextTag))
        {
            if (TryPlayActionByTagInternal(NextTag))
            {
                RuntimeState.ComboCount++;
                BroadcastComboChange(RuntimeState.ComboCount);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("下一段连击播放失败，等待当前 Montage 正常结束。"));
            }
        }
        else
        {
            // 已经是最后一段了，重置连击计数
            RuntimeState.ComboCount = 0;
            BroadcastComboChange(RuntimeState.ComboCount);

            // 注意：不要在这里直接 RemoveTag(Attacking)！
            // 让 HandleAttackMontageEnded 在动画彻底播完后去清理，这样收刀动作才有防打断保护！
        }
    }
}

void UEscapeCombatComponent::BeginComboWindow()
{
    bComboWindowActive = true;
    BeginCombatWindow(ECombatWindowType::Combo);

    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ComboWindow] Begin ActionTag=%s SavedInput=%s ComboCount=%d"),
        *RuntimeState.CurrentActionTag.ToString(),
        RuntimeState.bHasSavedComboInput ? TEXT("true") : TEXT("false"),
        RuntimeState.ComboCount);
}

void UEscapeCombatComponent::TickComboWindow()
{
    if (!bComboWindowActive)
    {
        return;
    }

    TryConsumeBufferedInput();
}
void UEscapeCombatComponent::EndComboWindow()
{
    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ComboWindow] End ActionTag=%s SavedInput=%s ComboCount=%d"),
        *RuntimeState.CurrentActionTag.ToString(),
        RuntimeState.bHasSavedComboInput ? TEXT("true") : TEXT("false"),
        RuntimeState.ComboCount);

    bComboWindowActive = false;
    EndCombatWindow(ECombatWindowType::Combo);
}

void UEscapeCombatComponent::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != RuntimeState.CurrentPlayingMontage)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[EscapeCombat] HandleAttackMontageEnded ignored: montage mismatch. Interrupted=%s"),
            bInterrupted ? TEXT("true") : TEXT("false"));
        return;
    }
    RuntimeState.ResetAction();
    if (!StateMachineComp || !OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("缺失了状态机或者是角色"));
        return;
    }
    const ECharacterState CurrentState = StateMachineComp->GetCurrentState();
    if (CurrentState==ECharacterState::Dead || CurrentState==ECharacterState::Stunned)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[EscapeCombat] Montage ended while state=%s, combo cleanup only."),
            *UEnum::GetValueAsString(CurrentState));
        RuntimeState.RemoveCombatTag(EscapeGameplayTags::Action_State_Attacking);
        BroadcastComboChange(RuntimeState.ComboCount);
        return;
    }
    const bool bIsMoving = OwnerCharacter->GetVelocity().SizeSquared2D()>10.f;
    StateMachineComp->SetState(bIsMoving ? ECharacterState::Moving : ECharacterState::Idle);

    RuntimeState.RemoveCombatTag(EscapeGameplayTags::Action_State_Attacking);
    BroadcastComboChange(RuntimeState.ComboCount);
}
void UEscapeCombatComponent::DoAttackTrace(FName DamageSourceBone)
{
    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] DoAttackTrace called. Bone=%s ActionTag=%s"),
        *DamageSourceBone.ToString(),
        *RuntimeState.CurrentActionTag.ToString());

	ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar || !CharacterAnimData)
    {
		UE_LOG(LogTemp, Warning, TEXT("DoAttackTrace 失败：没有找到拥有者角色或者 CharacterAnimData！"));
        return;
    }
    USkeletalMeshComponent* Mesh = OwnerChar->GetMesh();
	const FCombatActionDefinition* ActionDef = CharacterAnimData->CombatActionMap.Find(RuntimeState.CurrentActionTag);
    if (!Mesh || !ActionDef)
    {
		UE_LOG(LogTemp, Warning, TEXT("DoAttackTrace 失败：没有找到 Mesh 组件或者当前动作定义！"));
        return;
    }
    float TraceRadius = ActionDef->TraceRadius;
    float BaseDamage = ActionDef->BaseDamage;
    FGameplayTag DamageTypeTag = EscapeGameplayTags::Data_Damage_Physical;

    FVector TraceStart;
    FVector TraceEnd;

    const bool bUseWeaponTrace =
        EquippedWeaponDef &&
        IsValid(EquippedWeaponMesh) &&
        EquippedWeaponMesh->DoesSocketExist(EquippedWeaponDef->TraceStartSocketName) &&
        EquippedWeaponMesh->DoesSocketExist(EquippedWeaponDef->TraceEndSocketName);

    if (bUseWeaponTrace)
    {
        TraceStart = EquippedWeaponMesh->GetSocketLocation(EquippedWeaponDef->TraceStartSocketName);
        TraceEnd = EquippedWeaponMesh->GetSocketLocation(EquippedWeaponDef->TraceEndSocketName);
        TraceRadius = EquippedWeaponDef->TraceRadius;
        BaseDamage = EquippedWeaponDef->BaseDamage;

        if (EquippedWeaponDef->DamageTypeTag.IsValid())
        {
            DamageTypeTag = EquippedWeaponDef->DamageTypeTag;
        }
    }
    else
    {
        TraceStart = Mesh->GetSocketLocation(DamageSourceBone);
        TraceEnd = TraceStart + OwnerChar->GetActorForwardVector() * ActionDef->TraceDistance;
    }

    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] TraceStart=%s TraceEnd=%s Radius=%.2f BaseDamage=%.2f FinalDamage=%.2f"),
        *TraceStart.ToString(),
        *TraceEnd.ToString(),
        TraceRadius,
        BaseDamage,
        BaseDamage * ActionDef->DamageMultiplier);

    if (GetWorld())
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Cyan, false, 2.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), TraceStart, TraceRadius, 12, FColor::Yellow, false, 2.0f, 0, 1.2f);
        DrawDebugSphere(GetWorld(), TraceEnd, TraceRadius, 12, FColor::Red, false, 2.0f, 0, 1.2f);
    }

    FCollisionShape SphereShape = FCollisionShape::MakeSphere(TraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerChar); // 不要碰到自己

	// 3. 只检测 Pawn 和 WorldDynamic 两种类型的物体，效率更高
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	// 4. 执行 SweepMultiByObjectType，获取所有被击中的对象
    TArray<FHitResult> OutHits;
    bool bHit = GetWorld()->SweepMultiByObjectType(
        OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, SphereShape, QueryParams
    );

	// 5. 遍历所有被击中的对象，应用伤害和击退效果
    if (bHit)
    {
        TSet<AActor*, DefaultKeyFuncs<AActor*>, TInlineSetAllocator<8>> ProcessedActors;

        for (const FHitResult& Hit : OutHits)
        {
			AActor* HitActor = Hit.GetActor();
            if (IsValid(HitActor) && !ProcessedActors.Contains(HitActor))
            {
                ProcessedActors.Add(HitActor);

                UAttributeComponent* HitAttribute = HitActor->FindComponentByClass<UAttributeComponent>();
                if (HitAttribute)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] Hit=%s HP Before=%.2f/%.2f"),
                        *GetNameSafe(HitActor),
                        HitAttribute->CurrentHealth,
                        HitAttribute->MaxHealth);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] Hit=%s has no AttributeComponent"),
                        *GetNameSafe(HitActor));
                }

                if (!HitActor->Implements<UEscapeCombatDamageable>())
                {
                    UE_LOG(LogTemp, Warning, TEXT("未实现可被攻击的接口！"));
                    continue;
                }

                const TObjectKey<AActor> HitKey(HitActor);
                if (RuntimeState.HitActorsThisAction.Contains(HitKey))
                {
                    continue;
                }

                RuntimeState.HitActorsThisAction.Add(HitKey);



				//1,触发攻击命中事件，传递伤害信息
                FVector Impulse = (Hit.ImpactNormal * -ActionDef->KnockbackImpulse) + (FVector::UpVector * ActionDef->LaunchImpulse);
				const float FinalDamage = BaseDamage * ActionDef->DamageMultiplier;

                FCombatDamageContext DamageContext;
                DamageContext.DamageValue = FinalDamage;
                DamageContext.InstigatorActor = OwnerChar;
                DamageContext.TargetActor = HitActor;
                DamageContext.HitLocation = Hit.ImpactPoint;
                DamageContext.HitImpulse = Impulse;
                DamageContext.ActionTag = RuntimeState.CurrentActionTag;
                DamageContext.DamageTypeTag = DamageTypeTag;
                DamageContext.HitResult = Hit;

                const FCombatDamageResult DamageResult =
                    IEscapeCombatDamageable::Execute_ApplyDamage(HitActor, DamageContext);

                if (HitAttribute)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] Hit=%s HP After=%.2f/%.2f Result Applied=%s Damage=%.2f Killed=%s"),
                        *GetNameSafe(HitActor),
                        HitAttribute->CurrentHealth,
                        HitAttribute->MaxHealth,
                        DamageResult.bApplied ? TEXT("true") : TEXT("false"),
                        DamageResult.ActualDamage,
                        DamageResult.bKilled ? TEXT("true") : TEXT("false"));
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] Result Applied=%s Damage=%.2f Killed=%s"),
                        DamageResult.bApplied ? TEXT("true") : TEXT("false"),
                        DamageResult.ActualDamage,
                        DamageResult.bKilled ? TEXT("true") : TEXT("false"));
                }

                if (DamageResult.bApplied)
                {
                    if (OwnerChar->Implements<UEscapeCombatAttacker>())
                    {
                        IEscapeCombatAttacker::Execute_NotifyHitConfirmed(
                            OwnerChar,
                            HitActor,
                            Hit
                        );
                    }

                    FAttackHitPayload Payload;
                    Payload.DamageCauser = HitActor;
                    Payload.DamageLocation = Hit.ImpactPoint;
                    Payload.DamageImpulse = Impulse;

                    if (OnAttackHit.IsBound())
                    {
                        OnAttackHit.Broadcast(Payload);
                        UE_LOG(LogTemp, Log, TEXT("武器命中目标并造成伤害，已发送 OnAttackHit 广播"));
                    }
                }




            }
        }

    }

    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DoAttackTrace 失败,没打到喵。"));
    }
}
void UEscapeCombatComponent::RequestLightAttack()
{
    // 如果已经在轻击了，只缓存输入，等待 CheckCombo 消耗。
    if (RuntimeState.ActiveTags.HasTagExact(EscapeGameplayTags::Action_State_Attacking))
    {
        RuntimeState.bHasSavedComboInput = true;
        UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：动作中检测到输入，锁存开启！"));
        return;
    }

    const FGameplayTag FirstAttackTag = EscapeGameplayTags::Action_Combat_Light_1;
    if (TryPlayActionByTagInternal(FirstAttackTag))
    {
        RuntimeState.AddCombatTag(EscapeGameplayTags::Action_State_Attacking);
        UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：贴上了攻击Tag，成功打出第一段轻击！"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("不好，现在并没有进入攻击状态，请检查DataAsset是否配置正确"));
    }
}


// ==========================================
// 核心逻辑实现：
// ==========================================
void UEscapeCombatComponent::Input_AttackStarted()
{
    // 【核心机密】：按下的瞬间，我们什么攻击都不做！
    // 而是定一个 0.55 秒的闹钟。如果 0.55 秒后主人还没松手，闹钟就会自动触发“重击蓄力”！
    GetWorld()->GetTimerManager().SetTimer(
        RuntimeState.InputBufferTimer,
        this,
        &UEscapeCombatComponent::HandleAttackHoldThresholdReached,
        CombatWindow, // 这个时间就是你的“长按判定阈值”，可以随便微调
        false
    );
    UE_LOG(LogTemp, Log, TEXT("香子兰汇报：检测到按下！已开启 0.55秒的输入缓冲闹钟..."));
}


void UEscapeCombatComponent::Input_AttackCompleted()
{
    // 玩家松开左键了！我们来看看闹钟的情况：

    // 【情况 1：闹钟还在滴答滴答走！】
    // 说明从按下到松手，连 0.55 秒都没到。这绝对是一个快速的【轻击 (Tap)】！
    if (GetWorld()->GetTimerManager().IsTimerActive(RuntimeState.InputBufferTimer))
    {
        // 杂鱼！想骗我出重击？没门！掐死准备触发重击的闹钟！
        GetWorld()->GetTimerManager().ClearTimer(RuntimeState.InputBufferTimer);
        BufferCombatInput(ECombatBufferedInput::Light);
        // 立刻判定为轻击请求！(这里会自然走进你之前写好的查 Tag 第一段攻击，或 bHasSavedComboInput 的连击逻辑)
        TryConsumeBufferedInput();
        UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：判定为【轻击】，闹钟已掐死！"));
    }
    // 【情况 2：闹钟已经不在走了】
    // 说明 0.25 秒前，闹钟已经响过了（已经自动执行了 BeginOrUpdateChargedAttack）。
    // 此时玩家松手，说明这是【蓄力结束，狠狠释放重击】！
    else if (RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_Combat_Heavy_Charge))
    {
        ReleaseChargedAttack();
        UE_LOG(LogTemp, Warning, TEXT("香子兰汇报：判定为【蓄力松手】，重击斩出！"));
    }
}

bool UEscapeCombatComponent::CanStartCombatAction(FGameplayTag ActionTag, FString* OutFailReason) const
{
    ACharacter *OwnerChar = GetOwnerCharacter();
    if (!ActionTag.IsValid() ||!OwnerChar || !CharacterAnimData)
    {
        if (OutFailReason)*OutFailReason=TEXT("ActionTag/角色/动作数据非法");
        return false;
    }
    const FCombatActionDefinition* ActionDef =
    CharacterAnimData->CombatActionMap.Find(ActionTag);
    if (!ActionDef)
    {
        if (OutFailReason)*OutFailReason=TEXT("AnimData中没有这个 tag");
         return false;
    }
    if (!StateMachineComp)
    {
        if (OutFailReason)*OutFailReason = TEXT("未找到状态机");
        return false;
    }
    ECharacterState Cur_state=StateMachineComp->GetCurrentState();

    if (Cur_state==ECharacterState::Dead || Cur_state==ECharacterState::Stunned)
    {
        if (OutFailReason)*OutFailReason = TEXT("处于死亡状态或者是迟缓状态");
        return false;
    }

    if (!SprintComp)
    {
        if (OutFailReason)*OutFailReason = TEXT("冲刺组件未配置");
        return false;

    }

    float CurrentStamina=SprintComp->GetCurrentStamina();
    if(CurrentStamina<10.f)
    {
        if (OutFailReason)*OutFailReason = TEXT("体力不足");
        return false;
    }

    // 9. 蓄力中或重击释放中，不允许启动新动作
    if (RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_Combat_Heavy_Charge))
    {
        if (OutFailReason)
        {
            *OutFailReason = TEXT("当前正在蓄力，不能启动新的战斗动作");
        }
        return false;
    }

    if (RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_ChargedAttack_Release))
    {
        if (OutFailReason)
        {
            *OutFailReason = TEXT("当前正在释放重击，不能启动新的战斗动作");
        }
        return false;
    }

    // 10. 攻击中只允许合法下一段连击
    if (RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_State_Attacking))
    {
        if (!CharacterAnimData || !RuntimeState.CurrentActionTag.IsValid())
        {
            if (OutFailReason)
            {
                *OutFailReason = TEXT("当前处于攻击状态，但当前动作 Tag 无效");
            }
            return false;
        }

        const FCombatActionDefinition* CurrentActionDef =
            CharacterAnimData->CombatActionMap.Find(RuntimeState.CurrentActionTag);

        if (!CurrentActionDef)
        {
            if (OutFailReason)
            {
                *OutFailReason = FString::Printf(
                    TEXT("当前动作 [%s] 没有找到动作配置"),
                    *RuntimeState.CurrentActionTag.ToString()
                );
            }
            return false;
        }

        const bool bIsHeavyCancelRequest =
            ActionTag == EscapeGameplayTags::Action_Combat_Heavy_Charge &&
            HasCombatWindow(ECombatWindowType::HeavyCancel);

        if (bIsHeavyCancelRequest)
        {
            return true;
        }

        if (CurrentActionDef->NextComboTag != ActionTag)
        {
            if (OutFailReason)
            {
                *OutFailReason = FString::Printf(
                    TEXT("攻击中只能启动下一段连击。当前动作: [%s], 期望下一段: [%s], 请求动作: [%s]"),
                    *RuntimeState.CurrentActionTag.ToString(),
                    *CurrentActionDef->NextComboTag.ToString(),
                    *ActionTag.ToString()
                );
            }
            return false;
        }
    }
    return true;
}


// EscapeCombatComponent.cpp
float UEscapeCombatComponent::GetCurrentActionBaseDamage() const
{
    if (EquippedWeaponDef)
    {
        return EquippedWeaponDef->BaseDamage;
    }

    if (!CharacterAnimData || !RuntimeState.CurrentActionTag.IsValid())
    {
        return 0.f;
    }

    const FCombatActionDefinition* ActionDef =
        CharacterAnimData->CombatActionMap.Find(RuntimeState.CurrentActionTag);

    return ActionDef ? ActionDef->BaseDamage : 0.f;
}
void UEscapeCombatComponent::SetEquippedWeapon(UWeaponDefinition* WeaponDef, UMeshComponent* WeaponMesh)
{
    EquippedWeaponDef = WeaponDef;
    EquippedWeaponMesh = WeaponMesh;
    RuntimeState.HitActorsThisAction.Reset();
}

void UEscapeCombatComponent::ClearEquippedWeapon()
{
    EquippedWeaponDef = nullptr;
    EquippedWeaponMesh = nullptr;
    RuntimeState.HitActorsThisAction.Reset();
}


void UEscapeCombatComponent::BeginAttackTrace(FName DamageSourceBone)
{
    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ANS] BeginAttackTrace Enter Bone=%s ActionTag=%s Owner=%s WeaponDef=%s WeaponMesh=%s"),
        *DamageSourceBone.ToString(),
        *RuntimeState.CurrentActionTag.ToString(),
        *GetNameSafe(GetOwner()),
        *GetNameSafe(EquippedWeaponDef),
        *GetNameSafe(EquippedWeaponMesh));

    AttackTraceInst.bAttackTraceActive=true;
    AttackTraceInst.bHasLastTracePoints=false;
    AttackTraceInst.ActiveDamageSourceBone=DamageSourceBone;

    RuntimeState.HitActorsThisAction.Reset();

    FVector CurrentStart;
    FVector CurrentEnd;


    if (GetCurrentTracePoints(DamageSourceBone, CurrentStart, CurrentEnd))
    {
        AttackTraceInst.LastTraceStart=CurrentStart;
        AttackTraceInst.LastTraceEnd=CurrentEnd;
        AttackTraceInst.bHasLastTracePoints=true;
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ANS] BeginAttackTrace InitPoints Start=%s End=%s"),
            *CurrentStart.ToString(),
            *CurrentEnd.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ANS] BeginAttackTrace failed: GetCurrentTracePoints returned false."));
    }
}


void UEscapeCombatComponent::EndAttackTrace()
{
    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ANS] EndAttackTrace Active=%s HasLast=%s HitActors=%d ActionTag=%s"),
        AttackTraceInst.bAttackTraceActive ? TEXT("true") : TEXT("false"),
        AttackTraceInst.bHasLastTracePoints ? TEXT("true") : TEXT("false"),
        RuntimeState.HitActorsThisAction.Num(),
        *RuntimeState.CurrentActionTag.ToString());

    AttackTraceInst.bAttackTraceActive = false;
    AttackTraceInst.bHasLastTracePoints = false;
    AttackTraceInst.ActiveDamageSourceBone = NAME_None;
}

void UEscapeCombatComponent::TickAttackTrace(FName DamageSourceBone)
{
    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ANS] TickAttackTrace Enter InputBone=%s ActiveBone=%s Active=%s HasLast=%s"),
        *DamageSourceBone.ToString(),
        *AttackTraceInst.ActiveDamageSourceBone.ToString(),
        AttackTraceInst.bAttackTraceActive ? TEXT("true") : TEXT("false"),
        AttackTraceInst.bHasLastTracePoints ? TEXT("true") : TEXT("false"));

    if (!AttackTraceInst.bAttackTraceActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ANS] TickAttackTrace skipped: trace is not active."));
        return;
    }

    const FName TraceBone =
        DamageSourceBone.IsNone()? AttackTraceInst.ActiveDamageSourceBone : DamageSourceBone;


    FVector CurrentStart;
    FVector CurrentEnd;
    //如果没有
    if (!GetCurrentTracePoints(TraceBone, CurrentStart, CurrentEnd))
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ANS] TickAttackTrace failed: cannot get trace points. TraceBone=%s"), *TraceBone.ToString());
        return;
    }

    if (AttackTraceInst.bHasLastTracePoints)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ANS] TickAttackTrace Points LastStart=%s LastEnd=%s CurrentStart=%s CurrentEnd=%s"),
            *AttackTraceInst.LastTraceStart.ToString(),
            *AttackTraceInst.LastTraceEnd.ToString(),
            *CurrentStart.ToString(),
            *CurrentEnd.ToString());

        SweepAttackSegment(AttackTraceInst.LastTraceStart, CurrentStart);
        SweepAttackSegment(AttackTraceInst.LastTraceEnd, CurrentEnd);
        SweepAttackSegment(CurrentStart, CurrentEnd);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][ANS] TickAttackTrace has no last points yet; storing current points only."));
    }

    AttackTraceInst.LastTraceStart = CurrentStart;
    AttackTraceInst.LastTraceEnd = CurrentEnd;
    AttackTraceInst.bHasLastTracePoints = true;


}

bool UEscapeCombatComponent::GetCurrentTracePoints(FName DamageSourceBone, FVector& OutStart, FVector& OutEnd) const
{
    ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][TracePoints] failed: owner character is null."));
        return false;
    }

    if (EquippedWeaponDef && IsValid(EquippedWeaponMesh)
            && EquippedWeaponMesh -> DoesSocketExist(EquippedWeaponDef -> TraceStartSocketName)
            && EquippedWeaponMesh -> DoesSocketExist(EquippedWeaponDef -> TraceEndSocketName))
    {
        OutStart = EquippedWeaponMesh->GetSocketLocation(EquippedWeaponDef->TraceStartSocketName);
        OutEnd = EquippedWeaponMesh->GetSocketLocation(EquippedWeaponDef->TraceEndSocketName);
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][TracePoints] Using weapon sockets StartSocket=%s EndSocket=%s Start=%s End=%s"),
            *EquippedWeaponDef->TraceStartSocketName.ToString(),
            *EquippedWeaponDef->TraceEndSocketName.ToString(),
            *OutStart.ToString(),
            *OutEnd.ToString());
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][TracePoints] Weapon socket path unavailable. WeaponDef=%s WeaponMesh=%s"),
            *GetNameSafe(EquippedWeaponDef),
            *GetNameSafe(EquippedWeaponMesh));
    }

    USkeletalMeshComponent* Mesh = OwnerChar->GetMesh();
    if (!Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][TracePoints] failed: skeletal mesh is null."));
        return false;
    }

    if (!DamageSourceBone.IsNone() && Mesh->DoesSocketExist(DamageSourceBone))
    {
        OutStart = Mesh->GetSocketLocation(DamageSourceBone);
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][TracePoints] Using character socket/bone=%s Start=%s"),
            *DamageSourceBone.ToString(),
            *OutStart.ToString());
    }
    else if (Mesh->DoesSocketExist(TEXT("右手首")))
    {
        OutStart = Mesh->GetSocketLocation(TEXT("右手首"));
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][TracePoints] DamageSourceBone invalid. Fallback to 右手首 Start=%s InputBone=%s"),
            *OutStart.ToString(),
            *DamageSourceBone.ToString());
    }
    else
    {
        OutStart = OwnerChar->GetActorLocation();
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][TracePoints] no valid socket. Fallback to actor location Start=%s InputBone=%s"),
            *OutStart.ToString(),
            *DamageSourceBone.ToString());
    }

    const FCombatActionDefinition* ActionDef =
      CharacterAnimData ? CharacterAnimData->CombatActionMap.Find(RuntimeState.CurrentActionTag) : nullptr;

    const float FallbackDistance = ActionDef ? ActionDef->TraceDistance : 80.f;
    OutEnd = OutStart + OwnerChar->GetActorForwardVector() * FallbackDistance;

    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][TracePoints] Character fallback End=%s Distance=%.2f ActionDef=%s"),
        *OutEnd.ToString(),
        FallbackDistance,
        ActionDef ? TEXT("valid") : TEXT("null"));

    return true;
}

void UEscapeCombatComponent::SweepAttackSegment(const FVector& TraceStart, const FVector& TraceEnd)
{
    ACharacter* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar || !CharacterAnimData || !GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][Sweep] skipped: Owner=%s AnimData=%s World=%s"),
            *GetNameSafe(OwnerChar),
            *GetNameSafe(CharacterAnimData),
            GetWorld() ? TEXT("valid") : TEXT("null"));
        return;
    }

    const FCombatActionDefinition* ActionDef =
      CharacterAnimData->CombatActionMap.Find(RuntimeState.CurrentActionTag);

    if (!ActionDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][Sweep] skipped: no ActionDef for ActionTag=%s"),
            *RuntimeState.CurrentActionTag.ToString());
        return;
    }

    float TraceRadius = ActionDef->TraceRadius;
    float BaseDamage = ActionDef->BaseDamage;
    FGameplayTag DamageTypeTag = EscapeGameplayTags::Data_Damage_Physical;

    if (EquippedWeaponDef)
    {
        TraceRadius = EquippedWeaponDef->TraceRadius;
        BaseDamage = EquippedWeaponDef->BaseDamage;

        if (EquippedWeaponDef->DamageTypeTag.IsValid())
        {
            DamageTypeTag = EquippedWeaponDef->DamageTypeTag;
        }
    }

    FCollisionShape SphereShape = FCollisionShape::MakeSphere(TraceRadius);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerChar);

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

    TArray<FHitResult> OutHits;

    const bool bHit = GetWorld()->SweepMultiByObjectType(
        OutHits,
        TraceStart,
        TraceEnd,
        FQuat::Identity,
        ObjectParams,
        SphereShape,
        QueryParams
    );

    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][Sweep] Start=%s End=%s Radius=%.2f Hit=%s HitCount=%d BaseDamage=%.2f FinalDamage=%.2f"),
        *TraceStart.ToString(),
        *TraceEnd.ToString(),
        TraceRadius,
        bHit ? TEXT("true") : TEXT("false"),
        OutHits.Num(),
        BaseDamage,
        BaseDamage * ActionDef->DamageMultiplier);

    DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Cyan, false, 1.0f, 0, 1.5f);
    DrawDebugSphere(GetWorld(), TraceStart, TraceRadius, 8, FColor::Yellow, false, 1.0f);
    DrawDebugSphere(GetWorld(), TraceEnd, TraceRadius, 8, FColor::Red, false, 1.0f);

    if (!bHit)
    {
        return;
    }

    for (const FHitResult& Hit : OutHits)
    {
        ProcessAttackHit(Hit, *ActionDef, BaseDamage, DamageTypeTag);
    }
    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][Sweep] segment finished."));
}

void UEscapeCombatComponent::ProcessAttackHit(const FHitResult& Hit, const FCombatActionDefinition& ActionDef, float BaseDamage, FGameplayTag DamageTypeTag)
{
    ACharacter* OwnerChar = GetOwnerCharacter();
    AActor* HitActor = Hit.GetActor();

    if (!OwnerChar || !IsValid(HitActor) || HitActor == OwnerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][Hit] skipped: Owner=%s HitActor=%s SelfHit=%s"),
            *GetNameSafe(OwnerChar),
            *GetNameSafe(HitActor),
            HitActor == OwnerChar ? TEXT("true") : TEXT("false"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][Hit] Candidate Actor=%s Impact=%s"),
        *GetNameSafe(HitActor),
        *Hit.ImpactPoint.ToString());

    const TObjectKey<AActor> HitKey(HitActor);

    if (RuntimeState.HitActorsThisAction.Contains(HitKey))
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat][Hit] skipped duplicate actor in same action: %s"), *GetNameSafe(HitActor));
        return;
    }

    if (!HitActor->Implements<UEscapeCombatDamageable>())
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] Hit actor has no damageable interface: %s"), *GetNameSafe(HitActor));
        return;
    }

    RuntimeState.HitActorsThisAction.Add(HitKey);

    UAttributeComponent* HitAttribute = HitActor->FindComponentByClass<UAttributeComponent>();
    if (HitAttribute)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] Hit=%s HP Before=%.2f/%.2f"),
            *GetNameSafe(HitActor),
            HitAttribute->CurrentHealth,
            HitAttribute->MaxHealth);
    }

    const FVector Impulse =
        (Hit.ImpactNormal * -ActionDef.KnockbackImpulse) +
        (FVector::UpVector * ActionDef.LaunchImpulse);

    const float FinalDamage = BaseDamage * ActionDef.DamageMultiplier;

    FCombatDamageContext DamageContext;
    DamageContext.DamageValue = FinalDamage;
    DamageContext.InstigatorActor = OwnerChar;
    DamageContext.TargetActor = HitActor;
    DamageContext.HitLocation = Hit.ImpactPoint;
    DamageContext.HitImpulse = Impulse;
    DamageContext.ActionTag = RuntimeState.CurrentActionTag;
    DamageContext.DamageTypeTag = DamageTypeTag;
    DamageContext.HitResult = Hit;

    const FCombatDamageResult DamageResult =
        IEscapeCombatDamageable::Execute_ApplyDamage(HitActor, DamageContext);

    if (HitAttribute)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EscapeCombat] Hit=%s HP After=%.2f/%.2f Applied=%s Damage=%.2f Killed=%s"),
            *GetNameSafe(HitActor),
            HitAttribute->CurrentHealth,
            HitAttribute->MaxHealth,
            DamageResult.bApplied ? TEXT("true") : TEXT("false"),
            DamageResult.ActualDamage,
            DamageResult.bKilled ? TEXT("true") : TEXT("false"));
    }

    if (!DamageResult.bApplied)
    {
        return;
    }

    if (OwnerChar->Implements<UEscapeCombatAttacker>())
    {
        IEscapeCombatAttacker::Execute_NotifyHitConfirmed(
            OwnerChar,
            HitActor,
            Hit
        );
    }

    FAttackHitPayload Payload;
    Payload.DamageCauser = HitActor;
    Payload.DamageLocation = Hit.ImpactPoint;
    Payload.DamageImpulse = Impulse;

    if (OnAttackHit.IsBound())
    {
        OnAttackHit.Broadcast(Payload);
    }
}


/*
重构区域

- BufferInput 负责“记住玩家想干什么”
- TryConsumeBufferedInput 负责“判断现在能不能干”

这里到底有哪些东西在变化？
哪些东西其实是同一种模式？
 */

void UEscapeCombatComponent::BufferCombatInput(ECombatBufferedInput Input)
{
    RuntimeState.CombatBufferedInput = Input;

    UE_LOG(LogTemp, VeryVerbose, TEXT("[EscapeCombat][InputBuffer] BufferedInput=%s"),
        *UEnum::GetValueAsString(Input));
}

void UEscapeCombatComponent::ClearBufferedInput()
{
    RuntimeState.CombatBufferedInput = ECombatBufferedInput::None;
}

//用于给timer使用的回调函数
void UEscapeCombatComponent::HandleAttackHoldThresholdReached()
{
    BufferCombatInput(ECombatBufferedInput::Heavy);
    TryConsumeBufferedInput();
}

void UEscapeCombatComponent::TryConsumeBufferedInput()
{
    if (RuntimeState.CombatBufferedInput == ECombatBufferedInput::None)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[EscapeCombat][InputBuffer] No combat input to consume."));
        return;
    }
    //若不在攻击中
    //1 若第一次攻击
    const bool bIsAttacking = RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_State_Attacking);
    if (!bIsAttacking)
    {
        if (RuntimeState.CombatBufferedInput == ECombatBufferedInput::Light)
        {
            RuntimeState.CombatBufferedInput = ECombatBufferedInput::None;
            RequestLightAttack();
            return;
        }

        if (RuntimeState.CombatBufferedInput == ECombatBufferedInput::Heavy)
        {
            RuntimeState.CombatBufferedInput = ECombatBufferedInput::None;
            BeginOrUpdateChargedAttack();
            return;
        }
    }
    //如果在攻击中 判断是否可以处于切换攻击形态的区间
    //建议逐步把 `CheckCombo()` 的核心逻辑迁移到 `CommitNextLightCombo()`。

    if (RuntimeState.CombatBufferedInput == ECombatBufferedInput::Light &&
        HasCombatWindow(ECombatWindowType::Combo))
    {
        RuntimeState.CombatBufferedInput = ECombatBufferedInput::None;
        CommitNextLightCombo();
        return;
    }
    if (RuntimeState.CombatBufferedInput == ECombatBufferedInput::Heavy &&
            HasCombatWindow(ECombatWindowType::HeavyCancel))
    {
        RuntimeState.CombatBufferedInput = ECombatBufferedInput::None;
        BeginOrUpdateChargedAttack();
        return;
    }
}

bool UEscapeCombatComponent::CommitNextLightCombo()
{
    if (!RuntimeState.ActiveTags.HasTag(EscapeGameplayTags::Action_State_Attacking))
    {
        return false;
    }

    if (!CharacterAnimData || !RuntimeState.CurrentActionTag.IsValid())
    {
        return false;
    }

    const FCombatActionDefinition* CurrentActionDef =
        CharacterAnimData->CombatActionMap.Find(RuntimeState.CurrentActionTag);

    if (!CurrentActionDef || !CurrentActionDef->NextComboTag.IsValid())
    {
        RuntimeState.ComboCount = 0;
        BroadcastComboChange(RuntimeState.ComboCount);
        return false;
    }

    if (!CharacterAnimData->CombatActionMap.Contains(CurrentActionDef->NextComboTag))
    {
        return false;
    }

    if (!TryPlayActionByTagInternal(CurrentActionDef->NextComboTag))
    {
        return false;
    }

    RuntimeState.ComboCount++;
    BroadcastComboChange(RuntimeState.ComboCount);
    return true;
}

void UEscapeCombatComponent::BeginCombatWindow(ECombatWindowType WindowType)
{
    RuntimeState.ActiveWindows.Add(WindowType);
    TryConsumeBufferedInput();
}


void UEscapeCombatComponent::EndCombatWindow(ECombatWindowType WindowType)
{
    RuntimeState.ActiveWindows.Remove(WindowType);
}
