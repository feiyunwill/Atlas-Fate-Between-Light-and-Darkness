#pragma once

#include "components/comp_base.h"
class TCompTempPlayerController;

typedef void (TCompTempPlayerController::*actionfinish)();
typedef void (TCompTempPlayerController::*actionhandler)(float);

struct TMsgStateStart {
	
	actionhandler action_start;
	std::string meshname;
	float speed;

	DECL_MSG_ID();
};

struct TMsgStateFinish {

	actionfinish action_finish;
	std::string meshname;
	float speed;

	DECL_MSG_ID();
};

class TCompTempPlayerController : public TCompBase
{
	/* DEPRECATED */
	std::map<std::string, CRenderMesh*> mesh_states;

	physx::PxQueryFilterData shadowMergeFilter;
	physx::PxQueryFilterData playerFilter;

	actionhandler state;
	CHandle current_camera;

	float currentSpeed = 4.f;
	float rotationSpeed = 10.f;
	float mergeAngle = 0.45f;

	/* Stamina private variables */
	float stamina = 100.f;
	const float minStamina = 0.f;
	const float maxStamina = 100.f;
	const float incrStamina = 15.f;
	const float decrStaticStamina = 0.75f;
	const float decrStaminaHorizontal = 12.5f;
	const float decrStaminaVertical = 17.5f;

	void onStateStart(const TMsgStateStart& msg);
	void onStateFinish(const TMsgStateFinish& msg);

	DECL_SIBLING_ACCESS();

public:

	bool isMerged;
	bool isGrounded;
	bool isInhibited;

	void debugInMenu();
	void renderDebug();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	/* State functions */
	void walkState(float dt);
	void mergeState(float dt);
	void idleState(float dt);

	/* Player condition tests */
	const bool concaveTest(void);
	const bool convexTest(void);
	const bool onMergeTest(float dt);
	const bool groundTest(float dt);

	/* Auxiliar functions */
	void staminaTest(float dt);
	void shadowRender(float dt);
	void resetState();

	static void registerMsgs();
};

