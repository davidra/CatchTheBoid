#include "stdafx.h"

#include "debugrenderer.h"

#include "CPR_Framework.h"
#include "game\modelrepository.h"

namespace Debug
{
	//----------------------------------------------------------------------------
	void cRenderer::Update(float )
	{
		mRenderEntries.clear();
	}

	//----------------------------------------------------------------------------
	void cRenderer::Render()
	{
		for (const tDebugRenderEntry& entry : mRenderEntries)
		{
			CPR_assert(entry.mMesh != nullptr, "Invalid model!");

			entry.mMesh->Render(entry.mWorldPos, cVector3::ZERO(), entry.mScale, entry.mColor);
		}
	}

	//----------------------------------------------------------------------------
	void cRenderer::AddSphere(const cVector3& pos, float radius, const cColor& color)
	{
		mRenderEntries.emplace_back(pos, cVector3(radius * 2.0f), color, ModelRepo::GetModel(MID_SPHERE));
	}

	//----------------------------------------------------------------------------
	cRenderer::cRenderer()
	{
		// Reserve some reasonable initial size to reduce runtime allocations
		mRenderEntries.reserve(20);
	}
}