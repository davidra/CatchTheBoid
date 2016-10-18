#include "stdafx.h"

#include "GameObjectManager.h"
#include "gameobject.h"

std::unique_ptr<cGameObjectManager> cGameObjectManager::sGameObjectManager;
cGameObjectManager::tGameObjectRegistry cGameObjectManager::sGameObjectRegistry;
unsigned cGameObjectManager::sGameObjectTypeIds = 0;

namespace
{
	static const size_t MAX_GAME_OBJECTS = 300;
	static const size_t INITIAL_GAMEOBJECT_REGISTERS = 30;
}

//----------------------------------------------------------------------------
cGameObjectManager::cGameObjectManager()
	: mUpdating(false)
	, mRendering(false)
	, mCurrentTime(0.0f)
{
}

//----------------------------------------------------------------------------
void cGameObjectManager::InitInstance()
{
	sGameObjectManager = std::unique_ptr<cGameObjectManager>(new cGameObjectManager);

	// We are using the pointers as handles for simplicity of the test, so we can't allow growth in this manager. Reserve now the maximum number and fail if we allocate more than that
	sGameObjectManager->mGameObjects.reserve(MAX_GAME_OBJECTS);

	sGameObjectManager->mDeferredGameObjectCreation.reserve(20);

	// We resize here to some initial value because we will index registers through indices
	sGameObjectRegistry.resize(INITIAL_GAMEOBJECT_REGISTERS);
}

//----------------------------------------------------------------------------
void cGameObjectManager::RegisterGameObject(tGameObjectTypeId type_id, tGameObjCreationFnc game_obj_creation_fnc, tGameObjStateCreationFnc  game_obj_state_creation_fnc)
{
	if (type_id > sGameObjectRegistry.size())
	{
		sGameObjectRegistry.resize(static_cast<unsigned>(sGameObjectRegistry.size() * 1.618f));
	}

	sGameObjectRegistry[type_id] = tGameObjectRegister(game_obj_creation_fnc, game_obj_state_creation_fnc);
}

//----------------------------------------------------------------------------
tGameObjectId cGameObjectManager::CreateGameObject(tGameObjectTypeId game_object_type_id, const IGameObjectDef& game_object_def, const IGameObjectState& initial_state)
{
	if (mGameObjects.size() == MAX_GAME_OBJECTS)
	{
		CPR_assert(false, "Can't make room for more game objects!");
		return INVALID_GAMEOBJECT_ID;
	}

	const auto& go_register = sGameObjectRegistry[game_object_type_id];
	CPR_assert((go_register.mCreationFunc != nullptr) && (go_register.mStateCreationFunc != nullptr), "Could not find gameobject type id %d, did you call RegisterInManager already?", game_object_type_id);
	IGameObject* new_game_object = go_register.mCreationFunc();
	IGameObjectState* new_game_object_state = go_register.mStateCreationFunc(initial_state);
	bool success = new_game_object->Init(&game_object_def, std::move(new_game_object_state));
	if (success)
	{
		if (mUpdating || mRendering)
		{
			mDeferredGameObjectCreation.emplace_back(new_game_object);
		}
		else
		{
			mGameObjects.push_back(new_game_object);
		}
	}
	else
	{
		Debug::WriteLine("Error initializing gameobject with type id %d!", game_object_type_id);
		delete new_game_object;
		new_game_object = nullptr;
	}

	return new_game_object;
}

//----------------------------------------------------------------------------
void cGameObjectManager::DestroyGameObject(tGameObjectId game_object)
{
	game_object->SetPendingDestroy();
}

//----------------------------------------------------------------------------
void cGameObjectManager::DestroyAllGameObjects()
{
	while (!mGameObjects.empty())
	{
		DestroyGameObject_Internal(mGameObjects.back());
	}
}

//----------------------------------------------------------------------------
IGameObject* cGameObjectManager::GetGameObject(tGameObjectId game_object_id) const
{
	// Yeah, if I had more time, game object ids would have been proper handles
	return static_cast<IGameObject*>(game_object_id);
}

//----------------------------------------------------------------------------
void cGameObjectManager::Update(float elapsed)
{
	mCurrentTime += elapsed;

	static std::vector<IGameObject**> objs_to_destroy;

	// Finish the creation of the gameobject that were deferred
	for (IGameObject* new_game_object : mDeferredGameObjectCreation)
	{
		mGameObjects.push_back(new_game_object);
	}
	mDeferredGameObjectCreation.clear();

	mUpdating = true;
	for (IGameObject*& game_object : mGameObjects)
	{
		if (!game_object->IsPendingDestroy())
		{
			game_object->Update(elapsed);
		}

		if (game_object->IsPendingDestroy())
		{
			objs_to_destroy.push_back(&game_object);
		}
	}
	mUpdating = false;

	while (!objs_to_destroy.empty())
	{
		DestroyGameObject_Internal(*objs_to_destroy.back());
		objs_to_destroy.pop_back();
	}
}

//----------------------------------------------------------------------------
void cGameObjectManager::Render()
{
	mRendering = true;
	for (IGameObject* game_object : mGameObjects)
	{
		CPR_assert(game_object && !game_object->IsPendingDestroy(), "Game objects pending destroy should have been destroyed by now in the Update call");
		game_object->Render();
	}
	mRendering = false;
}

//----------------------------------------------------------------------------
void cGameObjectManager::DestroyGameObject_Internal(tGameObjectId& game_object)
{
	const size_t idx = static_cast<IGameObject**>(&game_object) - mGameObjects.data();
	const size_t last_idx = mGameObjects.size() - 1;
	if (IsWithinRange(0u, idx, last_idx))
	{
		if (idx != last_idx)
		{
			using namespace std;
			swap(mGameObjects[idx], mGameObjects[last_idx]);
		}

		delete mGameObjects[last_idx];
		mGameObjects.pop_back();
	}
}