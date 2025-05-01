#include "imguiUi.hpp"
#include "app.hpp"


void MainImguiMenu(AppContext* context)
{
    float curTime = SDL_GetTicks();

    static ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_MenuBar;

    ImGui::Begin("SpaceTesting", nullptr, window_flags);

    //ImGui::Text("Version %s", EngVer.c_str());
    ImGui::Text("%.3f ms/frame (%.1f FPS)", (1.0f / context->imguiIO->Framerate) * 1000.0f, context->imguiIO->Framerate);
    //ImGui::Text("%u verts, %u indices (%u tris)", vertCount, indCount, indCount / 3);
    //ImGui::Text("DrawCall Avg: (%.1f) DC/frame, DrawCall Total (%d)", drawCallAvg, DrawCallCount);
    ImGui::Text("Time Open %.0f:%.2d", floorf(curTime / (60.0f * 1000.0f)), (int)(curTime / 1000.0f) % 60);
    //ImGui::Text("Time taken for Update run %.2fms ", fabs(updateTime));
    //ImGui::Text("Time taken for Fixed Update run %.2fms ", fabs(updateFixedTime));

    ImGui::Spacing();
    ImGui::Text("Number of loaded images: %d", context->sipManager.currentImageCount);

    double SIPtime = context->sipManager.time;

    float totalTime = context->sipManager.baseTime;
    Uint64 utcHours = floor(totalTime / 10000);
    Uint64 utcMins = (Uint64)floor(totalTime / 100) % 100;
    Uint64 utcSeconds = (Uint64)totalTime % 100;

    Uint64 utcHoursCur = (Uint64)(floorf(((SIPtime + utcSeconds) / 60.0f + utcMins) / 60.0f) + utcHours);
    Uint64 utcMinsCur = (Uint64)(floorf((SIPtime + utcSeconds) / 60.0f) + utcMins) % 60;
    Uint64 utcSecondsCur = ((Uint64)(SIPtime + utcSeconds) % 60);

    ImGui::Text("Base Time (UTC): %02lu:%02lu:%02lu", utcHours, utcMins, utcSeconds);
    if(utcHoursCur >= 24.0f)
    {
        ImGui::Text("Time From Base (UTC): %02lu:%02lu:%02lu:%02lu", utcHoursCur / 24, utcHoursCur % 24, utcMinsCur, utcSecondsCur);
    }
    else
        ImGui::Text("Time From Base (UTC): %02lu:%02lu:%02lu", utcHoursCur, utcMinsCur, utcSecondsCur);

    ImGui::InputFloat("TimeScale", &context->sipManager.speed, 1.0f, 10.0f, "%.1fx");

    ImGui::End();
}

void DrawImgui(SDL_GPUCommandBuffer* cmdBuf, SDL_GPURenderPass* renderPass)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, cmdBuf);
    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, cmdBuf, renderPass);
}
