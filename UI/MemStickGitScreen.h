#pragma once

#include "Common/UI/UIScreen.h"
#include "Common/UI/ViewGroup.h"
#include "UI/TabbedDialogScreen.h"

class MemStickGitSettingsScreen : public TabbedUIDialogScreenWithGameBackground {
public:
	MemStickGitSettingsScreen(const Path &gamePath) : TabbedUIDialogScreenWithGameBackground(gamePath) {};
    ~MemStickGitSettingsScreen();
	const char *tag() const override { return "MemStickGitScreen";}
    void CreateTabs() override;
    void CreateActionsTab(UI::ViewGroup* vg);
    void onFinish(DialogResult result) override;
};
