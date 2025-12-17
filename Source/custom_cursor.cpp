#include "custom_cursor.h"
#include "system_cursor.h"
#include "System/Graphics.h"
#include "System/Sprite.h"
#include "System/RenderState.h"
#include "Lerp.h"
#include "mathUtils.h"
#include <algorithm>

CustomCursor& CustomCursor::Instance()
{
    static CustomCursor instance;
    return instance;
}

void CustomCursor::Initialize(const char* sprite_path)
{
    sprite_ = std::make_unique<Sprite>(Graphics::Instance().GetDevice(), sprite_path);
}

void CustomCursor::Update(float elapsed_time)
{
    position_ = SystemCursor::GetPosition();

    UpdateParticles(elapsed_time);
    UpdateFade(elapsed_time);

    if (is_visible_ && ShouldSpawnParticle())
    {
        SpawnParticle();
    }
}

void CustomCursor::Render(ID3D11DeviceContext* dc)
{
    if (!is_visible_ || !sprite_) return;

    const float current_alpha = GetCurrentAlpha();
    constexpr float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    RenderState* rs = Graphics::Instance().GetRenderState();
    dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
    dc->OMSetBlendState(rs->GetBlendState(BlendState::Transparency), blendFactor, 0xffffffff);

    RenderParticles(dc, current_alpha);
    RenderCursor(dc, current_alpha);
}

void CustomCursor::Show()
{
    is_visible_ = true;
    alpha_ = 1.0f;
    StopFade();
}

void CustomCursor::Hide()
{
    is_visible_ = false;
    particles_.clear();
}

void CustomCursor::FadeOut(float duration, std::function<void()> on_complete)
{
    StartFade(alpha_, 0.0f, duration, std::move(on_complete));
}

void CustomCursor::FadeIn(float duration, std::function<void()> on_complete)
{
    is_visible_ = true;
    StartFade(0.0f, 1.0f, duration, std::move(on_complete));
}

void CustomCursor::SetAlpha(float alpha)
{
    alpha_ = std::clamp(alpha, 0.0f, 1.0f);
    StopFade();
}

void CustomCursor::SetParticleSpawnInterval(int frames)
{
    particle_spawn_interval_ = frames;
}

void CustomCursor::EnableParticles(bool enable)
{
    particles_enabled_ = enable;
}

bool CustomCursor::IsVisible() const
{
    return is_visible_;
}

bool CustomCursor::IsFading() const
{
    return is_fading_;
}

DirectX::XMFLOAT2 CustomCursor::GetPosition() const
{
    return position_;
}

void CustomCursor::UpdateParticles(float elapsed_time)
{
    for (auto& particle : particles_)
    {
        particle.lifetime_lerp->Update(elapsed_time, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::ADD);
        particle.angle += 1.0f;
        if (particle.angle >= 360.0f) particle.angle = 0.0f;
    }

    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [](const Particle& p) {
                return p.lifetime_lerp->GetAmount() >= p.lifetime_lerp->GetMaxAmount();
            }),
        particles_.end()
    );
}

void CustomCursor::UpdateFade(float elapsed_time)
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

bool CustomCursor::ShouldSpawnParticle()
{
    if (!particles_enabled_) return false;

    particle_timer_++;
    if (particle_timer_ >= 400) particle_timer_ = 0;

    return particle_timer_ % particle_spawn_interval_ == 0;
}

void CustomCursor::SpawnParticle()
{
    particles_.push_back({
        MathUtils::RandomRangeInt(1, 3),
        std::make_unique<Lerp>(1.0f),
        position_,
        0.0f
        });
}

float CustomCursor::GetCurrentAlpha() const
{
    if (!is_fading_ || !fade_lerp_) return alpha_;

    return fade_lerp_->GetOffset(
        Lerp::EASING_TYPE::Normal,
        fade_start_alpha_,
        fade_target_alpha_
    );
}

void CustomCursor::RenderParticles(ID3D11DeviceContext* dc, float alpha)
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

void CustomCursor::RenderCursor(ID3D11DeviceContext* dc, float alpha)
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

void CustomCursor::StartFade(float start_alpha, float target_alpha, float duration, std::function<void()> callback)
{
    fade_start_alpha_ = start_alpha;
    fade_target_alpha_ = target_alpha;
    fade_lerp_ = std::make_unique<Lerp>(duration);
    is_fading_ = true;
    on_fade_complete_ = std::move(callback);
}

void CustomCursor::StopFade()
{
    is_fading_ = false;
    fade_lerp_.reset();
    on_fade_complete_ = nullptr;
}