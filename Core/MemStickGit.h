#include "Common/System/System.h"
#include "Common/Log.h"

#include "Core/Config.h"
#include "Core/System.h"

#include "git2.h"

namespace MemStickGit{
    bool isRepo(const std::string path);
	bool InitMemStick();
	bool SyncMemStick();
}
