// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"


class IPropertyHandle;
class SWidget;


/**
 * Implements a details view customization for the UNdiMediaSource class.
 */
class FNdiMediaSourceCustomization
	: public IDetailCustomization
{
public:

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

public:

	/**
	 * Creates an instance of this class.
	 *
	 * @return The new instance.
	 */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FNdiMediaSourceCustomization());
	}

private:

	/** Callback for generating the menu content of the SourceName combo box. */
	TSharedRef<SWidget> HandleSourceNameComboButtonMenuContent() const;

private:

	/** Pointer to the SourceName property handle. */
	TSharedPtr<IPropertyHandle> SourceNameProperty;
};
