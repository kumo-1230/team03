#pragma once
#include "ui_panel.h"
#include "Lerp.h"
#include "System/AudioSource.h"

class Pose {
public:
    enum class SENSITIVITY_TYPE {
        LOW = 0,
        NORMALE,
        HIGH,
    };
    enum class FOV_TYPE {
        NORMALE = 0,
        HIGH,
    };
    enum class SHAKE_TYPE {
        ON = 0,
        OFF
    };
    enum class DRUNKENNESS_TYPE {
        NONE = 0,
        LOW,
        HIGH,
    };

    static Pose& Instance() {
        static Pose pose;
        return pose;
    }

    void SetPose(bool b) { onPose = b; }
    bool GetPose() const { return onPose; }
    int GetSenitivity() const { return sensitivity; }
    int GetFov() const { return fov; }
    int GetShake() const { return shake; }
    int GetDrunkenness() const { return drunkenness; }
    void SetTutorial(bool b) { tutorial = b; }

    void Update(float elapsedTime);
    void Render(const RenderContext& rc);

private:
    Pose();
    ~Pose();
    void Initializer();
    bool PoseOff();

    bool onPose{ false };
    std::unique_ptr<UiPanel> settingMenu{ nullptr };
    std::unique_ptr<UiPanel> poseMenu{ nullptr };
    std::unique_ptr<Lerp> lerp{ nullptr };
    std::unique_ptr<Lerp> lerpButton{ nullptr };
    std::unique_ptr<Sprite> sprPoseBack{ nullptr };
    std::unique_ptr<Sprite> sprSettingBack{ nullptr };
    std::unique_ptr<Sprite> sprSetting{ nullptr };
    AudioSource* setPause{ nullptr };
    AudioSource* pauseBackSE{ nullptr };
    AudioSource* pauseSelectSE{ nullptr };

    int sensitivity{ 0 };
    int fov{ 0 };
    int shake{ 0 };
    int drunkenness{ 0 };
    int buttonSensitivity{ 0 };
    int buttonFov{ 0 };
    int buttonShake{ 0 };
    int buttonDrunkenness{ 0 };
    int holdNum{ -1 };
    int num{ -1 };
    int numBank{ -1 };

    bool fadeIn{ false };
    bool fadeOut{ false };
    float alhpa{ 0.0f };
    bool tutorial{ false };

    UiButton* poseButtons[4]{ nullptr };
    UiButton* settingButtons[11]{ nullptr };
};