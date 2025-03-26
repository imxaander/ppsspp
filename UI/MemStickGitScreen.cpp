#include <UI/MemStickGitScreen.h>

#include "Core/Config.h"
#include "Core/MemStickGit.h"


MemStickGitSettingsScreen::~MemStickGitSettingsScreen() = default;

void MemStickGitSettingsScreen::CreateTabs(){
    auto sy = GetI18NCategory(I18NCat::SYSTEM);
    using namespace UI;

	AddTab("MemStickGit Action", sy->T("Actions"), [this](UI::LinearLayout* parent) {
		CreateActionsTab(parent);
	});

	AddTab("MemStickGit Account", sy->T("Account"), [this](UI::LinearLayout* parent) {
		CreateAccountTab(parent);
	});
    return;
};

void MemStickGitSettingsScreen::CreateActionsTab(UI::ViewGroup* viewGroup){
    VERBOSE_LOG(Log::System, "Created Test Tab");
	//TODO: Fix all category issues and translations
    auto sy = GetI18NCategory(I18NCat::SYSTEM);

    using namespace UI;
    viewGroup->Add(new CheckBox(&g_Config.bUseMemstickGit, "Enable MemstickGit"));
	viewGroup->Add(new CheckBox(&g_Config.bMemstickGitSyncOnStartup, "Sync On Startup"));

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

	viewGroup->Add(new Choice("Commit Memstick to Git"))->OnClick.Add([=](UI::EventParams&) -> UI::EventReturn {
		if (MemStickGit::CommitMemStick()) {
			System_Toast(sy->T("Commit Succeded!"));
		}
		else {
			System_Toast(sy->T("Commit Failed!"));
		}
		return UI::EVENT_DONE;
	});

	viewGroup->Add(new Choice("Push Memstick to Git"))->OnClick.Add([=](UI::EventParams&) -> UI::EventReturn {
		if (MemStickGit::PushMemStick()) {
			System_Toast(sy->T("Pushing Succeeded!"));
		}
		else {
			System_Toast(sy->T("Pushing Failed!"));
		}
		return UI::EVENT_DONE;
		});
}

void MemStickGitSettingsScreen::CreateAccountTab(UI::ViewGroup* viewGroup){
	auto sy = GetI18NCategory(I18NCat::SYSTEM);

	using namespace UI;

	viewGroup->Add(new ItemHeader("Git Credentials"));
	viewGroup->Add(new PopupTextInputChoice(GetRequesterToken(), &g_Config.sMemstickGitPersonalToken, sy->T("Personal Token"), "ghp_XXXXXXXXXXXXXX", 64, screenManager()));
	viewGroup->Add(new PopupTextInputChoice(GetRequesterToken(), &g_Config.sMemstickGitUsername, sy->T("Username"), "hrydgard", 39, screenManager()));
	viewGroup->Add(new PopupTextInputChoice(GetRequesterToken(), &g_Config.sMemstickGitAuthorName, sy->T("Author Name"), "Preferrably your device \"tag\"", 64, screenManager()));
	viewGroup->Add(new PopupTextInputChoice(GetRequesterToken(), &g_Config.sMemstickGitAuthorEmail, sy->T("Author Email"), "Any Email Formatted Text :) ", 64, screenManager()));

	viewGroup->Add(new ItemHeader("Git Repository"));
	viewGroup->Add(new PopupTextInputChoice(GetRequesterToken(), &g_Config.sMemstickGitRepo, sy->T("Repo Name"), "hrydgard-memstick", 250, screenManager()));
}

void MemStickGitSettingsScreen::onFinish(DialogResult result){
    if(g_Config.Save("Saving MemstickGit Config")){
        VERBOSE_LOG(Log::System, "Can't Save MemStickGit Config");
    }
    return;
};

