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
	Result.bSucceeded = true;
	Result.AddMessage(FString::Printf(TEXT("\u627E\u5230 OBJ \u6587\u4EF6\uFF1A%s"), *ObjFile));
	Result.AddMessage(FString::Printf(TEXT("\u627E\u5230 Diffuse \u8D34\u56FE\uFF1A%s"), *DiffuseTextureFile));
	if (!LightmapTextureFile.IsEmpty())
	{
		Result.AddMessage(FString::Printf(TEXT("\u627E\u5230 Lightmap \u8D34\u56FE\uFF1A%s"), *LightmapTextureFile));
	}
	Result.AddMessage(TEXT("\u53C2\u6570\u6821\u9A8C\u901A\u8FC7\uFF0C\u5BFC\u5165\u6D41\u7A0B\u5C06\u5728\u540E\u7EED\u4EFB\u52A1\u4E2D\u63A5\u5165\u3002"));
#else
	(void)LightmapTextureFile;
	Result.AddMessage(TEXT("\u6B66\u5668\u5BFC\u5165\u5DE5\u5177\u53EA\u80FD\u5728 UE Editor \u4E2D\u6267\u884C\u3002"));
#endif

	return Result;
}
