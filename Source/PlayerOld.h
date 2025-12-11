#pragma once

#include "System/ModelRenderer.h"
#include "Character.h"
#include "Effect.h"
#include "System/AudioSource.h"
#include "Camera.h"
#include <memory>
#include "imgui.h"
#include "Lerp.h"

#define CHRINK				(600)
#define CT					(0.5f)
#define ADD_SPEED_MAX		(1000.0f)
#define ADD_SPEED_MIN		(0.0f)
#define NORMAL_SPEED		(500.0f)
#define LINE_SIZE_MAX		(10)
#define FOV_SPEED_OFFSET	(50)
#define FOV_SLOW_OFFSET		(30)
#define FOV					(80.0f)
#define FOV_H				(100.0f)
#define SPPED_RING			(10.0f)

#define FOV_MAX				(FOV + FOV_SPEED_OFFSET + SPPED_RING)
#define FOV_H_MAX			(FOV_H + FOV_SPEED_OFFSET + SPPED_RING)

#define RINGDISTNACE		(800)

#define Player_StartPosX	(0)

//プレイヤー
class PlayerOld : public Character
{
private:
	Effect* hitEffect = nullptr;

public:
	PlayerOld();
	~PlayerOld() override;

	//初期化
	void Initialize();

	//終了化
	void Finalize();

public:
	///////////////////////////////////////
	///////////////Get/Set/////////////////



	///////////////////////////////////////
public:
	//更新処理
	void Update(float elapsedTime,Camera* camera);

	//描画処理
	void Render(const RenderContext& rc, ModelRenderer* render);

	//デバッグ用GUI描画
	void DrawDebugGUI();

	//デバッグプリミティブ描画
	void RenderDebugPrimitive(const RenderContext& rc, ShapeRenderer* renderer);
};