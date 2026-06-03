# 一键武器导入工具设计

日期：2026-06-03

## 1. 目标

为 `EscapeGame` 增加一个 Editor-only 的一键武器导入工具，用于把外部武器资源文件夹导入为项目可用的战斗武器数据。

第一版范围固定为“武器配置版”：

- 输入一个包含 `.obj` 与贴图 `.png` 的源文件夹。
- 自动导入 Static Mesh 与贴图。
- 自动创建材质并赋给导入的 Static Mesh。
- 自动创建 `UWeaponDefinition` DataAsset，并写入 Mesh、伤害、Trace Socket 名称等字段。
- 不自动修改角色蓝图或关卡资产。
- 不自动生成或调整 Mesh Socket 的空间位置。

## 2. 背景

当前战斗系统已经支持武器驱动的攻击判定：

- `AEscapeGameCharacter::BeginPlay` 会在 `DefaultWeaponDefinition` 有效时调用 `EquipWeapon`。
- `EquipWeapon` 会把 `WeaponDefinition` 与 `EquippedWeaponMesh` 传给 `UEscapeCombatComponent`。
- `UEscapeCombatComponent::DoAttackTrace` 会优先使用武器 Mesh 上的 `TraceStart` / `TraceEnd` Socket 作为判定起止点。
- 最终伤害使用 `WeaponDefinition.BaseDamage * FCombatActionDefinition.DamageMultiplier`。

因此导入工具的主要职责是减少重复资产创建和字段配置工作。

## 3. 推荐方案

采用 `Editor Utility Widget + C++ BlueprintFunctionLibrary`。

### 3.1 C++ 工具库

新增一个 Editor-only Blueprint Function Library，例如：

```cpp
UWeaponImportToolLibrary
```

核心函数建议：

```cpp
ImportWeaponFromObjFolder(
    SourceFolder,
    DestinationPath,
    AssetBaseName,
    BaseDamage,
    TraceRadius,
    AttachSocketName,
    TraceStartSocketName,
    TraceEndSocketName
)
```

函数返回一个结果结构体，包含：

- 是否成功。
- 创建的 Static Mesh 路径。
- 创建的 Material 路径。
- 创建的 WeaponDefinition 路径。
- 警告或错误信息。

### 3.2 Editor Utility Widget

新增 `EUW_WeaponImporter`，提供可视化输入：

- 源文件夹路径。
- 目标 Content 路径，例如 `/Game/Weapons/Sword_Darker`。
- 资产基础名，例如 `Sword_Darker`。
- `BaseDamage`，默认 `20`。
- `TraceRadius`，默认 `12`。
- `AttachSocketName`，默认 `WeaponSocket`。
- `TraceStartSocketName`，默认 `TraceStart`。
- `TraceEndSocketName`，默认 `TraceEnd`。

点击按钮后调用 C++ 工具库函数。

## 4. 命名规则

若 `AssetBaseName = Sword_Darker`，则生成：

- Static Mesh：`SM_Sword_Darker`
- Material：`M_Sword_Darker`
- Texture：保留原贴图名，必要时可加 `T_` 前缀作为后续优化
- WeaponDefinition：`DA_Weapon_Sword_Darker`

目标目录由用户指定，例如：

```text
/Game/Weapons/Sword_Darker
```

## 5. 导入与资产生成流程

1. 验证源文件夹存在。
2. 查找第一个 `.obj` 文件。
3. 查找包含 `Diffuse` 的 `.png` 贴图。
4. 可选查找包含 `Lightmap` 的 `.png` 贴图。
5. 使用 UE Editor 导入 API 导入 `.obj` 为 Static Mesh。
6. 导入贴图资源。
7. 创建材质：
   - Diffuse 贴图接 `Base Color`。
   - 默认 `Roughness = 0.45`。
   - 默认 `Metallic = 0.3`。
   - Lightmap 第一版不强制接入，只作为已导入资源保留。
8. 把材质赋给 Static Mesh 第 0 个材质槽。
9. 创建 `UWeaponDefinition`：
   - `WeaponMesh = SM_*`
   - `AttachSocketName = WeaponSocket`
   - `TraceStartSocketName = TraceStart`
   - `TraceEndSocketName = TraceEnd`
   - `TraceRadius = 用户输入`
   - `BaseDamage = 用户输入`
   - `DamageTypeTag = Data.Damage.Physical`
10. 保存新建和修改过的包。
11. 返回导入结果和操作提示。

## 6. 错误处理

工具需要明确报告以下情况：

- 源文件夹不存在。
- 找不到 `.obj` 文件。
- 找不到 Diffuse 贴图。
- 目标路径不是 `/Game/...` 格式。
- 资源导入失败。
- 材质创建失败。
- `WeaponDefinition` 创建失败。
- Static Mesh 没有 `TraceStart` / `TraceEnd` Socket。

Socket 缺失不应阻止工具完成导入，但应作为警告输出，因为 Socket 位置通常需要手工校准。

## 7. 非目标

第一版不做以下事情：

- 不自动修改 `BP_ThirdPersonCharacter` 或 `BP_Yuno`。
- 不自动把生成的 `WeaponDefinition` 填到 `DefaultWeaponDefinition`。
- 不自动生成精确武器 Socket 位置。
- 不做批量多文件夹导入。
- 不处理 FBX、glTF、骨骼武器或动画导入。
- 不做复杂 PBR 材质识别。

## 8. 验收标准

在 UE 编辑器中：

1. 用户选择一个包含 OBJ 与贴图的文件夹。
2. 点击导入按钮。
3. 目标 Content 路径下生成 Static Mesh、贴图、材质和 `DA_Weapon_*`。
4. Static Mesh 已绑定生成的材质。
5. `DA_Weapon_*` 的 `WeaponMesh`、`BaseDamage`、`TraceRadius` 和 Socket 名称正确。
6. 用户手动把 `DA_Weapon_*` 填到角色 `DefaultWeaponDefinition` 后，PIE 中角色能装备武器。
7. 若武器 Mesh 上存在 `TraceStart` / `TraceEnd` Socket，攻击 Trace 使用武器起止点。

## 9. 后续扩展

可在第一版稳定后扩展：

- 自动创建默认 `TraceStart` / `TraceEnd` Socket。
- 自动配置角色蓝图默认武器。
- 批量导入多个 OBJ 文件夹。
- 支持材质模板和更完整的贴图通道识别。
- 支持右键 Content Browser 菜单。
