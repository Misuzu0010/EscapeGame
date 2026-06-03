#include "WeaponImportToolLibrary.h"

#include "EscapeGameplayTags.h"
#include "WeaponDefinition.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "ObjectTools.h"
#include "UObject/Package.h"
#endif

namespace WeaponImportTool
{
#if WITH_EDITOR
	FString BuildAssetPackageName(const FString& DestinationPath, const FString& AssetName)
	{
		return DestinationPath / AssetName;
	}

	FString JoinMessages(const TArray<FString>& Messages)
	{
		return FString::Join(Messages, TEXT(" | "));
	}

	UObject* ImportSingleAsset(const FString& Filename, const FString& DestinationPath, FWeaponImportResult& Result, const UClass* ExpectedClass = nullptr)
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
			Result.AddMessage(FString::Printf(TEXT("\u8D44\u6E90\u5BFC\u5165\u5931\u8D25\uFF1A%s"), *Filename));
			return nullptr;
		}

		UObject* FirstImportedObject = nullptr;
		for (const FString& ImportedObjectPath : ImportTask->ImportedObjectPaths)
		{
			UObject* ImportedObject = LoadObject<UObject>(nullptr, *ImportedObjectPath);
			if (!ImportedObject)
			{
				continue;
			}

			if (!FirstImportedObject)
			{
				FirstImportedObject = ImportedObject;
			}

			if (!ExpectedClass || ImportedObject->IsA(ExpectedClass))
			{
				return ImportedObject;
			}
		}

		if (ExpectedClass)
		{
			Result.AddMessage(FString::Printf(TEXT("Imported asset did not match expected class %s: %s"), *ExpectedClass->GetName(), *Filename));
			return nullptr;
		}

		if (!FirstImportedObject)
		{
			Result.AddMessage(FString::Printf(TEXT("\u8D44\u6E90\u5BFC\u5165\u540E\u52A0\u8F7D\u5931\u8D25\uFF1A%s"), *ImportTask->ImportedObjectPaths[0]));
		}

		return FirstImportedObject;
	}

	UMaterial* CreateBasicDiffuseMaterial(
		const FString& DestinationPath,
		const FString& AssetBaseName,
		UTexture2D* DiffuseTexture,
		FWeaponImportResult& Result
	)
	{
		const FString SanitizedAssetBaseName = ObjectTools::SanitizeObjectName(AssetBaseName);
		if (SanitizedAssetBaseName.IsEmpty())
		{
			Result.AddMessage(FString::Printf(TEXT("Material create failed: sanitized asset base name is empty: %s"), *AssetBaseName));
			return nullptr;
		}

		const FString DesiredMaterialName = FString::Printf(TEXT("M_%s"), *SanitizedAssetBaseName);
		const FString DesiredPackageName = BuildAssetPackageName(DestinationPath, DesiredMaterialName);
		FString PackageName;
		FString MaterialName;

		FAssetToolsModule& AssetToolsModule =
			FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().CreateUniqueAssetName(
			DesiredPackageName,
			TEXT(""),
			PackageName,
			MaterialName
		);

		UPackage* Package = CreatePackage(*PackageName);
		if (!Package)
		{
			Result.AddMessage(FString::Printf(TEXT("Material package create failed: %s"), *PackageName));
			return nullptr;
		}

		UMaterial* Material = NewObject<UMaterial>(
			Package,
			*MaterialName,
			RF_Public | RF_Standalone
		);

		if (!Material)
		{
			Result.AddMessage(FString::Printf(TEXT("Material create failed: %s"), *MaterialName));
			return nullptr;
		}

		UMaterialExpressionTextureSample* TextureSample =
			NewObject<UMaterialExpressionTextureSample>(Material);
		if (!TextureSample)
		{
			Result.AddMessage(FString::Printf(TEXT("Material texture sample create failed: %s"), *MaterialName));
			return nullptr;
		}

		TextureSample->Texture = DiffuseTexture;
		TextureSample->MaterialExpressionEditorX = -400;
		TextureSample->MaterialExpressionEditorY = 0;
		Material->GetExpressionCollection().AddExpression(TextureSample);
		Material->GetEditorOnlyData()->BaseColor.Expression = TextureSample;

		UMaterialExpressionConstant* Roughness =
			NewObject<UMaterialExpressionConstant>(Material);
		if (!Roughness)
		{
			Result.AddMessage(FString::Printf(TEXT("Material roughness constant create failed: %s"), *MaterialName));
			return nullptr;
		}

		Roughness->R = 0.45f;
		Roughness->MaterialExpressionEditorX = -400;
		Roughness->MaterialExpressionEditorY = 160;
		Material->GetExpressionCollection().AddExpression(Roughness);
		Material->GetEditorOnlyData()->Roughness.Expression = Roughness;

		UMaterialExpressionConstant* Metallic =
			NewObject<UMaterialExpressionConstant>(Material);
		if (!Metallic)
		{
			Result.AddMessage(FString::Printf(TEXT("Material metallic constant create failed: %s"), *MaterialName));
			return nullptr;
		}

		Metallic->R = 0.3f;
		Metallic->MaterialExpressionEditorX = -400;
		Metallic->MaterialExpressionEditorY = 320;
		Material->GetExpressionCollection().AddExpression(Metallic);
		Material->GetEditorOnlyData()->Metallic.Expression = Metallic;

		FAssetRegistryModule::AssetCreated(Material);
		Material->PostEditChange();
		Material->MarkPackageDirty();

		Result.MaterialObjectPath = Material->GetPathName();
		Result.AddMessage(FString::Printf(TEXT("Created Material: %s"), *Result.MaterialObjectPath));
		return Material;
	}

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
		const FString SanitizedAssetBaseName = ObjectTools::SanitizeObjectName(AssetBaseName);
		if (SanitizedAssetBaseName.IsEmpty())
		{
			Result.AddMessage(FString::Printf(TEXT("WeaponDefinition create failed: sanitized asset base name is empty: %s"), *AssetBaseName));
			return nullptr;
		}

		const FString DesiredDataAssetName = FString::Printf(TEXT("DA_Weapon_%s"), *SanitizedAssetBaseName);
		const FString DesiredPackageName = BuildAssetPackageName(DestinationPath, DesiredDataAssetName);
		FString PackageName;
		FString DataAssetName;

		FAssetToolsModule& AssetToolsModule =
			FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().CreateUniqueAssetName(
			DesiredPackageName,
			TEXT(""),
			PackageName,
			DataAssetName
		);

		UPackage* Package = CreatePackage(*PackageName);
		if (!Package)
		{
			Result.AddMessage(FString::Printf(TEXT("WeaponDefinition package create failed: %s"), *PackageName));
			return nullptr;
		}

		UWeaponDefinition* WeaponDefinition = NewObject<UWeaponDefinition>(
			Package,
			*DataAssetName,
			RF_Public | RF_Standalone
		);

		if (!WeaponDefinition)
		{
			Result.AddMessage(FString::Printf(TEXT("WeaponDefinition create failed: %s"), *DataAssetName));
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
		Result.AddMessage(FString::Printf(TEXT("Created WeaponDefinition: %s"), *Result.WeaponDefinitionObjectPath));
		return WeaponDefinition;
	}

	void AssignMaterialToStaticMesh(UStaticMesh* ImportedMesh, UMaterial* Material, FWeaponImportResult& Result)
	{
		if (ImportedMesh->GetStaticMaterials().Num() == 0)
		{
			ImportedMesh->AddMaterial(Material);
			Result.AddMessage(TEXT("Added generated material to new Static Mesh material slot."));
		}
		else
		{
			ImportedMesh->SetMaterial(0, Material);
			Result.AddMessage(TEXT("Assigned generated material to Static Mesh slot 0."));
		}

		ImportedMesh->MarkPackageDirty();
	}
#endif

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
			TEXT("*.*"),
			true,
			false
		);

		Files.Sort();
		for (const FString& File : Files)
		{
			if (FPaths::GetExtension(File, false).Equals(Extension, ESearchCase::IgnoreCase))
			{
				return File;
			}
		}

		return FString();
	}

	FString FindFirstTextureByKeyword(const FString& SourceFolder, const FString& Keyword)
	{
		TArray<FString> Files;
		IFileManager::Get().FindFilesRecursive(
			Files,
			*SourceFolder,
			TEXT("*.*"),
			true,
			false
		);

		Files.Sort();
		for (const FString& File : Files)
		{
			if (FPaths::GetExtension(File, false).Equals(TEXT("png"), ESearchCase::IgnoreCase) &&
				FPaths::GetBaseFilename(File).Contains(Keyword, ESearchCase::IgnoreCase))
			{
				return File;
			}
		}

		return FString();
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
		Result.AddMessage(FString::Printf(TEXT("\u6E90\u6587\u4EF6\u5939\u4E0D\u5B58\u5728\uFF1A%s"), *SourceFolder));
		return Result;
	}

	if (!WeaponImportTool::IsValidGameDestinationPath(DestinationPath))
	{
		Result.AddMessage(FString::Printf(TEXT("\u76EE\u6807\u8DEF\u5F84\u5FC5\u987B\u662F\u6709\u6548\u7684 /Game/... \u8DEF\u5F84\uFF1A%s"), *DestinationPath));
		return Result;
	}

	if (AssetBaseName.TrimStartAndEnd().IsEmpty())
	{
		Result.AddMessage(TEXT("\u8D44\u4EA7\u57FA\u7840\u540D\u4E0D\u80FD\u4E3A\u7A7A\u3002"));
		return Result;
	}

	if (BaseDamage < 0.0f)
	{
		Result.AddMessage(TEXT("BaseDamage \u4E0D\u80FD\u5C0F\u4E8E 0\u3002"));
		return Result;
	}

	if (TraceRadius <= 0.0f)
	{
		Result.AddMessage(TEXT("TraceRadius \u5FC5\u987B\u5927\u4E8E 0\u3002"));
		return Result;
	}

	const FString ObjFile = WeaponImportTool::FindFirstFileByExtension(SourceFolder, TEXT("obj"));
	if (ObjFile.IsEmpty())
	{
		Result.AddMessage(FString::Printf(TEXT("\u6E90\u6587\u4EF6\u5939\u4E2D\u6CA1\u6709\u627E\u5230 .obj \u6587\u4EF6\uFF1A%s"), *SourceFolder));
		return Result;
	}

	const FString DiffuseTextureFile = WeaponImportTool::FindFirstTextureByKeyword(SourceFolder, TEXT("Diffuse"));
	if (DiffuseTextureFile.IsEmpty())
	{
		Result.AddMessage(FString::Printf(TEXT("\u6E90\u6587\u4EF6\u5939\u4E2D\u6CA1\u6709\u627E\u5230\u5305\u542B Diffuse \u7684 .png \u8D34\u56FE\uFF1A%s"), *SourceFolder));
		return Result;
	}

	const FString LightmapTextureFile = WeaponImportTool::FindFirstTextureByKeyword(SourceFolder, TEXT("Lightmap"));

#if WITH_EDITOR
	UObject* ImportedMeshObject = WeaponImportTool::ImportSingleAsset(ObjFile, DestinationPath, Result, UStaticMesh::StaticClass());
	UStaticMesh* ImportedMesh = Cast<UStaticMesh>(ImportedMeshObject);
	if (!ImportedMesh)
	{
		Result.AddMessage(TEXT("OBJ did not import as Static Mesh."));
		return Result;
	}

	UObject* ImportedDiffuseObject = WeaponImportTool::ImportSingleAsset(DiffuseTextureFile, DestinationPath, Result, UTexture2D::StaticClass());
	UTexture2D* DiffuseTexture = Cast<UTexture2D>(ImportedDiffuseObject);
	if (!DiffuseTexture)
	{
		Result.AddMessage(TEXT("Diffuse texture did not import as Texture2D."));
		return Result;
	}

	if (!LightmapTextureFile.IsEmpty())
	{
		FWeaponImportResult LightmapResult;
		UObject* ImportedLightmapObject = WeaponImportTool::ImportSingleAsset(
			LightmapTextureFile,
			DestinationPath,
			LightmapResult,
			UTexture2D::StaticClass()
		);

		if (UTexture2D* LightmapTexture = Cast<UTexture2D>(ImportedLightmapObject))
		{
			Result.AddMessage(FString::Printf(TEXT("Imported optional Lightmap Texture: %s"), *LightmapTexture->GetPathName()));
		}
		else
		{
			const FString LightmapMessage = LightmapResult.Messages.Num() > 0 ?
				WeaponImportTool::JoinMessages(LightmapResult.Messages) :
				TEXT("import returned no Texture2D");
			Result.AddMessage(FString::Printf(TEXT("Optional Lightmap import skipped: %s"), *LightmapMessage));
		}
	}

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

	WeaponImportTool::AssignMaterialToStaticMesh(ImportedMesh, Material, Result);

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

	if (!ImportedMesh->FindSocket(TraceStartSocketName))
	{
		Result.AddMessage(FString::Printf(TEXT("Warning: Static Mesh missing trace start socket: %s"), *TraceStartSocketName.ToString()));
	}

	if (!ImportedMesh->FindSocket(TraceEndSocketName))
	{
		Result.AddMessage(FString::Printf(TEXT("Warning: Static Mesh missing trace end socket: %s"), *TraceEndSocketName.ToString()));
	}

	Result.StaticMeshObjectPath = ImportedMesh->GetPathName();
	Result.AddMessage(FString::Printf(TEXT("Imported Static Mesh: %s"), *Result.StaticMeshObjectPath));
	Result.AddMessage(FString::Printf(TEXT("Imported Diffuse Texture: %s"), *DiffuseTexture->GetPathName()));
	Result.bSucceeded = true;
	Result.AddMessage(TEXT("Weapon import complete. Check Static Mesh sockets and assign DA_Weapon_* to DefaultWeaponDefinition."));
	return Result;
#else
	(void)AttachSocketName;
	(void)TraceStartSocketName;
	(void)TraceEndSocketName;
	(void)LightmapTextureFile;
	Result.AddMessage(TEXT("Weapon import tool can only run in UE Editor."));
#endif

	return Result;
}
