#include <filesystem>
#include "Core/MemStickGit.h"

namespace fs = std::filesystem;

namespace MemStickGit{
    
	std::string generateRepoURL() {
		//always secured http req, idk
		std::string repoURL = "https://" + g_Config.sMemstickGitUsername + ":" + g_Config.sMemstickGitPersonalToken + "@github.com/" + g_Config.sMemstickGitUsername + "/" + g_Config.sMemstickGitRepo + ".git";

		System_Toast(repoURL.c_str());
		return repoURL;
	}
    bool isRepo(const std::string path){
        bool result = false;
        git_libgit2_init();
        if(git_repository_open_ext(NULL, path.c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0){
            result = true;
        }
        git_libgit2_shutdown();
        return result;
    }

    bool InitMemStick(){
		std::string memstickDir = GetSysDirectory(DIRECTORY_MEMSTICK_ROOT).ToString();
		std::string repoURL = generateRepoURL();
		if (g_Config.sMemstickGitRepo.empty() || g_Config.sMemstickGitUsername.empty() || g_Config.sMemstickGitPersonalToken.empty()) {
			ERROR_LOG(Log::FileSystem, "Repo URL, Name, Git Username might be empty.");
			return false;
		}

        if(isRepo(memstickDir)){
			ERROR_LOG(Log::FileSystem, "The memstick is a git repo. You might want to Sync.");
            return false;
        }

		//check if there is an existing PSP folder, delete it if it exists. well, it exists most of the time.
		//TODO: preserve ppsspp.ini, should not be deleted deleted :)_
		fs::path pspMemStickPath = std::string(memstickDir + "/PSP/");
		if (fs::is_directory(pspMemStickPath)) {
			INFO_LOG(Log::FileSystem, "A directory, delete this");
			fs::remove_all(pspMemStickPath);
			INFO_LOG(Log::FileSystem, "PSP Folder Deleted.");
		}

        git_libgit2_init();
        git_clone_options clone_opts  = GIT_CLONE_OPTIONS_INIT;
        clone_opts.fetch_opts = GIT_FETCH_OPTIONS_INIT;
        clone_opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        git_repository* out;
        int clone_error = git_clone(&out, repoURL.c_str(), memstickDir.c_str(), &clone_opts);
        if(clone_error < 0){
            ERROR_LOG(Log::FileSystem, "Cannot Proceeed to Git Fetch:  %s", git_error_last()->message);
        }else{
            ERROR_LOG(Log::FileSystem, "Git Clone Success");
			return true;
        }

        git_repository_free(out);
        git_libgit2_shutdown();
        return false;
    }

	//this is like git pull, ONLY PULLS FROM THE REMOTE
	bool SyncMemStick() {
		std::string memstickDir = GetSysDirectory(DIRECTORY_MEMSTICK_ROOT).ToString();

		if (!isRepo(memstickDir)) {
			return false;
		}

		git_libgit2_init();
		git_repository* repo = NULL; //git repo
		git_remote* remote = NULL; //git remote
		INFO_LOG(Log::FileSystem, "STARTING SYNC...");

		INFO_LOG(Log::FileSystem, "OPENING REPOSITORY...");
		int error = git_repository_open(&repo, memstickDir.c_str()); //now we opened this, we can now initiate the remote

		if (error != 0) {
			ERROR_LOG(Log::FileSystem, "Error opening repo : %s", git_error_last()->message);
			return false;
		}
		error = git_remote_lookup(&remote, repo, "origin"); //we get the remote origin

		if (error != 0) {
			ERROR_LOG(Log::FileSystem, "Error remote lookup: %s", git_error_last()->message);
			return false;
		}

		INFO_LOG(Log::FileSystem, "FETCHING FROM REMOTE...");
		git_fetch_options fetch_options = GIT_FETCH_OPTIONS_INIT;
		error = git_remote_fetch(remote, NULL, &fetch_options, NULL); //we fetch the remote origin branch
		if (error != 0) {
			ERROR_LOG(Log::FileSystem, "Error remote fetch: %s", git_error_last()->message);
			return false;
		}

		git_oid remote_oid;
		git_annotated_commit* annotated_commit;
		INFO_LOG(Log::FileSystem, "GET REFERENCE NAME TO ID...");
		error = git_reference_name_to_id(&remote_oid, repo, "refs/remotes/origin/main");
		if (error != 0) {
			ERROR_LOG(Log::FileSystem, "Error ref name to id : %s", git_error_last()->message);
			return false;
		}

		INFO_LOG(Log::FileSystem, "GET ANNOTATED COMMIT FROM FETCHHEAD...");
		error = git_annotated_commit_from_fetchhead(&annotated_commit, repo, "refs/remotes/origin/main", git_remote_url(remote), &remote_oid);
		if (error != 0) {
			ERROR_LOG(Log::FileSystem, "Error commit from fetch head: %s", git_error_last()->message);
			return false;
		}

		git_merge_analysis_t merge_analysis;
		git_merge_preference_t preference;
		error = git_merge_analysis(&merge_analysis, &preference, repo, (const git_annotated_commit**)&annotated_commit, 1);
		if (error != 0) {
			ERROR_LOG(Log::FileSystem, "Error merge analysis: %s", git_error_last()->message);
			return false;
		}

		if (merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
			WARN_LOG(Log::FileSystem, "ALREADY UP TO DATE");
		}
		else{
			if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
				//fast forawrd
				INFO_LOG(Log::FileSystem, "CAN FAST FORWARD. FAST FORWARDING...");

				if (!FastForwardMerge(repo, annotated_commit)) {
					throw std::runtime_error("Fast-forward merge failed");
				}
				INFO_LOG(Log::FileSystem, "Fast-forward complete!");
			}
			else {
				//last resort, merge and commit
				INFO_LOG(Log::FileSystem, "NOT UP TO DATE, UPDATING...");
				git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;
				git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;

				INFO_LOG(Log::FileSystem, "MERGING...");
				error = git_merge(repo, (const git_annotated_commit**)&annotated_commit, 1, &merge_options, &checkout_options);
				if (error != 0) {
					ERROR_LOG(Log::FileSystem, "Error git merge: %s", git_error_last()->message);
					return false;
				}

				INFO_LOG(Log::FileSystem, "COMMITING MERGE...");
				//commit
			}
		}
		INFO_LOG(Log::FileSystem, "SUCCESS!...");

		// Cleanup
		if (annotated_commit) git_annotated_commit_free(annotated_commit);
		if (remote) git_remote_free(remote);
		if (repo) {
			git_repository_state_cleanup(repo);
			git_repository_free(repo);
		}
		git_libgit2_shutdown();
		return true;
	}

	bool FastForwardMerge(git_repository* repo, const git_annotated_commit* annotated_commit) {
		git_commit* target_commit = nullptr;
		const git_oid* commit_id = git_annotated_commit_id(annotated_commit);

		if (git_commit_lookup(&target_commit, repo, commit_id) != 0) {
			ERROR_LOG(Log::FileSystem, "Error git commit lookup: %s", git_error_last()->message);
		}

		git_reference* head_ref = nullptr;
		if (git_repository_head(&head_ref, repo) != 0) {
			git_commit_free(target_commit);
			ERROR_LOG(Log::FileSystem, "Error getting current head reference: %s", git_error_last()->message);
		}

		const git_oid* target_oid = git_commit_id(target_commit);
		git_reference* new_head_ref = nullptr;
		int error = git_reference_set_target(&new_head_ref, head_ref, target_oid, "Fast-forward");


		git_commit_free(target_commit);
		git_reference_free(head_ref);
		if (new_head_ref) git_reference_free(new_head_ref);

		if (error != 0) {
			ERROR_LOG(Log::FileSystem, "Error setting ref head to the new ref head: %s", git_error_last()->message);
		}

		git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
		checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE | GIT_CHECKOUT_FORCE;

		if (git_checkout_head(repo, &checkout_opts) != 0) {
			ERROR_LOG(Log::FileSystem, "Error checkout: %s", git_error_last()->message);
		}

		ERROR_LOG(Log::FileSystem, "SUccess??: %s", git_error_last()->message);

		return true;
	};

	//commit every changes then push
	bool CommitMemStick() {
		std::string memStickDir = GetSysDirectory(DIRECTORY_MEMSTICK_ROOT).ToString();
		std::string authorName = g_Config.sMemstickGitAuthorName;
		std::string authorEmail = g_Config.sMemstickGitAuthorEmail;


		//always check if the dir is a repo
		if (!isRepo(memStickDir)) {
			return false;
		}

		git_libgit2_init();

		git_repository* repo = NULL;
		if (git_repository_init(&repo, memStickDir.c_str(), 0)) {
			ERROR_LOG(Log::FileSystem, "Error initializing repo: %s", git_error_last()->message);
			return false;
		}

		git_signature* author = NULL;
		git_signature* commiter = NULL;

		git_signature_new(&author, authorName.c_str(), authorEmail.c_str(), std::time(nullptr), 0);
		git_signature_new(&commiter, authorName.c_str(), authorEmail.c_str(), std::time(nullptr), 0);
		
		git_oid tree_id, parent_id, commit_id;

		git_tree* tree;
		git_commit* parent;
		git_index* index;

		/* Get the index and write it to a tree */
		git_repository_index(&index, repo);

		git_index_add_all(index, NULL, GIT_INDEX_ADD_DEFAULT, NULL, NULL);
		git_index_write(index);
		git_index_write_tree(&tree_id, index);
		git_tree_lookup(&tree, repo, &tree_id);

		git_reference_name_to_id(&parent_id, repo, "refs/heads/main");
		git_commit_lookup(&parent, repo, &parent_id);

		/* Do the commit */
		git_commit_create_v(
			&commit_id,
			repo,
			"refs/heads/main",     /* The commit will update the position of HEAD */
			author,
			commiter,
			NULL,       /* UTF-8 encoding */
			"Update SaveFile - MemStickGit",
			tree,       /* The tree from the index */
			1,          /* Only one parent */
			parent      /* No need to make a list with create_v */
		);
		git_libgit2_shutdown();
		return true;
	}

	bool PushMemStick() {

		std::string memStickDir = GetSysDirectory(DIRECTORY_MEMSTICK_ROOT).ToString();


		git_libgit2_init();

		git_repository* repo = NULL;
		if (git_repository_init(&repo, memStickDir.c_str(), 0)) {
			ERROR_LOG(Log::FileSystem, "Error initializing repo: %s", git_error_last()->message);
			return false;
		}

		git_remote* remote = NULL;
		git_push_options push_opts;
		git_strarray refspecs = { 0 };

		// Initialize push options
		if (git_push_options_init(&push_opts, GIT_PUSH_OPTIONS_VERSION) < 0) {
			ERROR_LOG(Log::FileSystem, "Failed to initialize push options");
			return -1;
		}

		// Open the remote
		if (git_remote_lookup(&remote, repo, "origin") < 0) {
			ERROR_LOG(Log::FileSystem, "Failed to lookup remote: %s", "origin");
			return -1;
		}

		// Prepare the refspec (push the current branch)
		const char* ref_to_push = "refs/heads/main";
		refspecs.count = 1;
		refspecs.strings = (char**)malloc(sizeof(char*));
		refspecs.strings[0] = strdup(ref_to_push);

		// Perform the push
		int result = git_remote_push(remote, &refspecs, &push_opts);

		// Cleanup
		if (refspecs.strings) {
			free(refspecs.strings[0]);
			free(refspecs.strings);
		}

		if (remote) git_remote_free(remote);

		if (result < 0) {
			const git_error* err = git_error_last();
			ERROR_LOG(Log::FileSystem, "Push failed: %s", err ? err->message : "Unknown error");
			return -1;
		}

		git_libgit2_shutdown();
		return true;
	}
}
