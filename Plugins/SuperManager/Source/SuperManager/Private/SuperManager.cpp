// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "CustomStyle/SuperManagerStyle.h"
#include "LevelEditor.h"
#include "Engine/Selection.h"
#include "Subsystems/EditorActorSubsystem.h"


#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	FSuperManagerStyle::InitializeIcons();
	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();
	InitLevelEditorExtention();

	InitCustomSelectionEvent();

}//StartupModule.

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvanceDeletion"));

	FSuperManagerStyle::ShutDown();
}


#pragma region ContentBrowserMenuExtention


void FSuperManagerModule::InitCBMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule =
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	//Get hold of all the menu extenders.
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	/*
	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;

	CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);

	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);
	*/


	//we add custom delegate to all the existing delegates.
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::
		CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));


}//InitCBMenuExtention.

//to define the position for inserting menu entry.
TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		FolderPathsSelected = SelectedPaths;

		MenuExtender->AddMenuExtension(FName("Delete"),//extention hook, position to insert
			EExtensionHook::After,//position in hook, here is after.
			TSharedPtr<FUICommandList>(),//custom hot keys.
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)//second binding, will define details for this menu entry.
			);
	}


	return MenuExtender;

}//CustomCBMenuExtender.


//Define details for the custom menu entry.
void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Unused Assets")),//title for menu entry.
		FText::FromString(TEXT("Safely delete all unused assets under folder")),//tool tips for menu entry.
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"ContentBrowser.DeleteUnusedAssets"),//custom icons for menu entry.
		FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)//third binding, the actual function to execute.
		);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Empty Folder")),//title for menu entry.
		FText::FromString(TEXT("Safely delete all empty folder")),//tool tips for menu entry.
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteEmptyFolder"),//custom icons for menu entry.
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFolderButtonClicked)//third binding, the actual function to execute.
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Advance Deletion")),//title for menu entry.
		FText::FromString(TEXT("List assets by specific in a tab for deleting.")),//tool tips for menu entry.
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"),//custom icons for menu entry.
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked)//third binding, the actual function to execute.
	);

}//AddCBMenuEntry.


void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{

	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	//if no asset is found under this dir the show msg and return.
	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No Assets Found Under Selected Folder"),false);
	}
	
	EAppReturnType::Type ConfirmResult =
		DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of ") + 
			FString::FromInt(AssetsPathNames.Num()) + 
			TEXT(" assets need to be checked.\nWould you like to procceed?"));

	if (ConfirmResult == EAppReturnType::No) return;

	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetPathName : AssetsPathNames)
	{
		//Don't touch root folder
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if (AssetReferencers.Num() == 0)//if no reference then add it to unused array.
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDataArray.Num() > 0)//if there is unused assets then delete it otherwise show msg.
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"));
	}

}//OnDeleteUnusedAssetButtonClicked.

void FSuperManagerModule::OnDeleteEmptyFolderButtonClicked()
{

	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}
	FixUpRedirectors();

	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0],true,true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for (const FString& FolderPath : FolderPathsArray)
	{
		//Don't touch root folder
		if (FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}

	}//loop

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No Empty Folder Found Under Selected Folder"),false);
		return;
	}


	EAppReturnType::Type ConfirmResult =  DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel, 
		TEXT("Empty Folder Found In:\n")+ EmptyFolderPathsNames + TEXT("\nWould You Like To Delete All"), false);

	if (ConfirmResult == EAppReturnType::Cancel)
	{
		return;
	}

	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			++Counter;
		}
		else {
			DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
		}		 
		
	}//loop.

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + TEXT(" folders"));
	}

}//OnDeleteEmptyFolderButtonClicked.



void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	FixUpRedirectors();
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));

}//OnAdvanceDeletionButtonClicked.

void FSuperManagerModule::FixUpRedirectors()
{
	
	IAssetRegistry& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");


	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	// Query for a list of assets in the selected paths
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	if (AssetList.Num() == 0) return;

	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : AssetList)
	{
		ObjectPaths.Add(Asset.GetObjectPathString());
	}

	TArray<UObject*> Objects;
	const bool bAllowedToPromptToLoadAssets = true;
	const bool bLoadRedirects = true;

	AssetViewUtils::FLoadAssetsSettings Settings;
	Settings.bFollowRedirectors = false;
	Settings.bAllowCancel = true;


	AssetViewUtils::ELoadAssetsResult Result = AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, Settings);

	if (Result != AssetViewUtils::ELoadAssetsResult::Cancelled)
	{
		// Transform Objects array to ObjectRedirectors array
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			Redirectors.Add(CastChecked<UObjectRedirector>(Object));
		}

		// Load the asset tools module
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors);

	}


}

#pragma endregion 


#pragma region CustomEditorTab


void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")))
		.SetIcon(FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"));

}
TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTab)
{
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvanceDeletionTab)
				.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
				.CurrentSelectedFolder(FolderPathsSelected[0])

		];
}//RegisterAdvanceDeletionTab.


TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetsData;

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	for (const FString& AssetPathName : AssetsPathNames)
	{
		//Don't touch root folder
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AvaiableAssetsData.Add(MakeShared<FAssetData>(Data));

	}//Loop.

	return AvaiableAssetsData;

}//GetAllAssetDataUnderSelectedFolder.



#pragma endregion

#pragma region ProccessDataForAdvanceDeletionTab


bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0)
	{
		return true;
	}
	return false;

}//DeleteSingleAssetForAssetList.

bool FSuperManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;
	}
	return false;

}//DeleteMultipleAssetsForAssetList.


void FSuperManagerModule::ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray<FString> AssetReferencers = 
		UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->ObjectPath.ToString());

		if (AssetReferencers.Num() ==0)
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}

}//ListUnusedAssetsForAssetList.


void FSuperManagerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();

	//Multimap for supporting finding assets with same name
	TMultiMap<FString, TSharedPtr<FAssetData> > AssetsInfoMultiMap;

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);
	}

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray< TSharedPtr <FAssetData> > OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataSharedPtr->AssetName.ToString(), OutAssetsData);

		if (OutAssetsData.Num() <= 1) continue;

		for (const TSharedPtr<FAssetData>& SameNameData : OutAssetsData)
		{
			if (SameNameData.IsValid())
			{
				OutSameNameAssetsData.AddUnique(SameNameData);
			}
		}
	}

}//ListSameNameAssetsForAssetList.

void FSuperManagerModule::SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync)
{
	TArray<FString> AssetPathsToSync;
	AssetPathsToSync.Add(AssetPathToSync);
	UEditorAssetLibrary::SyncBrowserToObjects(AssetPathsToSync);

}//SyncCBToClickedAssetForAssetList.

#pragma endregion



#pragma region LevelEditorMenuExtension

void FSuperManagerModule::InitLevelEditorExtention()
{

	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelEditorMenuExtenders =
		LevelEditorModule.GetAllLevelViewportContextMenuExtenders();

	LevelEditorMenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::
		CreateRaw(this, &FSuperManagerModule::CustomLevelEditorMenuExtender));


}//InitLevelEditorExtention.

TSharedRef<FExtender> FSuperManagerModule::CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());

	if (SelectedActors.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("ActorOptions"),
			EExtensionHook::Before,
			UICommandList,
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddLevelEditorMenuEntry)
		);
	}

	return MenuExtender;

}//CustomLevelEditorMenuExtender.


void FSuperManagerModule::AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Lock Actor Selection")),
		FText::FromString(TEXT("Prevent actor from being selected")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "LevelEditor.LockSelection"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnLockActorSelectionButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Unlock All Actor Selection")),
		FText::FromString(TEXT("Remove the selection constraint on all actor")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "LevelEditor.UnlockSelection"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnUnlockActorSelectionButtonClicked)
	);


}//AddLevelEditorMenuEntry.

void FSuperManagerModule::OnLockActorSelectionButtonClicked()
{
	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = WeakEditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor selected"));
		return;
	}

	FString CurrentLockedActorNames = TEXT("Locked selection for:");

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor) continue;

		LockActorSelection(SelectedActor);

		WeakEditorActorSubsystem->SetActorSelectionState(SelectedActor, false);

		CurrentLockedActorNames.Append(TEXT("\n"));
		CurrentLockedActorNames.Append(SelectedActor->GetActorLabel());
	}

	DebugHeader::ShowNotifyInfo(CurrentLockedActorNames);

}//OnLockActorSelectionButtonClicked.


void FSuperManagerModule::OnUnlockActorSelectionButtonClicked()
{

	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> AllActorsInLevel = WeakEditorActorSubsystem->GetAllLevelActors();
	TArray<AActor*> AllLockedActors;

	for (AActor* ActorInLevel : AllActorsInLevel)
	{
		if (!ActorInLevel) continue;

		if (CheckIsActorSelectionLocked(ActorInLevel))
		{
			AllLockedActors.Add(ActorInLevel);
		}
	}

	if (AllLockedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No selection locked actor currently"));
	}

	FString UnlockedActorNames = TEXT("Lifted selection constraint for:");

	for (AActor* LockedActor : AllLockedActors)
	{
		UnlockActorSelection(LockedActor);

		UnlockedActorNames.Append(TEXT("\n"));
		UnlockedActorNames.Append(LockedActor->GetActorLabel());
	}

	DebugHeader::ShowNotifyInfo(UnlockedActorNames);

}//OnUnlockActorSelectionButtonClicked.

#pragma endregion


#pragma region SelectionLock

void FSuperManagerModule::InitCustomSelectionEvent()
{
	USelection* UserSelection = GEditor->GetSelectedActors();

	UserSelection->SelectObjectEvent.AddRaw(this, &FSuperManagerModule::OnActorSelected);

}//InitCustomSelectionEvent.


void FSuperManagerModule::OnActorSelected(UObject* SelectedObject)
{
	if (!GetEditorActorSubsystem()) return;

	if (AActor* SelectedActor = Cast<AActor>(SelectedObject))
	{
		if (CheckIsActorSelectionLocked(SelectedActor))
		{
			//Deselect actor right away
			WeakEditorActorSubsystem->SetActorSelectionState(SelectedActor, false);
		}
	}
}//OnActorSelected.


void FSuperManagerModule::LockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess) return;

	if (!ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Add(FName("Locked"));
	}
}//LockActorSelection.

void FSuperManagerModule::UnlockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess) return;

	if (ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Remove(FName("Locked"));
	}
}//UnlockActorSelection.

bool FSuperManagerModule::CheckIsActorSelectionLocked(AActor* ActorToProcess)
{
	if (!ActorToProcess) return false;

	return ActorToProcess->ActorHasTag(FName("Locked"));
}//CheckIsActorSelectionLocked.

#pragma endregion

bool FSuperManagerModule::GetEditorActorSubsystem()
{
	if (!WeakEditorActorSubsystem.IsValid())
	{
		WeakEditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}

	return WeakEditorActorSubsystem.IsValid();
}//GetEditorActorSubsystem.

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)