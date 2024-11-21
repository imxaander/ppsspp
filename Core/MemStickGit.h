#include "Common/System/System.h"
#include "Common/Log.h"

#include "Core/Config.h"
#include "Core/System.h"

#include "git2.h"

namespace MemStickGit{
	std::string generateRepoURL();
    bool isRepo(const std::string path);
	bool InitMemStick();
	bool SyncMemStick();
	bool FastForwardMerge(git_repository* repo, const git_annotated_commit* annotated_commit);
}
