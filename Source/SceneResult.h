#pragma once

#include "Scene.h"
#include <memory>
#include "System/Sprite.h"
#include "CameraController.h"
#include "sky_map.h"
#include "ScoreRender.h"
#include "Lerp.h"
#include "System/AudioSource.h"

#define CAMERA_EYA						(-3000)
#define CAMERA_TARGET_X					(-2000)
#define CAMERA_TARGET_Y					(500)
#define CAMERA_FOV						(90)

class SceneResult : public Scene
{
private:
	std::unique_ptr<Camera> camera                     = nullptr;
	std::unique_ptr<CameraController> cameraController = nullptr;

	std::unique_ptr<sky_map> skyMap                    = nullptr;

public:
	SceneResult();
	~SceneResult();

	//èâä˙âª
	void Initialize()override {};

	//èIóπâª
	void Finalize()override {};

public:

	void Update(float elapsedTime) override;

	void Render() override;

	//GUIï`âÊ
	void DrawGUI()override {};
};