#ifndef INC_GAME_CORE_SKELETON_H_
#define INC_GAME_CORE_SKELETON_H_

#include "resources/resource.h"
#include "cal3d/cal3d.h"

// This is the CalCoreModel + IResource interface + other metadata
// we might need in the future and is shared between all instances
// of this model
class CGameCoreSkeleton : public IResource, public CalCoreModel {
  std::string root_path;

  bool convertCalCoreMesh2RenderMesh(CalCoreMesh* cal_mesh, const std::string& ofilename);

  struct AnimationCallback : public CalAnimationCallback {

	  const char* luaFunction;

	  void AnimationUpdate(float anim_time, CalModel *model);
	  void AnimationComplete(CalModel *model);
  };
  // ---------------------------------------------------
public:
  CGameCoreSkeleton( const std::string& aname ) : CalCoreModel(aname) {}
  bool create(const std::string& res_name);
  void debugInMenu() override;
  float              bone_ids_debug_scale = 1.f;
  std::vector< int > bone_ids_to_debug;

};

#endif

