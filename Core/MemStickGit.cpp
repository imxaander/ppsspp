#include <filesystem>

#include "Core/MemStickGit.h"
#include "Core/System.h"
#include "Core/Config.h"

namespace fs = std::filesystem;

namespace MemStickGit{
    

    bool isRepo(const std::string path){
        bool result = false;
        git_libgit2_init();
        ERROR_LOG(Log::FileSystem, "mEMSTICK DIR: %s", path.c_str());
        if(git_repository_open_ext(NULL, path.c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0){
            ERROR_LOG(Log::FileSystem, "is a git repo");
            result = true;
        }
        
        git_libgit2_shutdown();
        return result;
    }
    void FetchMemStick(){
        std::string memstickDir = GetSysDirectory(DIRECTORY_MEMSTICK_ROOT).ToString();
        if(isRepo(memstickDir)){
            ERROR_LOG(Log::FileSystem, "There is an existing repository. You might want to sync().");
            return;
        }

        git_libgit2_init();
        git_clone_options clone_opts  = GIT_CLONE_OPTIONS_INIT;
        clone_opts.fetch_opts = GIT_FETCH_OPTIONS_INIT;
        clone_opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        git_repository* out;
        
        int clone_error = git_clone(&out, g_Config.sMemstickGitRepo.c_str(), memstickDir.c_str(), &clone_opts);
        if(clone_error < 0){
            ERROR_LOG(Log::FileSystem, "Cannot Proceeed to Git Fetch:  %s", git_error_last()->message);
        }else{
            ERROR_LOG(Log::FileSystem, "Git Clone Success");
        }

        git_repository_free(out);
        git_libgit2_shutdown();

        return;
    }
}
