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

    float totalTime = context->sipManager.baseTime;
    int utcHours = floor(totalTime / 10000);
    int utcMins = (int)floor(totalTime / 100) % 100;
    int utcSeconds = (int)totalTime % 100;

    int utcHoursCur = (int)(floorf(((curTime / 1000.0f + utcSeconds) / 60.0f + utcMins) / 60.0f) + utcHours);
    int utcMinsCur = (int)(floorf((curTime / 1000.0f + utcSeconds) / 60.0f) + utcMins) % 60;
    int utcSecondsCur = ((int)(curTime / 1000.0f + utcSeconds) % 60);

    ImGui::Text("Base Time (UTC): %02d:%02d:%02d", utcHours, utcMins, utcSeconds);
    ImGui::Text("Time From Base (UTC): %02d:%02d:%02d", utcHoursCur, utcMinsCur, utcSecondsCur);

    ImGui::End();
}

void DrawImgui(SDL_GPUCommandBuffer* cmdBuf, SDL_GPURenderPass* renderPass)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, cmdBuf);
    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, cmdBuf, renderPass);
}
