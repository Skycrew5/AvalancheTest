// Scientific Ways

#include "VoxelChunkDetails.h"

#define LOCTEXT_NAMESPACE "VoxelChunkDetails"

TSharedRef<IDetailCustomization> FVoxelChunkDetails::MakeInstance()
{
	return MakeShareable(new FVoxelChunkDetails);
}

void FVoxelChunkDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) // IDetailCustomization
{
	AddVoxelCategory(InDetailBuilder);
}

void FVoxelChunkDetails::AddVoxelCategory(IDetailLayoutBuilder& InDetailBuilder)
{
	IDetailCategoryBuilder& CustomCategoryBuilder = InDetailBuilder.EditCategory("Voxels");
	CustomCategoryBuilder.SetSortOrder(0);
}

#undef LOCTEXT_NAMESPACE
