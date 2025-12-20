#include <filesystem>
#include <imgui.h>
#include "Graphics.h"
#include "ResourceManager.h"

// 新規追加: Model用（GLTFモデル）
std::shared_ptr<Model> ResourceManager::LoadModel(const char* filename, float sampleRate)
{
	// 読み込み済みなら既存を返す
	if (gltfModels.find(filename) != gltfModels.end())
	{
		if (gltfModels[filename].expired() == false)
		{
			return gltfModels[filename].lock();
		}
	}

	// 新規Model作成＆読み込み
	auto model = std::make_shared<Model>(Graphics::Instance().GetDevice(), filename, sampleRate);

	// 読み込み管理用変数に登録
	gltfModels[filename] = model;
	return model;
}

// デバッグGUI描画
void ResourceManager::DrawDebugGUI()
{
	if (ImGui::CollapsingHeader("Resource", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Model（GLTF形式）
		if (ImGui::TreeNode("Model (GLTF)"))
		{
			for (auto it = gltfModels.begin(); it != gltfModels.end(); ++it)
			{
				std::filesystem::path filepath(it->first);
				int use_count = it->second.use_count();
				ImGui::Text("use_count = %5d : %s", use_count, filepath.filename().u8string().c_str());
			}
			ImGui::TreePop();
		}
	}
}

