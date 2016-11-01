// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "TickableEditorObject.h"


class IPropertyHandle;
class UNdiMediaFinder;


/**
 * Implements a details view customization for the UNdiMediaFinder class.
 */
class FNdiMediaFinderCustomization
	: public FTickableEditorObject
	, public IDetailCustomization
{
public:

	/** Virtual destructor. */
	virtual ~FNdiMediaFinderCustomization();

public:

	//~ FTickableEditorObject interface

	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;

public:

	//~ IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

public:

	/**
	 * Creates an instance of this class.
	 *
	 * @return The new instance.
	 */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FNdiMediaFinderCustomization());
	}

private:

	/** Callback for generating the menu content of the FinderAddress combo box. */
	TSharedRef<SWidget> HandleFinderAddressComboButtonMenuContent() const;

	/** Callback for selecting a track in the FinderAddress combo box. */
	void HandleFinderAddressComboButtonMenuEntryExecute(int32 TrackIndex);

private:

	/** The finder object being customized. */
	TWeakObjectPtr<UNdiMediaFinder> Finder;

	/** Pointer to the FinderAddress property handle. */
	TSharedPtr<IPropertyHandle> FinderAddressProperty;

	/** Text block widget showing the NDI finder preview. */
	TSharedPtr<SEditableTextBox> PreviewTextBlock;
};
