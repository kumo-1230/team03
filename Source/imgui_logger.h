#pragma once
#include <imgui.h>
#include <cstring>
#include <string>
#include <vector>

class ImGuiLogger {
public:
    static ImGuiLogger& Instance() {
        static ImGuiLogger inst;
        return inst;
    }

    void AddLog(const char* txt) {
        if (!txt || txt[0] == '\0') return;

        size_t old_size = buf.size();
        size_t txt_len = strlen(txt);

        buf.resize(old_size + txt_len + 1);
        memcpy(buf.data() + old_size, txt, txt_len);
        buf[old_size + txt_len] = '\n';

        scroll_to_bottom_ = true;
    }

    void AddLog(const std::string& txt) { AddLog(txt.c_str()); }
    void AddLog(int v) { char tmp[32]; snprintf(tmp, sizeof(tmp), "%d", v); AddLog(tmp); }
    void AddLog(float v) { char tmp[32]; snprintf(tmp, sizeof(tmp), "%.3f", v); AddLog(tmp); }

    void Clear() {
        buf.clear();
    }

    void Render() {
        ImGui::Begin("Log");

        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &auto_scroll_);

        ImGui::Separator();

        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (!buf.empty()) {
            ImGui::TextUnformatted(buf.data(), buf.data() + buf.size());
        }

        if (scroll_to_bottom_ && auto_scroll_) {
            ImGui::SetScrollHereY(1.0f);
            scroll_to_bottom_ = false;
        }

        ImGui::EndChild();
        ImGui::End();
    }

private:
    ImGuiLogger() = default;
    ImGuiLogger(const ImGuiLogger&) = delete;
    ImGuiLogger& operator=(const ImGuiLogger&) = delete;

    std::vector<char> buf;
    bool auto_scroll_ = true;
    bool scroll_to_bottom_ = false;
};
