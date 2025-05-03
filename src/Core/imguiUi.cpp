#include "imguiUi.hpp"
#include "app.hpp"

void SIPImageMenu(AppContext* context);
void SIPImageCreationMenu(AppContext* context, bool* showMenu);
void ShowError(std::string message);

static std::string errorMessage;

void MainImguiMenu(AppContext* context)
{
    static bool showSIPImageMenu = false;
    static bool showSIPImageCreationMenu = false;
    static bool setEST = false;
    float curTime = SDL_GetTicks();

    static ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_MenuBar;

    ImGui::Begin("SphericImageProjector", nullptr, window_flags);

    //ImGui::Text("Version %s", EngVer.c_str());
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

    if(ImGui::RadioButton("TimeZone", setEST))
    {
        setEST = !setEST;
    }

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

            if (ImGui::TreeNode((void*)(intptr_t)i, "Image %s", context->sipManager.images[i].file.c_str()))
            {
                ImGui::Image(ImTextureID(&context->sipManager.images[i].image.samplerBinding), {1280 / 2, 720 / 2}, {1,1}, {0,0});
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

    if(ImGui::RadioButton("AbsolutePath", absolutePath))
    {
        absolutePath = !absolutePath;
    }

    if(!absolutePath)
        ImGui::InputText("File name", file, 128);
    else
        ImGui::InputText("Path", file, 128);

    ImGui::InputFloat("Azimuth", &azimuth, 0.1f, 1.0f, "%.2f");
    azimuth = std::min(std::max(azimuth, 0.0f), 360.0f);
    ImGui::InputFloat("Altitude", &altitude, 0.1f, 1.0f, "%.2f");
    altitude = std::min(std::max(altitude, 0.0f), 90.0f);

    ImGui::InputFloat2("Angular Size", glm::value_ptr(angularSize), "%.2f");
    ImGui::InputInt("Time (Seconds)", &timeSeconds, 1.0f, 1.0f);
    ImGui::InputInt("Time (Minutes)", &timeMins, 1.0f, 1.0f);
    ImGui::InputInt("Time (Hours)", &timeHours, 1.0f, 1.0f);

    float time = (timeHours * 10000) + (timeMins * 100) + timeSeconds;

    if(ImGui::RadioButton("ApplyTilt", applyTilt))
    {
        applyTilt = !applyTilt;
    }

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
