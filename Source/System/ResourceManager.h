#pragma once

#include <memory>
#include <string>
#include <map>
#include "ModelResource.h"

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
	std::shared_ptr<ModelResource> LoadModelResource(const char* filename);

	std::shared_ptr<Model> LoadModel(const char* filename, float sampleRate = 0.0f);

	void DrawDebugGUI();

private:
	using ModelMap = std::map<std::string, std::weak_ptr<ModelResource>>;

	ModelMap		models;
	std::map<std::string, std::weak_ptr<Model>> gltfModels;
};
