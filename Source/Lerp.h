#pragma once

#include <math.h>
#include <cmath>

class Lerp
{
private:
    float amount = 0;
    float maxAmount = 0;
    float speed = 1.0f;
    bool Up = true, Down = false;

public:
    enum class EASING_TYPE
    {
        Normal = 0,
        InSine,
        InQuad,
        InCubic,
        InQuart,
        InQuint,
        InCirc,
        InBack,
        OutCubic,
        OutBack,
        OutElastic,
        OutBounce,
        InOutCubic,
        InOutQuart,
        InOutBack,
    };

    enum class RESET_TYPE
    {
        Fixed = 0,
        ReSet,
        Loop,
    };

    enum class ADD_TYPE
    {
        ADD,
        Subtract,
    };

public:
    Lerp(float max)
    {
        maxAmount = max;
    }
    ~Lerp() {}

    //線形補完関数
    //v1～v2までの値をtを使って計算する
    float lerp(float v1, float v2,float t)
    {
        return (1.0f - t) * v1 + t * v2;
    };
private:
    //イージング関数
    float easeInSine(float x)
    {
        return 1.0f - cosf((x * DirectX::XM_PI) / 2.0f);
    };

    //イージング関数
    float easeInQuad(float x)
    {
        return x * x;
    };

    //イージング関数
    float easeInCubic(float x)
    {
        return x * x * x;
    };

    //イージング関数
    float easeInQuart(float x)
    {
        return x * x * x * x;
    };

    //イージング関数
    float easeInQuint(float x)
    {
        return x * x * x * x * x;
    };

    //イージング関数
    float easeInCirc(float x)
    {
        return 1.0f - static_cast<float>(sqrt(1.0 - pow(x, 2)));
    };

    //イージング関数
    float easeInBack(float x)
    {
        const float c1 = 1.70158f;
        //const float c1 = 2.0f;
        const float c3 = c1 + 1;

        return c3 * x * x * x - c1 * x * x;
    };

    //イージング関数
    float easeOutCubic(float x)
    {
        return static_cast < float>(1.0f - pow(1.0f - x, 3.0f));
    };

    //イージング関数
    float easeOutBack(float x)
    {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;

        return 1.0f + c3 * pow(x - 1.0f, 3.0f) + c1 * pow(x - 1.0f, 2.0f);
    };

    //イージング関数
    float easeOutElastic(float x)
    {
        const float c4 = (2.0f * DirectX::XM_PI) / 3.0f;

        return x == 0.0f
            ? 0.0f
            : x == 1.0f
            ? 1.0f
            : pow(2.0f, -10.0f * x) * sin((x * 10.0f - 0.75f) * c4) + 1.0f;
    };

    //イージング関数
    float easeOutBounce(float x)
    {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;

        if (x < 1.0f / d1) {
            return n1 * x * x;
        }
        else if (x < 2.0f / d1) {
            return n1 * (x -= 1.5f / d1) * x + 0.75f;
        }
        else if (x < 2.5 / d1) {
            return n1 * (x -= 2.25f / d1) * x + 0.9375f;
        }
        else {
            return n1 * (x -= 2.625f / d1) * x + 0.984375f;
        }
    };

    //イージング関数
    float easeInOutCubic(float x)
    {
        return static_cast<float>(x < 0.5f ? 4.0f * x * x * x : 1.0f - pow(-2.0f * x + 2.0f, 3.0f) / 2.0f);
    };

    //イージング関数
    float easeInOutQuart(float x)
    {
        return static_cast<float>(x < 0.5f ? 8.0f * x * x * x * x : 1.0f - pow(-2.0f * x + 2.0f, 4.0f) / 2.0f);
    };

    //イージング関数
    float easeInOutBack(float x)
    {
        const float c1 = 1.70158f;
        const float c2 = c1 * 1.525f;

        return x < 0.5f
            ? static_cast<float>((pow(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) / 2.0f)
            : static_cast<float>((pow(2.0f * x - 2.0f, 2.0f) * ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f);
    };

public:
    void SetAmount(float a) { amount = a; }
    void SetSpeed(float s) { speed = s; }

    float GetAmount() { return amount; }
    float GetMaxAmount() { return maxAmount; }

    float GetOffset(EASING_TYPE type, float min, float max)
    {
        float t;
        t = amount / maxAmount;

        switch (type)
        {
        case Lerp::EASING_TYPE::Normal:
            return lerp(min, max, t);
        case Lerp::EASING_TYPE::InSine:
            return lerp(min, max, easeInSine(t));

        case Lerp::EASING_TYPE::InQuad:
            return lerp(min, max, easeInQuad(t));

        case Lerp::EASING_TYPE::InCubic:
            return lerp(min, max, easeInCubic(t));

        case Lerp::EASING_TYPE::InQuart:
            return lerp(min, max, easeInQuart(t));

        case Lerp::EASING_TYPE::InQuint:
            return lerp(min, max, easeInQuint(t));

        case Lerp::EASING_TYPE::InCirc:
            return lerp(min, max, easeInCirc(t));

        case Lerp::EASING_TYPE::InBack:
            return lerp(min, max, easeInBack(t));

        case Lerp::EASING_TYPE::OutCubic:
            return lerp(min, max, easeOutCubic(t));

        case Lerp::EASING_TYPE::OutBack:
            return lerp(min, max, easeOutBack(t));

        case Lerp::EASING_TYPE::OutElastic:
            return lerp(min, max, easeOutBack(t));

        case Lerp::EASING_TYPE::OutBounce:
            return lerp(min, max, easeOutBounce(t));

        case Lerp::EASING_TYPE::InOutCubic:
            return lerp(min, max, easeInOutCubic(t));

        case Lerp::EASING_TYPE::InOutQuart:
            return lerp(min, max, easeInOutQuart(t));

        case Lerp::EASING_TYPE::InOutBack:
            return lerp(min, max, easeInOutBack(t));
        }
        return 0;
    }

    float GetProgress(EASING_TYPE type)
    {
        float t;
        t = amount / maxAmount;

        switch (type)
        {
        case Lerp::EASING_TYPE::Normal:
            return t;
        case Lerp::EASING_TYPE::InSine:
            return easeInSine(t);

        case Lerp::EASING_TYPE::InQuad:
            return easeInQuad(t);

        case Lerp::EASING_TYPE::InCubic:
            return easeInCubic(t);

        case Lerp::EASING_TYPE::InQuart:
            return easeInQuart(t);

        case Lerp::EASING_TYPE::InQuint:
            return easeInQuint(t);

        case Lerp::EASING_TYPE::InCirc:
            return easeInCirc(t);

        case Lerp::EASING_TYPE::InBack:
            return easeInBack(t);

        case Lerp::EASING_TYPE::OutCubic:
            return easeOutCubic(t);

        case Lerp::EASING_TYPE::OutBack:
            return easeOutBack(t);

        case Lerp::EASING_TYPE::OutElastic:
            return easeOutBack(t);

        case Lerp::EASING_TYPE::OutBounce:
            return easeOutBounce(t);

        case Lerp::EASING_TYPE::InOutCubic:
            return easeInOutCubic(t);

        case Lerp::EASING_TYPE::InOutQuart:
            return easeInOutQuart(t);

        case Lerp::EASING_TYPE::InOutBack:
            return easeInOutBack(t);
        }
        return 0;
    }


public:
    void Update(float elapsedTime, RESET_TYPE type, ADD_TYPE aType)
    {
        if (type != RESET_TYPE::Loop)
        {
            if (aType == ADD_TYPE::Subtract) speed = std::abs(speed) * -1;
            if (aType == ADD_TYPE::ADD) speed = std::abs(speed);
        }
        amount += speed * elapsedTime;

        switch (type)
        {
        case Lerp::RESET_TYPE::Fixed:
            if (speed > 0.0f)
            {
                if (amount > maxAmount)
                {
                    amount = maxAmount;
                }
            }
            else
            {
                if (amount < 0.0f)
                {
                    amount = 0.0f;
                }
            }
            break;
        case Lerp::RESET_TYPE::ReSet:
            if (speed > 0.0f)
            {
                if (amount > maxAmount)
                {
                    amount = 0.0f;
                }
            }
            else
            {
                if (amount < 0.0f)
                {
                    amount = maxAmount;
                }
            }
            break;
        case Lerp::RESET_TYPE::Loop:
            if (amount >= maxAmount || amount <= 0.0f)
            {
                speed *= -1;
            }
            break;
        default:
            break;
        }
    }
};