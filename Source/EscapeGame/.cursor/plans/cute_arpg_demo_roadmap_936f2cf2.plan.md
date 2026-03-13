---
name: ""
overview: ""
todos: []
isProject: false
---

---

name: Cute ARPG Demo Roadmap
overview: "萌系 ARPG 技术 Demo"工业级技术设计文档。从当前代码现状出发，制定从地基修复到可展示作品集的完整开发路线，覆盖架构设计、接口规格、数据结构、渲染方案、性能指标、验收标准和风险预案。
todos:

- id: phase0-bugfix
content: "P0 地基修复：修复 8 个已知崩溃/逻辑 Bug，清理调试输出"
status: pending
- id: phase1a-interface
content: "P1-A 战斗接口层：EscapeGameCharacter 实现 ICombatAttacker + ICombatDamageable"
status: pending
- id: phase1b-combat-core
content: "P1-B 战斗核心：EscapeCombatComponent 扩展（状态机/输入缓冲/Combo/Montage回调）"
status: pending
- id: phase1c-data-extend
content: "P1-C 数据扩展：FActionDefinition 补全战斗参数 + FDamageRequest 伤害数据结构"
status: pending
- id: phase1d-hit-feedback
content: "P1-D 打击反馈：闪白/顿帧/CameraShake/Niagara 命中特效"
status: pending
- id: phase1e-animation
content: "P1-E 动画资产：3段轻攻连招 + 蓄力重攻 Montage + AnimNotify 配置"
status: pending
- id: phase1f-dodge
content: "P1-F 闪避系统：翻滚动画 + 无敌帧 + 体力消耗 + 状态守卫"
status: pending
- id: phase2a-skin
content: "P2-A 皮肤渲染：Subsurface Profile + 双层法线 + 粗糙度分区"
status: pending
- id: phase2b-hair
content: "P2-B 头发渲染：Hair Shading Model + Alpha 处理 + 物理LOD统一"
status: pending
- id: phase2c-cloth-eye
content: "P2-C 衣物/眼睛：Cloth Shading Model + Eye 着色 + Master Material 架构"
status: pending
- id: phase2d-vfx-upgrade
content: "P2-D 战斗特效升级：Ribbon 拖尾 + 冲击波折射 + Decal + 描边系统"
status: pending
- id: phase2e-scene
content: "P2-E 场景氛围：Post Process 调色 + Lumen GI + 光照布局"
status: pending
- id: phase3a-level
content: "P3-A 关卡搭建：三区域微型关卡（安全区/战斗区/Boss区）"
status: pending
- id: phase3b-enemy
content: "P3-B 敌人系统：1 种小怪 + 1 种 Boss（复用 StateTree AI 框架）"
status: pending
- id: phase3c-flow
content: "P3-C 流程管理：波次触发/死亡重生/胜利结算"
status: pending
- id: phase4-polish
content: "P4 打磨：性能分析 + 展示视频录制 + 技术文档撰写"
status: pending
isProject: false

---

# EscapeGame — 萌系 ARPG 技术 Demo 设计文档 (TDD)

文档版本：v1.0
项目代号：EscapeGame
引擎版本：Unreal Engine 5
目标平台：Windows PC (DirectX 12)

---

## 1. 项目概述

### 1.1 产品定位

一个可玩时长 3-5 分钟的"萌系 ARPG"技术 Demo，作为个人作品集的核心展示项目。通过一个完整的微型关卡，集中展示以下技术能力：

- C++ Gameplay 架构设计（组件化、数据驱动、接口解耦）
- 实时角色渲染技术（PBR、SSS 皮肤、Hair 着色、Cloth 着色）
- 动作游戏战斗系统（连招、蓄力、闪避、打击反馈）
- 角色物理模拟（头发 Kawaii Physics、布料 LOD 控制、风力系统）
- 性能优化工程（无 Tick 组件、定时器驱动、LOD 融合、粒子优化）

### 1.2 核心体验描述

> 玩家操控角色 Yuno，在一个中世纪风格的小型场景中探索。
> 拾取武器和道具后进入战斗区域，面对 2-3 波小怪的挑战。
> 最终抵达 Boss 区域，击败 Boss 完成 Demo。
> 全程展示角色渲染品质、流畅的连招打击感和完整的系统串联。

### 1.3 技术关键指标 (KPI)

- 目标帧率：60 FPS 稳定（1080p，RTX 2060 级别硬件）
- Draw Call：单帧 < 2000
- 角色材质 Shader Complexity：黄色区域以内（Viewport 可视化）
- 特效 Overdraw：战斗高峰期不超过 4x
- 布料物理 LOD 切换无视觉跳变
- 内存占用：< 2GB GPU Memory

---

## 2. 现有系统资产清单

### 2.1 代码资产

**核心角色层**


| 文件                            | 类名                       | 状态  | 说明                                                         |
| ----------------------------- | ------------------------ | --- | ---------------------------------------------------------- |
| `EscapeGameCharacter.h/cpp`   | `AEscapeGameCharacter`   | 可用  | 主角：移动/跳跃/下蹲/视角切换/输入绑定                                      |
| `CharacterAnimInstance.h/cpp` | `UCharacterAnimInstance` | 可用  | AnimBP 数据桥：Kawaii Wind/PhysicsAlpha/DynamicDamping         |
| `CharacterAnimData.h/cpp`     | `UCharacterAnimData`     | 可用  | DataAsset：Locomotion BlendSpace + ActionMap (Tag->Montage) |
| `EscapeCombatComponent.h/cpp` | `UEscapeCombatComponent` | 骨架  | 仅有 TryPlayActionByTag，缺少全部战斗逻辑                             |


**组件层**


| 文件                                  | 类名                             | 状态       | 已知问题                                                 |
| ----------------------------------- | ------------------------------ | -------- | ---------------------------------------------------- |
| `SprintComponent.h/cpp`             | `USprintComponent`             | 可用(有Bug) | StaminaRegenDelay 未初始化；StaminaChange 硬编码 100；调试输出未清理 |
| `InventoryComponent.h/cpp`          | `UInventoryComponent`          | 可用(有Bug) | AddItem 背包满崩溃；RemoveItem 提前 break + 无广播              |
| `StateMachineComponent.h/cpp`       | `UStateMachineComponent`       | 可用       | Attacking 状态从未被使用                                    |
| `AttributeComponent.h/cpp`          | `UAttributeComponent`          | 可用(有Bug) | ApplyHealthChange 硬编码 100                            |
| `WindSimulationComponent.h/cpp`     | `UWindSimulationComponent`     | 不工作      | CachedOwner 未在 BeginPlay 赋值                          |
| `ClothLODControllerComponent.h/cpp` | `UClothLODControllerComponent` | 可用       | 新建，未经运行时验证                                           |
| `InterectComponent.h/cpp`           | `UInterectComponent`           | 可用       | 类名拼写错误 (Interect→Interact)                           |


**数据层**


| 文件                         | 类名                                     | 状态                    |
| -------------------------- | -------------------------------------- | --------------------- |
| `ItemData.h`               | `FItemData`, `FItemStack`, `EItemType` | 可用                    |
| `ItemDefinition.h/cpp`     | `UItemDefinition`                      | 可用，CoolDownTime 字段未使用 |
| `EscapeGameplayTags.h/cpp` | `FEscapeGameplayTags`                  | 可用，已定义完整战斗标签体系        |


**模板参考层（Variant_Combat）**


| 文件                              | 可复用价值                                             |
| ------------------------------- | ------------------------------------------------- |
| `CombatCharacter.h/cpp`         | DoAttackTrace Sweep 逻辑、Combo 流程、顿帧逻辑              |
| `CombatEnemy.h/cpp`             | AI 攻击模式、StateTree 集成                              |
| `ICombatAttacker`               | 攻击接口（DoAttackTrace/CheckCombo/CheckChargedAttack） |
| `ICombatDamageable`             | 受伤接口（ApplyDamage/HandleDeath/ApplyHealing）        |
| `AnimNotify_DoAttackTrace`      | 攻击检测 AnimNotify — 通过接口调用，可零修改复用                   |
| `AnimNotify_CheckCombo`         | Combo 窗口 AnimNotify — 通过接口调用，可零修改复用               |
| `AnimNotify_CheckChargedAttack` | 蓄力检测 AnimNotify — 通过接口调用，可零修改复用                   |


### 2.2 已知缺陷总表


| ID      | 严重度      | 文件                               | 描述                                                        | 修复方案                                 |
| ------- | -------- | -------------------------------- | --------------------------------------------------------- | ------------------------------------ |
| BUG-001 | **崩溃**   | `InventoryComponent.cpp:80`      | AddItem 背包满时 EmptySlotIndex=-1 访问 Items[-1]               | 检测 -1 后 break 跳出 while               |
| BUG-002 | **崩溃**   | `ItemAction_Healing.cpp:13`      | TargetActor 为 null 时无 return，继续解引用                        | 在 UE_LOG 后加 return false             |
| BUG-003 | **功能失效** | `WindSimulationComponent.cpp:35` | BeginPlay 中 CachedOwner 未赋值，TickComponent 每帧 early-return | BeginPlay 加 CachedOwner = GetOwner() |
| BUG-004 | 逻辑错误     | `InventoryComponent.cpp:152`     | RemoveItem 的 else 分支 break 应为 continue                    | 改为 continue；函数尾部补广播                  |
| BUG-005 | 逻辑错误     | `AttributeComponent.cpp:39`      | Clamp 后又硬编码 if(>100)                                      | 删除硬编码行                               |
| BUG-006 | 逻辑错误     | `SprintComponent.cpp:221`        | StaminaChange 硬编码 100.0f                                  | 改为 MaxStamina                        |
| BUG-007 | 未定义行为    | `SprintComponent.h:76`           | StaminaRegenDelay 未初始化                                    | 头文件中设默认值 = 0.0f                      |
| BUG-008 | 性能/发布    | `SprintComponent.cpp` 多处         | GEngine->AddOnScreenDebugMessage 每帧调用                     | 包裹 #if !UE_BUILD_SHIPPING 或删除        |


---

## 3. 阶段 0：地基修复

**工期估算**：3-5 天
**前置依赖**：无
**目标**：消除所有已知崩溃和逻辑错误，获得一个稳定的开发基础。

### 3.0.1 修复清单

逐条修复上表 BUG-001 至 BUG-008。每条修复后需：

- 编译通过（零 Warning 零 Error）
- 手动验证修复生效（例如 BUG-001：背包填满后继续捡物品不崩溃）
- 确认未引入新问题（修改周边逻辑后运行相关功能）

### 3.0.2 补充修复（低优先级但建议一并处理）


| 项目                                       | 文件                                      | 描述                                                     |
| ---------------------------------------- | --------------------------------------- | ------------------------------------------------------ |
| 门钥匙未消耗                                   | `InteractDoor.cpp:62`                   | bConsumeKey=true 时花括号内为空，应调用 InventoryComp->RemoveItem |
| SprintComponent 体力耗尽时速度未乘 BuffMultiplier | `SprintComponent.cpp:97`                | WalkSpeed 应乘以 CurrentBuffMultiplier                    |
| EscapeGameCharacter 构造函数重复设置             | `EscapeGameCharacter.cpp:35-36 与 76-77` | 删除重复的 bOrientRotationToMovement 和 RotationRate         |


### 3.0.3 验收标准

- 背包填满 30 格后继续捡物品：不崩溃，提示"背包满"
- 对 null Actor 使用治疗道具：不崩溃，返回 false
- 角色站立不动 5 秒：风力系统正常影响头发运动（非每帧报错）
- 删除物品后 UI 正确刷新
- 体力上限通过 ApplyMaxChange 提高到 150 后，恢复体力可超过 100
- Shipping 构建中无屏幕调试文字

---

## 4. 阶段 1：核心战斗循环

**工期估算**：3-4 周
**前置依赖**：阶段 0 完成
**目标**：在空白关卡中实现完整的攻击-命中-伤害-死亡循环，具备基础打击感。

### 4.1 模块 1A：战斗接口层

**目标**：让 `AEscapeGameCharacter` 具备攻击和受伤能力，复用模板 AnimNotify。

**4.1.1 类声明修改**

文件：`EscapeGameCharacter.h`

修改继承列表：

```cpp
class AEscapeGameCharacter : public ACharacter,
                              public ICombatAttacker,
                              public ICombatDamageable
```

新增头文件引用：

```cpp
#include "Variant_Combat/Interfaces/CombatAttacker.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"
```

**4.1.2 接口方法实现规格**


| 接口方法                 | 签名                                                                 | 对接目标                  | 实现要点                                          |
| -------------------- | ------------------------------------------------------------------ | --------------------- | --------------------------------------------- |
| `DoAttackTrace`      | `void DoAttackTrace(FName DamageSourceBone)`                       | EscapeCombatComponent | 从组件获取当前动作的 Trace 参数，执行 SweepMultiByObjectType |
| `CheckCombo`         | `void CheckCombo()`                                                | EscapeCombatComponent | 检查输入缓冲时间戳，决定是否跳转到下一段 Combo                    |
| `CheckChargedAttack` | `void CheckChargedAttack()`                                        | EscapeCombatComponent | 检查蓄力按键是否持续按住，决定循环或释放                          |
| `ApplyDamage`        | `void ApplyDamage(float, AActor*, const FVector&, const FVector&)` | AttributeComponent    | 扣血 + 击退 + 触发受击特效 + 检查死亡                       |
| `HandleDeath`        | `void HandleDeath()`                                               | StateMachineComponent | SetState(Dead) + 禁止移动 + 播放死亡动画                |
| `ApplyHealing`       | `void ApplyHealing(float, AActor*)`                                | AttributeComponent    | ApplyHealthChange(+Healing)                   |


**4.1.3 DoAttackTrace 详细实现规格**

```
输入：
  FName DamageSourceBone — 攻击判定起点的骨骼/Socket名称

执行流程：
  1. 从 EscapeCombatComponent 获取当前活跃动作的 FActionDefinition
  2. 读取 TraceDistance, TraceRadius, DamageMultiplier, KnockbackImpulse, LaunchImpulse
  3. 计算 TraceStart = GetMesh()->GetSocketLocation(DamageSourceBone)
  4. 计算 TraceEnd = TraceStart + GetActorForwardVector() * TraceDistance
  5. 构建 FCollisionShape::MakeSphere(TraceRadius)
  6. FCollisionQueryParams: AddIgnoredActor(this)
  7. SweepMultiByObjectType(OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, Shape, Params)
     ObjectParams: ECC_Pawn + ECC_WorldDynamic
  8. 遍历 OutHits:
     a. Cast<ICombatDamageable>(HitActor)
     b. 如果命中：
        - 计算 Impulse = ImpactNormal * -KnockbackImpulse + UpVector * LaunchImpulse
        - 调用 Damageable->ApplyDamage(BaseDamage * DamageMultiplier, this, ImpactPoint, Impulse)
        - 调用 PlayHitImpactFX(ImpactPoint, ImpactNormal) — 生成命中特效
        - 调用 TriggerHitStop() — 触发顿帧
```

**4.1.4 ApplyDamage 详细实现规格**

```
输入：
  float Damage — 伤害数值
  AActor* DamageCauser — 攻击来源
  const FVector& DamageLocation — 命中点世界坐标
  const FVector& DamageImpulse — 击退力向量

执行流程：
  1. 检查 StateMachineComponent 当前状态
     - 如果是 Dead → return（死人不再受伤）
     - 如果有 State_Status_Invincible 标签 → return（无敌帧）
  2. AttributeComponent->ApplyHealthChange(-Damage)
  3. 检查血量 <= 0 → HandleDeath()
  4. 否则：
     a. GetCharacterMovement()->AddImpulse(DamageImpulse, true) — 击退
     b. TriggerHitFlash() — 受击闪白
     c. PlayHitReactMontage(DamageImpulse 方向) — 受击动画（可选）
     d. StateMachineComponent->ApplyStun(HitStunDuration) — 短暂硬直（可选）
```

**4.1.5 依赖关系**

```
EscapeGameCharacter.h 需要新增 include：
  - CombatAttacker.h
  - CombatDamageable.h

Build.cs 不需要修改（接口已在现有 include path 中）

AnimNotify_DoAttackTrace/CheckCombo/CheckChargedAttack：
  零修改。它们通过 Cast<ICombatAttacker>(OwningActor) 调用接口。
  只要 EscapeGameCharacter 实现了接口，Notify 自动生效。
```

### 4.2 模块 1B：战斗核心组件扩展

**目标**：将 `UEscapeCombatComponent` 从"动画播放器"升级为完整的战斗状态管理器。

**4.2.1 新增成员变量**、

文件：`EscapeCombatComponent.h`

```
运行时状态：
  bool bIsAttacking = false             — 是否正在攻击动画中
  bool bIsChargingAttack = false        — 是否正在蓄力
  bool bHasLoopedCharge = false         — 蓄力是否已循环过至少一次
  int32 ComboCount = 0                  — 当前连招段数
  float CachedAttackInputTime = 0.0f    — 最近一次攻击输入的时间戳
  FGameplayTag CurrentActionTag         — 当前正在执行的动作 Tag
  FGameplayTagContainer ActiveTags      — 活跃标签容器（Attacking/Invincible等）

配置参数（UPROPERTY EditAnywhere）：
  float AttackInputCacheTolerance = 1.0f   — 非连招输入的有效缓存时间
  float ComboInputCacheTolerance = 0.45f   — 连招窗口内输入的有效缓存时间
  float BaseDamage = 10.0f                  — 基础攻击力
  float DodgeStaminaCost = 20.0f            — 闪避体力消耗
  float DodgeInvincibilityDuration = 0.4f   — 闪避无敌帧持续时间

委托：
  FOnMontageEnded OnAttackMontageEnded  — Montage 结束回调（用于重置状态）

缓存引用（BeginPlay 中获取）：
  ACharacter* OwnerCharacter
  UStateMachineComponent* StateMachine
  USprintComponent* SprintComp
  UAnimInstance* CachedAnimInstance
```

**4.2.2 核心方法规格**

**TryPlayActionByTag（重构）**

```
输入：FGameplayTag ActionTag
返回：void

执行流程：
  1. 前置检查（State Guard）：
     a. if (!CharacterAnimData) return
     b. if (StateMachine->IsState(Dead) || StateMachine->IsState(Stunned)) return
     c. if (bIsAttacking) → 缓存输入时间戳 CachedAttackInputTime = WorldTime, return

  2. 查表：
     const FActionDefinition* ActionDef = CharacterAnimData->ActionMap.Find(ActionTag)
     if (!ActionDef) return

  3. 体力检查：
     if (SprintComp && ActionDef->StaminaCost > 0)
       if (SprintComp->CurrentStamina < ActionDef->StaminaCost) return
       SprintComp->StaminaChange(-ActionDef->StaminaCost)

  4. 状态切换：
     bIsAttacking = true
     ComboCount = 0
     CurrentActionTag = ActionTag
     ActiveTags.AddTag(FEscapeGameplayTags::Get().Action_State_Attacking)
     StateMachine->SetState(ECharacterState::Attacking)
     SprintComp->StopSprinting()  // 攻击时停止冲刺

  5. 加载并播放 Montage：
     UAnimMontage* Montage = ActionDef->Montage.LoadSynchronous()
     // 注意：阶段 4 中改为预加载
     float Duration = OwnerCharacter->PlayAnimMontage(Montage, ActionDef->PlayRate)

  6. 注册结束回调：
     if (Duration > 0 && CachedAnimInstance)
       CachedAnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, Montage)
```

**OnAttackMontageEnded（新增）**

```
触发条件：攻击 Montage 播放完成或被打断

执行流程：
  1. bIsAttacking = false
  2. bHasLoopedCharge = false
  3. ActiveTags.RemoveTag(Action_State_Attacking)
  4. CurrentActionTag = FGameplayTag()

  5. 状态恢复：
     if (StateMachine->IsState(Attacking))
       if (OwnerCharacter->GetVelocity().SizeSquared() > 10.0f)
         StateMachine->SetState(Moving)
       else
         StateMachine->SetState(Idle)

  6. 检查缓冲输入：
     float TimeSinceInput = WorldTime - CachedAttackInputTime
     if (TimeSinceInput <= AttackInputCacheTolerance)
       if (bIsChargingAttack)
         → 执行蓄力攻击
       else
         → 执行普通攻击
```

**HandleCheckCombo（供角色接口调用）**

```
由 AnimNotify_CheckCombo 通过 ICombatAttacker::CheckCombo() 触发

执行流程：
  1. if (!bIsAttacking || bIsChargingAttack) return
  2. float TimeSinceInput = WorldTime - CachedAttackInputTime
  3. if (TimeSinceInput > ComboInputCacheTolerance) return
  4. CachedAttackInputTime = 0  // 消费输入，防止重复触发

  5. 查找下一招：
     const FActionDefinition* CurrentAction = ActionMap.Find(CurrentActionTag)
     if (!CurrentAction || !CurrentAction->NextComboTag.IsValid()) return

  6. 查找下一招的定义：
     const FActionDefinition* NextAction = ActionMap.Find(CurrentAction->NextComboTag)
     if (!NextAction) return

  7. 体力检查（同 TryPlayActionByTag）

  8. 跳转到下一段：
     ComboCount++
     CurrentActionTag = CurrentAction->NextComboTag
     UAnimMontage* NextMontage = NextAction->Montage.LoadSynchronous()
     CachedAnimInstance->Montage_JumpToSection(NextMontage的SectionName, 当前Montage)
     // 或者直接播放新 Montage（取决于动画资产结构）
```

### 4.3 模块 1C：数据结构扩展

**4.3.1 FActionDefinition 扩展**

文件：`CharacterAnimData.h`

```
现有字段（保留）：
  TSoftObjectPtr<UAnimMontage> Montage
  float PlayRate = 1.0f
  float DamageMultiplier = 1.0f

新增字段：
  // --- 命中检测 ---
  UPROPERTY(EditDefaultsOnly, Category = "Trace")
  float TraceDistance = 75.0f           // Sweep 前方延伸距离 (cm)

  UPROPERTY(EditDefaultsOnly, Category = "Trace")
  float TraceRadius = 60.0f            // Sweep 球体半径 (cm)

  UPROPERTY(EditDefaultsOnly, Category = "Trace")
  FName TraceBone = "weapon_r"          // Sweep 起点骨骼/Socket

  // --- 击退 ---
  UPROPERTY(EditDefaultsOnly, Category = "Knockback")
  float KnockbackImpulse = 200.0f      // 水平击退力 (cm/s)

  UPROPERTY(EditDefaultsOnly, Category = "Knockback")
  float LaunchImpulse = 150.0f          // 垂直击飞力 (cm/s)

  // --- 消耗 ---
  UPROPERTY(EditDefaultsOnly, Category = "Cost")
  float StaminaCost = 0.0f             // 体力消耗（0=不消耗）

  // --- 连招路由 ---
  UPROPERTY(EditDefaultsOnly, Category = "Combo")
  FGameplayTag NextComboTag             // 下一段连招的 Tag（空=终结招）

  // --- 打击反馈 ---
  UPROPERTY(EditDefaultsOnly, Category = "Feedback")
  float HitStopDuration = 0.05f        // 顿帧时长 (秒)

  UPROPERTY(EditDefaultsOnly, Category = "Feedback")
  float CameraShakeScale = 0.5f        // 震屏强度 (0-1)

  UPROPERTY(EditDefaultsOnly, Category = "Feedback")
  TSoftObjectPtr<UNiagaraSystem> HitImpactEffect  // 命中特效

  // --- 状态标签 ---
  UPROPERTY(EditDefaultsOnly, Category = "Tags")
  FGameplayTagContainer GrantedTags     // 攻击期间赋予的标签（如霸体）

  UPROPERTY(EditDefaultsOnly, Category = "Tags")
  bool bCanBeInterrupted = true         // 是否可被打断
```

**4.3.2 Combo 路由数据示例**

```
在 CharacterAnimData DataAsset 中配置：

Tag: Action.Combat.Light.1
  Montage: AM_Light_01
  DamageMultiplier: 1.0
  TraceDistance: 75
  NextComboTag: Action.Combat.Light.2  ← 指向下一段
  HitStopDuration: 0.03
  CameraShakeScale: 0.3

Tag: Action.Combat.Light.2
  Montage: AM_Light_02
  DamageMultiplier: 1.2
  TraceDistance: 80
  NextComboTag: Action.Combat.Light.3  ← 指向下一段
  HitStopDuration: 0.04
  CameraShakeScale: 0.4

Tag: Action.Combat.Light.3
  Montage: AM_Light_03
  DamageMultiplier: 1.5
  TraceDistance: 100
  NextComboTag: (空)                   ← 终结招，无后续
  HitStopDuration: 0.08
  CameraShakeScale: 0.7

Tag: Action.Combat.Heavy.Charge
  Montage: AM_Heavy_Charge
  DamageMultiplier: 3.0
  StaminaCost: 25.0
  HitStopDuration: 0.12
  CameraShakeScale: 1.0
```

### 4.4 模块 1D：打击反馈系统

**4.4.1 受击闪白**

```
实现位置：可在 EscapeGameCharacter 中直接实现，或新建 UCombatFXComponent

数据：
  UPROPERTY(Transient)
  TArray<UMaterialInstanceDynamic*> CachedMIDs  // 缓存所有材质槽的 MID
  FTimerHandle HitFlashTimer
  float HitFlashDecaySpeed = 15.0f

初始化（BeginPlay 中）：
  遍历 GetMesh()->GetNumMaterials()
  对每个槽位 CreateDynamicMaterialInstance()
  存入 CachedMIDs

触发（TriggerHitFlash）：
  for (UMaterialInstanceDynamic* MID : CachedMIDs)
    MID->SetScalarParameterValue("HitFlashIntensity", 1.0f)
  启动 Timer (0.016s 间隔, looping)

Timer 回调（UpdateHitFlash）：
  float Current = 当前 HitFlashIntensity
  float New = FMath::FInterpTo(Current, 0.0f, 0.016f, HitFlashDecaySpeed)
  for (MID : CachedMIDs)
    MID->SetScalarParameterValue("HitFlashIntensity", New)
  if (New < 0.01f) → ClearTimer + SetScalarParameterValue(0.0f)

材质端：
  所有角色材质的 Parent Material 中需要添加：
  Scalar Parameter "HitFlashIntensity" (默认 0)
  Final Color = Lerp(BaseColor, FlashColor, HitFlashIntensity)
  FlashColor = Lerp(白色, Fresnel发光色, Fresnel)
```

**4.4.2 顿帧 (Hit Stop)**

```
实现：
  void TriggerHitStop(float Duration)
  {
      OwnerCharacter->CustomTimeDilation = 0.01f;
      
      // 目标也暂停（如果有的话）
      // TargetActor->CustomTimeDilation = 0.01f;
      
      FTimerHandle HitStopTimer;
      GetWorld()->GetTimerManager().SetTimer(
          HitStopTimer,
          [this]() {
              if (OwnerCharacter)
                  OwnerCharacter->CustomTimeDilation = 1.0f;
          },
          Duration,
          false
      );
  }

注意事项：
  - CustomTimeDilation 只影响该 Actor 的动画/移动/物理
  - UI、音乐、其他 Actor 不受影响
  - Duration 典型值：轻攻击 0.03-0.05s，重攻击 0.08-0.12s
  - 需要在角色死亡/受击时清除残留的 HitStopTimer
```

**4.4.3 Camera Shake**

```
资产：
  创建 UCameraShakeBase 蓝图子类：
  - BP_CameraShake_Light（轻攻击用）
    Duration = 0.1s, LocationAmplitude = (1,1,0.5), RotationAmplitude = (0.5,0.5,0)
  - BP_CameraShake_Heavy（重攻击用）
    Duration = 0.2s, LocationAmplitude = (3,3,1), RotationAmplitude = (1.5,1.5,0)

触发：
  APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
  if (PC)
      PC->ClientStartCameraShake(ShakeClass, ActionDef->CameraShakeScale);
```

**4.4.4 命中粒子特效**

```
Niagara System 结构：NS_HitImpact_Basic

  Emitter 1: "Flash" — 核心闪光
    Renderer: Sprite (面向摄像机)
    Spawn: 1 个粒子
    Lifetime: 0.04s
    Size: 50→0 (快速缩小)
    Color: 白色→黄色, Alpha 1→0
    Material: Unlit + Additive, 径向渐变贴图

  Emitter 2: "Sparks" — 火花飞溅
    Renderer: Sprite (按速度方向拉伸)
    Spawn: 15-25 个粒子 (Burst)
    Lifetime: 0.15-0.3s
    InitialVelocity: 沿 ImpactNormal 半球方向, 速度 300-800
    Gravity: -980
    Drag: 2.0
    Size: 3-8, 随生命衰减
    Color: 黄→橙→暗红, Alpha 渐出
    Material: Unlit + Additive, 拉伸光条贴图

触发代码：
  UNiagaraFunctionLibrary::SpawnSystemAtLocation(
      GetWorld(),
      ActionDef->HitImpactEffect.LoadSynchronous(),
      HitResult.ImpactPoint,
      HitResult.ImpactNormal.Rotation(),
      FVector(1.0f),
      true,  // bAutoDestroy
      true   // bAutoActivate
  );
```

### 4.5 模块 1E：动画资产需求


| 动画                | Tag                        | 说明          | Notify 配置                                    |
| ----------------- | -------------------------- | ----------- | -------------------------------------------- |
| AM_Light_01       | Action.Combat.Light.1      | 轻攻击第1段（横斩）  | DoAttackTrace(weapon_r) + CheckCombo         |
| AM_Light_02       | Action.Combat.Light.2      | 轻攻击第2段（上挑）  | DoAttackTrace(weapon_r) + CheckCombo         |
| AM_Light_03       | Action.Combat.Light.3      | 轻攻击第3段（重劈）  | DoAttackTrace(weapon_r)                      |
| AM_Heavy_Charge   | Action.Combat.Heavy.Charge | 蓄力攻击（循环+释放） | CheckChargedAttack(循环点) + DoAttackTrace(释放点) |
| AM_Dodge          | Action.Combat.Dodge        | 闪避翻滚        | AnimNotify 标记无敌帧起止                           |
| AM_HitReact_Front | (无Tag)                     | 正面受击反应      | 无                                            |
| AM_HitReact_Back  | (无Tag)                     | 背后受击反应      | 无                                            |
| AM_Death          | (无Tag)                     | 死亡倒地        | 无                                            |


### 4.6 模块 1F：闪避系统

```
输入绑定：
  新增 UInputAction* DodgeAction（EscapeGameCharacter.h）
  绑定 ETriggerEvent::Started → DoDodge()

DoDodge() 执行流程：
  1. if (StateMachine->IsState(Dead) || StateMachine->IsState(Stunned)) return
  2. if (bIsAttacking && 当前动作 bCanBeInterrupted == false) return
  3. if (SprintComp->CurrentStamina < DodgeStaminaCost) return
  4. SprintComp->StaminaChange(-DodgeStaminaCost)
  5. 中断当前 Montage（如果正在攻击）
  6. 播放 AM_Dodge
  7. 赋予无敌帧标签 State_Status_Invincible
  8. Timer(DodgeInvincibilityDuration) → 移除无敌帧标签
  9. 闪避方向 = 当前移动输入方向（无输入则后退）
  10. RootMotion 或 AddImpulse 驱动位移
```

### 4.7 阶段 1 验收标准

- Yuno 按下攻击键打出 3 段轻攻击连招，AnimNotify 正常触发
- 每次命中 CombatDummy：火花特效 + 闪白 + 顿帧 + 震屏
- CombatDummy 扣血至 0 后触发死亡（不崩溃）
- Yuno 被 Dummy 反击时：扣血 + 闪白 + 击退
- Yuno 血量归零：进入 Dead 状态，禁止输入
- 蓄力攻击按住→循环蓄力动画→松开→释放重击
- 闪避翻滚中被攻击不扣血（无敌帧生效）
- 攻击消耗体力，体力不足时无法攻击
- 连招输入缓冲正常：在 ComboWindow 内按下攻击自动接下一段

---

## 5. 阶段 2：角色渲染与视觉提升

**工期估算**：3-4 周
**前置依赖**：阶段 1 的打击特效基础部分 (1D) 完成
**目标**：Yuno 角色在视觉上达到作品集水准，材质体系可维护可扩展。

### 5.1 模块 2A：皮肤渲染

**5.1.1 Subsurface Profile 资产**

```
创建资产：SSP_YunoSkin

参数设置（起始值，需要在引擎中迭代调整）：
  Scatter Radius:
    R = 1.2  (红光穿透最远 — 血液)
    G = 0.35 (绿光中等)
    B = 0.15 (蓝光穿透最短)
  
  Subsurface Color: (0.9, 0.35, 0.25) — 暖红色散射
  Falloff Color: (0.85, 0.55, 0.45) — 柔和衰减
  
  Boundary Color Bleed: (0.8, 0.4, 0.25)
  — 阴影边界的颜色渗透（让阴影边缘偏暖而非死黑）
```

**5.1.2 皮肤 Master Material**

```
材质设置：
  Shading Model: Subsurface Profile
  Subsurface Profile: SSP_YunoSkin
  Blend Mode: Opaque
  Two Sided: false

输入贴图：
  T_Yuno_Skin_BaseColor   (2K) — 基础肤色
  T_Yuno_Skin_Normal       (2K) — 粗法线（面部结构）
  T_Yuno_Skin_Normal_Pore  (1K, Tiling) — 毛孔微法线
  T_Yuno_Skin_Roughness    (1K) — 粗糙度分区
  T_Yuno_Skin_SSS_Mask     (1K) — SSS 强度遮罩

节点网络关键路径：
  BaseColor:
    T_Yuno_Skin_BaseColor（直接连接）

  Normal:
    粗法线 + 毛孔法线通过 BlendAngleCorrectedNormals 混合
    毛孔法线强度受 PixelDepth 控制（远处强度降低）

  Roughness:
    T_Yuno_Skin_Roughness
    T区(额头/鼻子) = 0.3 (油亮)
    脸颊 = 0.5
    嘴唇 = 0.35

  Opacity:
    T_Yuno_Skin_SSS_Mask
    耳朵/鼻翼/嘴唇 = 高SSS (0.8-1.0)
    额头/下巴 = 低SSS (0.3-0.5)

  HitFlash 叠加：
    Scalar Param "HitFlashIntensity"
    在最终输出前 Lerp 到白色
```

### 5.2 模块 2B：头发渲染

**5.2.1 头发材质**

```
材质设置：
  Shading Model: Hair (如果用卡片头发) 或 Default Lit (简化方案)
  Blend Mode: Masked (OpacityMask 裁切发丝轮廓)
  Two Sided: true

关键参数：
  BaseColor: 发色（根据 Yuno 设定调整）
  Scatter: 背光散射色（黑发用深棕/深红，金发用暖黄）
  Roughness: 0.35-0.55（光滑秀发 vs 蓬松毛躁）
  Specular: 0.5 (默认)
  Backlit: 0.5-1.0（逆光时头发边缘发亮程度）

Alpha 处理：
  Opacity Mask: T_Yuno_Hair_Alpha
  使用 DitheredTemporalAA 节点柔化 Masked 边缘
  TAA 会自动融合抖动采样，消除锯齿

自阴影处理：
  顶点色 R 通道 = 烘焙的层间 AO
  BaseColor *= VertexColor.R（内层更暗）
```

**5.2.2 头发渲染与物理 LOD 统一**

```
ClothLODControllerComponent 输出的 LODFactor 可用于材质 LOD：

AnimBP 中：
  LODFactor = ClothLODComp->GetLODFactor()
  PhysicsAlpha *= (1.0 - LODFactor)  // 远处 Kawaii Physics 减弱

材质中（通过 PerInstanceCustomData 或 MID）：
  LOD 0: Hair Shading Model + 双高光 + 背光散射
  LOD 1: Default Lit + 单高光（去掉次高光计算）
  LOD 2: Unlit 近似（极远距离）

切换方式：
  Quality Switch 节点（编译期）
  或不同 LOD Mesh 赋予不同材质（运行期无开销）
```

### 5.3 模块 2C：衣物与眼睛

**5.3.1 衣物 Master Material**

```
Shading Model: Cloth
Fuzz Color: 比 BaseColor 浅一阶的颜色
Cloth 遮罩: 贴图驱动（布料区域=1, 金属扣/皮革=0）
  金属扣区域自动 fallback 到 Default Lit 行为

法线贴图: 布料纤维方向的微法线
Roughness: 0.6-0.85（布料偏粗糙）
```

**5.3.2 眼睛材质**

```
Shading Model: Eye (如果模型支持分离眼球Mesh)
  或 Default Lit + 高光调整 (简化方案)

关键效果：
  虹膜视差: IrisDepth 参数控制深度感
  角膜高光: 高 Specular + 低 Roughness 的透明层
  瞳孔缩放: Scalar Param "PupilScale"（可选，运行时用 MID 控制）
```

**5.3.3 Master Material 架构**

```
建议的材质层级：

M_Character_Master
  ├── MI_Yuno_Skin      (Instance, Subsurface Profile)
  ├── MI_Yuno_Hair      (Instance, Hair)
  ├── MI_Yuno_Eye       (Instance, Eye 或 Default Lit)
  ├── MI_Yuno_Cloth_Top (Instance, Cloth)
  ├── MI_Yuno_Cloth_Bot (Instance, Cloth)
  └── MI_Yuno_Accessory (Instance, Default Lit)

如果需要不同 Shading Model，则每个着色模型需要独立 Master Material：
  M_Skin_Master    (Subsurface Profile)
  M_Hair_Master    (Hair)
  M_Cloth_Master   (Cloth)
  M_Hard_Master    (Default Lit)

所有 Master 共享的 Material Function：
  MF_HitFlash — 受击闪白逻辑（所有材质统一行为）
  MF_DistanceFade — 远距离材质简化
```

### 5.4 模块 2D：战斗特效升级

**5.4.1 武器拖尾**

```
Niagara System: NS_WeaponTrail

  Emitter: "Trail"
    Renderer: Ribbon
    Source: 两个 Socket (weapon_top, weapon_bottom)
    SpawnRate: 每帧 1 粒子（跟随武器移动路径）
    Lifetime: 0.15-0.25s
    Width: 武器长度（通过 Socket 间距自动计算）
    Color: 从亮白 → 武器主色调 → 透明
    Material: Unlit + Additive, 渐变贴图
    
触发：
  AnimNotify (攻击开始) → SpawnSystemAttached
  AnimNotify (攻击结束) → Deactivate (让现有粒子自然消亡)
```

**5.4.2 冲击波折射**

```
Niagara System: NS_ShockwaveDistortion

  Emitter: "Shockwave"
    Renderer: Mesh (扁平圆环或球壳)
    Spawn: 1 个粒子 (Burst)
    Lifetime: 0.2-0.3s
    Scale: 0 → 200 → 250 (快速膨胀)
    
    Material:
      Blend Mode: Translucent
      开启 Refraction
      法线贴图: 径向法线 (从中心向外的法线方向)
      Refraction: 法线 × 折射强度 × (1 - NormalizedAge)
      → NormalizedAge = 0→1, 折射从强到无

仅用于重攻击和 Boss 战，轻攻击不触发（控制性能）
```

**5.4.3 角色描边系统**

```
实现方案：Custom Depth + Post Process Material

C++ 端：
  // 可交互物体靠近时高亮
  MeshComp->SetRenderCustomDepth(true);
  MeshComp->SetCustomDepthStencilValue(StencilID);
  
  Stencil 分配：
    1 = 可拾取物品（白色描边）
    2 = 敌人（红色描边）
    3 = 可交互物体（黄色描边）

Post Process Material:
  1. 采样 CustomDepth 的相邻像素（上下左右偏移 1-2 像素）
  2. 检测深度差异 → 边缘检测
  3. 根据 CustomStencil 值选择描边颜色
  4. 描边宽度: 2-3 像素
  5. 输出: SceneColor + 描边叠加
```

### 5.5 模块 2E：场景氛围

```
光照设置：
  DirectionalLight: 模拟阳光，角度 45-60 度
  SkyLight: 提供环境光填充
  Lumen GI: 开启，让间接光染色角色

Post Process Volume (Unbound):
  色调映射: ACES Filmic
  Bloom: 强度 0.3-0.5, 阈值 1.0
  Auto Exposure: 开启, MinEV/MaxEV 根据场景调整
  Vignette: 0.2-0.3 (轻微暗角)
  Color Grading: 根据场景氛围调 LUT 或手动调 Shadows/Midtones/Highlights
```

### 5.6 阶段 2 验收标准

- 皮肤 SSS：光照穿过耳朵/手指时有温暖红色透光
- 头发高光：有可见的各向异性高光条带，随视角移动
- 头发 Alpha：无锯齿硬边，TAA 柔化有效
- 衣物 Fuzz：掠射角可见柔和的边缘发光
- 受击闪白：命中瞬间角色整体变白 → 3-4 帧内衰减回正常
- 武器拖尾：挥剑时有流畅的光弧
- 冲击波：重攻击命中有空气扭曲效果
- Shader Complexity：角色区域为绿色/浅黄色（非红色）

---

## 6. 阶段 3：关卡与体验流程

**工期估算**：2-3 周
**前置依赖**：阶段 1 + 阶段 2 核心部分完成
**目标**：构建一个 3-5 分钟的完整可玩关卡。

### 6.1 关卡设计文档

```
┌────────────────────────────────────────────────────┐
│                    关卡俯视布局                       │
│                                                      │
│  [A] 起始安全区                                       │
│   ├── 出生点                                         │
│   ├── 武器拾取台（APickupData, 武器道具）              │
│   ├── 3x 回复道具散落                                 │
│   └── 通向 [B] 的通道                                 │
│                                                      │
│  [B] 战斗区域                                        │
│   ├── 平坦的战斗场地，半径 15-20m                      │
│   ├── 波次 1：3 只小怪（从两侧刷新）                    │
│   ├── 波次 2：4 只小怪（从三侧刷新）                    │
│   ├── 全灭后 → 打开通向 [C] 的门                      │
│   ├── 场边有 2 个回复道具箱                            │
│   └── 可选：1 个需要钥匙的宝箱                        │
│                                                      │
│  [C] Boss 区域                                       │
│   ├── 封闭的圆形竞技场，半径 12-15m                    │
│   ├── 进入后门关闭（触发 Volume）                      │
│   ├── 1 只 Boss                                      │
│   └── Boss 死亡 → 胜利结算画面                         │
└────────────────────────────────────────────────────┘
```

### 6.2 敌人设计

**6.2.1 小怪：Slime（史莱姆）**

```
基础属性：
  MaxHP: 30
  MoveSpeed: 200 cm/s
  AttackDamage: 5
  AttackRange: 100 cm
  AttackCooldown: 2.0s

AI 行为（StateTree）：
  Idle → 感知到玩家(半径 800cm) → 移动向玩家
  → 进入攻击范围 → 播放攻击动画 → 冷却 → 循环

动画需求：
  Idle, Walk, Attack (1种), HitReact, Death

特效：
  被击时有小火花 + 闪白
  死亡时溶解消散（Dissolve 材质效果，可选）
```

**6.2.2 Boss：Knight（骑士）**

```
基础属性：
  MaxHP: 200
  MoveSpeed: 280 cm/s
  AttackDamage: 15-25 (根据招式)
  Phase 2 触发: HP < 50%

AI 行为（StateTree）：
  Phase 1:
    感知玩家 → 靠近 → 随机选择攻击模式：
      - 2段连击 (60%概率, 伤害15)
      - 蓄力重击 (30%概率, 伤害25, 有前摇警告)
      - 后跳拉距 (10%概率)
    攻击后冷却 1.5-2.5s

  Phase 2 (HP < 50%):
    攻速提升 (PlayRate 1.2)
    增加新招式：旋转攻击(AOE)
    攻击后冷却缩短到 1.0-1.5s

动画需求：
  Idle, Walk, Attack_Combo(2段), Attack_Heavy, Attack_Spin(AOE)
  HitReact, Death, Phase2_Transition(吼叫/蓄力)

技术复用：
  继承自 ACombatEnemy 或新建类实现 ICombatAttacker + ICombatDamageable
  StateTree 逻辑复用 Variant_Combat 的框架
  需要新增的 StateTree Task：Phase 检查、概率选招
```

### 6.3 流程管理

```
新增类：AEncounterVolume（战斗区域触发器）

成员：
  TArray<TSubclassOf<ACharacter>> WaveEnemies  // 每波敌人类型
  TArray<int32> WaveCounts                      // 每波数量
  TArray<FTransform> SpawnPoints                // 刷新点
  int32 CurrentWave = 0
  int32 AliveEnemyCount = 0
  AActor* DoorToOpen                            // 全灭后打开的门

流程：
  玩家进入 Volume → 关门（如果有）
  → SpawnWave(CurrentWave) → 等待 AliveEnemyCount == 0
  → CurrentWave++ → 下一波 / 全部完成 → 开门

敌人死亡通知：
  敌人 OnEnemyDied 委托 → EncounterVolume::OnEnemyKilled()
  → AliveEnemyCount-- → 检查是否全灭

角色死亡：
  AEscapeGameCharacter::HandleDeath()
  → Timer(3s) → Restart Level (UGameplayStatics::OpenLevel)
  或者 → Respawn at checkpoint

Boss 击杀：
  Boss OnEnemyDied → 显示胜利 UI Widget → 5 秒后可退出
```

### 6.4 阶段 3 验收标准

- 从起始区到 Boss 击杀，全程 3-5 分钟可流畅跑通
- 拾取武器后攻击生效（之前无武器时攻击无效或无伤害）
- 波次间有 2-3 秒间隔，给玩家喘息和使用道具的时间
- Boss 两阶段切换有视觉/音频提示
- 角色死亡 → 3 秒内重新开始
- Boss 死亡 → 显示胜利结算
- 全程帧率 > 55 FPS

---

## 7. 阶段 4：打磨与作品集包装

**工期估算**：1-2 周
**前置依赖**：阶段 1-3 全部完成

### 7.1 性能优化 Checklist


| 检查项         | 工具                     | 目标值              | 应对方案                      |
| ----------- | ---------------------- | ---------------- | ------------------------- |
| GPU 总耗时     | stat GPU               | < 16.6ms (60FPS) | 降低后处理质量 / 减少特效            |
| Draw Call   | stat SceneRendering    | < 2000           | 合并材质 / 减少 Actor           |
| 骨骼更新耗时      | stat Anim              | < 2ms            | ClothLODController 降低远处物理 |
| 特效 Overdraw | Shader Complexity View | 黄色以内             | 减少粒子数 / 缩小面积 / Cutout     |
| 内存占用        | stat Memory            | < 2GB GPU        | 降低贴图分辨率 / Mipmap          |
| Lumen 耗时    | stat GPU → Lumen       | < 4ms            | 降低 Lumen 质量设置             |


### 7.2 展示视频结构（建议 2-3 分钟）

```
00:00 - 00:15  开场：角色渲染特写
  → 缓慢旋转，展示皮肤 SSS、头发光泽、布料质感
  → 逆光镜头展示头发散射

00:15 - 00:30  角色物理展示
  → 头发随风飘动（WindSimulation）
  → 快速转身时头发惯性甩动
  → 远近距离切换展示布料 LOD 过渡

00:30 - 01:30  战斗演示
  → 3 段轻攻击连招（展示 Combo）
  → 蓄力重击（展示特效升级）
  → 闪避翻滚（展示无敌帧）
  → 多敌战斗场景（展示实战感）

01:30 - 02:00  Boss 战
  → Boss 两阶段切换
  → 使用道具回血
  → 最终击杀 + 胜利结算

02:00 - 02:30  技术分解（画中画/分屏对比）
  → 有/无 SSS 对比
  → 有/无打击特效对比
  → 有/无物理模拟对比
  → Shader Complexity 可视化展示

02:30 - 02:45  收尾
  → 项目名称 / 技术栈列表 / 联系方式
```

### 7.3 技术文档大纲

```
1. 项目概述 (0.5 页)
2. 架构总览图 — 各系统关系图 (1 页)
3. 战斗系统设计
   3.1 接口架构 (ICombatAttacker / ICombatDamageable)
   3.2 数据驱动设计 (GameplayTag + CharacterAnimData)
   3.3 Combo 状态机
   3.4 伤害流程图
4. 角色渲染 Pipeline
   4.1 着色模型选择依据
   4.2 皮肤 SSS 参数说明
   4.3 头发渲染方案（含 Alpha 处理对比图）
   4.4 材质 LOD 策略
5. 角色物理系统
   5.1 WindSimulationComponent 设计（柏林噪声 + 降频优化）
   5.2 ClothLODControllerComponent 设计（无 Tick + 物理LOD融合）
   5.3 物理与渲染 LOD 统一方案
6. 性能分析
   6.1 优化前后帧率对比
   6.2 关键热点及解决方案
7. 未来扩展方向
```

---

## 8. 风险预案


| 风险                    | 概率  | 影响        | 应对                                 |
| --------------------- | --- | --------- | ---------------------------------- |
| 攻击动画资产不足              | 高   | 阶段 1 阻塞   | 从 Mixamo / Marketplace 获取临时动画，后期替换 |
| Yuno 模型不支持 Eye 着色模型   | 中   | 阶段 2 视觉降级 | 改用 Default Lit + 高光调参的简化方案         |
| 头发 Alpha 排序闪烁严重       | 中   | 视觉质量下降    | 改用 Masked + DitheredTemporalAA 方案  |
| Boss StateTree 行为调试困难 | 中   | 阶段 3 延期   | 先用简单 BehaviorTree 替代，后期迁移          |
| 帧率不达标                 | 低   | 阶段 4 延期   | 降低 Lumen 质量 / 减少粒子 / 降渲染分辨率        |
| 战斗打击感不足               | 中   | 核心体验受损    | 参考 Action 游戏逐帧调整顿帧/震屏/特效时序         |


---

## 9. 阶段 5：装备系统与物品品质（第 13-17 周）

**前置依赖**：阶段 3 完成（关卡可跑通）
**目标**：玩家可以拾取不同品质的装备并穿戴，角色外观和属性同步变化。

### 9.1 模块 5A：物品稀有度系统

**9.1.1 数据结构扩展**

文件：`ItemData.h`

```
新增枚举：
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common      UMETA(DisplayName = "普通"),    // 白
    Uncommon    UMETA(DisplayName = "优秀"),    // 绿
    Rare        UMETA(DisplayName = "稀有"),    // 蓝
    Epic        UMETA(DisplayName = "史诗"),    // 紫
    Legendary   UMETA(DisplayName = "传说"),    // 金
};

FItemData 新增字段：
  UPROPERTY(EditAnywhere, Category = "Item Data")
  EItemRarity Rarity = EItemRarity::Common;

全局颜色映射（可用 DataAsset 或静态函数）：
  Common    → FLinearColor(0.7, 0.7, 0.7, 1)   // 灰白
  Uncommon  → FLinearColor(0.2, 0.8, 0.2, 1)   // 绿
  Rare      → FLinearColor(0.3, 0.5, 1.0, 1)   // 蓝
  Epic      → FLinearColor(0.7, 0.3, 0.9, 1)   // 紫
  Legendary → FLinearColor(1.0, 0.8, 0.2, 1)   // 金
```

**9.1.2 UI 渲染表现**

```
InventorySlotWidget 改造：
  新增 UPROPERTY(meta=(BindWidget)) UBorder* RarityBorder
  SetItem() 中根据 Rarity 设置：
    - RarityBorder->SetBrushColor(RarityColor)
    - NameText 颜色 = RarityColor（Tooltip 中）
    - Legendary 物品：背景 UImage 使用流光动画材质
      材质：Unlit + 半透明, UV Panner + 噪声驱动的金色流光

世界拾取物（APickupData）改造：
  根据 Rarity 添加不同视觉效果：
    Common/Uncommon: 无特效，仅悬浮旋转
    Rare: UNiagaraComponent 蓝色微粒上升
    Epic: UNiagaraComponent 紫色光环
    Legendary: UNiagaraComponent 金色光柱 + 地面 Decal 光圈
```

### 9.2 模块 5B：装备系统

**9.2.1 新增组件：UEquipmentComponent**

```
文件：EquipmentComponent.h/cpp

枚举：
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    Weapon,
    Head,
    Body,
    Accessory,
    MAX UMETA(Hidden)
};

成员：
  UPROPERTY(VisibleAnywhere)
  TMap<EEquipmentSlot, FItemStack> EquippedItems;

  UPROPERTY(BlueprintAssignable)
  FOnEquipmentChanged OnEquipmentChanged;

  // 缓存引用
  UInventoryComponent* InventoryComp;
  UAttributeComponent* AttributeComp;
  USkeletalMeshComponent* OwnerMesh;

核心方法：

  Equip(int32 InventorySlotIndex, EEquipmentSlot TargetSlot):
    1. 验证背包格子有物品
    2. 验证物品类型与装备槽匹配（Weapon→Weapon, Armor→Body...）
    3. 如果目标槽已有装备 → 先 Unequip 放回背包
    4. 从背包移除物品
    5. 放入 EquippedItems[TargetSlot]
    6. ApplyEquipmentStats(物品) — 加属性
    7. AttachEquipmentMesh(物品) — 挂载外观
    8. 广播 OnEquipmentChanged

  Unequip(EEquipmentSlot Slot):
    1. 从 EquippedItems 取出
    2. RemoveEquipmentStats — 减属性
    3. DetachEquipmentMesh — 移除外观
    4. AddItem 回背包
    5. 广播

  ApplyEquipmentStats(FItemData):
    根据物品属性修改角色数值：
    - 武器 → EscapeCombatComponent::BaseDamage += WeaponDamage
    - 护甲 → AttributeComponent::MaxHealth += ArmorHP
    - 饰品 → SprintComponent::MaxStamina += AccessoryStamina

  AttachEquipmentMesh(FItemData):
    SpawnActor or SetStaticMesh on a dedicated MeshComponent
    AttachToComponent(OwnerMesh, SocketName)
    武器: "weapon_r" Socket
    头饰: "head" Socket
```

**9.2.2 UItemDefinition 子类扩展**

```
UWeaponDefinition : public UItemDefinition
  float AttackDamage = 10.0f
  float AttackSpeed = 1.0f        // PlayRate 乘数
  float TraceRange = 75.0f        // 覆盖 FActionDefinition 的默认值
  TSoftObjectPtr<USkeletalMesh> WeaponMesh
  FName AttachSocket = "weapon_r"
  // 未来可加：元素类型、特殊效果Tag

UArmorDefinition : public UItemDefinition
  float BonusMaxHP = 20.0f
  float DamageReduction = 0.0f     // 0-1, 百分比减伤
  TSoftObjectPtr<USkeletalMesh> ArmorMesh  // 如果有外观替换

UAccessoryDefinition : public UItemDefinition
  float BonusMaxStamina = 10.0f
  FGameplayTagContainer PassiveEffectTags  // 被动效果标签
```

### 9.3 模块 5C：物品冷却系统

```
激活 UItemDefinition::CoolDownTime 字段

在 InventoryComponent 中新增：
  TMap<FName, double> CooldownEndTimes;  // ItemID → GameTimeSeconds 的结束时间

UseItem 修改：
  double CurrentTime = GetWorld()->GetTimeSeconds();
  if (CooldownEndTimes.Contains(ItemID) && CooldownEndTimes[ItemID] > CurrentTime)
  {
      // 还在冷却中，拒绝使用
      return;
  }
  // ... 原有使用逻辑 ...
  if (bUsedSuccessfully && CoolDownTime > 0)
  {
      CooldownEndTimes.Add(ItemID, CurrentTime + CoolDownTime);
      OnCooldownStarted.Broadcast(ItemID, CoolDownTime);  // 新增委托
  }

UI 端：
  InventorySlotWidget 中添加冷却遮罩 UImage（半透明黑色）
  + UTextBlock 显示剩余秒数
  监听 OnCooldownStarted → 启动 UMG 动画（从全遮罩扇形擦除到无遮罩）
  或用 Material 的 Dynamic Parameter 驱动圆形进度条
```

### 9.4 阶段 5 验收标准

- 物品有品质颜色区分，Legendary 物品在世界中有金色光柱
- 背包格子有品质边框，Tooltip 显示品质颜色的名字
- 可从背包装备武器 → 角色手持武器模型出现
- 装备武器后攻击力变化（数值正确）
- 卸下武器 → 武器回到背包，攻击力恢复
- 道具使用后进入冷却，冷却期间无法再次使用
- 冷却结束有音效/UI 提示

---

## 10. 阶段 6：存档系统与游戏流程完善（第 18-21 周）

**前置依赖**：阶段 5 完成
**目标**：游戏有完整的开始→游玩→结束→继续流程。

### 10.1 模块 6A：Save/Load 持久化系统

**10.1.1 存档数据结构**

```
新增文件：EscapeSaveGame.h/cpp

UCLASS()
class UEscapeSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    // 角色数据
    UPROPERTY() float SavedHealth;
    UPROPERTY() float SavedMaxHealth;
    UPROPERTY() float SavedStamina;
    UPROPERTY() float SavedMaxStamina;
    UPROPERTY() FTransform SavedTransform;  // 位置/朝向

    // 背包数据（轻量化：只存 ID + 数量）
    UPROPERTY() TArray<FInventorySaveSlot> SavedInventory;
    // FInventorySaveSlot = { FName ItemID, int32 Count }

    // 装备数据
    UPROPERTY() TMap<EEquipmentSlot, FName> SavedEquipment;
    // Slot → ItemID

    // 关卡进度
    UPROPERTY() FName CurrentLevelName;
    UPROPERTY() int32 CurrentWaveIndex;
    UPROPERTY() TArray<FName> DefeatedBosses;
    UPROPERTY() TArray<FName> CollectedKeys;

    // 元数据
    UPROPERTY() FDateTime SaveTimestamp;
    UPROPERTY() float TotalPlayTime;
};
```

**10.1.2 存档管理器**

```
新增文件：EscapeSaveManager.h/cpp（UBlueprintFunctionLibrary 或 UGameInstanceSubsystem）

核心接口：
  static bool SaveGame(UObject* WorldContext, int32 SlotIndex);
  static bool LoadGame(UObject* WorldContext, int32 SlotIndex);
  static bool DoesSaveExist(int32 SlotIndex);
  static bool DeleteSave(int32 SlotIndex);

SaveGame 流程：
  1. 创建 UEscapeSaveGame 实例
  2. 从各组件收集数据：
     - AttributeComp → Health, MaxHealth
     - SprintComp → Stamina, MaxStamina
     - InventoryComp → Items 遍历，只存非空格子的 {ID, Count}
     - EquipmentComp → EquippedItems 遍历
     - Character → GetActorTransform()
  3. UGameplayStatics::SaveGameToSlot(SaveInstance, SlotName, 0)

LoadGame 流程：
  1. UGameplayStatics::LoadGameFromSlot(SlotName, 0)
  2. 恢复各组件数据（顺序重要）：
     a. 先恢复 MaxHealth/MaxStamina（因为 Clamp 依赖上限）
     b. 再恢复当前值
     c. 恢复背包：清空 → 根据 ID 从 DataTable 查完整 FItemData → AddItem
     d. 恢复装备：对每个槽位查 ItemID → 创建装备 → Equip
     e. 恢复位置
  3. 广播 OnLoadCompleted → UI 刷新

自动存档触发点：
  - 击败每波敌人后
  - 进入新区域时
  - 打开背包时（可选）
  - 手动存档（Pause菜单）
```

### 10.2 模块 6B：主菜单与 Pause 菜单

```
新增 Widget：WBP_MainMenu
  ├── "新游戏" 按钮 → OpenLevel(GameLevel)
  ├── "继续游戏" 按钮 → LoadGame(0) → OpenLevel(SavedLevel)
  │   仅当 DoesSaveExist(0) 时显示
  ├── "设置" 按钮 → 画质/音量调节（可选）
  └── "退出" 按钮 → QuitGame

新增 Widget：WBP_PauseMenu
  ├── "继续" 按钮 → SetPause(false)
  ├── "存档" 按钮 → SaveGame(0) + 提示"已保存"
  ├── "设置" 按钮
  └── "返回主菜单" 按钮 → SaveGame + OpenLevel(MainMenuLevel)

  触发：ESC 键（已有 PauseAction 输入定义）
  暂停方式：UGameplayStatics::SetGamePaused(true)
  输入模式切换：GameAndUI（和背包菜单相同逻辑，可复用 SetInventoryVisibility 的模式）
```

### 10.3 模块 6C：死亡与重生完善

```
当前问题：HandleDeath 只禁止移动，没有完整的死亡/重生流程

完善后的死亡流程：
  1. HandleDeath()
     → StateMachine->SetState(Dead)
     → 禁止所有输入
     → 播放死亡动画 Montage
     → 屏幕渐暗（Post Process 的 Scene Color Tint 从白到黑）
     
  2. 死亡动画结束后（Timer 或 Montage 回调）
     → 显示 "You Died" Widget
     → 3 秒后显示选项：
        "从检查点重生" → LoadGame(AutoSaveSlot) 或 重置到最近 Checkpoint
        "返回主菜单" → OpenLevel(MainMenu)

  3. 重生：
     → 恢复满血满体力
     → 传送到 Checkpoint 位置
     → 当前区域的敌人重置
     → 屏幕渐亮
     → 恢复输入
```

### 10.4 阶段 6 验收标准

- 主菜单可以开始新游戏和继续游戏
- 按 ESC 打开暂停菜单，可手动存档
- 击败一波敌人后自动存档
- 退出游戏后重新打开 → "继续游戏" → 回到存档点，背包/装备/血量恢复
- 角色死亡 → 显示死亡界面 → 从检查点重生 → 状态正确恢复
- 存档文件可删除，删除后"继续游戏"按钮消失

---

## 11. 阶段 7：NPC、对话与任务系统（第 22-27 周）

**前置依赖**：阶段 6 完成
**目标**：关卡中有可交互的 NPC，有简单的任务驱动叙事。

### 11.1 模块 7A：对话系统

**11.1.1 对话数据结构**

```
新增文件：DialogueData.h

USTRUCT(BlueprintType)
struct FDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FText SpeakerName;
    UPROPERTY(EditAnywhere) FText Content;
    UPROPERTY(EditAnywhere) UTexture2D* SpeakerPortrait;  // 头像
    UPROPERTY(EditAnywhere) USoundBase* VoiceLine;        // 语音（可选）
    UPROPERTY(EditAnywhere) float AutoAdvanceTime = 0.0f; // 0=手动翻页
};

USTRUCT(BlueprintType)
struct FDialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FText ChoiceText;
    UPROPERTY(EditAnywhere) int32 JumpToNodeIndex = -1;   // 跳转到哪个节点
    UPROPERTY(EditAnywhere) FGameplayTag RequiredTag;     // 需要满足的条件
    UPROPERTY(EditAnywhere) FGameplayTag GrantTag;        // 选择后赋予的标签
};

USTRUCT(BlueprintType)
struct FDialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) TArray<FDialogueLine> Lines;     // 该节点的对话内容
    UPROPERTY(EditAnywhere) TArray<FDialogueChoice> Choices; // 节点结束后的选项（空=自动结束）
    UPROPERTY(EditAnywhere) int32 NextNodeIndex = -1;         // 无选项时的下一个节点（-1=结束）
};

UCLASS()
class UDialogueAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) TArray<FDialogueNode> Nodes;
};
```

**11.1.2 对话 UI Widget**

```
WBP_DialogueBox:
  ┌──────────────────────────────────────────┐
  │ [头像]  角色名                              │
  │         "对话内容，支持逐字打字机效果..."      │
  │                                            │
  │   → 选项 A                                  │
  │   → 选项 B                                  │
  │                              [点击继续 ▼]    │
  └──────────────────────────────────────────┘

打字机效果实现：
  用 Timer 每 0.03s 显示一个字符
  RichTextBlock 支持富文本标签（颜色/加粗）
  点击/按键 → 如果正在打字则立即显示全部，否则翻到下一行

输入模式：
  对话开始 → GameAndUI + ShowCursor + IgnoreMoveInput
  对话结束 → GameOnly
```

**11.1.3 NPC Actor**

```
新增文件：EscapeNPC.h/cpp

UCLASS()
class AEscapeNPC : public ACharacter, public IInteractableInterface
{
    // 外观
    USkeletalMeshComponent — NPC 模型
    UWidgetComponent — 头顶名字/任务标记

    // 数据
    UPROPERTY(EditAnywhere) UDialogueAsset* DefaultDialogue;
    UPROPERTY(EditAnywhere) FText NPCName;
    UPROPERTY(EditAnywhere) bool bHasQuest = false;

    // 交互接口实现
    Interact_Implementation:
      → 打开对话 UI
      → 播放 DialogueAsset 的内容
      → 对话结束时检查是否触发任务
    
    CanInteract_Implementation:
      → return true（NPC 总是可交互）
    
    GetInteractText_Implementation:
      → return "对话"
};
```

### 11.2 模块 7B：任务系统

```
新增文件：QuestSystem.h/cpp

USTRUCT(BlueprintType)
struct FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FText Description;    // "击败 3 只史莱姆"
    UPROPERTY(EditAnywhere) FGameplayTag EventTag; // Event.Quest.KillSlime
    UPROPERTY(EditAnywhere) int32 RequiredCount = 1;
    int32 CurrentCount = 0;
    bool bCompleted = false;
};

UCLASS()
class UQuestDefinition : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName QuestID;
    UPROPERTY(EditDefaultsOnly) FText QuestName;
    UPROPERTY(EditDefaultsOnly) FText QuestDescription;
    UPROPERTY(EditDefaultsOnly) TArray<FQuestObjective> Objectives;
    
    // 奖励
    UPROPERTY(EditDefaultsOnly) TArray<FName> RewardItemIDs;
    UPROPERTY(EditDefaultsOnly) TArray<int32> RewardItemCounts;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UQuestComponent : public UActorComponent
{
    // 当前活跃任务
    UPROPERTY() TArray<UQuestDefinition*> ActiveQuests;
    UPROPERTY() TArray<FName> CompletedQuestIDs;

    // 接口
    void AcceptQuest(UQuestDefinition* Quest);
    void UpdateObjective(FGameplayTag EventTag, int32 Count = 1);
    void CompleteQuest(UQuestDefinition* Quest);
    bool IsQuestCompleted(FName QuestID) const;

    // 委托
    FOnQuestAccepted OnQuestAccepted;
    FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;
    FOnQuestCompleted OnQuestCompleted;
};

事件驱动：
  敌人死亡时：
    QuestComp->UpdateObjective(FEscapeGameplayTags::Event_Quest_KillSlime);
  拾取物品时：
    QuestComp->UpdateObjective(FEscapeGameplayTags::Event_Quest_CollectItem);
  对话选择时：
    QuestComp->UpdateObjective(选项的 GrantTag);
```

### 11.3 模块 7C：任务 UI

```
WBP_QuestTracker（屏幕右上角常驻）：
  ┌────────────────────┐
  │ ► 消灭威胁          │
  │   击败史莱姆 2/3     │
  │   找到钥匙   0/1     │
  └────────────────────┘

  监听 OnQuestObjectiveUpdated → 刷新计数
  监听 OnQuestCompleted → 播放完成动画 → 显示奖励

NPC 头顶标记：
  有任务可接：黄色感叹号 !
  任务进行中：灰色问号 ?
  任务可交付：黄色问号 ?
  根据 QuestComponent 状态动态切换
```

### 11.4 Demo 任务设计示例

```
任务 1："新手之路"（教学任务）
  接取：起始区 NPC
  目标：拾取武器 (1/1)
  奖励：3 个治疗药水
  
任务 2："清除威胁"（主线任务）
  接取：战斗区入口 NPC（或自动触发）
  目标：击败所有史莱姆 (7/7)
  奖励：Boss 区钥匙

任务 3："最终试炼"（Boss任务）
  接取：自动触发（进入 Boss 区域）
  目标：击败 Boss (1/1)
  奖励：传说品质武器（胜利展示用）
```

### 11.5 阶段 7 验收标准

- 可与 NPC 对话，对话框逐字显示
- 对话中有分支选项，选择后跳转到不同内容
- NPC 头顶有任务状态标记（!/? 切换）
- 任务追踪器实时更新目标进度
- 任务完成后自动发放奖励到背包
- 任务数据在存档中正确保存/恢复

---

## 12. 阶段 8：音频系统与高级渲染（第 28-34 周）

**前置依赖**：阶段 7 完成
**目标**：补全游戏的听觉维度，并在渲染上做进一步的风格化和高级效果。

### 12.1 模块 8A：音频系统整合

```
音频分类与资产需求：

BGM（背景音乐）：
  ├── BGM_SafeZone     — 安全区，舒缓
  ├── BGM_Combat       — 战斗区，紧张
  ├── BGM_Boss         — Boss 战，激烈
  └── BGM_Victory      — 胜利，欢快

SFX（音效）：
  ├── 战斗音效
  │   ├── SFX_Swing_Light_01/02/03   — 挥砍空气声（每段不同）
  │   ├── SFX_Hit_Flesh_01/02        — 命中肉体
  │   ├── SFX_Hit_Metal_01/02        — 命中金属
  │   ├── SFX_Charged_Loop           — 蓄力循环声
  │   ├── SFX_Charged_Release        — 蓄力释放
  │   └── SFX_Dodge                  — 闪避翻滚
  │
  ├── 角色音效
  │   ├── SFX_Footstep_01-04         — 脚步声（多个随机）
  │   ├── SFX_Jump / SFX_Land        — 跳跃/落地
  │   ├── SFX_Hurt_01/02             — 受伤呻吟
  │   └── SFX_Death                  — 死亡
  │
  ├── UI 音效
  │   ├── SFX_UI_Click               — 按钮点击
  │   ├── SFX_UI_OpenMenu            — 打开菜单
  │   ├── SFX_Pickup                 — 拾取物品
  │   └── SFX_QuestComplete          — 任务完成
  │
  └── 环境音效
      ├── SFX_Ambient_Wind           — 环境风声
      └── SFX_Ambient_Birds          — 鸟鸣（安全区）

实现方式：
  BGM：UAudioComponent 挂在 PlayerController 上
       区域切换用 AudioVolume 或手动 CrossFade
  
  SFX：
    战斗：AnimNotify_PlaySound 或 代码中 UGameplayStatics::PlaySoundAtLocation
    脚步：AnimNotify 在 Walk/Run 动画中设置
    UI：Widget 按钮事件中 PlaySound2D
  
  音量管理：
    USoundMix + USoundClass 分类（BGM/SFX/Voice/UI）
    设置菜单中用 Slider 调节各类音量
```

### 12.2 模块 8B：风格化渲染进阶（如果走二次元方向）

```
如果 Yuno 是二次元角色风格，可以在 PBR 基础上叠加风格化处理：

SDF 面部阴影：
  创建面部 SDF 贴图（在 DCC 工具中烘焙）
  材质中：用 SDF 采样替代法线点积的明暗判断
  效果：脸上阴影边界平滑可控，不会出现三角形硬阴影
  
  实现：
    Light Direction 转到面部局部空间
    采样 SDF 贴图（灰度图，亮区=面向光，暗区=背光）
    SDF Value > Threshold → 亮面
    SDF Value < Threshold → 暗面
    Threshold 随光照角度移动 → 阴影形状自然过渡

Ramp Shading（色阶着色）：
  替代平滑的 PBR 明暗过渡
  用 1D 渐变贴图（Ramp Texture）控制明暗边界
  2-3 个色阶 → 赛璐珞风格
  5-7 个色阶 → 柔和卡通风格
  
  你的 UTACurveToolLibrary 的 Stepped 模式可以辅助生成 Ramp 曲线

屏幕空间描边升级：
  基础：Custom Depth 边缘检测（已在阶段 2 实现）
  进阶：法线差异检测 + 深度差异检测混合
  更进阶：描边粗细随距离/角度变化
  最进阶：描边颜色取角色材质的暗面颜色（非统一黑色）
```

### 12.3 模块 8C：高级后处理效果

```
受伤警告效果：
  HP < 30% 时：
    屏幕边缘渐红（Post Process Material 的 Vignette 颜色从黑→红）
    轻微画面抖动（Camera Shake 低频持续）
    心跳声音效 SFX_Heartbeat（频率随血量降低加快）

技能释放全屏效果：
  终结技命中时：
    屏幕闪白 0.1s（Post Process Exposure 瞬间拉高）
    径向模糊（从命中点向外）0.3s
    色彩饱和度瞬间提高后回落
    
  实现：用 Post Process Material + MID 控制参数
  由 CombatFXComponent 统一管理

场景氛围增强：
  体积雾（Exponential Height Fog + Volumetric Fog）
  God Rays（方向光的 Light Shaft）
  粒子灰尘（Niagara 全局灰尘粒子，带 Depth Fade）
```

### 12.4 阶段 8 验收标准

- BGM 在不同区域无缝切换（CrossFade 过渡）
- 每次攻击、命中、受伤都有对应音效
- 脚步声随地面材质变化（可选）
- 设置菜单可以分别调节 BGM/SFX 音量
- 面部阴影（如果二次元方向）无三角形硬边
- HP 低于 30% 时有持续的视觉警告
- 整体画面氛围统一协调

---

## 13. 阶段 9：第二关卡与高级战斗扩展（第 35-42 周）

**前置依赖**：阶段 8 完成
**目标**：扩充游戏内容量，证明系统的可扩展性。

### 13.1 第二关卡设计

```
关卡 2：地下遗迹

风格：暗色调，火把照明，石质建筑
与关卡 1 的视觉对比：证明材质系统在不同光照条件下的表现力

结构：
  [A] 入口走廊
    → 火把光照展示角色 SSS 在暖光下的效果
    → 陷阱机关（地刺/落石，展示环境危险源）
    
  [B] 大厅战斗
    → 新敌人类型：远程法师怪
    → 需要利用掩体（柱子）躲避远程攻击
    
  [C] 宝物房
    → 需要从 [B] 获得钥匙
    → 稀有/史诗装备奖励
    
  [D] 最终Boss：骨龙（大型敌人）
    → 多阶段 + AOE 攻击 + 弱点机制
    → 展示更复杂的 AI 和视觉特效

目的：证明架构的可扩展性——新关卡只需配置数据，不需要修改核心代码
```

### 13.2 新增敌人类型

```
远程法师怪：
  行为：保持距离 + 发射投射物
  投射物：AProjectile Actor（UProjectileMovementComponent）
  命中时对玩家造成伤害
  技术展示：投射物的渲染（自发光材质 + 粒子拖尾）

大型Boss（骨龙）：
  多段攻击模式：
    近战扑击、尾扫（AOE）、吐息（锥形范围）
  弱点机制：
    特定部位受伤加倍（头部 2x 伤害）
    用 Collision Channel 或 Bone Name 区分部位
  阶段转换：
    HP < 66% → Phase 2（加入尾扫）
    HP < 33% → Phase 3（加入吐息 + 速度提升）
```

### 13.3 高级战斗扩展

```
空中连击（Air Combo）：
  上挑招将敌人击飞 → 玩家跳跃追击 → 空中连击
  需要：空中攻击动画 + 空中状态下的 Combo 路由

弹反/格挡（Parry/Block）：
  在敌人攻击瞬间按防御键 → 完美弹反
  弹反成功：敌人进入长硬直 + 特殊音效/特效
  时间窗口：0.15-0.2s（需要精确操作）

元素系统（基础版）：
  火/冰/雷三种元素
  武器附魔：不同元素有不同的附加效果
    火：持续灼烧（DoT）
    冰：减速
    雷：范围传导（伤害扩散到附近敌人）
  材质表现：武器附魔发光颜色/粒子变化
```

### 13.4 阶段 9 验收标准

- 第二关卡可完整跑通，视觉风格与第一关明显不同
- 远程敌人有投射物攻击，可通过走位躲避
- 大型 Boss 有 3 阶段行为变化
- 至少实现 1 种高级战斗机制（空中连击/弹反/元素 三选一）
- 新关卡的敌人/奖励通过 DataAsset 配置，未硬编码

---

## 14. 阶段 10：最终打磨与完整作品集（第 43-48 周）

**前置依赖**：阶段 9 完成
**目标**：将项目打磨为完整的实习作品集，包含技术文档、展示视频和可运行 Build。

### 14.1 全流程QA测试

```
测试 Checklist：
  □ 主菜单 → 新游戏 → 关卡1全流程 → 关卡2全流程 → 胜利结算
  □ 任意点死亡 → 重生 → 继续正常
  □ 存档 → 退出 → 继续游戏 → 数据完整恢复
  □ 背包满 → 捡物品 → 不崩溃
  □ 装备/卸载 → 属性正确变化
  □ 连续 10 分钟战斗 → 帧率稳定 > 55 FPS
  □ 所有 NPC 对话可正常触发和完成
  □ 所有任务可正常接取/完成/领奖
  □ 所有品质的物品在 UI 中正确显示颜色
  □ 音频：BGM 切换无断裂，SFX 无缺失
```

### 14.2 作品集包装

```
最终交付物：

1. 可运行 Build（Windows 64-bit, Shipping 配置）
   包含完整的 2 个关卡流程

2. 展示视频（4-5 分钟，精心剪辑）
   00:00-00:30  角色渲染特写（多光照环境）
   00:30-01:00  角色物理展示（风/头发/布料/LOD 切换）
   01:00-02:30  关卡 1 战斗流程（连招/闪避/道具/Boss）
   02:30-03:30  关卡 2 战斗流程（新敌人/元素/大型Boss）
   03:30-04:30  系统展示（背包/装备/任务/对话/存档）
   04:30-05:00  技术分解（分屏对比/Shader Complexity/架构图）

3. 技术文档（PDF, 15-20页）
   ├── 项目架构总览图
   ├── 各系统技术说明
   │   ├── 战斗系统（接口设计/Combo 状态机/伤害管线）
   │   ├── 角色渲染（SSS/Hair/Cloth 参数文档 + 截图对比）
   │   ├── 物理系统（Wind/ClothLOD 组件设计文档）
   │   ├── 背包/装备/任务系统架构
   │   └── 性能优化报告（数据+图表）
   ├── 自研组件 API 文档
   └── 面对的技术挑战与解决方案

4. GitHub/Gitee 仓库
   README 包含：项目截图、技术栈、系统架构图、视频链接
   代码有基本的注释和文件头说明

5. 个人网站/ArtStation 页面（可选但加分）
   嵌入视频 + 技术截图 + 项目描述
```

### 14.3 预留缓冲期（第 49-52 周）

```
用途：
  - 处理测试中发现的 Bug
  - 根据他人反馈调整打击感/难度曲线
  - 准备面试话术（每个系统能讲 5 分钟）
  - 投递简历、准备面试
```

---

## 15. 完整一年时间线总览

```
月份    周数       阶段                               核心产出
─────────────────────────────────────────────────────────────────
月1     1-4       P0 地基修复 + P1 战斗核心             可打的连招 + 打击感
月2     5-8       P1 战斗完善 + P2 角色渲染             好看的角色 + 好打的战斗
月3     9-12      P2 渲染完善 + P3 关卡体验 + P4 打磨    ★ 第一个可展示版本 ★
─── 里程碑 1：最小可展示 Demo ──────────────────────────────────
月4     13-17     P5 装备系统 + 物品品质 + 冷却          装备穿戴 + 品质渲染
月5     18-21     P6 存档系统 + 菜单 + 死亡重生          完整游戏循环
─── 里程碑 2：完整可玩 Demo ───────────────────────────────────
月6     22-25     P7A 对话系统 + NPC                   可交互的世界
月7     26-27     P7B 任务系统                         目标驱动的游戏体验
─── 里程碑 3：有叙事的 Demo ───────────────────────────────────
月8     28-31     P8A 音频 + P8B 风格化渲染              听觉+视觉完整体验
月9     32-34     P8C 高级后处理                        画面表现力
月10    35-38     P9A 第二关卡 + 新敌人类型              内容量翻倍
月11    39-42     P9B 高级战斗机制                      战斗深度提升
─── 里程碑 4：内容丰富的完整作品 ──────────────────────────────
月12    43-48     P10 QA测试 + 作品集包装               可投递的完整作品集
        49-52     缓冲期：Bug 修复 + 面试准备             求职就绪
```

### 里程碑说明

```
★ 里程碑 1（月3, 第12周）：
  此时你已经有一个可以展示的最小 Demo。
  如果时间紧迫（比如暑假实习截止），可以在这里停下来打包。
  这个版本已经能展示：C++ 架构 + 战斗系统 + 角色渲染 + 物理系统。

★ 里程碑 2（月5, 第21周）：
  此时 Demo 有完整的"游戏感"——可以存档、可以死亡重生、有装备。
  作为大一暑假实习作品已经非常充分。

★ 里程碑 3（月7, 第27周）：
  有 NPC 和任务后，Demo 从"技术展示"升级为"游戏体验"。
  这对于偏 Gameplay 方向的实习申请更有说服力。

★ 里程碑 4（月11, 第42周）：
  两个关卡、多种敌人、高级战斗机制、完整音频——
  这已经不是 Demo 而是接近独立游戏原型的水平。
  对于大二暑假的实习申请，这个作品集非常有竞争力。
```

---

## 16. 风险预案（完整版）


| 风险               | 概率  | 影响       | 应对                                    |
| ---------------- | --- | -------- | ------------------------------------- |
| 攻击动画资产不足         | 高   | P1 阻塞    | Mixamo/Marketplace 获取临时动画             |
| 头发 Alpha 闪烁      | 中   | 视觉降级     | Masked + DitheredTemporalAA           |
| Boss AI 调试困难     | 中   | P3/P9 延期 | 先用 BehaviorTree 替代 StateTree          |
| 存档数据结构变更导致旧存档不兼容 | 高   | P6后反复出现  | SaveGame 加版本号字段，不兼容时重置                |
| 对话系统工作量超预期       | 中   | P7 延期    | 先实现线性对话，分支选项延后                        |
| 音频资产获取困难         | 中   | P8 延期    | 使用免费音效库（Freesound/Sonniss GDC Bundle） |
| 第二关卡美术资产不足       | 高   | P9 阻塞    | 使用 Marketplace 免费/廉价环境包               |
| 帧率在双关卡中不达标       | 中   | P10 延期   | 降低画质选项 / Scalability 设置               |
| 学业与开发时间冲突        | 高   | 全局延期     | 考试周暂停开发，保持每周至少 10 小时                  |
| 技术债务累积导致后期改动困难   | 中   | 后期阶段效率降低 | 每个里程碑后花 2-3 天重构/清理代码                  |


---

## 17. 不在一年范围内（远期 Backlog）

- 网络多人同步
- 多角色切换与队伍系统
- 程序化关卡生成
- Platforming / SideScrolling 变体模式
- 移动端适配
- Mod 支持
- 完整的剧情脚本与过场动画

