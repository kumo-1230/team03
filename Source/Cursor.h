#pragma once

#include "CursorManager.h"
#include "System/Sprite.h"
#include "Lerp.h"
#include "mathUtils.h"

class Cursor
{
private:
	struct Particles
	{
		int type;
		std::unique_ptr<Lerp> showLerp;
		DirectX::XMFLOAT2 particlesPos;
		float angle = 0.0f;
	};
private:
	std::unique_ptr<Sprite> sprCursor = nullptr;
	int timer = 0;
	std::vector<Particles> particles{};
	DirectX::XMFLOAT2 pos{};
	bool fade = false;
	float a{};
private:
	Cursor()
	{
		sprCursor = std::make_unique<Sprite>("Data/Sprite/new_cursor.png");
	};
	~Cursor() {};
public:
	void SetA(float ah) { a = ah; }
	void SetFade(bool f) { fade = f; }
public:
	static Cursor& Instance()
	{
		static Cursor cursor;
		return cursor;
	}

	void Update(float elapsedTime)
	{
		fade = false;
		pos = CursorManager::GetCursorPosition();

		timer++;
		if (timer >= 400) timer = 0;

		if (timer % 30 == 0)
		{
			particles.push_back({ MathUtils::RandomRangeInt(1,3),std::make_unique<Lerp>(1.0f),pos,0.0f });
		}

		for (auto& p : particles)
		{
			p.showLerp->Update(elapsedTime, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::ADD);

			p.angle += 1;
			if (p.angle >= 360) p.angle = 0.0f;

		}
	}

	void Render(const RenderContext& rc)
	{
		if (!fade)
		{
			for (const auto& p : particles)
			{
				sprCursor->Render(rc,
					p.particlesPos.x - 25.0f, p.particlesPos.y - p.showLerp->GetOffset(Lerp::EASING_TYPE::Normal, 0.0f, -100.0f) - 25.0f, 0.0f,
					50.0f, 50.0f,
					50.0f * p.type, 0.0f,
					50.0f, 50.0f,
					p.angle,
					1, 1, 1, p.showLerp->GetOffset(Lerp::EASING_TYPE::Normal, 1.0f, 0.0f));
			}

			sprCursor->Render(rc, pos.x - 25.0f, pos.y - 25.0f, 0.0f, 50.0f, 50.0f, 50 * 0, 0.0f, 50.0f, 50.0f, 0.0f, 1, 1, 1, 1);
		}
		else
		{
			for (const auto& p : particles)
			{
				sprCursor->Render(rc,
					p.particlesPos.x - 25.0f, p.particlesPos.y - p.showLerp->GetOffset(Lerp::EASING_TYPE::Normal, 0.0f, -100.0f) - 25.0f, 0.0f,
					50.0f, 50.0f,
					50.0f * p.type, 0.0f,
					50.0f, 50.0f,
					p.angle,
					1, 1, 1, a);
			}

			sprCursor->Render(rc, pos.x - 25.0f, pos.y - 25.0f, 0.0f, 50.0f, 50.0f, 50 * 0, 0.0f, 50.0f, 50.0f, 0.0f, 1, 1, 1, a);
		}
		particles.erase(
			std::remove_if(particles.begin(), particles.end(),
				[](const Particles& p) {
					// Lerp ‚ª‘¶Ý‚µ‚Ä‚¢‚ÄAamount ‚ª maxAmount ‚É’B‚µ‚½‚çíœ
					return p.showLerp->GetAmount() >= p.showLerp->GetMaxAmount();
				}),
			particles.end()
		);
	}
};