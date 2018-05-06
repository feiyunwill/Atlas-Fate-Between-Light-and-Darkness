#include "mcv_platform.h"
#include "gui_bar.h"

using namespace GUI;

void CBar::render()
{
  float ratio = _barParams._processValue;
  ratio = clamp(ratio, 0.f, 1.f);
  MAT44 sz = MAT44::CreateScale(_params._size.x, _params._size.y, 1.f);
  MAT44 w = MAT44::CreateScale(ratio, 1.f, 1.f) * sz * _absolute;
  VEC2 maxUV = _imageParams._maxUV;
  maxUV.x *= ratio;
  Engine.get().getGUI().renderTexture(w,
    _imageParams._texture,
    _imageParams._minUV,
    maxUV,
    _imageParams._color);
}

TImageParams* CBar::getImageParams()
{
  return &_imageParams;
}

void CBar::update(float dt)
{

	float value = _barParams._processValue;
	if (EngineInput[VK_LEFT].isPressed())
	{
		value = clamp(value - 0.5f * dt, 0.f, 1.f);
	}
	if (EngineInput[VK_RIGHT].isPressed())
	{
		value = clamp(value + 0.5f * dt, 0.f, 1.f);
	}
	_barParams._processValue = value;

}
