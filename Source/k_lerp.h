#ifndef TWEEN_H_
#define TWEEN_H_

#include <functional>
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <algorithm>

// イージング関数の種類
enum class EaseType {
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseInQuart,
    EaseOutQuart,
    EaseInOutQuart,
    EaseInBack,
    EaseOutBack,
    EaseInOutBack,
    EaseInElastic,
    EaseOutElastic,
    EaseInBounce,
    EaseOutBounce
};

// イージング関数の実装
class Easing {
public:
    static float Apply(EaseType type, float t) {
        switch (type) {
        case EaseType::Linear:
            return t;
        case EaseType::EaseInQuad:
            return t * t;
        case EaseType::EaseOutQuad:
            return t * (2.0f - t);
        case EaseType::EaseInOutQuad:
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        case EaseType::EaseInCubic:
            return t * t * t;
        case EaseType::EaseOutCubic:
            return (--t) * t * t + 1.0f;
        case EaseType::EaseInOutCubic:
            return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
        case EaseType::EaseInQuart:
            return t * t * t * t;
        case EaseType::EaseOutQuart:
            return 1.0f - (--t) * t * t * t;
        case EaseType::EaseInOutQuart:
            return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - 8.0f * (--t) * t * t * t;
        case EaseType::EaseInBack: {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1.0f;
            return c3 * t * t * t - c1 * t * t;
        }
        case EaseType::EaseOutBack: {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1.0f;
            return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
        }
        case EaseType::EaseInOutBack: {
            const float c1 = 1.70158f;
            const float c2 = c1 * 1.525f;
            return t < 0.5f
                ? (powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
                : (powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
        }
        case EaseType::EaseInElastic: {
            const float c4 = (2.0f * DirectX::XM_PI) / 3.0f;
            return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * c4);
        }
        case EaseType::EaseOutElastic: {
            const float c4 = (2.0f * DirectX::XM_PI) / 3.0f;
            return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
        }
        case EaseType::EaseInBounce:
            return 1.0f - Apply(EaseType::EaseOutBounce, 1.0f - t);
        case EaseType::EaseOutBounce: {
            const float n1 = 7.5625f;
            const float d1 = 2.75f;
            if (t < 1.0f / d1) {
                return n1 * t * t;
            }
            else if (t < 2.0f / d1) {
                return n1 * (t -= 1.5f / d1) * t + 0.75f;
            }
            else if (t < 2.5f / d1) {
                return n1 * (t -= 2.25f / d1) * t + 0.9375f;
            }
            else {
                return n1 * (t -= 2.625f / d1) * t + 0.984375f;
            }
        }
        default:
            return t;
        }
    }
};

// 基底Tweenクラス
class Tween {
public:
    Tween(float duration, EaseType ease = EaseType::Linear)
        : duration_(duration),
        elapsed_time_(0.0f),
        ease_type_(ease),
        is_complete_(false),
        is_playing_(true),
        loop_(false),
        yoyo_(false),
        going_forward_(true) {
    }

    virtual ~Tween() = default;

    virtual void Update(float delta_time) {
        if (!is_playing_ || is_complete_) return;

        elapsed_time_ += delta_time;

        if (elapsed_time_ >= duration_) {
            if (loop_) {
                if (yoyo_) {
                    going_forward_ = !going_forward_;
                    elapsed_time_ = 0.0f;
                }
                else {
                    elapsed_time_ = 0.0f;
                }
            }
            else {
                elapsed_time_ = duration_;
                is_complete_ = true;
            }
        }

        float t = duration_ > 0.0f ? elapsed_time_ / duration_ : 1.0f;
        t = std::clamp(t, 0.0f, 1.0f);

        if (yoyo_ && !going_forward_) {
            t = 1.0f - t;
        }

        float eased_t = Easing::Apply(ease_type_, t);
        ApplyTween(eased_t);

        if (is_complete_ && on_complete_) {
            on_complete_();
        }
    }

    virtual void ApplyTween(float t) = 0;

    void Play() { is_playing_ = true; }
    void Pause() { is_playing_ = false; }
    void Stop() {
        is_playing_ = false;
        is_complete_ = true;
        elapsed_time_ = 0.0f;
    }
    void Reset() {
        elapsed_time_ = 0.0f;
        is_complete_ = false;
        is_playing_ = true;
        going_forward_ = true;
    }

    void SetLoop(bool loop) { loop_ = loop; }
    void SetYoyo(bool yoyo) { yoyo_ = yoyo; }
    void SetOnComplete(std::function<void()> callback) { on_complete_ = callback; }

    bool IsComplete() const { return is_complete_; }
    bool IsPlaying() const { return is_playing_; }
    float GetProgress() const { return duration_ > 0.0f ? elapsed_time_ / duration_ : 1.0f; }

protected:
    float duration_;
    float elapsed_time_;
    EaseType ease_type_;
    bool is_complete_;
    bool is_playing_;
    bool loop_;
    bool yoyo_;
    bool going_forward_;
    std::function<void()> on_complete_;
};

// float値用のTween
class FloatTween : public Tween {
public:
    FloatTween(float* target, float start, float end, float duration, EaseType ease = EaseType::Linear)
        : Tween(duration, ease), target_(target), start_(start), end_(end) {
        *target_ = start_;
    }

    void ApplyTween(float t) override {
        *target_ = start_ + (end_ - start_) * t;
    }

private:
    float* target_;
    float start_;
    float end_;
};

// XMFLOAT3用のTween
class Vector3Tween : public Tween {
public:
    Vector3Tween(DirectX::XMFLOAT3* target, const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end,
        float duration, EaseType ease = EaseType::Linear)
        : Tween(duration, ease), target_(target), start_(start), end_(end) {
        *target_ = start_;
    }

    void ApplyTween(float t) override {
        target_->x = start_.x + (end_.x - start_.x) * t;
        target_->y = start_.y + (end_.y - start_.y) * t;
        target_->z = start_.z + (end_.z - start_.z) * t;
    }

private:
    DirectX::XMFLOAT3* target_;
    DirectX::XMFLOAT3 start_;
    DirectX::XMFLOAT3 end_;
};

// カラー用のTween
class ColorTween : public Tween {
public:
    ColorTween(DirectX::XMFLOAT4* target, const DirectX::XMFLOAT4& start, const DirectX::XMFLOAT4& end,
        float duration, EaseType ease = EaseType::Linear)
        : Tween(duration, ease), target_(target), start_(start), end_(end) {
        *target_ = start_;
    }

    void ApplyTween(float t) override {
        target_->x = start_.x + (end_.x - start_.x) * t;
        target_->y = start_.y + (end_.y - start_.y) * t;
        target_->z = start_.z + (end_.z - start_.z) * t;
        target_->w = start_.w + (end_.w - start_.w) * t;
    }

private:
    DirectX::XMFLOAT4* target_;
    DirectX::XMFLOAT4 start_;
    DirectX::XMFLOAT4 end_;
};

// Tweenマネージャー
class TweenManager {
public:
    static TweenManager& Instance() {
        static TweenManager instance;
        return instance;
    }

    template<typename T, typename... Args>
    T* AddTween(Args&&... args) {
        auto tween = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = tween.get();
        tweens_.emplace_back(std::move(tween));
        return ptr;
    }

    void Update(float delta_time) {
        for (auto& tween : tweens_) {
            if (tween) {
                tween->Update(delta_time);
            }
        }

        tweens_.erase(
            std::remove_if(tweens_.begin(), tweens_.end(),
                [](const std::unique_ptr<Tween>& tween) {
                    return !tween || tween->IsComplete();
                }),
            tweens_.end());
    }

    void Clear() {
        tweens_.clear();
    }

    size_t GetActiveTweenCount() const {
        return tweens_.size();
    }

private:
    TweenManager() = default;
    ~TweenManager() = default;
    TweenManager(const TweenManager&) = delete;
    TweenManager& operator=(const TweenManager&) = delete;

    std::vector<std::unique_ptr<Tween>> tweens_;
};

#endif // TWEEN_H_