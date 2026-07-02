# Weapon Importer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build an Editor-only weapon import backend that imports an OBJ weapon folder, creates mesh/texture/material assets, and creates a configured `UWeaponDefinition` for the combat component.

**Architecture:** Add a focused `UWeaponImportToolLibrary` Blueprint Function Library in the existing `EscapeGame` module. The function is safe to compile in non-editor builds, but performs import work only under `WITH_EDITOR`. The Editor Utility Widget asset is created in the UE editor after compilation and calls this C++ backend.

**Tech Stack:** UE 5.7 C++, `UBlueprintFunctionLibrary`, `AssetTools`, `AssetRegistry`, `UnrealEd`, `UWeaponDefinition`, Automation Tests.

---

## File Structure

- Create: `Source/EscapeGame/WeaponImportToolLibrary.h`  
  Defines `FWeaponImportResult` and `UWeaponImportToolLibrary`.

- Create: `Source/EscapeGame/WeaponImportToolLibrary.cpp`  
  Implements validation, file discovery, asset import, material creation, Static Mesh material assignment, `UWeaponDefinition` creation, and package saving.

- Create: `Source/EscapeGame/Tests/WeaponImportToolLibraryTest.cpp`  
  Adds editor automation tests for validation behavior.

- Modify: `Source/EscapeGame/EscapeGame.Build.cs`  
  Adds the editor-only `AssetRegistry` dependency. Existing editor dependencies already include `UnrealEd`, `AssetTools`, and `ContentBrowser`.

- Create: `Source/EscapeGame/Docs/Useful_Exp/weapon_importer_euw_setup.md`  
  Documents the one-time Editor Utility Widget setup inside UE.

Binary `.uasset` widgets are not created by text patch. The C++ backend will be available to an Editor Utility Widget, and the setup document will give exact in-editor steps for creating `EUW_WeaponImporter`.

## Task 1: Add Validation Tests

**Files:**
- Create: `Source/EscapeGame/Tests/WeaponImportToolLibraryTest.cpp`
- Create later in Task 2: `Source/EscapeGame/WeaponImportToolLibrary.h`

- [ ] **Step 1: Write validation automation tests**

Create `Source/EscapeGame/Tests/WeaponImportToolLibraryTest.cpp`:

```cpp
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "WeaponImportToolLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponImportToolInvalidSourceTest,
	"EscapeGame.Editor.WeaponImport.InvalidSourceFolder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FWeaponImportToolInvalidSourceTest::RunTest(const FString& Parameters)
{
	const FWeaponImportResult Result = UWeaponImportToolLibrary::ImportWeaponFromObjFolder(
		TEXT("Z:/Path/That/Does/Not/Exist"),
		TEXT("/Game/Weapons/TestSword"),
		TEXT("TestSword"),
		20.0f,
		12.0f,
		TEXT("WeaponSocket"),
		TEXT("TraceStart"),
		TEXT("TraceEnd")
	);

	TestFalse(TEXT("Invalid source folder must fail"), Result.bSucceeded);
	TestTrue(TEXT("Invalid source folder returns a message"), Result.Messages.Num() > 0);
	TestTrue(
		TEXT("Message mentions source folder"),
		Result.Messages[0].Contains(TEXT("源文件夹不存在"))
	);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponImportToolInvalidDestinationTest,
	"EscapeGame.Editor.WeaponImport.InvalidDestinationPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FWeaponImportToolInvalidDestinationTest::RunTest(const FString& Parameters)
{
	const FString ExistingFolder = FPaths::ProjectDir();
	const FWeaponImportResult Result = UWeaponImportToolLibrary::ImportWeaponFromObjFolder(
		ExistingFolder,
		TEXT("Weapons/TestSword"),
		TEXT("TestSword"),
		20.0f,
		12.0f,
		TEXT("WeaponSocket"),
		TEXT("TraceStart"),
		TEXT("TraceEnd")
	);

	TestFalse(TEXT("Destination path outside /Game must fail"), Result.bSucceeded);
	TestTrue(TEXT("Invalid destination returns a message"), Result.Messages.Num() > 0);
	TestTrue(
		TEXT("Message mentions /Game"),
		Result.Messages[0].Contains(TEXT("/Game"))
	);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponImportToolInvalidAssetNameTest,
	"EscapeGame.Editor.WeaponImport.InvalidAssetName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FWeaponImportToolInvalidAssetNameTest::RunTest(const FString& Parameters)
{
	const FString ExistingFolder = FPaths::ProjectDir();
	const FWeaponImportResult Result = UWeaponImportToolLibrary::ImportWeaponFromObjFolder(
		ExistingFolder,
		TEXT("/Game/Weapons/TestSword"),
		TEXT(""),
		20.0f,
		12.0f,
		TEXT("WeaponSocket"),
		TEXT("TraceStart"),
		TEXT("TraceEnd")
	);

	TestFalse(TEXT("Empty asset name must fail"), Result.bSucceeded);
	TestTrue(TEXT("Invalid asset name returns a message"), Result.Messages.Num() > 0);
	TestTrue(
		TEXT("Message mentions asset name"),
		Result.Messages[0].Contains(TEXT("资产基础名不能为空"))
	);
	return true;
}

#endif
```

- [ ] **Step 2: Build to verify the test fails because the tool class is missing**

Build `EscapeGameEditor` from Visual Studio or Rider.

Expected result:

```text
fatal error C1083: Cannot open include file: 'WeaponImportToolLibrary.h'
```

## Task 2: Add Tool API and Validation

**Files:**
- Create: `Source/EscapeGame/WeaponImportToolLibrary.h`
- Create: `Source/EscapeGame/WeaponImportToolLibrary.cpp`

- [ ] **Step 1: Create the public API header**

Create `Source/EscapeGame/WeaponImportToolLibrary.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WeaponImportToolLibrary.generated.h"

USTRUCT(BlueprintType)
struct ESCAPEGAME_API FWeaponImportResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	bool bSucceeded = false;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	FString StaticMeshObjectPath;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	FString MaterialObjectPath;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	FString WeaponDefinitionObjectPath;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Import")
	TArray<FString> Messages;

	void AddMessage(const FString& Message)
	{
		Messages.Add(Message);
	}
};

UCLASS()
class ESCAPEGAME_API UWeaponImportToolLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "TATools|Weapon Import", meta = (DevelopmentOnly))
	static FWeaponImportResult ImportWeaponFromObjFolder(
		const FString& SourceFolder,
		const FString& DestinationPath,
		const FString& AssetBaseName,
		float BaseDamage = 20.0f,
		float TraceRadius = 12.0f,
		FName AttachSocketName = TEXT("WeaponSocket"),
		FName TraceStartSocketName = TEXT("TraceStart"),
		FName TraceEndSocketName = TEXT("TraceEnd")
	);
};
```

- [ ] **Step 2: Implement validation-only behavior**

Create `Source/EscapeGame/WeaponImportToolLibrary.cpp`:

```cpp
#include "WeaponImportToolLibrary.h"

#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

namespace WeaponImportTool
{
	bool IsValidGameDestinationPath(const FString& DestinationPath)
	{
		return DestinationPath.StartsWith(TEXT("/Game/")) &&
			FPackageName::IsValidLongPackageName(DestinationPath);
	}
}

FWeaponImportResult UWeaponImportToolLibrary::ImportWeaponFromObjFolder(
	const FString& SourceFolder,
	const FString& DestinationPath,
	const FString& AssetBaseName,
	float BaseDamage,
	float TraceRadius,
	FName AttachSocketName,
	FName TraceStartSocketName,
	FName TraceEndSocketName
)
{
	FWeaponImportResult Result;

	if (SourceFolder.IsEmpty() || !IFileManager::Get().DirectoryExists(*SourceFolder))
	{
		Result.AddMessage(FString::Printf(TEXT("源文件夹不存在：%s"), *SourceFolder));
		return Result;
	}

	if (!WeaponImportTool::IsValidGameDestinationPath(DestinationPath))
	{
		Result.AddMessage(FString::Printf(TEXT("目标路径必须是有效的 /Game/... 路径：%s"), *DestinationPath));
		return Result;
	}

	if (AssetBaseName.TrimStartAndEnd().IsEmpty())
	{
		Result.AddMessage(TEXT("资产基础名不能为空。"));
		return Result;
	}

	if (BaseDamage < 0.0f)
	{
		Result.AddMessage(TEXT("BaseDamage 不能小于 0。"));
		return Result;
	}

	if (TraceRadius <= 0.0f)
	{
		Result.AddMessage(TEXT("TraceRadius 必须大于 0。"));
		return Result;
	}

#if WITH_EDITOR
	Result.AddMessage(TEXT("参数校验通过，导入流程将在后续任务中接入。"));
#else
	Result.AddMessage(TEXT("武器导入工具只能在 UE Editor 中执行。"));
#endif

	return Result;
}
```

- [ ] **Step 3: Build and run validation tests**

Build `EscapeGameEditor`, then run these automation tests in UE:

```text
Window -> Developer Tools -> Session Frontend -> Automation
Search: EscapeGame.Editor.WeaponImport
Run the three visible tests.
```

Expected result:

```text
InvalidSourceFolder: Passed
InvalidDestinationPath: Passed
InvalidAssetName: Passed
```

- [ ] **Step 4: Commit Task 2**

```bash
git add Source/EscapeGame/WeaponImportToolLibrary.h Source/EscapeGame/WeaponImportToolLibrary.cpp Source/EscapeGame/Tests/WeaponImportToolLibraryTest.cpp
git commit -m "test: add weapon importer validation"
```

## Task 3: Add Editor Dependencies and File Discovery

**Files:**
- Modify: `Source/EscapeGame/EscapeGame.Build.cs`
- Modify: `Source/EscapeGame/WeaponImportToolLibrary.cpp`

- [ ] **Step 1: Add editor-only dependency**

In `Source/EscapeGame/EscapeGame.Build.cs`, update the `Target.bBuildEditor` dependency list:

```csharp
if (Target.bBuildEditor == true)
{
    PrivateDependencyModuleNames.AddRange(new string[] {
        "NiagaraEditor",
        "UnrealEd",
        "AssetTools",
        "AssetRegistry",
        "ContentBrowser"
    });
}
```

- [ ] **Step 2: Add file discovery helpers**

In `Source/EscapeGame/WeaponImportToolLibrary.cpp`, extend the namespace:

```cpp
namespace WeaponImportTool
{
	bool IsValidGameDestinationPath(const FString& DestinationPath)
	{
		return DestinationPath.StartsWith(TEXT("/Game/")) &&
			FPackageName::IsValidLongPackageName(DestinationPath);
	}

	FString FindFirstFileByExtension(const FString& SourceFolder, const FString& Extension)
	{
		TArray<FString> Files;
		IFileManager::Get().FindFilesRecursive(
			Files,
			*SourceFolder,
			*FString::Printf(TEXT("*.%s"), *Extension),
			true,
			false
		);

		Files.Sort();
		return Files.Num() > 0 ? Files[0] : FString();
	}

	FString FindFirstTextureByKeyword(const FString& SourceFolder, const FString& Keyword)
	{
		TArray<FString> Files;
		IFileManager::Get().FindFilesRecursive(
			Files,
			*SourceFolder,
			TEXT("*.png"),
			true,
			false
		);

		Files.Sort();
		for (const FString& File : Files)
		{
			if (FPaths::GetBaseFilename(File).Contains(Keyword, ESearchCase::IgnoreCase))
			{
				return File;
			}
		}

		return FString();
	}
}
```

- [ ] **Step 3: Validate required OBJ and Diffuse files**

Inside `ImportWeaponFromObjFolder`, after numeric validation and before editor work:

```cpp
const FString ObjFile = WeaponImportTool::FindFirstFileByExtension(SourceFolder, TEXT("obj"));
if (ObjFile.IsEmpty())
{
	Result.AddMessage(FString::Printf(TEXT("源文件夹中没有找到 .obj 文件：%s"), *SourceFolder));
	return Result;
}

const FString DiffuseTextureFile = WeaponImportTool::FindFirstTextureByKeyword(SourceFolder, TEXT("Diffuse"));
if (DiffuseTextureFile.IsEmpty())
{
	Result.AddMessage(FString::Printf(TEXT("源文件夹中没有找到包含 Diffuse 的 .png 贴图：%s"), *SourceFolder));
	return Result;
}

const FString LightmapTextureFile = WeaponImportTool::FindFirstTextureByKeyword(SourceFolder, TEXT("Lightmap"));
```

- [ ] **Step 4: Run validation tests again**

Run:

```text
Session Frontend -> Automation -> EscapeGame.Editor.WeaponImport
```

Expected result:

```text
The three validation tests remain Passed.
```

- [ ] **Step 5: Commit Task 3**

```bash
git add Source/EscapeGame/EscapeGame.Build.cs Source/EscapeGame/WeaponImportToolLibrary.cpp
git commit -m "feat: add weapon importer file discovery"
```

## Task 4: Implement Asset Import

**Files:**
- Modify: `Source/EscapeGame/WeaponImportToolLibrary.cpp`

- [ ] **Step 1: Add editor imports**

At the top of `WeaponImportToolLibrary.cpp`, add:

```cpp
#if WITH_EDITOR
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "ObjectTools.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#endif
```

- [ ] **Step 2: Add import helper**

Inside the namespace, add this editor-only helper:

```cpp
#if WITH_EDITOR
UObject* ImportSingleAsset(const FString& Filename, const FString& DestinationPath, FWeaponImportResult& Result)
{
	UAssetImportTask* ImportTask = NewObject<UAssetImportTask>();
	ImportTask->Filename = Filename;
	ImportTask->DestinationPath = DestinationPath;
	ImportTask->bAutomated = true;
	ImportTask->bReplaceExisting = false;
	ImportTask->bSave = false;

	TArray<UAssetImportTask*> Tasks;
	Tasks.Add(ImportTask);

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().ImportAssetTasks(Tasks);

	if (ImportTask->ImportedObjectPaths.Num() == 0)
	{
		Result.AddMessage(FString::Printf(TEXT("资源导入失败：%s"), *Filename));
		return nullptr;
	}

	UObject* ImportedObject = LoadObject<UObject>(nullptr, *ImportTask->ImportedObjectPaths[0]);
	if (!ImportedObject)
	{
		Result.AddMessage(FString::Printf(TEXT("资源导入后加载失败：%s"), *ImportTask->ImportedObjectPaths[0]));
		return nullptr;
	}

	return ImportedObject;
}
#endif
```

- [ ] **Step 3: Import OBJ and textures**

Replace the temporary editor success message in `ImportWeaponFromObjFolder` with:

```cpp
#if WITH_EDITOR
UObject* ImportedMeshObject = WeaponImportTool::ImportSingleAsset(ObjFile, DestinationPath, Result);
UStaticMesh* ImportedMesh = Cast<UStaticMesh>(ImportedMeshObject);
if (!ImportedMesh)
{
	Result.AddMessage(TEXT("OBJ 没有导入为 Static Mesh。"));
	return Result;
}

UObject* ImportedDiffuseObject = WeaponImportTool::ImportSingleAsset(DiffuseTextureFile, DestinationPath, Result);
UTexture2D* DiffuseTexture = Cast<UTexture2D>(ImportedDiffuseObject);
if (!DiffuseTexture)
{
	Result.AddMessage(TEXT("Diffuse 贴图没有导入为 Texture2D。"));
	return Result;
}

if (!LightmapTextureFile.IsEmpty())
{
	WeaponImportTool::ImportSingleAsset(LightmapTextureFile, DestinationPath, Result);
}

Result.StaticMeshObjectPath = ImportedMesh->GetPathName();
Result.AddMessage(FString::Printf(TEXT("已导入 Static Mesh：%s"), *Result.StaticMeshObjectPath));
Result.AddMessage(FString::Printf(TEXT("已导入 Diffuse 贴图：%s"), *DiffuseTexture->GetPathName()));
#else
Result.AddMessage(TEXT("武器导入工具只能在 UE Editor 中执行。"));
return Result;
#endif
```

- [ ] **Step 4: Manual import smoke test**

In UE Editor:

```text
Create any temporary Editor Utility Widget.
Call ImportWeaponFromObjFolder with:
SourceFolder = the folder that contains Equip_Sword_Darker.obj
DestinationPath = /Game/Weapons/Sword_Darker
AssetBaseName = Sword_Darker
BaseDamage = 20
TraceRadius = 12
```

Expected result:

```text
/Game/Weapons/Sword_Darker contains the imported Static Mesh and Diffuse texture.
The function result contains StaticMeshObjectPath.
The function has not created Material or WeaponDefinition yet.
```

- [ ] **Step 5: Commit Task 4**

```bash
git add Source/EscapeGame/WeaponImportToolLibrary.cpp
git commit -m "feat: import weapon mesh and textures"
```

## Task 5: Create Material and Assign It

**Files:**
- Modify: `Source/EscapeGame/WeaponImportToolLibrary.cpp`

- [ ] **Step 1: Add material imports**

At the top of `WeaponImportToolLibrary.cpp`, inside `#if WITH_EDITOR`, add:

```cpp
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "UObject/Package.h"
```

- [ ] **Step 2: Add package and material helper**

Inside the namespace, add:

```cpp
#if WITH_EDITOR
FString BuildAssetPackageName(const FString& DestinationPath, const FString& AssetName)
{
	return DestinationPath / AssetName;
}

UMaterial* CreateBasicDiffuseMaterial(
	const FString& DestinationPath,
	const FString& AssetBaseName,
	UTexture2D* DiffuseTexture,
	FWeaponImportResult& Result
)
{
	const FString MaterialName = FString::Printf(TEXT("M_%s"), *ObjectTools::SanitizeObjectName(AssetBaseName));
	const FString PackageName = BuildAssetPackageName(DestinationPath, MaterialName);

	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		Result.AddMessage(FString::Printf(TEXT("材质包创建失败：%s"), *PackageName));
		return nullptr;
	}

	UMaterial* Material = NewObject<UMaterial>(
		Package,
		*MaterialName,
		RF_Public | RF_Standalone
	);

	if (!Material)
	{
		Result.AddMessage(FString::Printf(TEXT("材质创建失败：%s"), *MaterialName));
		return nullptr;
	}

	UMaterialExpressionTextureSample* TextureSample =
		NewObject<UMaterialExpressionTextureSample>(Material);
	TextureSample->Texture = DiffuseTexture;
	TextureSample->MaterialExpressionEditorX = -400;
	TextureSample->MaterialExpressionEditorY = 0;
	Material->GetExpressionCollection().AddExpression(TextureSample);
	Material->GetEditorOnlyData()->BaseColor.Expression = TextureSample;

	UMaterialExpressionConstant* Roughness =
		NewObject<UMaterialExpressionConstant>(Material);
	Roughness->R = 0.45f;
	Roughness->MaterialExpressionEditorX = -400;
	Roughness->MaterialExpressionEditorY = 160;
	Material->GetExpressionCollection().AddExpression(Roughness);
	Material->GetEditorOnlyData()->Roughness.Expression = Roughness;

	UMaterialExpressionConstant* Metallic =
		NewObject<UMaterialExpressionConstant>(Material);
	Metallic->R = 0.3f;
	Metallic->MaterialExpressionEditorX = -400;
	Metallic->MaterialExpressionEditorY = 320;
	Material->GetExpressionCollection().AddExpression(Metallic);
	Material->GetEditorOnlyData()->Metallic.Expression = Metallic;

	FAssetRegistryModule::AssetCreated(Material);
	Material->PostEditChange();
	Material->MarkPackageDirty();

	Result.MaterialObjectPath = Material->GetPathName();
	Result.AddMessage(FString::Printf(TEXT("已创建材质：%s"), *Result.MaterialObjectPath));
	return Material;
}
#endif
```

- [ ] **Step 3: Assign material to the imported mesh**

After texture import succeeds:

```cpp
UMaterial* Material = WeaponImportTool::CreateBasicDiffuseMaterial(
	DestinationPath,
	AssetBaseName,
	DiffuseTexture,
	Result
);

if (!Material)
{
	return Result;
}

ImportedMesh->SetMaterial(0, Material);
ImportedMesh->MarkPackageDirty();
Result.AddMessage(TEXT("已把生成材质赋给 Static Mesh 的第 0 个材质槽。"));
```

- [ ] **Step 4: Manual material smoke test**

In UE Editor, call the import function again with a clean destination path:

```text
DestinationPath = /Game/Weapons/Sword_Darker_MaterialTest
AssetBaseName = Sword_Darker_MaterialTest
```

Expected result:

```text
M_Sword_Darker_MaterialTest exists.
The material has a TextureSample connected to Base Color.
SM_* uses M_Sword_Darker_MaterialTest in material slot 0.
```

- [ ] **Step 5: Commit Task 5**

```bash
git add Source/EscapeGame/WeaponImportToolLibrary.cpp
git commit -m "feat: create material for imported weapon"
```

## Task 6: Create WeaponDefinition

**Files:**
- Modify: `Source/EscapeGame/WeaponImportToolLibrary.cpp`

- [ ] **Step 1: Add required includes**

At the top of `WeaponImportToolLibrary.cpp`, add:

```cpp
#include "EscapeGameplayTags.h"
#include "WeaponDefinition.h"
```

- [ ] **Step 2: Add WeaponDefinition helper**

Inside the namespace, add:

```cpp
#if WITH_EDITOR
UWeaponDefinition* CreateWeaponDefinitionAsset(
	const FString& DestinationPath,
	const FString& AssetBaseName,
	UStaticMesh* ImportedMesh,
	float BaseDamage,
	float TraceRadius,
	FName AttachSocketName,
	FName TraceStartSocketName,
	FName TraceEndSocketName,
	FWeaponImportResult& Result
)
{
	const FString DataAssetName = FString::Printf(TEXT("DA_Weapon_%s"), *ObjectTools::SanitizeObjectName(AssetBaseName));
	const FString PackageName = BuildAssetPackageName(DestinationPath, DataAssetName);

	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		Result.AddMessage(FString::Printf(TEXT("WeaponDefinition 包创建失败：%s"), *PackageName));
		return nullptr;
	}

	UWeaponDefinition* WeaponDefinition = NewObject<UWeaponDefinition>(
		Package,
		*DataAssetName,
		RF_Public | RF_Standalone
	);

	if (!WeaponDefinition)
	{
		Result.AddMessage(FString::Printf(TEXT("WeaponDefinition 创建失败：%s"), *DataAssetName));
		return nullptr;
	}

	WeaponDefinition->WeaponMesh = ImportedMesh;
	WeaponDefinition->AttachSocketName = AttachSocketName;
	WeaponDefinition->TraceStartSocketName = TraceStartSocketName;
	WeaponDefinition->TraceEndSocketName = TraceEndSocketName;
	WeaponDefinition->TraceRadius = TraceRadius;
	WeaponDefinition->BaseDamage = BaseDamage;
	WeaponDefinition->DamageTypeTag = EscapeGameplayTags::Data_Damage_Physical;
	WeaponDefinition->bConsumeOnUse = false;
	WeaponDefinition->bStackable = false;
	WeaponDefinition->MaxStackCount = 1;

	FAssetRegistryModule::AssetCreated(WeaponDefinition);
	WeaponDefinition->MarkPackageDirty();

	Result.WeaponDefinitionObjectPath = WeaponDefinition->GetPathName();
	Result.AddMessage(FString::Printf(TEXT("已创建 WeaponDefinition：%s"), *Result.WeaponDefinitionObjectPath));
	return WeaponDefinition;
}
#endif
```

- [ ] **Step 3: Create WeaponDefinition in the main flow**

After material assignment:

```cpp
UWeaponDefinition* WeaponDefinition = WeaponImportTool::CreateWeaponDefinitionAsset(
	DestinationPath,
	AssetBaseName,
	ImportedMesh,
	BaseDamage,
	TraceRadius,
	AttachSocketName,
	TraceStartSocketName,
	TraceEndSocketName,
	Result
);

if (!WeaponDefinition)
{
	return Result;
}
```

- [ ] **Step 4: Warn about missing trace sockets**

After creating the weapon definition:

```cpp
if (!ImportedMesh->FindSocket(TraceStartSocketName))
{
	Result.AddMessage(FString::Printf(TEXT("警告：Static Mesh 缺少 Trace 起点 Socket：%s"), *TraceStartSocketName.ToString()));
}

if (!ImportedMesh->FindSocket(TraceEndSocketName))
{
	Result.AddMessage(FString::Printf(TEXT("警告：Static Mesh 缺少 Trace 终点 Socket：%s"), *TraceEndSocketName.ToString()));
}
```

- [ ] **Step 5: Mark success**

At the end of the editor branch:

```cpp
Result.bSucceeded = true;
Result.AddMessage(TEXT("武器导入完成。请检查 Static Mesh Socket，并把生成的 DA_Weapon_* 配到角色 DefaultWeaponDefinition。"));
return Result;
```

- [ ] **Step 6: Manual WeaponDefinition smoke test**

In UE Editor, call the import function:

```text
DestinationPath = /Game/Weapons/Sword_Darker_DefinitionTest
AssetBaseName = Sword_Darker_DefinitionTest
BaseDamage = 25
TraceRadius = 14
```

Expected result:

```text
DA_Weapon_Sword_Darker_DefinitionTest exists.
WeaponMesh points to the imported SM_* asset.
BaseDamage is 25.
TraceRadius is 14.
AttachSocketName is WeaponSocket.
TraceStartSocketName is TraceStart.
TraceEndSocketName is TraceEnd.
DamageTypeTag is Data.Damage.Physical.
```

- [ ] **Step 7: Commit Task 6**

```bash
git add Source/EscapeGame/WeaponImportToolLibrary.cpp
git commit -m "feat: create weapon definition from import"
```

## Task 7: Save Packages and Improve Result Paths

**Files:**
- Modify: `Source/EscapeGame/WeaponImportToolLibrary.cpp`

- [ ] **Step 1: Add save include**

Inside the editor include block:

```cpp
#include "FileHelpers.h"
```

- [ ] **Step 2: Add package save helper**

Inside the namespace:

```cpp
#if WITH_EDITOR
void SaveDirtyPackagesForAssets(const TArray<UObject*>& Assets, FWeaponImportResult& Result)
{
	TArray<UPackage*> PackagesToSave;

	for (UObject* Asset : Assets)
	{
		if (!Asset)
		{
			continue;
		}

		UPackage* Package = Asset->GetOutermost();
		if (Package && Package->IsDirty())
		{
			PackagesToSave.AddUnique(Package);
		}
	}

	if (PackagesToSave.Num() == 0)
	{
		Result.AddMessage(TEXT("没有需要保存的资源包。"));
		return;
	}

	UEditorLoadingAndSavingUtils::SavePackages(
		PackagesToSave,
		false
	);

	Result.AddMessage(FString::Printf(TEXT("已保存资源包数量：%d"), PackagesToSave.Num()));
}
#endif
```

- [ ] **Step 3: Save created and modified assets**

After socket warnings and before success:

```cpp
TArray<UObject*> AssetsToSave;
AssetsToSave.Add(ImportedMesh);
AssetsToSave.Add(DiffuseTexture);
AssetsToSave.Add(Material);
AssetsToSave.Add(WeaponDefinition);
WeaponImportTool::SaveDirtyPackagesForAssets(AssetsToSave, Result);
```

- [ ] **Step 4: Run manual full import smoke test**

In UE Editor, call:

```text
SourceFolder = the folder that contains Equip_Sword_Darker.obj
DestinationPath = /Game/Weapons/Sword_Darker_FullTest
AssetBaseName = Sword_Darker_FullTest
BaseDamage = 20
TraceRadius = 12
```

Expected result:

```text
Result.bSucceeded is true.
StaticMeshObjectPath is not empty.
MaterialObjectPath is not empty.
WeaponDefinitionObjectPath is not empty.
The imported assets remain after closing and reopening the editor.
```

- [ ] **Step 5: Commit Task 7**

```bash
git add Source/EscapeGame/WeaponImportToolLibrary.cpp
git commit -m "feat: save weapon importer assets"
```

## Task 8: Document Editor Utility Widget Setup

**Files:**
- Create: `Source/EscapeGame/Docs/Useful_Exp/weapon_importer_euw_setup.md`

- [ ] **Step 1: Write setup document**

Create `Source/EscapeGame/Docs/Useful_Exp/weapon_importer_euw_setup.md`:

```markdown
# Weapon Importer EUW Setup

## 1. Create Widget

1. Open UE Editor.
2. In Content Browser, create folder `/Game/EditorTools`.
3. Right click -> Editor Utilities -> Editor Utility Widget.
4. Name it `EUW_WeaponImporter`.

## 2. Add Inputs

Add these variables to the widget:

- `SourceFolder` as String.
- `DestinationPath` as String, default `/Game/Weapons/Sword_Darker`.
- `AssetBaseName` as String, default `Sword_Darker`.
- `BaseDamage` as Float, default `20`.
- `TraceRadius` as Float, default `12`.
- `AttachSocketName` as Name, default `WeaponSocket`.
- `TraceStartSocketName` as Name, default `TraceStart`.
- `TraceEndSocketName` as Name, default `TraceEnd`.

Expose them on the widget with editable text boxes or detail-bound variables.

## 3. Add Button Logic

Create a button named `Import`.

On Clicked:

1. Call `ImportWeaponFromObjFolder`.
2. Pass all widget variables into the function.
3. Print each returned `Messages` entry to the Output Log.
4. If `bSucceeded` is true, print `WeaponDefinitionObjectPath`.

## 4. Use Result

After import:

1. Open the generated Static Mesh.
2. Add or adjust `TraceStart` and `TraceEnd` sockets.
3. Open the player character blueprint.
4. Set `DefaultWeaponDefinition` to the generated `DA_Weapon_*`.
5. PIE and test attack trace.
```

- [ ] **Step 2: Commit Task 8**

```bash
git add Source/EscapeGame/Docs/Useful_Exp/weapon_importer_euw_setup.md
git commit -m "docs: add weapon importer widget setup"
```

## Task 9: Final Verification

**Files:**
- Read: `Source/EscapeGame/WeaponImportToolLibrary.h`
- Read: `Source/EscapeGame/WeaponImportToolLibrary.cpp`
- Read: `Source/EscapeGame/EscapeGame.Build.cs`
- Read: `Source/EscapeGame/Tests/WeaponImportToolLibraryTest.cpp`
- Read: `Source/EscapeGame/Docs/Useful_Exp/weapon_importer_euw_setup.md`

- [ ] **Step 1: Run automation validation tests**

In UE Editor:

```text
Window -> Developer Tools -> Session Frontend -> Automation
Search: EscapeGame.Editor.WeaponImport
Run all matching tests.
```

Expected:

```text
InvalidSourceFolder: Passed
InvalidDestinationPath: Passed
InvalidAssetName: Passed
```

- [ ] **Step 2: Run full manual import**

Use the sword folder shown in the conversation:

```text
SourceFolder = the folder containing Equip_Sword_Darker.obj
DestinationPath = /Game/Weapons/Sword_Darker
AssetBaseName = Sword_Darker
BaseDamage = 20
TraceRadius = 12
AttachSocketName = WeaponSocket
TraceStartSocketName = TraceStart
TraceEndSocketName = TraceEnd
```

Expected:

```text
/Game/Weapons/Sword_Darker/SM_* exists.
/Game/Weapons/Sword_Darker/M_* exists.
/Game/Weapons/Sword_Darker/DA_Weapon_Sword_Darker exists.
DA_Weapon_Sword_Darker.WeaponMesh points to the imported mesh.
DA_Weapon_Sword_Darker.BaseDamage is 20.
DA_Weapon_Sword_Darker.TraceRadius is 12.
```

- [ ] **Step 3: Run combat linkage test**

In UE Editor:

```text
1. Add TraceStart and TraceEnd sockets to the imported Static Mesh.
2. Add WeaponSocket to the character skeleton if it is missing.
3. Set player character DefaultWeaponDefinition to DA_Weapon_Sword_Darker.
4. PIE.
5. Press attack.
```

Expected:

```text
The weapon appears in the character hand.
Attack Montage plays.
DoAttackTrace uses the weapon trace sockets when they exist.
A target implementing UEscapeCombatDamageable receives ApplyDamage.
```

- [ ] **Step 4: Commit final cleanup**

If Task 9 found small compile or include fixes, commit them:

```bash
git add Source/EscapeGame
git commit -m "chore: verify weapon importer"
```

If Task 9 found no changes, skip this commit.

## Self-Review

Spec coverage:

- OBJ folder import: Task 3 and Task 4.
- Diffuse and Lightmap discovery: Task 3 and Task 4.
- Material creation: Task 5.
- Static Mesh material assignment: Task 5.
- `UWeaponDefinition` creation: Task 6.
- Socket warning behavior: Task 6.
- Package saving: Task 7.
- EUW use path: Task 8.
- Validation and testing: Task 1, Task 2, and Task 9.

Placeholder scan:

- No banned placeholder markers.
- No unspecified test names.
- Each code task includes concrete file paths and code blocks.

Type consistency:

- `FWeaponImportResult` is defined before tests use it.
- `ImportWeaponFromObjFolder` signature is identical in tests, header, and implementation.
- `UWeaponDefinition` field names match `WeaponDefinition.h`.
- `EscapeGameplayTags::Data_Damage_Physical` matches `EscapeGameplayTags.h`.
