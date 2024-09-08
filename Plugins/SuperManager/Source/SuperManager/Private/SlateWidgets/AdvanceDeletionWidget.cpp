// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "SlateBasics.h"
#include "DebugHeader.h"
#include "SuperManager.h"


#define  ListAll TEXT("List All Available Assets")
#define  ListUnused TEXT("List Unused Assets")
#define  ListSameName TEXT("List Assets With Same Name")


void SAdvanceDeletionTab::Construct(const FArguments& Ina)
{
	bCanSupportFocus = true;

	StoredAssetsData = Ina._AssetsDataToStore;
	DisplayedAssetData = Ina._AssetsDataToStore;

	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	ComboSourceItems.Empty();


	ComboSourceItems.Add(MakeShared<FString>(ListAll));
	ComboSourceItems.Add(MakeShared<FString>(ListUnused));
	ComboSourceItems.Add(MakeShared<FString>(ListSameName));


	FSlateFontInfo TitleTextFont = GetEmboseedTextFont();
	TitleTextFont.Size = 30;

	ChildSlot
		[
			//Main Vertical Box.
			SNew(SVerticalBox)
				//First vertical slot for title text
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Advance Deletion")))
						.Font(TitleTextFont)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FColor::White)
				]

				//SecondSlot for drop down to specify the listing condition and help text
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						//Combobox Box Slot.(List Filter)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							ConstructComboBox()
						
						]

						//Help Text For Combo Box
						+ SHorizontalBox::Slot()
						.FillWidth(.6f)
						[
							ConstructComboHelpTexts(
								TEXT("Specify the listing condition in the drop down. Left mouse click to go to where asset is located"),
								ETextJustify::Center)
						]

						//Help text for folder path
						+ SHorizontalBox::Slot()
						.FillWidth(.1f)
						[
							ConstructComboHelpTexts(TEXT("Current Folder:\n") + Ina._CurrentSelectedFolder, ETextJustify::Left)
						]
													
				]

				//Third slot for the asset list
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							ConstructAssetListView()
						]

				]

				//Fourth slot for 3 buttons
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						//Button1 slot
						+ SHorizontalBox::Slot()
						.FillWidth(10.f)
						.Padding(5.f)
						[
							ConstructDeleteAllButton()
						]

						//Button2 slot
						+ SHorizontalBox::Slot()
						.FillWidth(10.f)
						.Padding(5.f)
						[
							ConstructSelectAllButton()
						]

						//Button3 slot
						+ SHorizontalBox::Slot()
						.FillWidth(10.f)
						.Padding(5.f)
						[
							ConstructDeselectAllButton()
						]

				]
		];
}//Construct.


TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView<TSharedPtr<FAssetData>>)
		.ItemHeight(24.f)
		.ListItemsSource(&DisplayedAssetData)
		.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList)
		.OnMouseButtonClick(this, &SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked);
		
	return ConstructedAssetListView.ToSharedRef();

}//ConstructAssetListView.

void SAdvanceDeletionTab::RefreshAssetListView()
{
	AssetsDataToDeleteArray.Empty();
	CheckBoxesArray.Empty();

	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}//RefreshAssetListView.

#pragma region ComboBoxForListingCondition

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvanceDeletionTab::ConstructComboBox()
{
	TSharedRef< SComboBox < TSharedPtr <FString > > > ConstructedComboBox =
		SNew(SComboBox < TSharedPtr <FString > >)
		.OptionsSource(&ComboSourceItems)
		.OnGenerateWidget(this, &SAdvanceDeletionTab::OnGenerateComboContent)
		.OnSelectionChanged(this, &SAdvanceDeletionTab::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
				.Text(FText::FromString(TEXT("List Assets Option")))
		];

	return ConstructedComboBox;
}//ConstructComboBox.

TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText = SNew(STextBlock).Text(FText::FromString(*SourceItem.Get()));
	return ConstructedComboText;

}//OnGenerateComboContent.

void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	DebugHeader::Print(*SelectedOption.Get(),FColor::Cyan);
	ComboDisplayTextBlock->SetText(FText::FromString((*SelectedOption.Get())));

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked< FSuperManagerModule>(TEXT("SuperManager"));
	//Pass data for our moudle to filter based on selected option.

	if (*SelectedOption.Get() == ListAll)
	{
		//List All Stored Data.
		DisplayedAssetData = StoredAssetsData;
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListUnused)
	{
		//List All Unused Assets.
		SuperManagerModule.ListUnusedAssetsForAssetList(StoredAssetsData, DisplayedAssetData);
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListSameName)
	{
		//List All Unused Assets.
		SuperManagerModule.ListSameNameAssetsForAssetList(StoredAssetsData, DisplayedAssetData);
		RefreshAssetListView();
	}


}//OnComboSelectionChanged.

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructComboHelpTexts(const FString& TextContent, ETextJustify::Type TextJustify)
{
	TSharedRef<STextBlock> ConstructedHelpText =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Justification(TextJustify)
		.AutoWrapText(true);

	return ConstructedHelpText;
}//ConstructComboHelpTexts.

#pragma endregion


#pragma region RowWidgetForAssetListView

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid())return SNew(STableRow < TSharedPtr <FAssetData> >, OwnerTable);

	const FString DisplayAssetClassName = AssetDataToDisplay->GetClass()->GetName();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmboseedTextFont();
	AssetClassNameFont.Size = 12;

	FSlateFontInfo AssetNameFont = GetEmboseedTextFont();
	AssetNameFont.Size = 15;

	TSharedRef< STableRow < TSharedPtr <FAssetData> > > ListViewRowWidget =
		SNew(STableRow < TSharedPtr <FAssetData> >, OwnerTable)
		.Padding(FMargin(6.f))
		[
			SNew(SHorizontalBox)

			//First slot for check box
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.FillWidth(0.05f)
				[
					ConstructCheckBox(AssetDataToDisplay)
				]

			//Second slot for displaying asset class name
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Fill)
				.FillWidth(0.5f)
				[
					ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
				]
			//Third slot for displaying asset name
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				.FillWidth(0.5f)
				[
					ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
				]
			//Fourth slot for a button
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Fill)
				[
					ConstructButtonForRowWidget(AssetDataToDisplay)
				
				]
		];

	return ListViewRowWidget;

}//OnGenerateRowForList.


void SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData)
{
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked< FSuperManagerModule>(TEXT("SuperManager"));

	SuperManagerModule.SyncCBToClickedAssetForAssetList(ClickedData->ObjectPath.ToString());

}//OnRowWidgetMouseButtonClicked.


TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this,& SAdvanceDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(ConstructedCheckBox);

	return ConstructedCheckBox;

}//ConstructCheckBox

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;

}//ConstructTextForRowWidget.


TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
		.Text(FText::FromString(TEXT("Delete")))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;

}//ConstructButtonForRowWidget.


void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:

		//DebugHeader::Print(AssetData->AssetName.ToString()+TEXT(" is unchecked"), FColor::Red);
		if (AssetsDataToDeleteArray.Contains(AssetData))
		{
			AssetsDataToDeleteArray.Remove(AssetData);
		}
		break;

	case ECheckBoxState::Checked:

		//DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is checked"), FColor::Green);
		AssetsDataToDeleteArray.AddUnique(AssetData);
		break;

	case ECheckBoxState::Undetermined:
		break;

	default:
		break;
	}
	
}//OnCheckBoxStateChanged.


FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FSuperManagerModule& SuperManagerModule =FModuleManager::LoadModuleChecked< FSuperManagerModule>(TEXT("SuperManager"));

	const bool bAssetDeleted = SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if (bAssetDeleted)
	{
		//Updating the list Source item
		if (StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}

		if (DisplayedAssetData.Contains(ClickedAssetData))
		{
			DisplayedAssetData.Remove(ClickedAssetData);
		}
		//Refresh the list
		RefreshAssetListView();
	}

	return FReply::Handled();

}//OnDeleteButtonClicked.

#pragma endregion


#pragma region TabButtons

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeleteAllButtonClicked);


	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All")));

	return DeleteAllButton;

}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnSelectAllButtonClicked);


	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return SelectAllButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeselectAllButtonClicked);


	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	//DebugHeader::Print(TEXT("Delete All Button Clicked "), FColor::Red);
	if (AssetsDataToDeleteArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No asset currently selected"));
		return FReply::Handled() ;

	}//if
	
	 //Pass data to our module for deletion.
	TArray<FAssetData> AssetDataToDelete;
	for (const TSharedPtr <FAssetData>& Data : AssetsDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked< FSuperManagerModule>(TEXT("SuperManager"));

	const bool bAssetsDeleted = SuperManagerModule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);

	if (bAssetsDeleted)
	{
		for (const TSharedPtr <FAssetData>& DeletedData : AssetsDataToDeleteArray)
		{
			if (StoredAssetsData.Contains(DeletedData))
			{
				StoredAssetsData.Remove(DeletedData);
			}

			if (DisplayedAssetData.Contains(DeletedData))
			{
				DisplayedAssetData.Remove(DeletedData);
			}

		}//loop

		RefreshAssetListView();
	}//if

	return FReply::Handled();
}//OnDeleteAllButtonClicked.

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	//DebugHeader::Print(TEXT("Select All Button Clicked "), FColor::Red);
	if(CheckBoxesArray.Num() == 0)return FReply::Handled();

	for (const TSharedRef<SCheckBox> CheckBox : CheckBoxesArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}

	}//loop.


	return FReply::Handled();
}//OnSelectAllButtonClicked.

FReply SAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	//DebugHeader::Print(TEXT("DeSelect All Button Clicked "), FColor::Red);

	if (CheckBoxesArray.Num() == 0)return FReply::Handled();

	for (const TSharedRef<SCheckBox> CheckBox : CheckBoxesArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}

	}//loop.

	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmboseedTextFont();
	ButtonTextFont.Size = 15;
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(ButtonTextFont)
		.Justification(ETextJustify::Center);

	return ConstructedTextBlock;



}


#pragma endregion

