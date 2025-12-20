#pragma once

#include <memory>
#include <string>
#include <map>

// リソースマネージャー
class ResourceManager
{
private:
	ResourceManager() {}
	~ResourceManager() {}

public:
	// 唯一のインスタンス取得
	static ResourceManager& Instance()
	{
		static ResourceManager instance;
		return instance;
	}

	// モデルリソース読み込み

	std::shared_ptr<Model> LoadModel(const char* filename, float sampleRate = 0.0f);

	void DrawDebugGUI();

private:

	std::map<std::string, std::weak_ptr<Model>> gltfModels;
};
