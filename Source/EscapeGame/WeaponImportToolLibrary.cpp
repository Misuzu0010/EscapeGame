#include "WeaponImportToolLibrary.h"

#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#endif

namespace WeaponImportTool
{
#if WITH_EDITOR
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

	(void)AttachSocketName;
	(void)TraceStartSocketName;
	(void)TraceEndSocketName;

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
		WeaponImportTool::ImportSingleAsset(LightmapTextureFile, DestinationPath, Result, UTexture2D::StaticClass());
	}

	Result.StaticMeshObjectPath = ImportedMesh->GetPathName();
	Result.bSucceeded = true;
	Result.AddMessage(FString::Printf(TEXT("Imported Static Mesh: %s"), *Result.StaticMeshObjectPath));
	Result.AddMessage(FString::Printf(TEXT("Imported Diffuse Texture: %s"), *DiffuseTexture->GetPathName()));
	return Result;
#else
	(void)LightmapTextureFile;
	Result.AddMessage(TEXT("Weapon import tool can only run in UE Editor."));
#endif

	return Result;
}
