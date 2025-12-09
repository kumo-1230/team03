#pragma once
#include "System/Sprite.h"
#include <math.h>
#include <cmath>
#include <vector>

class scoreRender {
public:
	//void ScoreRenderDigit(RenderContext rc, Sprite* sprite, int score,float texSizeX, float texSizeY, float worldPosX, float worldPosY,float scale = 1)
	//{
	//	if (score == 0)
	//	{
	//		sprite->Render(rc.deviceContext,
	//			worldPosX + 40,
	//			worldPosY,
	//			0.0f,
	//			texSizeX * scale,
	//			texSizeY * scale,
	//			0,
	//			0,
	//			texSizeX,
	//			texSizeY,
	//			0.0f,
	//			1.0f,
	//			1.0f,
	//			1.0f,
	//			1.0f);
	//		return;
	//	}
	//	int ketasuu = static_cast<int>(std::log10(std::abs(score))) + 1;

	//	bool isNegative = (score < 0);
	//	int absScore = std::abs(score);

	//	float w = worldPosX;
	//	if (score < 0)worldPosX = w + 20;
	//	else worldPosX = w;
	//	//マイナスかどうかの処理
	//	for (int i = ketasuu;i > 0;i--)
	//	{
	//		sprite->Render(rc.deviceContext,
	//			worldPosX + i*texSizeX,
	//			worldPosY,
	//			0.0f,
	//			texSizeX,
	//			texSizeY,
	//		    texSizeX * (absScore % 10),
	//			0,
	//			texSizeX,
	//			texSizeY,
	//			0.0f,
	//			1.0f,
	//			1.0f,
	//			1.0f,
	//			1.0f);
	//		absScore /= 10;
	//	}
	//}
    //回転バージョン
	void ScoreRenderDigit(RenderContext rc, Sprite* sprite, int score,
		float texSizeX, float texSizeY, float worldPosX, float worldPosY, float rotationDeg, float scale = 1)
	{
		if (score == 0)
		{
			sprite->Render(rc.deviceContext,
				worldPosX, worldPosY, 0.0f,
				texSizeX * scale, texSizeY * scale,
				0, 0, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, 1);
			return;
		}

		int ketasuu = static_cast<int>(std::log10(std::abs(score))) + 1;
		bool isNegative = (score < 0);
		int absScore = std::abs(score);

		// 全体の幅と中心
		float totalWidth = ketasuu * texSizeX;
		if (isNegative) totalWidth += texSizeX * 0.5f;
		float centerX = worldPosX + totalWidth * 0.5f;
		float centerY = worldPosY + texSizeY * 0.5f;

		float rad = DirectX::XMConvertToRadians(rotationDeg); // ←座標回転用

		for (int i = 0; i < ketasuu; ++i)
		{
			int digit = absScore % 10;
			absScore /= 10;

			// 各桁のローカル座標
			float localX = totalWidth - (i + 1) * texSizeX;
			float localY = 0.0f;

			// スコア全体の中心を基準に回転
			float dx = localX - totalWidth * 0.5f;
			float dy = localY - texSizeY * 0.5f;
			float rotatedX = dx * cosf(rad) - dy * sinf(rad);
			float rotatedY = dx * sinf(rad) + dy * cosf(rad);

			// ワールド座標
			float drawX = centerX + rotatedX;
			float drawY = centerY + rotatedY;

			sprite->Render(rc.deviceContext,
				drawX, drawY, 0.0f,
				texSizeX, texSizeY,
				texSizeX * digit, 0, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, 1);
		}
	}
	//回転と縦でフォント選べるバージョン
	void ScoreRenderDigit(RenderContext rc, Sprite* sprite, int score,
		float texSizeX, float texSizeY, float worldPosX, float worldPosY, float rotationDeg,int vertical)
	{
		if (score == 0)
		{
			sprite->Render(rc.deviceContext,
				worldPosX, worldPosY, 0.0f,
				texSizeX, texSizeY,
				0, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, 1);
			return;
		}

		int ketasuu = static_cast<int>(std::log10(std::abs(score))) + 1;
		bool isNegative = (score < 0);
		int absScore = std::abs(score);

		// 全体の幅と中心
		float totalWidth = ketasuu * texSizeX;
		if (isNegative) totalWidth += texSizeX * 0.5f;
		float centerX = worldPosX + totalWidth * 0.5f;
		float centerY = worldPosY + texSizeY * 0.5f;

		float rad = DirectX::XMConvertToRadians(rotationDeg); // ←座標回転用

		for (int i = 0; i < ketasuu; ++i)
		{
			int digit = absScore % 10;
			absScore /= 10;

			// 各桁のローカル座標
			float localX = totalWidth - (i + 1) * texSizeX;
			float localY = 0.0f;

			// スコア全体の中心を基準に回転
			float dx = localX - totalWidth * 0.5f;
			float dy = localY - texSizeY * 0.5f;
			float rotatedX = dx * cosf(rad) - dy * sinf(rad);
			float rotatedY = dx * sinf(rad) + dy * cosf(rad);

			// ワールド座標
			float drawX = centerX + rotatedX;
			float drawY = centerY + rotatedY;

			sprite->Render(rc.deviceContext,
				drawX, drawY, 0.0f,
				texSizeX, texSizeY,
				texSizeX * digit, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, 1);
		}
	}
	//回転と縦でフォント選べて不透明度変えれるバージョン
	void ScoreRenderDigit(RenderContext rc, Sprite* sprite, int score,
		float texSizeX, float texSizeY, float worldPosX, float worldPosY, float rotationDeg, int vertical,float alpha)
	{
		if (score == 0)
		{
			sprite->Render(rc.deviceContext,
				worldPosX, worldPosY, 0.0f,
				texSizeX, texSizeY,
				0, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
			return;
		}

		int ketasuu = static_cast<int>(std::log10(std::abs(score))) + 1;
		bool isNegative = (score < 0);
		int absScore = std::abs(score);

		// 全体の幅と中心
		float totalWidth = ketasuu * texSizeX;
		if (isNegative) totalWidth += texSizeX * 0.5f;
		float centerX = worldPosX + totalWidth * 0.5f;
		float centerY = worldPosY + texSizeY * 0.5f;

		float rad = DirectX::XMConvertToRadians(rotationDeg); // ←座標回転用

		for (int i = 0; i < ketasuu; ++i)
		{
			int digit = absScore % 10;
			absScore /= 10;

			// 各桁のローカル座標
			float localX = totalWidth - (i + 1) * texSizeX;
			float localY = 0.0f;

			// スコア全体の中心を基準に回転
			float dx = localX - totalWidth * 0.5f;
			float dy = localY - texSizeY * 0.5f;
			float rotatedX = dx * cosf(rad) - dy * sinf(rad);
			float rotatedY = dx * sinf(rad) + dy * cosf(rad);

			// ワールド座標
			float drawX = centerX + rotatedX;
			float drawY = centerY + rotatedY;

			sprite->Render(rc.deviceContext,
				drawX, drawY, 0.0f,
				texSizeX, texSizeY,
				texSizeX * digit, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
		}
	}

	//回転と縦でフォント選べて不透明度変えれて先頭になんか書けるバージョン
	void ScoreRenderDigit(RenderContext rc, Sprite* sprite, int score,
		float texSizeX, float texSizeY, float worldPosX, float worldPosY,
		float rotationDeg, int vertical, float alpha,int num)
	{
		if (score == 0)
		{
			sprite->Render(rc.deviceContext,
				worldPosX, worldPosY, 0.0f,
				texSizeX, texSizeY,
				0, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
			return;
		}

		int ketasuu = static_cast<int>(std::log10(std::abs(score))) + 1;
		bool isNegative = (score < 0);
		int absScore = std::abs(score);

		//------------------------------------------
		// 桁を左→右順に格納（逆順に取得して反転）
		//------------------------------------------
		std::vector<int> digits;
		for (int i = 0; i < ketasuu; ++i)
		{
			digits.push_back(absScore % 10);
			absScore /= 10;
		}
		std::reverse(digits.begin(), digits.end()); // ←ここ重要！

		//------------------------------------------
		// 全体の幅と中心
		//------------------------------------------
		float totalWidth = (ketasuu + 1) * texSizeX; // ×分を追加
		float centerX = worldPosX + totalWidth * 0.5f;
		float centerY = worldPosY + texSizeY * 0.5f;

		float rad = DirectX::XMConvertToRadians(rotationDeg);

		//------------------------------------------
		// 1. 「×」を一番左に描画
		//------------------------------------------
		{
			float localX = 0.0f;
			float localY = 0.0f;

			float dx = localX - totalWidth * 0.5f;
			float dy = localY - texSizeY * 0.5f;
			float rotatedX = dx * cosf(rad) - dy * sinf(rad);
			float rotatedY = dx * sinf(rad) + dy * cosf(rad);

			float drawX = centerX + rotatedX;
			float drawY = centerY + rotatedY;

			sprite->Render(rc.deviceContext,
				drawX, drawY, 0.0f,
				texSizeX, texSizeY,
				texSizeX * num, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
		}

		//------------------------------------------
		// 2. 数字を左→右に描画
		//------------------------------------------
		for (int i = 0; i < ketasuu; ++i)
		{
			int digit = digits[i];

			float localX = texSizeX * (i + 1); // ×の分ずらす
			float localY = 0.0f;

			float dx = localX - totalWidth * 0.5f;
			float dy = localY - texSizeY * 0.5f;
			float rotatedX = dx * cosf(rad) - dy * sinf(rad);
			float rotatedY = dx * sinf(rad) + dy * cosf(rad);

			float drawX = centerX + rotatedX;
			float drawY = centerY + rotatedY;

			sprite->Render(rc.deviceContext,
				drawX, drawY, 0.0f,
				texSizeX, texSizeY,
				texSizeX * digit, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
		}
	}

	//回転と縦でフォント選べて不透明度変えれて先頭になんか書けて大きさ指定できるバージョン
	void ScoreRenderDigit(RenderContext rc, Sprite* sprite, int score,
		float texSizeX, float texSizeY, float worldPosX, float worldPosY,
		float rotationDeg, int vertical, float alpha, int num, float scale)
	{
		if (score == 0)
		{
			sprite->Render(rc.deviceContext,
				worldPosX, worldPosY, 0.0f,
				texSizeX * scale, texSizeY * scale,
				0, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
			return;
		}

		int ketasuu = static_cast<int>(std::log10(std::abs(score))) + 1;
		int absScore = std::abs(score);

		std::vector<int> digits;
		digits.reserve(ketasuu);
		for (int i = 0; i < ketasuu; ++i)
		{
			digits.push_back(absScore % 10);
			absScore /= 10;
		}
		std::reverse(digits.begin(), digits.end());

		// ★ scale を反映
		float totalWidth = (ketasuu + 1) * texSizeX * scale;
		float totalHeight = texSizeY * scale;

		float centerX = worldPosX + totalWidth * 0.5f;
		float centerY = worldPosY + totalHeight * 0.5f;

		float rad = DirectX::XMConvertToRadians(rotationDeg);

		//------------------------------------------
		// 1. × の描画（左端）
		//------------------------------------------
		{
			float localX = 0.0f;
			float localY = 0.0f;

			float dx = localX - totalWidth * 0.5f;
			float dy = localY - totalHeight * 0.5f;

			float rotatedX = dx * cosf(rad) - dy * sinf(rad);
			float rotatedY = dx * sinf(rad) + dy * cosf(rad);

			float drawX = centerX + rotatedX;
			float drawY = centerY + rotatedY;

			sprite->Render(rc.deviceContext,
				drawX, drawY, 0.0f,
				texSizeX * scale, texSizeY * scale,
				texSizeX * num, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
		}

		//------------------------------------------
		// 2. 数字を描画
		//------------------------------------------
		for (int i = 0; i < ketasuu; ++i)
		{
			int digit = digits[i];

			// ★ scale を反映
			float localX = texSizeX * scale * (i + 1);
			float localY = 0.0f;

			float dx = localX - totalWidth * 0.5f;
			float dy = localY - totalHeight * 0.5f;

			float rotatedX = dx * cosf(rad) - dy * sinf(rad);
			float rotatedY = dx * sinf(rad) + dy * cosf(rad);

			float drawX = centerX + rotatedX;
			float drawY = centerY + rotatedY;

			sprite->Render(rc.deviceContext,
				drawX, drawY, 0.0f,
				texSizeX * scale, texSizeY * scale,
				texSizeX * digit, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
		}
	}

	void ScoreRenderDigit_NoHead_Spacing(RenderContext rc, Sprite* sprite, int score,
		float texSizeX, float texSizeY, float worldPosX, float worldPosY,
		float rotationDeg, int vertical, float alpha, float scale, float spacing)
	{
		if (score == 0)
		{
			sprite->Render(rc.deviceContext,
				worldPosX, worldPosY, 0.0f,
				texSizeX * scale, texSizeY * scale,
				0, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
			return;
		}

		int ketasuu = static_cast<int>(std::log10(std::abs(score))) + 1;
		int absScore = std::abs(score);

		std::vector<int> digits;
		digits.reserve(ketasuu);
		for (int i = 0; i < ketasuu; i++)
		{
			digits.push_back(absScore % 10);
			absScore /= 10;
		}
		std::reverse(digits.begin(), digits.end());

		// ★ spacing 分だけ幅が狭くなる
		float charWidth = texSizeX * scale - spacing;
		float totalWidth = charWidth * ketasuu;
		float totalHeight = texSizeY * scale;

		float centerX = worldPosX + totalWidth * 0.5f;
		float centerY = worldPosY + totalHeight * 0.5f;

		float rad = DirectX::XMConvertToRadians(rotationDeg);

		for (int i = 0; i < ketasuu; i++)
		{
			int digit = digits[i];

			float localX = charWidth * i;
			float localY = 0;

			float dx = localX - totalWidth * 0.5f;
			float dy = localY - totalHeight * 0.5f;

			float rotatedX = dx * cosf(rad) - dy * sinf(rad);
			float rotatedY = dx * sinf(rad) + dy * cosf(rad);

			float drawX = centerX + rotatedX;
			float drawY = centerY + rotatedY;

			sprite->Render(rc.deviceContext,
				drawX, drawY, 0.0f,
				texSizeX * scale, texSizeY * scale,
				texSizeX * digit, texSizeY * vertical, texSizeX, texSizeY,
				rotationDeg, 1, 1, 1, alpha);
		}
	}
};
