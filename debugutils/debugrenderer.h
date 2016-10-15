/***************************************************************************************************
debugrenderer.h

Class that manages rendering some debug-related primitives

by David Ramos
***************************************************************************************************/
#pragma once

namespace Debug
{
	class cRenderer
	{
	public:
		~cRenderer()
		{
		}

		static cRenderer& Get()
		{
			static std::unique_ptr<cRenderer> sDebugRendererInstance(new cRenderer());
			return *sDebugRendererInstance;
		}

		void Update(float elapsed);
		void Render();

		void AddSphere(const cVector3& pos, float radius, const cColor& color);

	private:
		cRenderer();

		struct tDebugRenderEntry
		{
			tDebugRenderEntry(const cVector3& world_pos, const cVector3& scale, const cColor& color, Mesh* mesh)
				: mWorldPos(world_pos)
				, mScale(scale)
				, mColor(color)
				, mMesh(mesh)
			{}

			cVector3	mWorldPos;
			cVector3	mScale;
			cColor		mColor;
			Mesh*		mMesh;
		};

		typedef std::vector<tDebugRenderEntry> tDebugRenderEntries;

		tDebugRenderEntries mRenderEntries;
	};
}
