#pragma once

#include "Scene.h"

//シーンマネージャー
class SceneManager
{
private:
	Scene* currrentScene = nullptr;
	Scene* nextScene = nullptr;

public:
	SceneManager() {}
	~SceneManager() {}

public:
	//シングルトン
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}

	//更新処理
	void Update(float elapsedTime);

	//描画処理
	void Render();

	//GUI描画
	void DrawGUI();

	//シーンクリア
	void Clear();

	//シーン切り替え
	void ChangeScene(Scene* scene);
};
