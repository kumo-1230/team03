#include "SceneManager.h"

void SceneManager::Update(float elapsedTime)
{
	if (nextScene != nullptr)
	{
		//終了
		Clear();

		//新しいシーン設定
		currrentScene = nextScene;
		nextScene = nullptr;

		//初期化
		if (currrentScene->IsReady() == false)
		{
			currrentScene->Initialize();
		}
	}

	if (currrentScene != nullptr)
	{
		currrentScene->Update(elapsedTime);
	}

}

void SceneManager::Render()
{
	if (currrentScene != nullptr)
	{
		currrentScene->Render();
	}
}

void SceneManager::DrawGUI()
{
	if (currrentScene != nullptr)
	{
		currrentScene->DrawGUI();
	}
}

void SceneManager::Clear()
{
	if (currrentScene != nullptr)
	{
		currrentScene->Finalize();
		delete currrentScene;
		currrentScene = nullptr;
	}
}

//シーン切り替え
void SceneManager::ChangeScene(Scene* scene)
{
	nextScene = scene;
}
