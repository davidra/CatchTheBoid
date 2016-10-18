/***************************************************************************************************
gameobject.h

Interfaces for gameobjects

by David Ramos
***************************************************************************************************/
#pragma once

//----------------------------------------------------------------------------
struct IGameObjectDef
{
	virtual ~IGameObjectDef() {}
};

//----------------------------------------------------------------------------
struct IGameObjectState
{
	virtual void Init(const IGameObjectState& game_object_state) = 0;
	virtual ~IGameObjectState() {}
};

//----------------------------------------------------------------------------
struct IGameObject 
{
	IGameObject() 
		: mIsPendingDestroy(false)
		, mGameObjectDef(nullptr)
	{}
	virtual ~IGameObject() {}

	bool IsPendingDestroy() const { return mIsPendingDestroy; }
	void SetPendingDestroy() { mIsPendingDestroy = true;  }

	virtual bool Init(const IGameObjectDef* def, IGameObjectState*&& initial_state) 
	{
		if (!def || !initial_state)
			return false;

		mGameObjectDef = def;
		mGameObjectState = std::unique_ptr<IGameObjectState>(initial_state);
		initial_state = nullptr;

		return true;
	}

	const IGameObjectDef&	GetDef() const { return *mGameObjectDef;  }
	const IGameObjectState&	GetState() const { return *mGameObjectState; }
	IGameObjectState&		GetState() { return *mGameObjectState; }

	virtual void Update(float elapsed) = 0;
	virtual void Render() = 0;

private:
	bool mIsPendingDestroy;

	const IGameObjectDef*				mGameObjectDef;
	std::unique_ptr<IGameObjectState>	mGameObjectState;
};
