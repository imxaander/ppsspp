#include <UI/MemStickGitScreen.h>

#include "Core/Config.h"
#include "Core/MemStickGit.h"


MemStickGitSettingsScreen::~MemStickGitSettingsScreen() = default;

void MemStickGitSettingsScreen::CreateTabs(){
    auto sy = GetI18NCategory(I18NCat::SYSTEM);
    using namespace UI;
    CreateActionsTab(AddTab("MemStickGit Action", sy->T("Actions")));
    return;
};


void MemStickGitSettingsScreen::CreateActionsTab(UI::ViewGroup* viewGroup){
    VERBOSE_LOG(Log::System, "Created Test Tab");
    auto sy = GetI18NCategory(I18NCat::SYSTEM);

    using namespace UI;

    viewGroup->Add(new ItemHeader("Settings"));
    viewGroup->Add(new CheckBox(&g_Config.bUseMemstickGit, "Enable Memstick Git"));

    viewGroup->Add(new ItemHeader("Git Information"));
    viewGroup->Add(new PopupTextInputChoice(GetRequesterToken(), &g_Config.sMemstickGitRepo, sy->T("Memstick Git Repo"), "<https://privatekey@github.com/username/repo>", 256, screenManager()));
    viewGroup->Add(new ItemHeader("Operations"));
    viewGroup->Add(new Choice("Fetch Memstick from Git"))->OnClick.Add([=](UI::EventParams &) -> UI::EventReturn {
			MemStickGit::FetchMemStick();
			return UI::EVENT_DONE;
		});
    return;
}


void MemStickGitSettingsScreen::onFinish(DialogResult result){
    if(g_Config.Save("Saving MemstickGit Config")){
        VERBOSE_LOG(Log::System, "Can't Save MemStickGit Config");
    }
    return;
};

