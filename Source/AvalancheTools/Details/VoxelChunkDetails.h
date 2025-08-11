// Scientific Ways

#pragma once

#include "AvalancheToolsModule.h"

class FVoxelChunkDetails : public IDetailCustomization
{

public:

	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override; // IDetailCustomization
private:
	void AddVoxelCategory(IDetailLayoutBuilder& InDetailBuilder);
};
