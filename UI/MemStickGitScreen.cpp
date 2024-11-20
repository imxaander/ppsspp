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
    viewGroup->Add(new Choice("Initialize Memstick from Git"))->OnClick.Add([=](UI::EventParams &) -> UI::EventReturn {
		if (MemStickGit::InitMemStick()) {
			System_Toast(sy->T("Initialization Succeded!"));
		}
		else {
			System_Toast(sy->T("Already Initiated or Repo URL Errors. Can't Initialize!"));
		};
		return UI::EVENT_DONE;
	});
	viewGroup->Add(new Choice("Sync Memstick to Git"))->OnClick.Add([=](UI::EventParams&) -> UI::EventReturn {
		if (MemStickGit::SyncMemStick()) {
			System_Toast(sy->T("Synchronization Succeded!"));
		}
		else {
			System_Toast(sy->T("Synchronization Failed! Is the initialization done in the past?"));
		}
		return UI::EVENT_DONE;
	});
}

void MemStickGitSettingsScreen::onFinish(DialogResult result){
    if(g_Config.Save("Saving MemstickGit Config")){
        VERBOSE_LOG(Log::System, "Can't Save MemStickGit Config");
    }
    return;
};

