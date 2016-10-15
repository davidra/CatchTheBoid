/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Debug-related functions
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "core\base.h"
#include "core\utils.h"

#include "CPR_Framework.h"

namespace Debug
{
	bool HasDebuggerAttached();

	void ErrorMsg(const char* file, int line, const char* expr, const char* format, ...);

	void WriteLine(const char* fmt, ...);

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
};