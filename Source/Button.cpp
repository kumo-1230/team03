#include "Button.h"

//ポジション
//長さ
//レイヤー
//モード
//判定
Button::Button(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, int la, int n, bool v)
{
	sprite.reset(new Sprite(fileName));
	pos = p;
	length = l;
	layer = la;
	mode = n;
	valid = v;
	sPos = sp;
	sLeng = sl;
	hitPos = pos;
	hitLeng = length;
}

Button::Button(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, DirectX::XMFLOAT2 hp, DirectX::XMFLOAT2 hl, int la, int n, bool v)
	: Button(fileName, p, l, sp, sl, la, n, v)
{
	hitPos = hp;
	hitLeng = hl;
}

void Button::Render(const RenderContext& rc)
{
	DirectX::XMFLOAT4 color = { 1,1,1,1 };
	if (!alphaChange)
	{
		switch (renderMode)
		{
		case BUTTON_R_MODE::NORMAL:
			color = { 1,1,1,1 };
			alphaMin = 1.0f;
			break;
		case BUTTON_R_MODE::HALF_INVISIBLE:
			color = { 1,1,1,0.5f };
			alphaMin = 0.5f;
			break;
		case BUTTON_R_MODE::INVISIBLE:
			return;
		}
	}
	else
	{
		color.w = lerp(alphaMin, alphaMax, progress);
	}

	sprite->Render(rc, pos.x - sizeOffset, pos.y - sizeOffset, 0, length.x - sizeOffset, length.y - sizeOffset, sPos.x, sPos.y, sLeng.x, sLeng.y, 0, color.x, color.y, color.z, color.w);
}

//当たったか
bool Button::HitButton(const DirectX::XMFLOAT2& p, const DirectX::XMFLOAT2& le)
{
	//四角に当たっていなかったらfalse
	return !(hitPos.x + hitLeng.x < p.x ||
			 hitPos.x > p.x + le.x ||
			 hitPos.y + hitLeng.y < p.y ||
			 hitPos.y > p.y + le.y);
}
