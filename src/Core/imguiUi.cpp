#include "imguiUi.hpp"
#include "app.hpp"

void SIPImageMenu(AppContext* context);
void SIPImageCreationMenu(AppContext* context, bool* showMenu);
void ShowError(std::string message);
void SetImGuiStyle();
static void HelpMarker(const char* desc);

static std::string errorMessage;
static bool setEST = false;

void MainImguiMenu(AppContext* context)
{
    static bool showSIPImageMenu = false;
    static bool showSIPImageCreationMenu = false;
    float curTime = SDL_GetTicks();

    static ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_MenuBar;

    ImGui::Begin("SphericImageProjector", nullptr, window_flags);

    //ImGui::Text("Version %s", EngVer.c_str());
    ImGui::Text("Gpu: %s", context->gpuName);
    ImGui::Text("API: %s", context->api);
    ImGui::Text("%.3f ms/frame (%.1f FPS)", (1.0f / context->imguiIO->Framerate) * 1000.0f, context->imguiIO->Framerate);
    //ImGui::Text("%u verts, %u indices (%u tris)", vertCount, indCount, indCount / 3);
    //ImGui::Text("DrawCall Avg: (%.1f) DC/frame, DrawCall Total (%d)", drawCallAvg, DrawCallCount);
    ImGui::Text("Time Open %.0f:%.2d", floorf(curTime / (60.0f * 1000.0f)), (int)(curTime / 1000.0f) % 60);
    //ImGui::Text("Time taken for Update run %.2fms ", fabs(updateTime));
    //ImGui::Text("Time taken for Fixed Update run %.2fms ", fabs(updateFixedTime));

    ImGui::Spacing();
    ImGui::Text("Number of loaded images: %d / %d", context->sipManager.currentImageCount, context->sipManager.maxImages);

    double SIPtime = context->sipManager.time;

    float totalTime = context->sipManager.baseTime;
    int64_t utcHours = floor(totalTime / 10000);
    int64_t utcMins = (int64_t)floor(totalTime / 100) % 100;
    int64_t utcSeconds = (int64_t)totalTime % 100;

    int64_t utcHoursCur = (int64_t)(floorf(((SIPtime + utcSeconds) / 60.0f + utcMins) / 60.0f) + utcHours);
    Uint64 utcMinsCur = (Uint64)(floorf((SIPtime + utcSeconds) / 60.0f) + utcMins) % 60;
    Uint64 utcSecondsCur = ((Uint64)(SIPtime + utcSeconds) % 60);

    ImGui::Checkbox("TimeZone", &setEST);
    HelpMarker("The current timzeone option (UTC/EST)");

    if(!setEST)
    {
        ImGui::Text("Base Time (UTC): %02lu:%02lu:%02lu", utcHours, utcMins, utcSeconds);
        if(utcHoursCur >= 24.0f)
        {
            ImGui::Text("Time From Base (UTC): %02ld:%02ld:%02lu:%02lu", utcHoursCur / 24, utcHoursCur % 24, utcMinsCur, utcSecondsCur);
        }
        else
            ImGui::Text("Time From Base (UTC): %02ld:%02lu:%02lu", utcHoursCur, utcMinsCur, utcSecondsCur);
    }
    else
    {
        utcHours -= 4;
        if(utcHours < 0)
            utcHours += 24;
        utcHoursCur -= 4;
        if(utcHoursCur < 0)
            utcHoursCur += 24;

        ImGui::Text("Base Time (EST): %02lu:%02lu:%02lu", utcHours, utcMins, utcSeconds);
        if(utcHoursCur >= 24.0f)
        {
            ImGui::Text("Time From Base (EST): %02ld:%02ld:%02lu:%02lu", utcHoursCur / 24, utcHoursCur % 24, utcMinsCur, utcSecondsCur);
        }
        else
            ImGui::Text("Time From Base (EST): %02ld:%02lu:%02lu", utcHoursCur, utcMinsCur, utcSecondsCur);
    }

    ImGui::InputFloat("TimeScale", &context->sipManager.speed, 1.0f, 10.0f, "%.1fx");

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menus"))
        {
            ImGui::MenuItem("SIPImageMenu", NULL, &showSIPImageMenu);
            ImGui::MenuItem("SIPImageCreationMenu", NULL, &showSIPImageCreationMenu);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if(showSIPImageMenu)
    {
        SIPImageMenu(context);
    }
    if(showSIPImageCreationMenu)
    {
        SIPImageCreationMenu(context, &showSIPImageCreationMenu);
    }

    ImGui::End();
}

void DrawImgui(SDL_GPUCommandBuffer* cmdBuf, SDL_GPURenderPass* renderPass)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, cmdBuf);
    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, cmdBuf, renderPass);
}

void SIPImageMenu(AppContext* context)
{
    float radToDeg = 180.0f / M_PI;

    ImGui::Begin("SIPImageMenu");
    ImGui::SetWindowSize({1280 / 2, 720 / 2}, ImGuiCond_Once);

    float width = ImGui::GetWindowWidth();
    float height = ImGui::GetWindowHeight();

    if (ImGui::TreeNode("Images"))
    {
        for (unsigned int i = 4; i < context->sipManager.maxImages; i++)
        {
            if(!context->sipManager.images[i].created)
                continue;

            ImGui::Separator();
            if (ImGui::TreeNode((void*)(intptr_t)i, "Image %s", context->sipManager.images[i].file.c_str()))
            {

                ImGui::BulletText("Location: %s", context->sipManager.images[i].file.c_str());
                ImGui::BulletText("Azimuth: %.2f°", context->sipManager.images[i].azimuth);
                ImGui::BulletText("Altitude: %.2f°", context->sipManager.images[i].altitude);
                ImGui::BulletText("Angular Size: %.2f°, %.2f°", context->sipManager.images[i].angularSize.x * radToDeg, context->sipManager.images[i].angularSize.y * radToDeg);

                float totalTime = context->sipManager.images[i].time;
                int64_t utcHours = floor(totalTime / 10000);
                int64_t utcMins = (int64_t)floor(totalTime / 100) % 100;
                int64_t utcSeconds = (int64_t)totalTime % 100;
                if(!setEST)
                    ImGui::BulletText("Time (UTC): %02lu:%02lu:%02lu", utcHours, utcMins, utcSeconds);
                else
                {
                    utcHours -= 4;
                    if(utcHours < 0)
                        utcHours += 24;

                    ImGui::BulletText("Time (EST): %02lu:%02lu:%02lu", utcHours, utcMins, utcSeconds);
                }

                if(ImGui::CollapsingHeader("Stored Image"))
                {
                    ImGui::Image(ImTextureID(&context->sipManager.images[i].image.samplerBinding), {1280 / 2, 720 / 2}, {1,1}, {0,0});
                }

                ImGui::TreePop();
            }
            else
            {
                ImGui::SameLine();
                std::string buttonText = "Delete##" + std::to_string(i);
                if(ImGui::Button(buttonText.c_str()))
                {
                    context->sipManager.DeleteImage(context, i);
                }
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
}

void SIPImageCreationMenu(AppContext* context, bool* showMenu)
{
    static bool absolutePath = false;
    static char file[128];
    static float azimuth = 0.0f;
    static float altitude = 0.0f;
    static glm::vec2 angularSize = glm::vec2(0.0f);
    static int timeSeconds = 0;
    static int timeMins = 0;
    static int timeHours = 0;
    static bool applyTilt = true;

    ImGui::Begin("SIPImageCreator");
    ImGui::SetWindowSize({300, 400}, ImGuiCond_Once);

    ImGui::Checkbox("AbsolutePath", &absolutePath);

    if(!absolutePath)
        ImGui::InputText("File name", file, 128);
    else
        ImGui::InputText("Path", file, 128);

    ImGui::InputFloat("Azimuth", &azimuth, 0.1f, 1.0f, "%.2f°");
    azimuth = std::min(std::max(azimuth, 0.0f), 360.0f);
    ImGui::InputFloat("Altitude", &altitude, 0.1f, 1.0f, "%.2f°");
    altitude = std::min(std::max(altitude, -90.0f), 90.0f);

    ImGui::InputFloat2("Angular Size", glm::value_ptr(angularSize), "%.2f°");
    if(setEST)
        ImGui::Text("Time input expects in EST");
    ImGui::InputInt("Time (s)", &timeSeconds, 1.0f, 1.0f);
    HelpMarker("The seconds of when the image was taken");
    timeSeconds = std::max(timeSeconds, 0);
    ImGui::InputInt("Time (m)", &timeMins, 1.0f, 1.0f);
    HelpMarker("The minutes of when the image was taken");
    timeMins = std::max(timeMins, 0);
    ImGui::InputInt("Time (h)", &timeHours, 1.0f, 1.0f);
    HelpMarker("The hours of when the image was taken");
    timeHours = std::max(timeHours, 0);

    ImGui::Checkbox("ApplyTilt", &applyTilt);

    if(ImGui::Button("Add Image"))
    {
        bool allowCreation = true;

        std::string path = std::string(context->basePath) + std::string(file);

        //check if the file exsists
        if(!absolutePath)
        {
            if(!SDL_GetPathInfo(path.c_str(), NULL))
            {
                std::string text;
                ShowError("File not found " + std::string(file));
                allowCreation = false;
            }
        }
        else
        {
            if(!SDL_GetPathInfo(path.c_str(), NULL))
            {
                ShowError("Path not found " + path);
                allowCreation = false;
            }
        }
        if(std::string(file).size() == 0)
        {
            ShowError("File/Path is empty");
            allowCreation = false;
        }

        if(allowCreation)
        {
            //do conversion if we are in est to utc
            if(setEST)
            {
                timeHours += 4;
                if(timeHours >= 24)
                    timeHours -= 24;
            }

            float time = (timeHours * 10000) + (timeMins * 100) + timeSeconds;

            if(absolutePath)
            {
                if(!applyTilt)
                    context->sipManager.LoadImageAbsolute(std::string(file), azimuth, altitude, angularSize, time, applyTilt, context);
                else
                    context->sipManager.LoadImageAbsolute(std::string(file), azimuth, altitude, angularSize, time, applyTilt, context);
            }
            else
            {
                if(!applyTilt)
                    context->sipManager.LoadImage(std::string(file), azimuth, altitude, angularSize, time, applyTilt, context);
                else
                    context->sipManager.LoadImage(std::string(file), azimuth, altitude, angularSize, time, applyTilt, context);
            }

            *showMenu = false;
        }
    }

    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", errorMessage.c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }


    ImGui::End();
}


void ShowError(std::string message)
{
    ImGui::OpenPopup("Error");
    errorMessage = message;
}

void SetImGuiStyle()
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.19f, 0.17f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.73f, 0.25f, 0.28f, 0.40f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.95f, 0.38f, 0.38f, 0.55f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.56f, 0.41f, 0.38f, 0.58f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.31f, 0.09f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.51f, 0.29f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.31f, 0.09f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(1.00f, 0.49f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 0.24f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.00f, 0.49f, 0.00f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(1.00f, 0.42f, 0.00f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.81f, 0.43f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.87f, 0.66f, 0.00f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(1.00f, 0.49f, 0.00f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(1.00f, 0.59f, 0.00f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(1.00f, 0.59f, 0.00f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.60f, 0.27f, 0.00f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.39f, 0.00f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.60f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.47f, 0.14f, 0.11f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.57f, 0.20f, 0.19f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.57f, 0.20f, 0.20f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.51f, 0.13f, 0.11f, 0.86f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.55f, 0.13f, 0.11f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.55f, 0.13f, 0.11f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.45f, 0.10f, 0.07f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.45f, 0.11f, 0.09f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.51f, 0.15f, 0.11f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.49f, 0.14f, 0.11f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    ImGui::GetStyle().FrameRounding = 1;
}

static void HelpMarker(const char* desc)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
