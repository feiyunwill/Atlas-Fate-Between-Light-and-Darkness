#include "mcv_platform.h"
#include "comp_skeleton.h"
#include "game_core_skeleton.h"
#include "cal3d/cal3d.h"
#include "resources/resources_manager.h"
#include "render/render_utils.h"
#include "components/comp_transform.h"

DECL_OBJ_MANAGER("skeleton", TCompSkeleton);

// ---------------------------------------------------------------------------------------
// Cal2DX conversions, VEC3 are the same, QUAT must change the sign of w
CalVector DX2Cal(VEC3 p) {
  return CalVector(p.x, p.y, p.z);
}
CalQuaternion DX2Cal(QUAT q) {
  return CalQuaternion(q.x, q.y, q.z, -q.w);
}
VEC3 Cal2DX(CalVector p) {
  return VEC3(p.x, p.y, p.z);
}
QUAT Cal2DX(CalQuaternion q) {
  return QUAT(q.x, q.y, q.z, -q.w);
}
MAT44 Cal2DX(CalVector trans, CalQuaternion rot) {
  return
    MAT44::CreateFromQuaternion(Cal2DX(rot))
    * MAT44::CreateTranslation(Cal2DX(trans))
    ;
}

// --------------------------------------------------------------------
TCompSkeleton::TCompSkeleton()
  : cb_bones("Bones")
{
  // Each instance will have it's own cte buffer
  bool is_ok = cb_bones.create(CB_SKIN_BONES);
  assert(is_ok);
}

TCompSkeleton::~TCompSkeleton() {
  if (model)
    delete model;
  cb_bones.destroy();
}

// ---------------------------------------------------------------------------------------
void TCompSkeleton::load(const json& j, TEntityParseContext& ctx) {

  CalLoader::setLoadingMode(LOADER_ROTATE_X_AXIS | LOADER_INVERT_V_COORD);

  std::string skel_name= j.value("skeleton", "");
  assert(!skel_name.empty());
  auto res_skel = Resources.get(skel_name)->as< CGameCoreSkeleton >();
  CalCoreModel* core_model = const_cast<CGameCoreSkeleton*>(res_skel);
  model = new CalModel(core_model);

  for (int i = 0; i < model->getCoreModel()->getCoreAnimationCount();i++) {

	  auto core_anim = model->getCoreModel()->getCoreAnimation(i);
	  if (core_anim)
		stringAnimationIdMap[core_anim->getName()] = i;
  }

  // Play the first animation, at weight 100%, now!
  actualCycleAnimId[0] = -1;
  actualCycleAnimId[1] = -1;

  actualCycleAnimId[0] = 0;
  model->getMixer()->blendCycle(actualCycleAnimId[0], 1.f, 0.f);

  // Do a time zero update just to have the bones in a correct place
  model->update(0.f);
}

void TCompSkeleton::update(float dt) {
  PROFILE_FUNCTION("updateSkel");
  assert(model);

  if (actualCycleAnimId[1] != -1 && lastFrameCyclicAnimationWeight != cyclicAnimationWeight) {
	  model->getMixer()->blendCycle(actualCycleAnimId[0], cyclicAnimationWeight, 0.f);
	  model->getMixer()->blendCycle(actualCycleAnimId[1], 1.f - cyclicAnimationWeight, 0.f);
  }

  TCompTransform* tmx = get<TCompTransform>();
  VEC3 pos = tmx->getPosition();
  QUAT rot = tmx->getRotation();
  model->getMixer()->setWorldTransform(DX2Cal(pos), DX2Cal(rot));
  model->update(dt);

  lastFrameCyclicAnimationWeight = cyclicAnimationWeight;
}

void TCompSkeleton::debugInMenu() {
  static int anim_id = 0;
  static float weight = 1.0f;
  static float lastWeight = 1.0f;
  static float in_delay = 0.3f;
  static float out_delay = 0.3f;
  static bool auto_lock = false;
  dbg("%f\n\n", weight);
  // Play aacton/cycle from the menu
  ImGui::DragInt("Anim Id", &anim_id, 0.1f, 0, model->getCoreModel()->getCoreAnimationCount()-1);
  auto core_anim = model->getCoreModel()->getCoreAnimation(anim_id);
  if(core_anim)
    ImGui::Text("%s", core_anim->getName().c_str());
  ImGui::DragFloat("In Delay", &in_delay, 0.01f, 0, 1.f);
  ImGui::DragFloat("Out Delay", &out_delay, 0.01f, 0, 1.f);
  ImGui::DragFloat("Weight", &weight, 0.01f, 0, 1.f);
  if (lastWeight != weight) {
	  model->getMixer()->blendCycle(actualCycleAnimId[0], weight, 0.f);
	  model->getMixer()->blendCycle(actualCycleAnimId[1], 1.f - weight, 0.f);
  }
  ImGui::Checkbox("Auto lock", &auto_lock);
  if (ImGui::SmallButton("As Cycle")) {
	  //model->getMixer()->clearCycle(actualCycleAnimId1, out_delay); 
	  model->getMixer()->blendCycle(actualCycleAnimId[1], weight, in_delay);
	
	//actualCycleId = anim_id;
  }
  if (ImGui::SmallButton("As Action")) {
    model->getMixer()->executeAction(anim_id, in_delay, out_delay, 1.0f, auto_lock);
  }
  if (ImGui::SmallButton("Set Zero")) {
	  model->getMixer()->setAnimationTime(0.f);
  }

  // Dump Mixer
  auto mixer = model->getMixer();
  for (auto a : mixer->getAnimationActionList()) {
    ImGui::Text("Action %s S:%d W:%1.2f Time:%1.4f/%1.4f"
      , a->getCoreAnimation()->getName().c_str()
      , a->getState()
      , a->getWeight()
      , a->getTime()
      , a->getCoreAnimation()->getDuration()
    );
    ImGui::SameLine();
    if (ImGui::SmallButton("X")) {
      auto core = (CGameCoreSkeleton*)model->getCoreModel();
      int id = core->getCoreAnimationId(a->getCoreAnimation()->getName());
      a->remove(out_delay);
    }
  }

  for (auto a : mixer->getAnimationCycle()) {
    ImGui::PushID(a);
    ImGui::Text("Cycle %s S:%d W:%1.2f Time:%1.4f"
      , a->getCoreAnimation()->getName().c_str()
      , a->getState()
      , a->getWeight()
      , a->getCoreAnimation()->getDuration()
    );
    ImGui::SameLine();
    if (ImGui::SmallButton("X")) {
      auto core = (CGameCoreSkeleton*)model->getCoreModel();
      int id = core->getCoreAnimationId(a->getCoreAnimation()->getName());
      mixer->clearCycle(id, out_delay);
    }
    ImGui::PopID();
  }

  // Show Skeleton Resource
  if (ImGui::TreeNode("Core")) {
    auto core_skel = (CGameCoreSkeleton*)model->getCoreModel();
    if (core_skel)
      core_skel->debugInMenu();
    ImGui::TreePop();
  }
  lastWeight = weight;
}

void TCompSkeleton::updateCtesBones() {
  PROFILE_FUNCTION("updateCtesBones");

  float* fout = &cb_bones.Bones[0]._11;

  CalSkeleton* skel = model->getSkeleton();
  auto& cal_bones = skel->getVectorBone();
  assert(cal_bones.size() < MAX_SUPPORTED_BONES);

  // For each bone from the cal model
  for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {
    CalBone* bone = cal_bones[bone_idx];

    const CalMatrix& cal_mtx = bone->getTransformMatrix();
    const CalVector& cal_pos = bone->getTranslationBoneSpace();

    *fout++ = cal_mtx.dxdx;
    *fout++ = cal_mtx.dydx;
    *fout++ = cal_mtx.dzdx;
    *fout++ = 0.f;
    *fout++ = cal_mtx.dxdy;
    *fout++ = cal_mtx.dydy;
    *fout++ = cal_mtx.dzdy;
    *fout++ = 0.f;
    *fout++ = cal_mtx.dxdz;
    *fout++ = cal_mtx.dydz;
    *fout++ = cal_mtx.dzdz;
    *fout++ = 0.f;
    *fout++ = cal_pos.x;
    *fout++ = cal_pos.y;
    *fout++ = cal_pos.z;
    *fout++ = 1.f;
  }

  cb_bones.updateGPU();
}

void TCompSkeleton::renderDebug() {
  assert(model);
  VEC3 lines[MAX_SUPPORTED_BONES][2];
  int nrLines = model->getSkeleton()->getBoneLines(&lines[0][0].x);
  TCompTransform* transform = get<TCompTransform>();
  float scale = transform->getScale();
  for (int currLine = 0; currLine < nrLines; currLine++)
    renderLine(lines[currLine][0] * scale, lines[currLine][1] * scale, VEC4(1, 1, 1, 1));
}

void TCompSkeleton::changeCyclicAnimation(int animId, float in_delay,  float out_delay) {

	model->getMixer()->clearCycle(actualCycleAnimId[0], out_delay);
	if (actualCycleAnimId[1] != -1) {
		model->getMixer()->clearCycle(actualCycleAnimId[1], out_delay);
	}
	model->getMixer()->blendCycle(animId, 1.0f, in_delay);
	actualCycleAnimId[0] = animId;
	actualCycleAnimId[1] = -1;
}

void TCompSkeleton::changeCyclicAnimation(int anim1Id, int anim2Id, float weight, float in_delay, float out_delay) {

	model->getMixer()->clearCycle(actualCycleAnimId[0], out_delay);
	if (actualCycleAnimId[1] != -1) {
		model->getMixer()->clearCycle(actualCycleAnimId[1], out_delay);
	}
	model->getMixer()->blendCycle(anim1Id, weight, in_delay);
	model->getMixer()->blendCycle(anim2Id, 1.f - weight, in_delay);
	actualCycleAnimId[0] = anim1Id;
	actualCycleAnimId[1] = anim2Id;
}

void TCompSkeleton::executeActionAnimation(int animId, float in_delay, float out_delay) {

	bool auto_lock = false;
	model->getMixer()->executeAction(animId, in_delay, out_delay, 1.0f, auto_lock);
}

//Set the weight added to a combination of twi cyclic animations
void TCompSkeleton::setCyclicAnimationWeight(float new_value) {

	cyclicAnimationWeight = new_value;
}

//Get the weight added to a combination of twi cyclic animations
float TCompSkeleton::getCyclicAnimationWeight() {

	return cyclicAnimationWeight;
}

//Returns the id value of the animation in the skeleton component. 
//If -1 is returned is because the animation name doesen't exists.
int TCompSkeleton::getAnimationIdByName(std::string animName) {
	
	if (stringAnimationIdMap.find(animName) != stringAnimationIdMap.end()) {
		return stringAnimationIdMap[animName];
	}
	return -1;
}

//Return if there is an animation of the type action executing
bool TCompSkeleton::actionAnimationOnExecution() {

	return false;
}

//return if the specified animation is executing
bool TCompSkeleton::isExecutingAnimation(int animId) {
	
	//return model->getMixer()->getAnimationActionList().size > 0;
	return false;
}
