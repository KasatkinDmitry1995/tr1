// menu.cpp
#include "menu.h"
#include "imgui.h"

namespace {
    Menu::Settings g_settings;
    bool g_open = true;

    void ApplyStyle() {
        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowRounding = 6.0f;
        s.FrameRounding = 4.0f;
        s.GrabRounding = 4.0f;
        s.WindowPadding = ImVec2(12, 12);
        s.FramePadding = ImVec2(8, 4);
        s.ItemSpacing = ImVec2(8, 8);

        ImVec4* c = s.Colors;
        c[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.96f);
        c[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
        c[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
        c[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.20f, 0.20f, 1.00f);
        c[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.20f, 0.20f, 1.00f);
        c[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
        c[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
    }
}

namespace Menu {

    Settings& Get() { return g_settings; }
    bool IsOpen() { return g_open; }
    void Toggle() { g_open = !g_open; }

    void Render() {
        static bool styled = false;
        if (!styled) { ApplyStyle(); styled = true; }

        if (!g_open) return;

        ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

        ImGui::Begin("CS 1.6 Overlay", &g_open,
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoCollapse);

        // ---------- Triggerbot ----------
        ImGui::Text("Triggerbot");
        ImGui::Separator();

        ImGui::Checkbox("Enable Triggerbot", &g_settings.triggerbotEnabled);

        // Слайдер появляется только если триггербот включён
        if (g_settings.triggerbotEnabled) {
            ImGui::Indent(20.0f);  // сдвигаем вправо, чтобы визуально был "внутри"
            ImGui::SetNextItemWidth(250);  // на всю доступную ширину
            ImGui::SliderInt("Hold (ms)##trig", &g_settings.triggerHoldMs,
                10, 500, "%d ms");
            ImGui::Unindent(20.0f);
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // ---------- ESP ----------
        ImGui::Text("ESP");
        ImGui::Separator();

        ImGui::Checkbox("Enable ESP", &g_settings.espEnabled);
        ImGui::Checkbox("Show Names", &g_settings.showNames);

        ImGui::Spacing();
        ImGui::Spacing();

        // ---------- Кнопки управления ----------
        ImGui::Text("Control");
        ImGui::Separator();

        float fullWidth = ImGui::GetContentRegionAvail().x;
        float halfWidth = (fullWidth - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

        if (ImGui::Button("OK", ImVec2(halfWidth, 32))) {
            g_open = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Unload", ImVec2(halfWidth, 32))) {
            g_settings.requestUnload = true;
        }

        ImGui::End();
    }

} // namespace Menu