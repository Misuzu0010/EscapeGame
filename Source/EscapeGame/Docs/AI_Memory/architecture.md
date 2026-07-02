# EscapeGame 项目架构记忆文档

自动更新于：2026-06-27 11:31  
下次更新时请修改此处时间戳。

分析范围：本次增量读取了 `Source/EscapeGame/Dialogue`、`Source/EscapeGame/Core/EscapeGameplayTags.*`、`Source/EscapeGame/Tests/DialogueQuestSubsystemTest.cpp`、`Docs/Plan/dialogue_folder_architecture_overview.md` 与 `Docs/Plan/dialogue_select_option_execution_plan.md`。旧的战斗、背包、交互、TA 与武器导入记忆继续保留；本次重点补入 Dialogue / Quest 系统骨架、Native GameplayTag、SelectOption 当前暂停点，以及新的开发日志目录约定。

## 1. 项目概览

项目名称：`EscapeGame`

项目配置名：`Config/DefaultGame.ini` 中仍显示 `ProjectName=Third Person Game Template`，但模块、uproject 与源码均使用 `EscapeGame`。

游戏类型：UE 5.7 第三人称逃脱/探索原型，包含第三人称/第一人称视角切换、背包拾取、钥匙开门、HUD、耐力冲刺、武器装备、近战轻击连段与蓄力重击。

核心玩法：

```text
玩家控制 AEscapeGameCharacter 移动、跳跃、下蹲、冲刺和切换视角。
UInterectComponent 负责交互扫描，按接口调用 APickupData 或 AInteractDoor。
APickupData 从 DT_ItemData 查询 FItemData，把物品写入 UInventoryComponent。
背包 UI、快捷栏和 Tooltip 监听 OnInventoryUpdated 刷新。
物品使用由 UItemDefinition::OnUse 多态分发到治疗、恢复体力、提升体力上限和加速 Buff。
武器由 UWeaponDefinition 描述 Mesh、挂载 Socket、Trace Socket、TraceRadius、BaseDamage 和 DamageTypeTag，AEscapeGameCharacter 在 BeginPlay 可装备 DefaultWeaponDefinition。
战斗由 UEscapeCombatComponent 通过 GameplayTag 查询 UCharacterAnimData，播放 Montage，管理轻击连段、蓄力重击、Legacy 单帧 Trace 与 ANS 连续攻击 Trace、伤害上下文与命中广播。
AAICharacter_1 是当前主源码中的基础敌人角色，接入 IEscapeCombatDamageable，受击后通过 UAttributeComponent 扣血并在死亡时停止 AI/移动、关闭碰撞、设置黑板死亡状态。
生命、耐力、角色状态、背包、命中反馈通过动态委托通知 UI 或其他系统。
Editor 侧 UWeaponImportToolLibrary + EUW_WeaponImporter 可从 OBJ 文件夹导入武器 Static Mesh、Diffuse/Lightmap 贴图，生成基础材质和 DA_Weapon_*。
动画侧通过 UCharacterAnimInstance 缓存移动、冲刺、布料 LOD 和风力数据，输出给 AnimBP / Kawaii Physics。
```

UE 版本：

```text
EscapeGame.uproject EngineAssociation:
  5.7
```

默认地图与 GameMode：

```text
DefaultEngine.ini:
  GameDefaultMap=/Game/SoStylized/Maps/CompleteVol1/Demonstration.Demonstration
  EditorStartupMap=/Game/SoStylized/Maps/CompleteVol1/Demonstration.Demonstration
  GlobalDefaultGameMode=/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode.BP_ThirdPersonGameMode_C
```

模块依赖：

```text
EscapeGame.Build.cs PublicDependencyModuleNames:
  Core
  CoreUObject
  Engine
  InputCore
  EnhancedInput
  AIModule
  StateTreeModule
  GameplayStateTreeModule
  UMG
  SlateCore
  Slate
  Niagara
  NiagaraCore
  NiagaraShader
  VectorVM
  GameplayTags

EscapeGame.Build.cs PrivateDependencyModuleNames:
  RenderCore
  RHI
  Projects

Editor-only private modules:
  NiagaraEditor
  UnrealEd
  AssetTools
  AssetRegistry
  ContentBrowser
```

启用插件：

```text
ModelingToolsEditorMode 仅 Editor 目标
StateTree
GameplayStateTree
```

当前说明：`Build.cs` 仍保留 `Variant_*` 示例目录的 `PublicIncludePaths`，但主架构记忆不把这些示例目录作为核心系统来源。`ComboAttackComponent/` 当前存在但为空目录。

## 2. 核心类架构图（文字版）

```text
ACharacter
  AEscapeGameCharacter
    玩家角色入口，聚合相机、状态机、冲刺、属性、背包、交互、战斗、风力、布料 LOD 和 EquippedWeaponMesh 组件。
    实现 IEscapeCombatDamageable::ApplyDamage_Implementation 和 IEscapeCombatAttacker，负责装备默认武器、扣血、死亡状态切换、命中冲量与命中确认日志。

  AAICharacter_1
    当前基础敌人 Character，创建 UAttributeComponent，使用 AAIController 自动占有，实现 IEscapeCombatDamageable；死亡时停止移动和 AI，关闭 Capsule 碰撞，写入 Blackboard 的 bIsdead 并清空 TargetActor，3 秒后销毁。

AGameModeBase
  AEscapeGameGameMode
    当前仅保留模板构造逻辑。

APlayerController
  AEscapeGamePlayerController
    添加 Enhanced Input Mapping Context，创建移动端触控 UI，创建/切换背包菜单，并切换输入模式与鼠标显示。

AActor
  APickupData
    世界拾取物，从 DataTable 按 ItemID 查 FItemData，写入拾取者背包；OnConstruction 可按表格 WorldMesh 同步模型。

  AInteractDoor
    通用交互门，检查背包内 RequireKeyID.ID，成功后触发蓝图事件 OnDoorOpen，可选择消耗钥匙。

UActorComponent
  UAttributeComponent
    生命值组件，维护 CurrentHealth/MaxHealth，ApplyHealthChange 后广播 OnHealthChanged。

  UStateMachineComponent
    角色状态机，维护 Idle/Moving/Attacking/Sprinting/Stunned/Dead，保护 Dead/Stunned 状态，并广播 OnStateChanged。

  USprintComponent
    冲刺与耐力组件，Tick 中计算真实冲刺条件、平滑 MaxWalkSpeed、消耗/恢复体力、处理速度 Buff，并广播 OnStaminaChanged。

  UInventoryComponent
    固定容量背包，维护 TArray<FItemStack>，负责添加、移除、使用、交换槽位，并广播 OnInventoryUpdated。

  UInterectComponent
    交互扫描组件，执行半径 150 的 Sphere Sweep，优先处理 UPickupInterface，再处理 UInteractableInterface。

  UEscapeCombatComponent
    近战战斗组件，用 FCombatRuntimeState 保存当前动作、ActiveTags、连击缓存、HitActorsThisAction 和输入 Timer；用 FAttackTraceInst 保存 ANS 连续 Trace 的上一帧点位；负责播放动作、连击、蓄力、武器/角色 Socket Trace、伤害上下文和命中广播。

  UClothLODControllerComponent
    布料 LOD 控制器，使用定时器按相机距离、可见性和 Mesh LOD 调整布料模拟质量，不使用 Tick。

  UWindSimulationComponent
    风力模拟组件，Tick 中合成基础全局风、移动风、Perlin/Sine 噪声风，输出给动画蓝图/Kawaii Physics。

UAnimInstance
  UCharacterAnimInstance
    缓存角色移动、冲刺、风力和布料 LOD 数据；NativeThreadSafeUpdateAnimation 计算 LocomotionAngle、PlayRate、PhysicsAlpha、HairRunBounceForce 和 RuntimeHairDamping。

UUserWidget
  UGameHUDWidget
    HUD 根控件，绑定生命/耐力委托，初始化快捷栏。

  UInventoryMenuWidget
    背包网格 UI，监听 OnInventoryUpdated 后生成 UInventorySlotWidget。

  UInventoryHotbarWidget
    快捷栏 UI，显示背包前 NumSlot 个槽位，当前默认 5 格。

  UInventorySlotWidget
    单个物品槽，显示图标/数量/Tooltip，支持点击使用与同背包拖拽交换。

  UItemToolTipWidget
    物品 Tooltip，显示 FItemData 的名称和描述。

UDragDropOperation
  UInventoryDragDropOperation
    背包拖拽载荷，记录 SourceSlotIndex 和 SourceComponent。

UDataAsset
  UCharacterAnimData
    动画/动作数据资产，用 GameplayTag 映射战斗动作和通用动作。

  UItemDefinition
    物品逻辑基类，定义堆叠、消耗、冷却和 OnUse。

    UWeaponDefinition
      武器定义资产，继承 UItemDefinition，描述 WeaponMesh、AttachSocketName、TraceStart/End、TraceRadius、BaseDamage 和 DamageTypeTag。

    UItemAction_Healing
      治疗道具，调用 UAttributeComponent::ApplyHealthChange。

    UItemAction_RestoreStamina
      回复当前体力，调用 USprintComponent::StaminaChange。

    UItemAction_EnhanceStamina
      提升最大体力，调用 USprintComponent::ApplyMaxChange。

    UItemAction_Boosting
      临时速度倍率 Buff，调用 USprintComponent::StartSpeedBuff。

UBlueprintFunctionLibrary
  UUTACurveToolLibrary
    TA 曲线烘焙工具，将数学模型采样写入 UCurveFloat。

  UWeaponImportToolLibrary
    Editor-only 武器导入工具，导入 OBJ/PNG，生成 Static Mesh、基础材质和 UWeaponDefinition DataAsset，并保存脏包。

UInterface
  UEscapeCombatAttacker / IEscapeCombatAttacker
    攻击者信息与命中确认接口。

  UEscapeCombatDamageable / IEscapeCombatDamageable
    受击接口，暴露 ApplyDamage。

  UInteractableInterface / IInteractableInterface
    通用交互接口，用于门等对象。

  UPickupInterface / IPickupInterface
    拾取接口，用于 APickupData。

Automation Test
  FWeaponImportToolInvalidSourceTest / FWeaponImportToolInvalidDestinationTest / FWeaponImportToolInvalidAssetNameTest
    位于 Tests/WeaponImportToolLibraryTest.cpp，验证武器导入工具对非法源目录、非法 /Game 目标路径和空资产名的失败处理。
```

关键结构体与枚举：

```text
FCombatRuntimeState
  UEscapeCombatComponent 的运行时状态容器，保存 ComboCount、bHasSavedComboInput、CurrentActionTag、ActiveTags、CurrentActionTags、CurrentPlayingMontage、HitActorsThisAction 和 InputBufferTimer。

FCombatActionDefinition
  战斗动作配置，包含 Montage、PlayRate、TraceDistance、TraceRadius、BaseDamage、DamageMultiplier、KnockbackImpulse、LaunchImpulse、输入缓存、GrantedTags、命中反馈 Niagara、NextComboTag 和可打断标记。

FGeneralActionDefinition
  通用动作配置，包含 Montage、PlayRate、GrantedTags、StaminaCost 和可打断标记。

FAttackHitPayload
  OnAttackHit 的命中载荷，包含 DamageCauser、DamageLocation 和 DamageImpulse。

FAttackTraceInst
  UEscapeCombatComponent 的连续攻击判定运行时状态，保存 bAttackTraceActive、bHasLastTracePoints、LastTraceStart、LastTraceEnd 和 ActiveDamageSourceBone。由 BeginAttackTrace / TickAttackTrace / EndAttackTrace 驱动，主要服务 ANS_MeleeAttackTrace。

FCombatDamageContext
  伤害上下文，包含 DamageValue、InstigatorActor、TargetActor、HitLocation、HitImpulse、ActionTag、DamageTypeTag 和 HitResult。

FCombatDamageResult
  伤害结果，包含 bApplied、ActualDamage 和 bKilled。

FClothLODProfile
  布料 LOD 档位，包含 MaxDistance、ClothBlendWeight、MaxDistanceScale 和 bEnableSimulation。

FItemText
  物品名称与描述。

FItemData
  DataTable 行数据，描述物品类型、ID、文本、图标、默认数量、逻辑资产、效果数值和世界模型；EItemType 当前包含 Weapon/Armor/Accessory 等装备类型。

FItemStack
  背包运行时槽位数据，包含 FItemData 和 Count。

ECharacterState
  Idle / Moving / Attacking / Sprinting / Stunned / Dead。

ECombatState
  Idle / Attacking / Charging / HeavyAttacking，目前声明在 EscapeCombatComponent.h 中，主战斗逻辑实际以 GameplayTag + FCombatRuntimeState 为核心。

ECurveMathModel
  Linear、SmoothStep、SineWave、CosineWave、Power、DampedSpring、PerlinNoise、Stepped。
```

## 3. 核心系统与数据流

### 3.1 角色、输入与 UI 初始化

```text
AEscapeGameCharacter::SetupPlayerInputComponent
  JumpAction Started -> DoJumpStart
  JumpAction Completed -> ACharacter::StopJumping
  MoveAction Triggered -> Move
  LookAction / MouseLookAction Triggered -> Look
  SprintAction Started/Completed -> USprintComponent::StartSprinting / StopSprinting
  CrouchAction Started/Completed -> StartCrouch / StopCrouch
  InventoryAction Started -> UInterectComponent::RequestToggleInventory
  InteractAction Started -> UInterectComponent::OnInteract
  ToggleCameraAction Started -> ToggleCameraMode
  UseItemAction Started -> Input_UseItem
  AttackAction Started/Completed -> UEscapeCombatComponent::Input_AttackStarted / Input_AttackCompleted
```

```text
AEscapeGameCharacter::BeginPlay
  本地控制且 HUDWidgetClass 有效时：
    CreateWidget<UGameHUDWidget>
    AddToViewport
    InitializeWidget(AttributeComp, SprintComp, InventoryComp)

  若 PlayerController 与 InteractComp 有效：
    InteractComp.OnRequestToggleInventory -> AEscapeGamePlayerController::ToggleInventoryUI
```

`AEscapeGamePlayerController` 在 `SetupInputComponent` 中把 `DefaultMappingContexts` 加入 `UEnhancedInputLocalPlayerSubsystem`；非移动触控时额外加入 `MobileExcludedMappingContexts`。背包菜单第一次打开时创建 `InventoryMenuClass`，从 Pawn 查 `UInventoryComponent`，调用 `UInventoryMenuWidget::InitializeInventory`。打开背包时使用 `FInputModeGameAndUI`、显示鼠标并忽略移动/视角输入；关闭时恢复 `FInputModeGameOnly`。

### 3.2 移动、冲刺与状态机

```text
Move 输入
  AEscapeGameCharacter::Move
    如果输入轴非零且 StateMachine 当前是 Idle -> SetState(Moving)
    DoMove 按 Controller Yaw 计算前/右方向并 AddMovementInput
```

```text
USprintComponent::TickComponent
  缓存 OwnerCharacter / MovementComp / StateMachine。
  bSprintRequested、状态机状态、速度、体力、地面状态和下蹲状态共同决定 bIsActurallySprinting。
  MaxWalkSpeed 使用 FInterpTo 从 CurrentSmoothedSpeed 平滑到 WalkSpeed 或 SprintSpeed，再乘 CurrentBuffMultiplier。
  冲刺时消耗体力并把状态切到 Sprinting。
  停止冲刺后经过 StaminaRegenDelay 再恢复体力。
  仅在当前状态是 Sprinting 时负责退回 Moving/Idle，不主动覆盖 Attacking、Stunned 等高优先级状态。
```

```text
UStateMachineComponent
  Dead 状态不可切走。
  Stunned 只能切到 Dead 或 Idle。
  ApplyStun 使用 Timer 在 Duration 后恢复 Idle。
  ApplyDeath 切到 Dead 并 DisableMovement / StopMovementImmediately。
```

### 3.3 战斗系统

战斗核心由 `UEscapeCombatComponent` 驱动。当前实现从早期零散成员变量收束到了 `FCombatRuntimeState`：

```text
FCombatRuntimeState
  BeginAction(ActionTag, Montage)
    清理上一轮 CurrentActionTags
    记录 CurrentActionTag / CurrentPlayingMontage
    把 ActionTag 加入 ActiveTags 和 CurrentActionTags
    清空 HitActorsThisAction，避免上一动作命中缓存污染本次攻击

  ResetAction()
    ComboCount = 0
    bHasSavedComboInput = false
    CurrentActionTag = Empty
    CurrentPlayingMontage = nullptr
    HitActorsThisAction.Reset()
    清理 CurrentActionTags 对应的 ActiveTags

  AddCombatTag / RemoveCombatTag
    管理全局战斗锁，如 Action.State.Attacking。

  AddActionTag / ClearCurrentActionTags
    管理当前动作自身标签，避免 Montage 结束后遗留动作 Tag。
```

攻击输入路径：

```text
AttackAction Started
  Input_AttackStarted
    设置 RuntimeState.InputBufferTimer，0.55 秒后触发 BeginOrUpdateChargedAttack。

AttackAction Completed
  如果 InputBufferTimer 仍活跃：
    ClearTimer
    RequestLightAttack()

  否则如果 ActiveTags 有 Action.Combat.Heavy.Charge：
    ReleaseChargedAttack()
```

轻击连段：

```text
RequestLightAttack
  如果没有 Action.State.Attacking 且没有 Action.Combat.Heavy.Charge：
    TryPlayActionByTagInternal(Action.Combat.Light.1)
    成功后 AddCombatTag(Action.State.Attacking)

  如果已处于 Action.State.Attacking：
    bHasSavedComboInput = true

CheckCombo
  只在 Action.State.Attacking 存在时处理。
  当前动作定义里读取 NextComboTag。
  如果 bHasSavedComboInput 且 NextComboTag 有效，则尝试播放下一段并递增 ComboCount。
```

蓄力重击：

```text
BeginOrUpdateChargedAttack
  若已处于 Heavy.Charge 或 ChargedAttack.Release，则直接返回。
  TryPlayActionByTagInternal(Action.Combat.Heavy.Charge)
  成功后移除 Action.State.Attacking 并清除轻击缓存。

ReleaseChargedAttack
  仅当 ActiveTags 有 Action.Combat.Heavy.Charge 时执行。
  移除 Heavy.Charge，加入 Action.ChargedAttack.Release。
  在当前 Montage 中跳转到名为 Attack 的 Section。
```

动作播放：

```text
TryPlayActionByTagInternal(ActionTag)
  先走 CanStartCombatAction，检查 ActionTag、OwnerCharacter、CharacterAnimData、动作配置、StateMachine、SprintComp、体力、死亡/眩晕、蓄力/重击释放锁和连击合法性。
  检查 SprintComp 当前体力，低于 10 时拒绝播放。
  使用 ActionDef->Montage.LoadSynchronous() 同步加载 Montage。
  OwnerChar->PlayAnimMontage(Montage, PlayRate) 成功后调用 RuntimeState.BeginAction。
```

攻击 Trace 与伤害：

```text
DoAttackTrace(DamageSourceBone)  [Legacy 单帧入口，旧 AN_MeleeAttackTrace 使用]
  从当前动作定义读取 TraceDistance、TraceRadius、BaseDamage、DamageMultiplier、KnockbackImpulse、LaunchImpulse。
  如果 EquippedWeaponDef 和 EquippedWeaponMesh 有效，且武器 Mesh 存在 TraceStart/TraceEnd Socket：
    TraceStart/TraceEnd 使用武器 Socket。
    TraceRadius/BaseDamage/DamageTypeTag 优先使用 UWeaponDefinition。
  否则退回角色 Mesh 的 DamageSourceBone + 角色前方向 TraceDistance。
  查询 Pawn 和 WorldDynamic，忽略 OwnerCharacter。
  用本地 ProcessedActors 和 RuntimeState.HitActorsThisAction 避免同一 Actor 在同帧/同动作重复受击。
  命中实现 UEscapeCombatDamageable 的 Actor 后：
    构造 FCombatDamageContext，写入 DamageValue、InstigatorActor、TargetActor、HitLocation、HitImpulse、ActionTag、DamageTypeTag 和 HitResult。
    IEscapeCombatDamageable::Execute_ApplyDamage(HitActor, DamageContext)
    如果 FCombatDamageResult.bApplied，且 OwnerChar 实现 IEscapeCombatAttacker，则 Execute_NotifyHitConfirmed。
  广播 OnAttackHit(FAttackHitPayload)。
```

ANS 连续攻击 Trace：

```text
BeginAttackTrace(DamageSourceBone)
  标记 AttackTraceInst.bAttackTraceActive=true。
  记录 ActiveDamageSourceBone。
  清空 RuntimeState.HitActorsThisAction，确保一次动作内同一目标只受击一次。
  通过 GetCurrentTracePoints 初始化 LastTraceStart / LastTraceEnd。

TickAttackTrace(DamageSourceBone)
  若 AttackTraceInst 未激活则直接跳过并输出日志。
  获取当前 TraceStart / TraceEnd。
  当上一帧点位有效时，分别 Sweep：
    LastTraceStart -> CurrentStart
    LastTraceEnd -> CurrentEnd
    CurrentStart -> CurrentEnd
  每段 Sweep 调用 SweepAttackSegment，再由 ProcessAttackHit 统一处理去重、接口检查、伤害上下文和 OnAttackHit。
  更新 LastTraceStart / LastTraceEnd。

EndAttackTrace()
  清空 bAttackTraceActive / bHasLastTracePoints / ActiveDamageSourceBone。

GetCurrentTracePoints(DamageSourceBone)
  优先使用装备武器 TraceStart / TraceEnd Socket。
  武器 Socket 不可用时退回角色 SkeletalMesh 的 DamageSourceBone。
  DamageSourceBone 不存在时优先退回“右手首”，再退回 ActorLocation。
  终点使用角色前方向乘以当前动作 TraceDistance，若动作配置缺失则默认 80。
```

武器装备链路：

```text
AEscapeGameCharacter 构造
  创建 EquippedWeaponMesh，挂在角色 Mesh 下，关闭碰撞和 Overlap。

AEscapeGameCharacter::BeginPlay
  如果 DefaultWeaponDefinition 有效：
    EquipWeapon(DefaultWeaponDefinition)

EquipWeapon(WeaponDef)
  检查 WeaponDef、WeaponMesh、EquippedWeaponMesh、角色 Mesh。
  检查角色 Mesh 是否存在 WeaponDef->AttachSocketName。
  EquippedWeaponMesh->SetStaticMesh(WeaponDef->WeaponMesh)
  AttachToComponent(GetMesh(), SnapToTargetNotIncludingScale, AttachSocketName)
  EscapeCombatComp->SetEquippedWeapon(WeaponDef, EquippedWeaponMesh)
  CurrentWeaponDefinition = WeaponDef

UnequipWeapon
  清空 CurrentWeaponDefinition 与 EquippedWeaponMesh StaticMesh。
  EscapeCombatComp->ClearEquippedWeapon()
```

Montage 结束：

```text
BeginPlay
  缓存 Mesh AnimInstance。
  CachedAnimInstance->OnMontageEnded.AddDynamic(this, &UEscapeCombatComponent::HandleAttackMontageEnded)

HandleAttackMontageEnded
  仅处理 RuntimeState.CurrentPlayingMontage。
  非当前 Montage 的结束回调会被忽略并输出 VeryVerbose 日志。
  ResetAction。
  如果角色处于 Dead / Stunned，仅清理攻击 Tag 与连击计数。
  其他情况按角色速度恢复 Moving 或 Idle。
  RemoveCombatTag(Action.State.Attacking)。
  BroadcastComboChange(0)。
```

当前测试：

```text
Tests/WeaponImportToolLibraryTest.cpp
  EscapeGame.Editor.WeaponImport.InvalidSourceFolder
  EscapeGame.Editor.WeaponImport.InvalidDestinationPath
  EscapeGame.Editor.WeaponImport.InvalidAssetName
  覆盖 ImportWeaponFromObjFolder 的基础输入校验失败路径。
```

### 3.4 受击、生命与死亡

`AEscapeGameCharacter` 当前实现了 `IEscapeCombatDamageable::ApplyDamage_Implementation`：

```text
ApplyDamage_Implementation(FCombatDamageContext DamageContext)
  如果 AttributeComp / StateMachineComp 缺失，或当前已经 Dead，则返回 bApplied=false。
  DamageToApply = Max(0, DamageContext.DamageValue)
  记录 OldHealth，AttributeComp->ApplyHealthChange(-DamageToApply)
  计算 ActualDamage = OldHealth - NewHealth
  FCombatDamageResult.bApplied = ActualDamage > 0
  FCombatDamageResult.ActualDamage = ActualDamage
  FCombatDamageResult.bKilled = NewHealth <= 0 && OldHealth > 0
  如果 bKilled：
    StateMachineComp->ApplyDeath()
  如果 bApplied 且 CharacterMovement 有效：
    GetCharacterMovement()->AddImpulse(DamageContext.HitImpulse)
```

`UAttributeComponent` 只负责生命数值和广播，不直接处理死亡：

```text
ApplyHealthChange
  Clamp(CurrentHealth + Delta, 0, MaxHealth)
  OnHealthChanged.Broadcast(CurrentHealth, MaxHealth)
```

`AAICharacter_1` 当前也实现了 `IEscapeCombatDamageable::ApplyDamage_Implementation`：

```text
AAICharacter_1::BeginPlay
  如果 AttributeComp 有效：
    CurrentHealth = MaxHealth
    输出生命初始化日志

ApplyDamage_Implementation(FCombatDamageContext DamageContext)
  如果 AttributeComp 缺失或 DamageValue <= 0，则返回 bApplied=false。
  AttributeComp->ApplyHealthChange(-Damage)
  Result.bApplied = NewHealth < OldHealth
  Result.ActualDamage = OldHealth - NewHealth
  Result.bKilled = NewHealth <= 0 && OldHealth > 0
  如果 bKilled：
    HandleDeath()

HandleDeath
  CharacterMovement DisableMovement + StopMovementImmediately。
  Capsule SetCollisionEnabled(NoCollision)。
  如果 Controller 是 AAIController：
    Blackboard.bIsdead = true
    Blackboard.ClearValue(TargetActor)
    AIController.StopMovement()
  SetLifeSpan(3.0)
```

### 3.5 交互、拾取与门

```text
UInterectComponent::OnInteract
  在 Owner 位置做半径 150 的 Sphere Sweep。
  Channel 使用 ECC_WorldDynamic。
  忽略 Owner。
  命中 Actor 后：
    优先检查 UPickupInterface -> Execute_AttemptPickUp(HitActor, PawnOwner)
    否则检查 UInteractableInterface -> Execute_Interact(HitActor, PawnOwner)
    Owner 为空或 Owner 不是 Pawn 的防御分支会输出 UE_LOG。
  单次交互找到目标后 break。
```

```text
APickupData
  MeshComp 为 Root，Mesh 关闭碰撞。
  SphereComp QueryOnly：
    Pawn -> Overlap
    WorldDynamic -> Block
    Visibility -> Block

  AttemptPickUp:
    ItemDataTable.FindRow<FItemData>(ItemID)
    InstigatorPawn.FindComponentByClass<UInventoryComponent>()
    Inventory->AddItem(*RowData, ItemCount)
    全部放入则 Destroy；部分放入则更新 ItemCount。
    InstigatorPawn 为空、DataTable/ItemID 缺失、Row 查找失败、背包缺失或背包满都会输出日志。

  OnConstruction:
    按 ItemID 查表，如果 RowData->WorldMesh 有效则 SetStaticMesh。
```

```text
AInteractDoor
  RequireKeyID.ID 为空：直接打开并触发 OnDoorOpen。
  RequireKeyID.ID 非空：从 Pawn 查 UInventoryComponent。
  GetTotalCountOfItem(RequireKeyID.ID) > 0 时打开门。
  bConsumeKey 为 true 时 RemoveItem(RequireKeyID, 1)。
  失败时输出原因日志；缺钥匙时触发 OnDoorLocked。
```

### 3.6 背包、物品与 UI

```text
UInventoryComponent
  BeginPlay:
    Items.SetNum(InventoryCapacity)

  AddItem:
    检查 ID 与数量。
    若 ItemLogic 存在，读取 bStackable 和 MaxStackCount。
    先堆叠同 ID 未满槽，再填入空槽。
    非堆叠物品每个槽只放 1 个。
    成功添加部分或全部时广播 OnInventoryUpdated。
    ItemID 为空、数量非法和背包满会输出日志。

  RemoveItem:
    从后往前移除指定 ID 数量。
    Count 归零时重置为默认 FItemStack。
    最后广播 OnInventoryUpdated。
    移除数量小于等于 0 会输出日志并直接返回。

  UseItem:
    检查槽位、数量和 ItemLogic。
    调用 LogicAsset->OnUse(GetOwner())。
    仅当 bUsedSuccessfully && bConsumeOnUse 时扣数量。
    数量归零时清空槽位并广播。
    OnUse 返回 false 时输出 Slot、ItemID 和 LogicAsset；非消耗类使用成功只记录日志，不扣数量。

  SwapSlots:
    检查 IndexA/IndexB 后 Items.Swap，并广播。
```

物品逻辑：

```text
UItemDefinition
  bConsumeOnUse
  bStackable
  MaxStackCount
  CoolDownTime
  BlueprintNativeEvent OnUse(AActor* TargetActor)

UItemAction_Healing
  查 UAttributeComponent。
  满血时返回 false，否则 ApplyHealthChange(HealAmount)。

UItemAction_RestoreStamina
  查 USprintComponent。
  调用 StaminaChange(StaminaHealing)。
  User 为空或缺少 SprintComponent 时输出失败日志。

UItemAction_EnhanceStamina
  查 USprintComponent。
  调用 ApplyMaxChange(StaminaBoostAmount)。
  User 为空或缺少 SprintComponent 时输出失败日志。

UItemAction_Boosting
  查 USprintComponent。
  调用 StartSpeedBuff(Duration, SpeedMultiplier)。
  TargetActor 为空或缺少 SprintComponent 时输出失败日志。
```

UI 刷新链路：

```text
UGameHUDWidget::InitializeWidget
  AttributeComp.OnHealthChanged -> OnHealthUpdate
  SprintComp.OnStaminaChanged -> OnStaminaUpdate
  HotbarWidget.InitializeHotbar(NewInventoryComp)

UInventoryMenuWidget::InitializeInventory
  InventoryComp.OnInventoryUpdated -> RefreshInventory
  RefreshInventory 创建 InventoryCapacity 个 UInventorySlotWidget

UInventoryHotbarWidget::InitializeHotbar
  InventoryComp.OnInventoryUpdated -> RefreshHotbar
  RefreshHotbar 创建 NumSlot 个 UInventorySlotWidget

UInventorySlotWidget
  NativeOnInitialized 绑定 SlotButton.OnClicked -> OnSlotClicked
  SetItem 设置图标、数量和 Tooltip。
  OnSlotClicked 调用 OwnerComponent->UseItem(SlotIndex)。
  NativeOnDragDetected 创建 UInventoryDragDropOperation。
  NativeOnDrop 当前只处理同一 UInventoryComponent 内部 SwapSlots。
```

### 3.7 动画、布料 LOD 与风力

```text
UCharacterAnimInstance::NativeInitializeAnimation
  缓存 OwnerCharacter。
  缓存 CharacterMovementComponent。
  查找 UWindSimulationComponent。
  查找 UClothLODControllerComponent。
  查找 USprintComponent。

NativeUpdateAnimation
  GameThread 安全读取：
    Velocity / VerticalVelocity / GroundSpeed / Falling / Acceleration
    SprintComp->ReturnSprintState()
    ActorRotation
    WindComponent->GetfinalWind()
    ClothLODComponent->GetLODFactor()

NativeThreadSafeUpdateAnimation
  只读缓存数据：
    根据速度计算 HairRunBounceForce。
    插值 RuntimeHairDamping。
    速度大于 2000 时关闭 PhysicsAlpha，否则按 Cloth LOD 缩放。
    计算 LocomotionPlayRate。
    用 CachedRotation.UnrotateVector(CachedVelocity) 计算 LocomotionAngle。
```

```text
UClothLODControllerComponent
  BeginPlay 查找或使用 TargetMesh。
  LODProfiles 按 MaxDistance 排序。
  用 Timer 以 EvaluationInterval 周期评估，初始延迟随机错峰。
  不可见且 bDisableWhenHidden 时关闭模拟。
  计算到 PlayerCameraManager 的距离，得到 DistanceLOD。
  可与 SkeletalMesh PredictedLODLevel 融合。
  平滑 ClothBlendWeight 和 MaxDistanceScale。
  应用到 SkeletalMeshComponent::ClothBlendWeight / SetClothMaxDistanceScale / bDisableClothSimulation。
  传送距离超过 TeleportThreshold 时 ForceClothNextUpdateTeleportAndReset。
```

```text
UWindSimulationComponent
  Tick 只在非 Dedicated Server 运行。
  BasicGlobalWind + Clamp(OwnerVelocity * MovementWindScale, MaxVelocityWindForce) 得到基础风。
  CurrentWind 使用 VInterpTo 平滑。
  PerlinNoise3D + SineWave 生成 CachedNoiseWind。
  TargetWind = CurrentWind + CachedNoiseWind。
  GetfinalWind 返回 TargetWind，动画实例乘以 KawaiiMultiplier 后输出给 KawaiiWind。
```

### 3.8 TA 曲线工具

```text
UUTACurveToolLibrary::BakeMathToCurve
  输入 UCurveFloat、ECurveMathModel、StartValue、EndValue、ExtraParamA、Decay、SampleCount。
  支持 Linear、SmoothStep、SineWave、CosineWave、Power、DampedSpring、PerlinNoise、Stepped。
  WITH_EDITOR 下调用 TargetCurveAsset->Modify()。
  重置 RichCurve 后采样写入，并设置 RCIM_Linear。
  MarkPackageDirty。
```

### 3.9 武器导入工具链

```text
UWeaponImportToolLibrary::ImportWeaponFromObjFolder
  输入 SourceFolder、DestinationPath、AssetBaseName、BaseDamage、TraceRadius 和 Socket 名称。
  先校验源目录、/Game 目标路径、资产名、BaseDamage 和 TraceRadius。
  在 SourceFolder 中查找第一个 .obj 文件、第一个文件名包含 Diffuse 的 .png，Lightmap .png 为可选。
  WITH_EDITOR 下通过 UAssetImportTask + AssetTools 导入 Static Mesh 与贴图。
  CreateBasicDiffuseMaterial 创建 M_* 材质：
    Diffuse TextureSample -> BaseColor
    Constant 0.45 -> Roughness
    Constant 0.3 -> Metallic
  AssignMaterialToStaticMesh 把生成材质赋给 Static Mesh 第 0 材质槽。
  CreateWeaponDefinitionAsset 创建 DA_Weapon_*：
    WeaponMesh 指向导入 Mesh
    TraceRadius/BaseDamage/Socket 名称写入参数
    DamageTypeTag 默认 Data.Damage.Physical
    bConsumeOnUse=false, bStackable=false, MaxStackCount=1
  保存 Static Mesh、贴图、材质和 WeaponDefinition 的脏包。
  成功后提示检查 Static Mesh Socket，并把 DA_Weapon_* 配到角色 DefaultWeaponDefinition。
```

```text
Docs/Useful_Exp/weapon_importer_euw_setup.md
  记录 EUW_WeaponImporter 的手动蓝图搭建方式、输入字段、按钮逻辑、Sashimi 示例路径、WeaponSocket / TraceStart / TraceEnd 配置和注意事项。
```

### 3.10 Dialogue / Quest 系统

当前 Dialogue 文件夹已经从早期计划推进到“会话骨架阶段”。它不是单纯 NPC 文本系统，而是面向普通 NPC、Boss 开场对白、任务、全局剧情状态和存档读档的统一框架雏形。

文件边界：

```text
Dialogue/EscapeDialogueTypes.h
  公共枚举与结构体：节点、选项、条件、效果、任务状态、目标、奖励、中断策略、运行时状态、存档记录。

Dialogue/DialogueDefinition.h/.cpp
  UDialogueDefinition : UDataAsset
  一份对话资产，包含 DialogueID、StartNodeID、Nodes、bAllowRestart、bUseLocalizedText、DefaultInterruptPolicy。

Dialogue/QuestDefinition.h/.cpp
  UQuestDefinition : UDataAsset
  一份任务资产，包含 QuestID、标题、描述、接任务 NPC、交任务 NPC、前置条件、目标和奖励。

Dialogue/DialogueParticipantComponent.h/.cpp
  UDialogueParticipantComponent
  可挂在普通 NPC、Boss、敌人、机关上的对话参与者身份与触发配置组件。

Dialogue/DialogueNPC.h/.cpp
  ADialogueNPC : AActor, IInteractableInterface
  普通非战斗 NPC 包装类，只负责交互入口和把请求转给 UDialogueQuestSubsystem。

Dialogue/DialogueQuestSubsystem.h/.cpp
  UDialogueQuestSubsystem : UGameInstanceSubsystem
  当前对话、任务、旗标、资产缓存、会话事件和存档对象的运行时管理中心。

Dialogue/EscapeDialogueSaveGame.h/.cpp
  UEscapeDialogueSaveGame : USaveGame
  对话、任务、全局旗标、世界事件的长期存档容器。
```

核心数据分层：

```text
静态配置:
  UDialogueDefinition
  UQuestDefinition
  FDialogueNode
  FDialogueOption
  FQuestObjectiveDefinition
  FQuestRewardDefinition

当前会话:
  FConversationSession ActiveSession
    记录本次正在进行的对话：SessionId、NPC_ID、DialogueID、CurrentNodeID、LastSelectedOptionID、bIsActive、bLockPlayer、bCanSkip、InterruptPolicy。

运行时状态:
  DialogueRuntimeMap: TMap<FGameplayTag, FDialogueRuntimeState>
    Key 是 DialogueID，Value 记录 CurrentNodeID、bHasSeenDialogue、SeenNodeIDs、VisitedOptionIDs、BranchFlags、LastTalkPartnerID。

  QuestRuntimeMap: TMap<FGameplayTag, FQuestRuntimeState>
    Key 是 QuestID，Value 记录 QuestState、ObjectiveStates、bRewardClaimed、QuestGiverNPC_ID、TurnInNPC_ID、StartedTimeSeconds、LastUpdateReason。

  GlobalFlagMap: TMap<FGameplayTag, FGlobalFlagState>
    Key 是 FlagID，Value 记录 bValue、NumericValue、TextValue、LastChangedTimeSeconds。

资产缓存:
  LoadedDialogueDefinitions
    Key 是 DialogueID，Value 是已加载 UDialogueDefinition。

  LoadedQuestDefinitions
    Key 是 QuestID，Value 是已加载 UQuestDefinition。

存档:
  UEscapeDialogueSaveGame
  CurrentSaveGame
  SaveSlotName / UserIndex / bAutoSaveEnabled
```

当前已落地的运行流：

```text
ADialogueNPC::Interact_Implementation
  检查 bCanInteract、InstigatorPawn、DialogueParticipantComp、GameInstance、UDialogueQuestSubsystem
  -> UDialogueQuestSubsystem::StartConversation(InstigatorPawn, DialogueParticipantComp)

UDialogueQuestSubsystem::StartConversation
  检查 Instigator / Participant / bCanTalk / bConversationOpen / DialogueDefinition
  同步加载 Participant->DialogueDefinition
  选择 Participant->DefaultStartNodeID 或 DialogueAsset->StartNodeID
  写入 ActiveSession
  bConversationOpen = true
  更新 DialogueRuntimeMap[DialogueID]
  广播 OnConversationStarted

UDialogueQuestSubsystem::GetCurrentNode
  检查会话、DialogueID、CurrentNodeID
  从 LoadedDialogueDefinitions 找 UDialogueDefinition
  在 Nodes 中查找 CurrentNodeID 对应的 FDialogueNode

UDialogueQuestSubsystem::EndConversation
  把当前节点回写 DialogueRuntimeMap
  记录 InterruptReason
  ActiveSession.bIsActive = false
  bConversationOpen = false
  广播 OnConversationEnded
```

当前正在推进的切片：

```text
SelectOption 最小跳转闭环
  目标:
    从当前节点查找 OptionID
    记录 ActiveSession.LastSelectedOptionID
    记录 DialogueRuntimeMap[DialogueID].VisitedOptionIDs
    如果 bCloseDialogueAfterSelected=true，EndConversation(NormalEnd)
    如果 NextNodeID 有效，切换 ActiveSession.CurrentNodeID
    同步更新 DialogueRuntimeMap[DialogueID].CurrentNodeID 和 SeenNodeIDs

  当前源码状态:
    SelectOption 已有初版实现，但仍需收尾。
    已创建 Tests/DialogueQuestSubsystemTest.cpp，只包含 ChangesCurrentNode 测试。

  暂停时已发现的待修点:
    DialogueQuestSubsystem.cpp 中 SelectOption 有一行重复且不推荐的 operator[] 写入:
      DialogueRuntimeMap[ActiveSession.DialogueID].VisitedOptionIDs.Add(OptionID);
    应删除该行，保留 FindOrAdd 后通过 State.VisitedOptionIDs.Add(OptionID) 写入。

    SelectOption 在 GetCurrentNode 后最好增加 CurrentNode.NodeID.IsValid() 检查，避免空节点继续查选项。

    DialogueQuestSubsystemTest.cpp 当前使用未确认注册的测试 Tag:
      Dialogue.Test.SelectOption
      Dialogue.Node.Test.Root
      Dialogue.Node.Test.Next
      Dialogue.Option.Test.GoNext
      NPC.Test.Dialogue
    建议改为 include Core/EscapeGameplayTags.h，并使用已注册 Native Tag，如 EscapeGameplayTags::Dialogue_Gatekeeper_Intro 等。
```

未落地能力：

```text
GetAvailableOptions 目前原样返回当前节点 Options，尚未进行条件筛选。
EvaluateCondition 仍为占位，返回 false。
ApplyEffect 仍为占位，仅输出日志。
StartQuest / SetGlobalFlag / GiveItem / RemoveItem / GrantReward 尚未接入。
UEscapeDialogueSaveGame 目前只保存字段，Save/Load 映射尚未落地。
Dialogue UI Widget 尚未接入。
Boss / Encounter 强制对白字段已预留，实际触发链尚未接入。
```

## 4. 关键接口与通信模式

### 4.1 UINTERFACE

```text
UEscapeCombatAttacker / IEscapeCombatAttacker
  文件: Interface/EscapeCombatAttacker.h
  方法:
    GetBaseDamage() const
    GetCurrentComboCount() const
    NotifyHitConfirmed(AActor* HitTarget, const FHitResult& HitResult)
  用途:
    攻击者信息与命中确认回调。当前 DoAttackTrace 在 FCombatDamageResult.bApplied 为 true 后调用 NotifyHitConfirmed。

UEscapeCombatDamageable / IEscapeCombatDamageable
  文件: Interface/EscapeCombatDamageable.h
  方法:
    ApplyDamage(const FCombatDamageContext& DamageContext) -> FCombatDamageResult
  用途:
    UEscapeCombatComponent 的 Legacy 单帧 Trace 与 ANS 连续 Trace 命中后调用。AEscapeGameCharacter 和 AAICharacter_1 当前均已实现 ApplyDamage_Implementation，并返回 bApplied / ActualDamage / bKilled。

UInteractableInterface / IInteractableInterface
  文件: Interface/InteractableInterface.h
  方法:
    Interact(APawn* InstigatorPawn)
    CanInteract(AActor* Interactor) const
    GetInteractText(AActor* Interactor) const
  用途:
    通用世界交互。AInteractDoor 已实现。

UPickupInterface / IPickupInterface
  文件: Interface/PickupInterface.h
  方法:
    AttemptPickUp(APawn* InstigatorPawn)
    CanPickUp(AActor* Interactor) const
    GetPickUpText(AActor* Interactor) const
  用途:
    世界拾取物交互。APickupData 已实现 AttemptPickUp。
```

### 4.2 动态多播委托

```text
FOnHealthChanged
  声明: HealthController/AttributeComponent.h
  参数: CurrentHealth, MaxHealth
  广播: UAttributeComponent::ApplyHealthChange
  监听: UGameHUDWidget::OnHealthUpdate

FOnStaminaChanged
  声明: SprintComponent.h
  参数: CurrentStamina, MaxStamina
  广播: USprintComponent::ApplyStaminaChange / StaminaChange / ApplyMaxChange
  监听: UGameHUDWidget::OnStaminaUpdate

FOnStateChanged
  声明: statemachine/StateMachineComponent.h
  参数: NewState, OldState
  广播: UStateMachineComponent::SetState

FOnInventoryUpdated
  声明: InventoryComponent.h
  广播: AddItem / RemoveItem / UseItem / SwapSlots
  监听: UInventoryMenuWidget::RefreshInventory, UInventoryHotbarWidget::RefreshHotbar

FOnRequestToggleInventory
  声明: InterectComponent.h
  广播: UInterectComponent::RequestToggleInventory
  监听: AEscapeGamePlayerController::ToggleInventoryUI

FOnAttackHitSignature
  声明: EscapeCombatComponent.h
  参数: FAttackHitPayload
  广播: UEscapeCombatComponent::DoAttackTrace 命中后

FOnComboCountChangedSignature
  声明: EscapeCombatComponent.h
  参数: NewComboCount
  广播: UEscapeCombatComponent::BroadcastComboChange

UAnimInstance::OnMontageEnded
  绑定: UEscapeCombatComponent::BeginPlay
  监听: UEscapeCombatComponent::HandleAttackMontageEnded
```

### 4.3 GameplayTag

定义位置：

```text
EscapeGameplayTags.h
EscapeGameplayTags.cpp
```

当前 Native GameplayTags：

```text
Input.Action.LightAttack
Input.Action.HeavyAttack
Input.Action.Dodge
Input.Action.Jump
Input.Action.Skill.1
Input.Action.Skill.2
Input.Action.UseItem

Action.State.Attacking
Action.State.Dodging
Action.State.Dead

Action.Combat.Light.1
Action.Combat.Light.2
Action.Combat.Light.3
Action.Combat.Light.4
Action.ChargedAttack.Release
Action.Combat.Heavy.Charge
Action.Combat.AirAttack

State.Movement.Grounded
State.Movement.Airborne
State.Status.Invincible
State.Status.HyperArmor
State.Status.Blocking
State.Debuff.Stun
State.Debuff.Knockdown
State.Debuff.Burn

Event.Montage.ComboWindow.Open
Event.Montage.ComboWindow.Close
Event.Combat.Hit

Data.Damage.Physical
Data.Damage.Fire
Data.HitDirection.Front
Data.HitDirection.Back

Cooldown.Skill.1
Cooldown.Dodge

Dialogue.Gatekeeper.Intro
Dialogue.Node.Gatekeeper.Root
Dialogue.Node.Gatekeeper.KeyHint
Dialogue.Node.Gatekeeper.OpenGate
Dialogue.Option.Gatekeeper.AskKey
Dialogue.Option.Gatekeeper.TurnInKey
Dialogue.Option.Gatekeeper.Leave

NPC.Village.Gatekeeper
Enemy.Boss.Gatekeeper
Quest.Main.FindGateKey
Quest.Objective.FindGateKey.CollectKey

Item.Key.MainGate
Flag.World.MainGateUnlocked
Flag.Dialogue.Gatekeeper.IntroSeen
Flag.Combat.GatekeeperIntroPlayed
Encounter.Boss.Gatekeeper
Puzzle.Courtyard.StatueOrder
Reward.Item.GateKey
Reward.Unlock.MainGate
WorldEvent.MainGate.Opened
Unlock.Area.MainGate
Attribute.Player.MaxHealth
```

关键用途：

```text
UCharacterAnimData::CombatActionMap
  FGameplayTag -> FCombatActionDefinition

UCharacterAnimData::GeneralActionMap
  FGameplayTag -> FGeneralActionDefinition

FCombatRuntimeState::ActiveTags
  运行时战斗锁，如 Action.State.Attacking、Action.Combat.Heavy.Charge、Action.ChargedAttack.Release。

FCombatRuntimeState::CurrentActionTags
  当前动作生命周期内自动管理的动作 Tag。

FCombatActionDefinition::NextComboTag
  连击链路，决定下一段动作。

Dialogue / Quest 运行时:
  DialogueRuntimeMap 使用 DialogueID 作为 Key。
  QuestRuntimeMap 使用 QuestID 作为 Key。
  FQuestRuntimeState::ObjectiveStates 使用 ObjectiveID 作为 Key。
  GlobalFlagMap 使用 FlagID 作为 Key。
  SelectOption 应使用当前 ActiveSession.DialogueID 定位 FDialogueRuntimeState，再向 VisitedOptionIDs 写入 OptionID。
```

### 4.4 通信模式总结

```text
组件查找:
  FindComponentByClass 用于角色、控制器、UI、物品逻辑、拾取、门、战斗缓存和动画实例。

动态委托:
  UI 刷新、体力/生命、状态变化、背包变化、命中反馈、背包开关请求使用动态多播委托。

接口:
  交互、拾取、受击、攻击者命中确认以 UINTERFACE 解耦具体 Actor 类型。

GameplayTag:
  战斗动作查询、动作状态锁、连击链路和未来事件/冷却扩展点。

DataAsset/DataTable:
  UCharacterAnimData 驱动动作；DT_ItemData + UItemDefinition 驱动物品展示和逻辑；UWeaponDefinition 驱动武器 Mesh、Socket、Trace 和基础伤害。

Editor 工具:
  UWeaponImportToolLibrary 通过 AssetTools/AssetRegistry/UnrealEd 导入武器资源，EUW_WeaponImporter 作为编辑器界面入口。

Timer:
  战斗输入长按判定、速度 Buff、眩晕恢复、布料 LOD 评估使用 Timer。

Subsystem/GAS/网络:
  当前主源码已存在 UDialogueQuestSubsystem，用于对话/任务运行时状态管理。
  当前仍未发现 Gameplay Ability System 使用，也没有显式 Replication/RPC/NetMulticast。
```

## 5. 数据资产与配置

### 5.1 UDataAsset

```text
UCharacterAnimData
  文件: CharacterAnimData.h
  用途: 动画与动作数据库。
  字段:
    TSoftObjectPtr<UBlendSpace> MovementBlendSpace
    TSoftObjectPtr<UAnimSequenceBase> IdleAnim
    TMap<FGameplayTag, FCombatActionDefinition> CombatActionMap
    TMap<FGameplayTag, FGeneralActionDefinition> GeneralActionMap

UItemDefinition
  文件: ItemDefinition.h
  用途: 物品逻辑资产基类。
  字段:
    bConsumeOnUse
    bStackable
    MaxStackCount
    CoolDownTime
  事件:
    BlueprintNativeEvent OnUse(AActor* TargetActor)

UWeaponDefinition : public UItemDefinition
  文件: WeaponDefinition.h
  用途: 武器数据资产，当前由角色默认装备和战斗 Trace 读取。
  字段:
    TObjectPtr<UStaticMesh> WeaponMesh
    FName AttachSocketName = WeaponSocket
    FName TraceStartSocketName = TraceStart
    FName TraceEndSocketName = TraceEnd
    float TraceRadius = 12
    float BaseDamage = 20
    FGameplayTag DamageTypeTag

UItemAction_Healing
  文件: Items/ItemAction_Healing.h/.cpp
  用途: 治疗生命。

UItemAction_RestoreStamina
  文件: Items/ItemAction_RestoreStamina.h/.cpp
  用途: 回复当前耐力。

UItemAction_EnhanceStamina
  文件: Items/ItemAction_EnhanceStamina.h/.cpp
  用途: 增加最大耐力。

UItemAction_Boosting
  文件: Items/ItemAction_Boosting.h/.cpp
  用途: 临时加速 Buff。

UDialogueDefinition
  文件: Dialogue/DialogueDefinition.h/.cpp
  用途: 对话内容 DataAsset，一份资产表示一棵对话树。
  字段:
    FGameplayTag DialogueID
    FGameplayTag StartNodeID
    TArray<FDialogueNode> Nodes
    bool bAllowRestart
    bool bUseLocalizedText
    FInterruptPolicy DefaultInterruptPolicy

UQuestDefinition
  文件: Dialogue/QuestDefinition.h/.cpp
  用途: 任务配置 DataAsset，只描述任务内容，不保存运行时进度。
  字段:
    FGameplayTag QuestID
    FText QuestTitle
    FText QuestDescription
    FGameplayTag QuestGiverNPC_ID
    FGameplayTag TurnInNPC_ID
    TArray<FDialogueCondition> Prerequisites
    TArray<FQuestObjectiveDefinition> Objectives
    TArray<FQuestRewardDefinition> Rewards
    bool bCanRepeat
    bool bAutoComplete
```

### 5.2 DataTable 与结构

```text
FItemData : public FTableRowBase
  文件: ItemData.h
  用途: 物品表行，APickupData::AttemptPickUp 和 OnConstruction 按 ItemID 查行。
  字段:
    EItemType ItemType（Key / Tool / Consumable / BuffItem / Weapon / Armor / Accessory / QuestItem）
    FName ID
    FItemText ItemText
    UTexture2D* Icon
    int32 DefaultCount
    TObjectPtr<UItemDefinition> ItemLogic
    float RestoreHealthAmount
    float SpeedBoostAmount
    float DamageBoostAmount
    float DamageBoostTime
    TObjectPtr<UStaticMesh> WorldMesh
```

### 5.3 当前可见 Content 资产速查

```text
核心数据:
  Content/Data/DT_ItemData.uasset
  Content/Data/Da_Logic_Boosting.uasset
  Content/BP_BoostPotion.uasset

输入:
  Content/Input/IMC_Default.uasset
  Content/Input/IMC_MouseLook.uasset
  Content/Input/Actions/IA_Attack.uasset
  Content/Input/Actions/IA_Crouch.uasset
  Content/Input/Actions/IA_Interact.uasset
  Content/Input/Actions/IA_Inventory.uasset
  Content/Input/Actions/IA_Jump.uasset
  Content/Input/Actions/IA_Look.uasset
  Content/Input/Actions/IA_MouseLook.uasset
  Content/Input/Actions/IA_Move.uasset
  Content/Input/Actions/IA_Sprint.uasset
  Content/Input/Actions/IA_ToggleCameraMode.uasset
  Content/Input/Actions/IA_UseItemAction.uasset

UI:
  Content/UI/WBP_HUD.uasset
  Content/UI/WBP_HotBar.uasset
  Content/UI/WBP_InventoryMenu.uasset
  Content/UI/WBP_InventorySlot.uasset
  Content/UI/WBP_ItemToolTip.uasset

AI:
  Content/AI/BB_Enemy_Base.uasset
  Content/AI/BP_EnemyAIController.uasset
  Content/AI/BP_Enemy_Base.uasset
  Content/AI/BTT_FindRandomPatrolLocation.uasset
  Content/AI/BT_EmemyBase.uasset

角色蓝图:
  Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset
  Content/ThirdPerson/Blueprints/BP_ThirdPersonPlayerController.uasset
  Content/ThirdPerson/Blueprints/BP_ThirdPersonGameMode.uasset
  Content/ThirdPerson/Blueprints/BP_Yuno.uasset
  Content/ThirdPerson/Blueprints/BP_Pickup_Base.uasset

武器资产:
  Content/ThirdPerson/Weapon/Sashimi/DA_Weapon_Sword_Sashimi.uasset
  Content/ThirdPerson/Weapon/Sashimi/Equip_Sword_Sashimi.uasset
  Content/ThirdPerson/Weapon/Sashimi/M_Sword_Sashimi.uasset
  Content/ThirdPerson/Weapon/Sashimi/Equip_Sword_Sashimi_01_Tex_Diffuse.uasset
  Content/ThirdPerson/Weapon/Sashimi/Equip_Sword_Sashimi_01_Tex_Lightmap.uasset
  Content/ThirdPerson/Weapon/Sashimi_V2/DA_Weapon_Equip_Sword-Sashimi_V2.uasset
  Content/ThirdPerson/Weapon/Sashimi_V2/Equip_Sword-Sashimi_V2.uasset
  Content/ThirdPerson/Weapon/Sashimi_V2/M_Equip_Sword-Sashimi_V2.uasset
  Content/ThirdPerson/Weapon/Sashimi_V2/Equip_Sword_Sashimi_01_Tex_Diffuse.uasset
  Content/ThirdPerson/Weapon/Sashimi_V2/Equip_Sword_Sashimi_01_Tex_Lightmap.uasset

地图:
  Content/Maps/NewMap.umap
  DefaultEngine 当前默认启动 SoStylized/Maps/CompleteVol1/Demonstration

战斗/角色动画:
  Content/Katana_Animations/AN_CheckCombo.uasset
  Content/Katana_Animations/AnimationForYuno/AS_Combo_Attack_05_01_Seq_Montage.uasset
  Content/Katana_Animations/AnimationForYuno/AS_Combo_Attack_05_02_Seq_Montage.uasset
  Content/Katana_Animations/AnimationForYuno/AS_Combo_Attack_05_03_Seq_Montage.uasset
  Content/Katana_Animations/AnimationForYuno/AS_Combo_Attack_05_04_Seq_Montage.uasset
  Content/Katana_Animations/AnimationForYuno/AS_Power_Attack_Seq_Montage.uasset
  Content/Katana_Animations/AnimationForYuno/AS_Attack_Air_to_Floor_02_Loop_Seq_Montage.uasset
  Content/Katana_Animations/Animations/BS_Yuno_Locomotion.uasset

TA 工具测试:
  Content/ToolTest/CT_TestCurve.uasset
  Content/ToolTest/EUV_CurveGenerator.uasset

Editor 工具:
  Content/EditorTools/EUW_WeaponImporter.uasset
```

## 6. 文件夹结构速查

```text
Source/EscapeGame/
  EscapeGame.Build.cs
  EscapeGame.h / EscapeGame.cpp
  AICharacter_1.h / AICharacter_1.cpp
  EscapeGameCharacter.h / EscapeGameCharacter.cpp
  EscapeGamePlayerController.h / EscapeGamePlayerController.cpp
  EscapeGameGameMode.h / EscapeGameGameMode.cpp
  EscapeGameplayTags.h / EscapeGameplayTags.cpp
  CharacterAnimData.h / CharacterAnimData.cpp
  CharacterAnimInstance.h / CharacterAnimInstance.cpp
  EscapeCombatType.h
  EscapeCombatComponent.h / EscapeCombatComponent.cpp
  SprintComponent.h / SprintComponent.cpp
  InventoryComponent.h / InventoryComponent.cpp
  InterectComponent.h / InterectComponent.cpp
  PickupData.h / PickupData.cpp
  GameHUDWidget.h / GameHUDWidget.cpp
  InventoryMenuWidget.h / InventoryMenuWidget.cpp
  InventoryHotbarWidget.h / InventoryHotbarWidget.cpp
  InventorySlotWidget.h / InventorySlotWidget.cpp
  InventoryDragDropOperation.h / InventoryDragDropOperation.cpp
  ItemData.h / ItemData.cpp
  ItemDefinition.h / ItemDefinition.cpp
  ItemToolTipWidget.h / ItemToolTipWidget.cpp
  ClothLODControllerComponent.h / ClothLODControllerComponent.cpp
  WindSimulationComponent.h / WindSimulationComponent.cpp
  UTACurveToolLibrary.h / UTACurveToolLibrary.cpp
  WeaponDefinition.h / WeaponDefinition.cpp
  WeaponImportToolLibrary.h / WeaponImportToolLibrary.cpp

  ComboAttackComponent/
    当前为空目录。

  Dialogue/
    DialogueDefinition.h / DialogueDefinition.cpp
    DialogueNPC.h / DialogueNPC.cpp
    DialogueParticipantComponent.h / DialogueParticipantComponent.cpp
    DialogueQuestSubsystem.h / DialogueQuestSubsystem.cpp
    EscapeDialogueSaveGame.h / EscapeDialogueSaveGame.cpp
    EscapeDialogueTypes.h
    QuestDefinition.h / QuestDefinition.cpp

  HealthController/
    AttributeComponent.h / AttributeComponent.cpp

  Interface/
    EscapeCombatAttacker.h / EscapeCombatAttacker.cpp
    EscapeCombatDamageable.h / EscapeCombatDamageable.cpp
    InteractableInterface.h / InteractableInterface.cpp
    PickupInterface.h / PickupInterface.cpp

  Items/
    ItemAction_Boosting.h / ItemAction_Boosting.cpp
    ItemAction_EnhanceStamina.h / ItemAction_EnhanceStamina.cpp
    ItemAction_Healing.h / ItemAction_Healing.cpp
    ItemAction_RestoreStamina.h / ItemAction_RestoreStamina.cpp

  statemachine/
    StateMachineComponent.h / StateMachineComponent.cpp

  Tests/
    WeaponImportToolLibraryTest.cpp
    DialogueQuestSubsystemTest.cpp

  WorldInteractObject/
    InteractDoor.h / InteractDoor.cpp

  Docs/
    AI_Memory/
      architecture.md
      code_review_bugs.md
      collaboration_rules.md
    Debug_Lessons/
    DevelopmentLogs/
      README.md
      YYYY-MM-DD.md
    Plan/
      dialogue_folder_architecture_overview.md
      dialogue_select_option_execution_plan.md
    Useful_Exp/
      combat_runtime_state_patterns.md
      threading_snapshot_and_wind_patterns.md
      weapon_importer_euw_setup.md
```

```text
Content/ 当前非 Variant 资产数量概览:
  AI: 5
  BP_BoostPotion.uasset: 1
  Characters: 128
  Collections: 0
  Data: 2
  Developers: 0
  EditorTools: 1
  Input: 16
  Katana_Animations: 289
  LevelPrototyping: 27
  Maps: 0
  SoStylized: 544
  ThirdPerson: 111
  ToolTest: 2
  UI: 5
  __ExternalActors__: 71
  __ExternalObjects__: 2
```

## 7. 渲染与 TA 资产速查

代码侧 TA/渲染相关系统：

```text
UClothLODControllerComponent
  默认 LODProfiles:
    1000cm  Blend=1.0  MaxDistanceScale=1.0  Simulation=true
    2500cm  Blend=0.7  MaxDistanceScale=0.7  Simulation=true
    4000cm  Blend=0.3  MaxDistanceScale=0.4  Simulation=true
    6000cm  Blend=0.0  MaxDistanceScale=0.0  Simulation=false
  EvaluationInterval=0.15
  BlendSpeed=4.0
  TeleportThreshold=500
  bDisableWhenHidden=true

UWindSimulationComponent
  BasicGlobalWind=(300,300,200)
  MovementWindScale=-0.00001
  WindInterpSpeed=1.2
  MaxVelocityWindForce=150
  NoiseSpatialScale=0.01
  NoiseTimeScale=1.0
  NoiseIntensity=95

UCharacterAnimInstance
  输出 PhysicsAlpha、KawaiiWind、RuntimeHairDamping、HairRunBounceForce。
```

可见材质/特效资产：

```text
SoStylized 主环境材质:
  Content/SoStylized/Environment/Foliage/Materials/M_Foliage.uasset
  Content/SoStylized/Environment/Landscape/Materials/M_Landscape.uasset
  Content/SoStylized/Environment/Trees/Materials/M_Bark.uasset
  Content/SoStylized/Environment/Trees/Materials/M_Leaves.uasset
  Content/SoStylized/Environment/Water/Materials/M_Water.uasset
  Content/SoStylized/Environment/Water/Materials/M_Waterfall.uasset
  Content/SoStylized/Environment/Water/Materials/M_UnderwaterPP.uasset

SoStylized 关键材质函数:
  Content/SoStylized/Environment/Foliage/Materials/MF_FoliageWind.uasset
  Content/SoStylized/Environment/Foliage/Materials/MF_FoliageInteraction.uasset
  Content/SoStylized/Environment/Trees/Materials/MF_TreeSway.uasset
  Content/SoStylized/Environment/Landscape/Materials/MF_WindColor.uasset
  Content/SoStylized/Materials/MF_Grass.uasset
  Content/SoStylized/Materials/MF_Moss.uasset
  Content/SoStylized/Materials/MF_Occlusion.uasset
  Content/SoStylized/Materials/MF_Sparkle.uasset

Niagara / VFX:
  Content/SoStylized/Effects/NS_WaterRipple.uasset
  Content/SoStylized/Environment/Misc/NS_WindLines.uasset
  Content/SoStylized/Environment/Water/NS_WaterfallSplash.uasset
  Content/LevelPrototyping/Interactable/JumpPad/NS_JumpPad.uasset

LevelPrototyping 材质:
  Content/LevelPrototyping/Materials/M_FlatCol.uasset
  Content/LevelPrototyping/Materials/M_PrototypeGrid.uasset
  Content/LevelPrototyping/Materials/MF_ProcGrid.uasset
  Content/LevelPrototyping/Interactable/JumpPad/Assets/Materials/M_GradientGlow.uasset
  Content/LevelPrototyping/Interactable/JumpPad/Assets/Materials/M_SimpleGlow.uasset
  Content/LevelPrototyping/Interactable/JumpPad/Assets/Materials/MI_GlowNT.uasset
```

性能提示：

```text
布料模拟已具备距离 LOD、可见性关闭、Mesh LOD 融合、传送重置和定时器错峰。
风力组件当前每帧计算 Perlin/Sine 噪声，并每 0.5 秒输出日志；调性能时优先检查日志频率和 Noise 计算开销。
SoStylized 环境资产较多，默认地图使用 Lumen 与 MegaLights 配置，场景性能排查优先看 GPU Profiler 中 Lumen、Virtual Shadow Map、透明水体和 Niagara。
```

## 8. 编码约定与模式

命名前缀规律：

```text
A*     Actor 或 Character，如 AEscapeGameCharacter、APickupData、AInteractDoor。
U*     UObject、组件、Widget、DataAsset，如 UInventoryComponent、UGameHUDWidget、UItemDefinition。
F*     USTRUCT 数据结构，如 FItemData、FItemStack、FCombatRuntimeState、FCombatActionDefinition。
E*     UENUM 枚举，如 EItemType、ECharacterState、ECombatState、ECurveMathModel。
I*     接口实现侧，如 IInteractableInterface、IPickupInterface。
```

命名习惯：

```text
*Component       角色或 Actor 组件，如 SprintComponent、InventoryComponent、EscapeCombatComponent。
*Widget          UMG 控件，如 GameHUDWidget、InventorySlotWidget。
ItemAction_*     物品效果逻辑资产子类。
WeaponDefinition / DA_Weapon_*  武器数据资产与生成资产命名。
M_* / SM_* / DA_*  武器导入工具生成材质、Static Mesh 和 DataAsset 时遵循 UE 常见前缀。
*Interface       UINTERFACE 文件与类型。
Escape*          项目核心类型和 GameplayTag 命名空间。
```

需要保留的历史拼写：

```text
UInterectComponent / InterectComponent.h
  代码中使用的是 Interect 而不是 Interact。未来重命名前要同步所有 include、蓝图引用和绑定点。

bIsActurallySprinting
  当前代码中拼写为 Acturally，不是 Actually。重命名会影响引用和蓝图暴露时机，需谨慎。
```

主要设计模式：

```text
组件化角色:
  AEscapeGameCharacter 作为聚合入口和输入分发中心，移动、体力、背包、交互、战斗等功能拆在组件中。

观察者模式:
  生命、耐力、背包、状态、战斗命中和背包开关通过动态多播委托通知 UI 或控制器。

接口解耦:
  UInterectComponent 不关心具体对象类型，只检查 UPickupInterface / UInteractableInterface。
  UEscapeCombatComponent 不关心受击对象类型，只检查 UEscapeCombatDamageable。

数据驱动动作:
  GameplayTag -> FCombatActionDefinition，动作 Montage、伤害、Trace、连击链路和反馈参数在 UCharacterAnimData 中配置。

数据驱动武器:
  UWeaponDefinition 驱动装备 Mesh、挂载 Socket、Trace Socket、TraceRadius、BaseDamage 和 DamageTypeTag；UEscapeCombatComponent 在 DoAttackTrace 与 ANS 连续 Trace 中优先读取装备武器。

运行时状态封装:
  UEscapeCombatComponent 使用 FCombatRuntimeState 集中管理当前动作、ActiveTags、CurrentActionTags、HitActorsThisAction、输入缓存和 Timer。
  ANS 连续攻击判定使用 FAttackTraceInst 管理是否激活、上一帧 Start/End 点和当前 DamageSourceBone，避免把临时点位散落在组件成员中。

基础 AI 受击:
  AAICharacter_1 通过 IEscapeCombatDamageable 接入战斗伤害，用 UAttributeComponent 计算生命，死亡时负责停止 AIController、关闭碰撞并写 Blackboard 状态。

策略/多态物品效果:
  UItemDefinition::OnUse 由 UItemAction_* 子类覆盖，背包组件只负责库存和调用逻辑。

状态机:
  UStateMachineComponent 用 ECharacterState 保护基础角色状态，USprintComponent 只负责冲刺状态的进入和退出。

Timer 驱动:
  战斗长按判定、速度 Buff、眩晕恢复、布料 LOD 评估都使用 Timer。

防御性日志:
  空 Owner、缺组件、非法数量、错误状态、受击无效、缺钥匙、UI 初始化失败等早退分支按当前编码规则在 return 前输出 UE_LOG；高频或非错误路径使用 VeryVerbose 降低噪声。

UI 数据绑定:
  InventoryComponent 是数据源，Menu/Hotbar/Slot Widgets 监听刷新或调用组件方法。

拖拽载荷对象:
  UInventoryDragDropOperation 携带 SourceSlotIndex 和 SourceComponent，由目标 Slot 调用 SwapSlots。

Editor 自动化资产导入:
  UWeaponImportToolLibrary 用 AssetTools 导入 OBJ/PNG，用 AssetRegistry 注册新材质和 WeaponDefinition，并通过 UEditorLoadingAndSavingUtils 保存资产包。
```

TODO 与注释扩展点：

```text
没有发现源码字面量 TODO/FIXME。

EscapeCombatComponent.cpp
  TryPlayActionByTagInternal 中仍有“进阶预留位”，后续可加入更完整的体力、冷却、状态和动作优先级检查。
  当前 BeginPlay 绑定 OnMontageEnded，但源码仍未看到 EndPlay 清理；生命周期修复时需要解绑委托并清理 RuntimeState.InputBufferTimer。
  当前同时保留 Legacy 单帧 DoAttackTrace 和 ANS 连续 Trace；迁移动画 Notify 时优先使用 BeginAttackTrace/TickAttackTrace/EndAttackTrace。

InventorySlotWidget.cpp
  当前 NativeOnDrop 只处理同一个背包内部交换。未来做箱子/容器时，可用 SourceComponent != OwnerComponent 分支处理跨容器移动。

AICharacter_1.cpp
  ApplyDamage_Implementation 当前缺少 AttributeComp 缺失、无效伤害等早退日志；如果继续按编码规则细化，可补 UE_LOG，但不要改变受击/死亡流程。

WeaponImportToolLibrary.cpp
  ImportWeaponFromObjFolder 当前只自动建立 Diffuse -> BaseColor 的基础材质，Lightmap 仅导入并保存，尚未接入材质图。
```

## 9. Agent 操作建议

1. 修改战斗动作、连击、蓄力或伤害 Trace 时，优先看 `UEscapeCombatComponent`、`FCombatRuntimeState`、`FAttackTraceInst`、`UCharacterAnimData`、`UWeaponDefinition` 和 `EscapeGameplayTags`，不要把战斗判断写回 `AEscapeGameCharacter`。

2. 修改玩家受击、死亡或击退时，入口是 `AEscapeGameCharacter::ApplyDamage_Implementation(const FCombatDamageContext&)`，生命数值在 `UAttributeComponent`，死亡状态在 `UStateMachineComponent`，返回结果要维护 `FCombatDamageResult`。

3. 调整攻击手感时，优先改 `UCharacterAnimData` 中 `FCombatActionDefinition` 的 Montage、PlayRate、DamageMultiplier、Knockback、Launch、NextComboTag 和输入容忍时间；如果是武器长度、判定半径或基础伤害，优先改 `UWeaponDefinition`。

4. 如果扩展战斗状态，优先给 `FCombatRuntimeState` 增加明确字段或封装函数；如果扩展连续攻击判定，优先扩展 `FAttackTraceInst`，避免在组件里散落多个 bool、Tag 和临时点位。

5. 当前 `UEscapeCombatComponent` 在 BeginPlay 绑定 `OnMontageEnded`，但源码中没有 EndPlay 清理；做生命周期修复时应解绑 Montage 委托并清理 `InputBufferTimer`。

6. 当前动作播放使用 `TSoftObjectPtr::LoadSynchronous()`；如果战斗中出现卡顿，优先考虑预加载 Montage 或改为异步加载并缓存。

7. 新 Montage 的攻击判定优先接 `BeginAttackTrace/TickAttackTrace/EndAttackTrace` 这组 ANS 风格入口；`DoAttackTrace` 当前保留给旧 `AN_MeleeAttackTrace`，不要贸然删除。

8. 修改物品效果时，优先新增或调整 `UItemDefinition` 子类，例如 `Items/ItemAction_Healing`；修改武器数据时使用 `UWeaponDefinition`，背包组件只负责库存和调用 `OnUse`。

9. 做拾取、门、机关等世界交互时，实现 `UPickupInterface` 或 `UInteractableInterface`，并确认碰撞能被 `UInterectComponent` 的 `ECC_WorldDynamic` 球形 Sweep 命中。

10. 改 UI 刷新时，沿用动态委托链路：属性/耐力/背包组件广播，Widget 监听刷新。不要在 Tick 中轮询 UI 数据。

11. 处理移动速度、冲刺、眩晕或死亡状态时，同时检查 `USprintComponent` 和 `UStateMachineComponent`。冲刺组件会持续平滑写入 `CharacterMovement->MaxWalkSpeed`，单独改 CharacterMovement 可能被下一帧覆盖。

12. 修改敌人受击或死亡逻辑时，当前入口是 `AAICharacter_1::ApplyDamage_Implementation` 和 `HandleDeath`；如果接入更复杂 AI，注意同步 Blackboard Key（当前是 `bIsdead` 和 `TargetActor`）与碰撞/生命周期。

13. 修改布料、头发、飘带等效果时，同时看 `UClothLODControllerComponent`、`UWindSimulationComponent` 和 `UCharacterAnimInstance`。跨线程动画更新只能读取缓存数据，不要在 `NativeThreadSafeUpdateAnimation` 里访问组件。

14. 做 TA 批处理或曲线生成时，优先复用 `UUTACurveToolLibrary::BakeMathToCurve` 和 `Content/ToolTest/EUV_CurveGenerator`，不要直接在运行时 Tick 里临时生成曲线。

15. 做武器 OBJ 导入时，优先复用 `UWeaponImportToolLibrary::ImportWeaponFromObjFolder` 和 `/Game/EditorTools/EUW_WeaponImporter`；导入后必须检查武器 Static Mesh 的 `TraceStart` / `TraceEnd` Socket，并把生成的 `DA_Weapon_*` 配到角色 `DefaultWeaponDefinition`。

16. 继续 Dialogue 系统时，先读 `Docs/Plan/dialogue_folder_architecture_overview.md` 和 `Docs/Plan/dialogue_select_option_execution_plan.md`，再看 `UDialogueQuestSubsystem`；当前学习切片暂停在 `SelectOption` 最小跳转闭环。

17. 修改 `SelectOption` 时只处理“找选项、记录选择、跳节点、关闭对话”，不要顺手接入 `EvaluateCondition`、`ApplyEffect`、任务、奖励、存档或 UI。

18. `DialogueRuntimeMap` 的 Key 是 `ActiveSession.DialogueID`，不要用 `OptionID` 或 `NodeID` 作为该 Map 的 Key；选项历史应写入 `FDialogueRuntimeState::VisitedOptionIDs`。

19. 当前 `Tests/DialogueQuestSubsystemTest.cpp` 应优先改用 `Core/EscapeGameplayTags.h` 中已经注册的 Native Tag，避免 `FGameplayTag::RequestGameplayTag` 请求未注册测试 Tag 导致测试不稳定。

20. 对话系统后续小切片建议顺序：`SelectOption` 跳转闭环 -> `GetAvailableOptions` 最小条件筛选 -> `ApplyEffect` 最小 `CloseDialogue` / `SetGlobalFlag` -> 最小 Dialogue UI -> `StartQuest` -> 存档映射 -> Boss / Encounter 强制对白。
