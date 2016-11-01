// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaEditorPCH.h"
#include "NdiMediaSourceCustomization.h"


#define LOCTEXT_NAMESPACE "FNdiMediaSourceCustomization"


/* IDetailCustomization interface
 *****************************************************************************/

void FNdiMediaSourceCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// customize 'NDI' category
	IDetailCategoryBuilder& NdiCategory = DetailBuilder.EditCategory("NDI");
	{
		// FilePath
		SourceEndpointProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UNdiMediaSource, SourceEndpoint));
		{
			IDetailPropertyRow& SourceEndpointRow = NdiCategory.AddProperty(SourceEndpointProperty);

			SourceEndpointRow.CustomWidget()
				.NameContent()
				[
					SourceEndpointProperty->CreatePropertyNameWidget()
				]
				.ValueContent()
				.MaxDesiredWidth(0.0f)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SourceEndpointProperty->CreatePropertyValueWidget(false)
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SComboButton)
								.OnGetMenuContent(this, &FNdiMediaSourceCustomization::HandleSourceEndpointComboButtonMenuContent)
								.ContentPadding(FMargin(4.0, 2.0))
						]
				];
		}
	}
}


/* FNdiMediaSourceCustomization callbacks
 *****************************************************************************/

TSharedRef<SWidget> FNdiMediaSourceCustomization::HandleSourceEndpointComboButtonMenuContent() const
{
	// get default NDI source finder object
	auto DefaultFinder = GetDefault<UNdiMediaFinder>();

	if (DefaultFinder == nullptr)
	{
		return SNullWidget::NullWidget;
	}

	// fetch found NDI sources
	TArray<FNdiMediaSourceId> OutSources;

	if (!DefaultFinder->GetSources(OutSources))
	{
		return SNullWidget::NullWidget;
	}

	// generate menu
	FMenuBuilder MenuBuilder(true, nullptr);

	for (auto Source : OutSources)
	{
		const FString Endpoint = Source.Endpoint;

		MenuBuilder.AddMenuEntry(
			FText::FromString(Source.ToString()),
			FText::FromString(Source.Url),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([=] { SourceEndpointProperty->SetValue(Endpoint); }),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([=]{
					FString CurrentValue;
					return ((SourceEndpointProperty->GetValue(CurrentValue) == FPropertyAccess::Success) && CurrentValue == Endpoint);
				})
			),
			NAME_None,
			EUserInterfaceActionType::RadioButton
		);
	}

	return MenuBuilder.MakeWidget();
}


#undef LOCTEXT_NAMESPACE
