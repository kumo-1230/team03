// k_cursor.h
#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <memory>
#include <functional>
#include <vector>
#include "System/Graphics.h"
#include "System/Sprite.h"
#include "Lerp.h"
#include "mathUtils.h"

// カーソルの表示状態管理（システムカーソル）
class SystemCursor
{
public:
    static void Show()
    {
        if (!is_visible_)
        {
            ::ShowCursor(TRUE);
            is_visible_ = true;
        }
    }

    static void Hide()
    {
        if (is_visible_)
        {
            ::ShowCursor(FALSE);
            is_visible_ = false;
        }
    }

    static bool IsVisible() { return is_visible_; }

    static DirectX::XMFLOAT2 GetPosition()
    {
        POINT cursor;
        ::GetCursorPos(&cursor);
        ::ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);
        return { static_cast<float>(cursor.x), static_cast<float>(cursor.y) };
    }

    static void SetPosition(const DirectX::XMFLOAT2& pos)
    {
        POINT screenPos{ static_cast<LONG>(pos.x), static_cast<LONG>(pos.y) };
        ::ClientToScreen(Graphics::Instance().GetWindowHandle(), &screenPos);
        ::SetCursorPos(screenPos.x, screenPos.y);
    }

    static void CenterCursor()
    {
        const float centerX = Graphics::Instance().GetScreenWidth() / 2.0f;
        const float centerY = Graphics::Instance().GetScreenHeight() / 2.0f;
        SetPosition({ centerX, centerY });
    }

private:
    inline static bool is_visible_ = true;
};

// カスタムカーソルの描画管理
class CustomCursor
{
private:
    struct Particle
    {
        int type;
        std::unique_ptr<Lerp> lifetime_lerp;
        DirectX::XMFLOAT2 position;
        float angle = 0.0f;
    };

public:
    static CustomCursor& Instance()
    {
        static CustomCursor instance;
        return instance;
    }

    void Initialize(const char* sprite_path = "Data/Sprite/new_cursor.png")
    {
        sprite_ = std::make_unique<Sprite>(Graphics::Instance().GetDevice(), sprite_path);
    }

    void Update(float elapsed_time)
    {
        position_ = SystemCursor::GetPosition();

        UpdateParticles(elapsed_time);
        UpdateFade(elapsed_time);

        // パーティクル生成
        if (is_visible_ && ShouldSpawnParticle())
        {
            SpawnParticle();
        }
    }

    void Render(ID3D11DeviceContext* dc)
    {
        if (!is_visible_ || !sprite_) return;

        const float current_alpha = GetCurrentAlpha();

        // パーティクル描画
        RenderParticles(dc, current_alpha);

        // カーソル本体描画
        RenderCursor(dc, current_alpha);
    }

    // 表示制御
    void Show()
    {
        is_visible_ = true;
        alpha_ = 1.0f;
        StopFade();
    }

    void Hide()
    {
        is_visible_ = false;
        particles_.clear();
    }

    // フェード制御
    void FadeOut(float duration, std::function<void()> on_complete = nullptr)
    {
        StartFade(alpha_, 0.0f, duration, std::move(on_complete));
    }

    void FadeIn(float duration, std::function<void()> on_complete = nullptr)
    {
        is_visible_ = true;
        StartFade(0.0f, 1.0f, duration, std::move(on_complete));
    }

    // 即座にアルファ値設定
    void SetAlpha(float alpha)
    {
        alpha_ = std::clamp(alpha, 0.0f, 1.0f);
        StopFade();
    }

    // パーティクル設定
    void SetParticleSpawnInterval(int frames) { particle_spawn_interval_ = frames; }
    void EnableParticles(bool enable) { particles_enabled_ = enable; }

    // 状態取得
    bool IsVisible() const { return is_visible_; }
    bool IsFading() const { return is_fading_; }
    DirectX::XMFLOAT2 GetPosition() const { return position_; }

private:
    CustomCursor() = default;
    ~CustomCursor() = default;
    CustomCursor(const CustomCursor&) = delete;
    CustomCursor& operator=(const CustomCursor&) = delete;

    void UpdateParticles(float elapsed_time)
    {
        for (auto& particle : particles_)
        {
            particle.lifetime_lerp->Update(elapsed_time, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::ADD);
            particle.angle += 1.0f;
            if (particle.angle >= 360.0f) particle.angle = 0.0f;
        }

        // 寿命切れのパーティクルを削除
        particles_.erase(
            std::remove_if(particles_.begin(), particles_.end(),
                [](const Particle& p) {
                    return p.lifetime_lerp->GetAmount() >= p.lifetime_lerp->GetMaxAmount();
                }),
            particles_.end()
        );
    }

    void UpdateFade(float elapsed_time)
    {
        if (!is_fading_ || !fade_lerp_) return;

        fade_lerp_->Update(elapsed_time, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::ADD);

        if (fade_lerp_->GetAmount() >= fade_lerp_->GetMaxAmount())
        {
            alpha_ = fade_target_alpha_;
            is_fading_ = false;

            if (on_fade_complete_)
            {
                auto callback = std::move(on_fade_complete_);
                on_fade_complete_ = nullptr;
                callback();
            }
        }
    }

    bool ShouldSpawnParticle()
    {
        if (!particles_enabled_) return false;

        particle_timer_++;
        if (particle_timer_ >= 400) particle_timer_ = 0;

        return particle_timer_ % particle_spawn_interval_ == 0;
    }

    void SpawnParticle()
    {
        particles_.push_back({
            MathUtils::RandomRangeInt(1, 3),
            std::make_unique<Lerp>(1.0f),
            position_,
            0.0f
            });
    }

    float GetCurrentAlpha() const
    {
        if (!is_fading_ || !fade_lerp_) return alpha_;

        return fade_lerp_->GetOffset(
            Lerp::EASING_TYPE::Normal,
            fade_start_alpha_,
            fade_target_alpha_
        );
    }

    void RenderParticles(ID3D11DeviceContext* dc, float alpha)
    {
        for (const auto& particle : particles_)
        {
            const float offset_y = particle.lifetime_lerp->GetOffset(
                Lerp::EASING_TYPE::Normal, 0.0f, -100.0f
            );
            const float particle_alpha = particle.lifetime_lerp->GetOffset(
                Lerp::EASING_TYPE::Normal, 1.0f, 0.0f
            ) * alpha;

            sprite_->Render(dc,
                particle.position.x - 25.0f,
                particle.position.y - offset_y - 25.0f,
                0.0f,
                50.0f, 50.0f,
                50.0f * particle.type, 0.0f,
                50.0f, 50.0f,
                particle.angle,
                1.0f, 1.0f, 1.0f, particle_alpha
            );
        }
    }

    void RenderCursor(ID3D11DeviceContext* dc, float alpha)
    {
        sprite_->Render(dc,
            position_.x - 25.0f,
            position_.y - 25.0f,
            0.0f,
            50.0f, 50.0f,
            0.0f, 0.0f,
            50.0f, 50.0f,
            0.0f,
            1.0f, 1.0f, 1.0f, alpha
        );
    }

    void StartFade(float start_alpha, float target_alpha, float duration, std::function<void()> callback)
    {
        fade_start_alpha_ = start_alpha;
        fade_target_alpha_ = target_alpha;
        fade_lerp_ = std::make_unique<Lerp>(duration);
        is_fading_ = true;
        on_fade_complete_ = std::move(callback);
    }

    void StopFade()
    {
        is_fading_ = false;
        fade_lerp_.reset();
        on_fade_complete_ = nullptr;
    }

private:
    // スプライト
    std::unique_ptr<Sprite> sprite_;

    // 位置
    DirectX::XMFLOAT2 position_{};

    // 表示制御
    bool is_visible_ = true;
    float alpha_ = 1.0f;

    // フェード制御
    bool is_fading_ = false;
    float fade_start_alpha_ = 1.0f;
    float fade_target_alpha_ = 1.0f;
    std::unique_ptr<Lerp> fade_lerp_;
    std::function<void()> on_fade_complete_;

    // パーティクル
    std::vector<Particle> particles_;
    int particle_timer_ = 0;
    int particle_spawn_interval_ = 30;
    bool particles_enabled_ = true;
};
