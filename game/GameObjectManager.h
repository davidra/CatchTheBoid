/***************************************************************************************************
GameObjectManager.h

Simple manager of game objects
 
by David Ramos
***************************************************************************************************/
#pragma once

struct IGameObject;
struct IGameObjectDef;
struct IGameObjectState;

typedef unsigned tGameObjectTypeId;
typedef IGameObject* tGameObjectId;
static const tGameObjectId INVALID_GAMEOBJECT_ID = 0;

#define REGISTER_GAMEOBJECT(class, def, state) \
	public:																																																		\
	static_assert(std::is_base_of<IGameObjectDef, def>::value, #def "should inherit from IGameObjectDef");																										\
	static_assert(std::is_base_of<IGameObjectState, state>::value, #state "should inherit from IGameObjectState");																								\
	static tGameObjectTypeId GetTypeId() { static tGameObjectTypeId sThisTypeId = ++cGameObjectManager::sGameObjectTypeIds; return sThisTypeId; }																\
	static void RegisterInManager()	{ cGameObjectManager::GetInstance()->RegisterGameObject(class::GetTypeId(), []()->IGameObject* { return new class;  }														\
		, [](const IGameObjectState& init_state)->IGameObjectState* { auto* const new_state = new state; new_state->Init(init_state); return new_state; }); }													\
	const def& Def() const { return static_cast<const def&>(GetDef()); }																																		\
	const state& State() const { return static_cast<const state&>(GetState()); }																																\
	state& State() { return static_cast<state&>(GetState()); }

//----------------------------------------------------------------------------
class cGameObjectManager
{
public:
	cGameObjectManager();

	static void					InitInstance();
	static cGameObjectManager*	GetInstance() { return sGameObjectManager.get(); }

	typedef IGameObject* (*tGameObjCreationFnc)();
	typedef IGameObjectState* (*tGameObjStateCreationFnc)(const IGameObjectState&);
	static void					RegisterGameObject(tGameObjectTypeId type_id, tGameObjCreationFnc game_obj_creation_fnc, tGameObjStateCreationFnc  game_obj_state_creation_fnc);

	template <class tGameObjectClass> 
	tGameObjectId				CreateGameObject(const IGameObjectDef& game_object_def, const IGameObjectState& initial_state);

	void						DestroyGameObject(tGameObjectId game_object);
	void						DestroyAllGameObjects();

	IGameObject*				GetGameObject(tGameObjectId game_object_id) const;

	void						Update(float elapsed);
	void						Render();

	float						GetCurrTime() const { return mCurrentTime;  }

	static unsigned	sGameObjectTypeIds;

private:
	tGameObjectId CreateGameObject(tGameObjectTypeId game_object_type_id, const IGameObjectDef& game_object_def, const IGameObjectState& initial_state);

	void DestroyGameObject_Internal(tGameObjectId& game_object);

	typedef std::vector<IGameObject*> tGameObjectContainer;
	tGameObjectContainer mGameObjects;

	bool mUpdating;
	bool mRendering;

	tGameObjectContainer mDeferredGameObjectCreation;

	float mCurrentTime;


	static std::unique_ptr<cGameObjectManager> sGameObjectManager;

	struct tGameObjectRegister
	{
		tGameObjectRegister() 
			: mCreationFunc(nullptr)
			, mStateCreationFunc(nullptr) {}

		tGameObjectRegister(tGameObjCreationFnc creation_func, tGameObjStateCreationFnc state_creation_func)
			: mCreationFunc(creation_func)
			, mStateCreationFunc(state_creation_func)
		{
		}

		tGameObjCreationFnc			mCreationFunc;
		tGameObjStateCreationFnc	mStateCreationFunc;
	};
	typedef std::vector<tGameObjectRegister> tGameObjectRegistry;
	static tGameObjectRegistry sGameObjectRegistry;
};

//----------------------------------------------------------------------------
template <class tGameObjectClass>
tGameObjectId cGameObjectManager::CreateGameObject(const IGameObjectDef& game_object_def, const IGameObjectState& initial_state)
{
	return CreateGameObject(tGameObjectClass::GetTypeId(), game_object_def, initial_state);
}
