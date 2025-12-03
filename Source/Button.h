#pragma once
#include "System/Sprite.h"
#include <memory>
#include <DirectXMath.h>

//レンダーモード
namespace BUTTON_R_MODE
{
	//通常
	const int NORMAL(0);
	//半透明
	const int HALF_INVISIBLE(1);
	//透明
	const int INVISIBLE(2);
}

class Button
{
private:
	//ボタンのスプライト
	std::unique_ptr<Sprite> sprite;
	//場所
	DirectX::XMFLOAT2 pos;
	//長さ
	DirectX::XMFLOAT2 length;
	//切り取り開始場所
	DirectX::XMFLOAT2 sPos;
	//切り取り長さ
	DirectX::XMFLOAT2 sLeng;
	//当たり判定場所
	DirectX::XMFLOAT2 hitPos;
	//当たり判定長さ
	DirectX::XMFLOAT2 hitLeng;
	//レイヤー
	int layer;
	//押したときに帰ってくる数字
	int mode;
	//判定
	bool valid;
	//レンダーモード
	int renderMode = BUTTON_R_MODE::NORMAL;
	//表示するときの大きさ変更
	float sizeOffset = 0.0f;
	//アルファ値変更
	bool alphaChange = false;
	//ラープ進行度
	float progress = 1.0f;
	//アルファ値
	float alphaMin = 0.0f;
	//アルファ値
	float alphaMax = 1.0f;

public:
	Button(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, int la,int n,bool v);
	Button(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, DirectX::XMFLOAT2 hp, DirectX::XMFLOAT2 hl, int la, int n, bool v);
	~Button() = default;
public:
	/////////////////////////////////////////////////////

	//秒を始める場所
	void SetPos(DirectX::XMFLOAT2 p) { pos = p; }
	//描画する画像の長さ
	void SetLength(DirectX::XMFLOAT2 l) { length = l; }
	//ボタンのレイヤー
	int GetLayer() { return layer; }
	//ヒットしたボタンが返す数値
	int GetMode() { return mode; }
	//判定を取るか設定
	void SetValid(bool i) { valid = i; }
	//今の判定モードを設定
	bool GetValid() { return valid; }
	//レンダーモード
	void SetRenderMode(int m) { renderMode = m; }
	//オフセットのセット
	void SetSizeOffset(float f) { sizeOffset = f; }

	DirectX::XMFLOAT2 GetPos() { return pos; }
	DirectX::XMFLOAT2 Getlength() { return length; }

	void SetAlphaChange(bool b) { alphaChange = b; }
	void SetAlphaMax(float a) { alphaMax = a; }
	void SetProgress(float p) { progress = p; }
	void SetSPos(DirectX::XMFLOAT2 p) { sPos = p; }
	void SetSLeng(DirectX::XMFLOAT2 p) { sLeng = p; }

	/////////////////////////////////////////////////////

public:
	void Render(const RenderContext& rc);

	bool HitButton(const DirectX::XMFLOAT2& p,const DirectX::XMFLOAT2& le);
private:
	//線形補完関数
	//v1～v2までの値をtを使って計算する
	float lerp(float v1, float v2, float t)
	{
		return (1.0f - t) * v1 + t * v2;
	};

};