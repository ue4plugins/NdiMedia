// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaSourceCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IDetailPropertyRow.h"
#include "NdiMediaFinder.h"
#include "NdiMediaSource.h"
#include "Widgets/Input/SComboButton.h"


#define LOCTEXT_NAMESPACE "FNdiMediaSourceCustomization"


/* IDetailCustomization interface
 *****************************************************************************/

void FNdiMediaSourceCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// customize 'NDI' category
	IDetailCategoryBuilder& NdiCategory = DetailBuilder.EditCategory("NDI");
	{
		// FilePath
		SourceNameProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UNdiMediaSource, SourceName));
		{
			IDetailPropertyRow& SourceNameRow = NdiCategory.AddProperty(SourceNameProperty);

			SourceNameRow.CustomWidget()
				.NameContent()
				[
					SourceNameProperty->CreatePropertyNameWidget()
				]
				.ValueContent()
				.MaxDesiredWidth(0.0f)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SourceNameProperty->CreatePropertyValueWidget(false)
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SComboButton)
								.OnGetMenuContent(this, &FNdiMediaSourceCustomization::HandleSourceNameComboButtonMenuContent)
								.ContentPadding(FMargin(4.0, 2.0))
						]
				];
		}
	}
}


/* FNdiMediaSourceCustomization callbacks
 *****************************************************************************/

TSharedRef<SWidget> FNdiMediaSourceCustomization::HandleSourceNameComboButtonMenuContent() const
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
		const FString Name = Source.Name;

		MenuBuilder.AddMenuEntry(
			FText::FromString(Source.ToString()),
			FText::FromString(Source.Url),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([=] { SourceNameProperty->SetValue(Name); }),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([=]{
					FString CurrentValue;
					return ((SourceNameProperty->GetValue(CurrentValue) == FPropertyAccess::Success) && CurrentValue == Name);
				})
			),
			NAME_None,
			EUserInterfaceActionType::RadioButton
		);
	}

	return MenuBuilder.MakeWidget();
}


#undef LOCTEXT_NAMESPACE
