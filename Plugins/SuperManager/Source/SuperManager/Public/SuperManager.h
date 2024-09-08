// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


private:

#pragma region ContentBrowserMenuExtention

	void InitCBMenuExtention();

	TArray<FString> FolderPathsSelected;

	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);

	void OnDeleteUnusedAssetButtonClicked();

	void OnDeleteEmptyFolderButtonClicked();

	void OnAdvanceDeletionButtonClicked();

	void FixUpRedirectors();

#pragma endregion ContentBrowserMenuExtention


#pragma region CustomEditorTab

	void RegisterAdvanceDeletionTab();

	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTab);

	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();


#pragma endregion

public:
#pragma region ProccessDataForAdvanceDeletionTab

	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);

	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);

	void ListUnusedAssetsForAssetList(const TArray<TSharedPtr <FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr <FAssetData>>& OutUnusedAssetsData);

	void ListSameNameAssetsForAssetList(const TArray<TSharedPtr <FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr <FAssetData>>& OutSameNameAssetsData);

	void SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync);

#pragma endregion
};
