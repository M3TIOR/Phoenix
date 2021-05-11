// Copyright 2019-20 Genten Studios
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// A slightly modified version of my personal datastore library,
// for use in the Phoenix project, with all copyright and royalties waived
// in absence of the license I was supposed to have written for myself by now.
// https://softwareengineering.stackexchange.com/questions/84071
/**
 * @file datastore.cpp
 * @brief - A loose reimplementation of https://github.com/m3tior/nodejs-datastore
 *
 * @description - Implements functions to aid in cross system compatability.
 *   Specifically, this file primarily implements accessors for datastores.
 *   In my research, I've concluded there are three essential datastore
 *   locations:
 *     * The temporary store holds replaceable, single use files for each
 *       instance of an application.
 *     * The roaming store holds portable application data (config, saves)
 *     * The local store holds static application data (auth tokens, etc.)
 *     * And the cache, which you can denote.
 *
 * @copyright Ruby Allison Rose (c) 2021-2022
 */

/* External Includes */
//...

/* Internal Includes */
#include "util.hpp"

/* Standard Includes */
#include <string_view>
#include <filesystem>
#include <cstdlib> // getenv
#include <string>
#include <random>
#include <set>

/* Special Headers */
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
#include <sys/types.h>
#include <unistd.h> // geteuid, sysconf, malloc, free
#include <errno.h> // (posix error codes.)
#include <pwd.h> // getpwuid
#elif defined(_WIN32) || defined(_WIN64)
#include <processthreadsapi.h> // GetCurrentProcessToken
#include <userenv.h> // GetUserProfileDirectoryA
#else
// May be worth introducing these supported systems if this library is
// broken out: https://sourceforge.net/p/predef/wiki/OperatingSystems/
#error "Couldn't recognize OS, inquire with the developers for support."
#endif


// Safer than `using namespace std::filesystem`
namespace fs = std::filesystem;


/* Constants */
static std::set<fs::path> = tracked_temporaries;
static bool is_tracking = false;


static void clean_temporaries(void) {
	for (auto dir : tracked_temporaries)
		fs::remove_all(dir);
}

// #ifdef _DEBUG
// static inline fs::path debug_store(const char** identity, const char* name) {
// 	std::string id = identity[0];
// 	auto id_fragments = sizeof(identity)/sizeof(identity[0]);
// 	for (auto l=1; l < id_fragments; l++) {
// 		id.push_back('.');
// 		id+=identity[l];
// 	}
// 	id.push_back('@');
// 	id+=name;
//
// 	// Doesn't autoclean, this way the contents can be examined by maintainers.
// 	fs::path p = temporary(id.c_str(), false);
// 	fs::create_directory(p);
// 	return p;
// }
// #endif

namespace phx::util {
	/**
	 * @brief - A loose reimplementation of *libuv's* `uv_os_homedir()` function.
	 * @description - On Windows, uses the C Standard `getenv` to check for the
	 *   `USERPROFILE` variable, which holds the mutable home path. Otherwise,
	 *   when unavailable, it uses the Windows internals `GetUserProfileDirectoryA`
	 *   function. On Posix systems, it checks the environment for the `HOME`
	 *   variable. When unavailble, uses `getpwuid_r` to fetch the path from
	 *   the system `/etc/passwd` database file.
	 * @return - The path to the current user's home directory.
	 */
	fs::path homedir() {
		#if defined(_WIN32) || defined(_WIN64)
		if(const char* home = std::getenv("USERPROFILE")) {
			return home;
		}
		else {
			// should be long enough to cover legacy window max path. It's a stack
			// buffer so it shouldn't impact performance.
			char* buff[1024];
			auto security_clearance = GetCurrentProcessToken();
			if (GetUserProfileDirectoryA(security_clearance, buff, 1024)) {
				// Should be 100% okay if buff is null terminated.
				// Implicit conversion initializer: char* -> string_view -> string;
				return buff;
			}
			else {
				// TODO: GetLastError() // handle a failure.
			}
		}
		#elif defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
		if (const char* home = std::getenv("HOME")) {
			return home;
		}
		else {
			passwd pw;
			passwd* result;
			// Static buffer to remove malloc call. 4kb should be enough unless
			// someone has a really wierd system configuration. This gives room
			// for 2048 length passwords for older systems that still use the
			// etc/passwd file to hold them or their hashes.
			// https://linuxize.com/post/etc-passwd-file/
			char buf[4096];
			if (int status = getpwuid_r(geteuid(), &pw, buf, 4096, &result)) {
				// NOTE: Can return NULL, but since we used `geteuid` to generate our
				//       UID, we guarantee this won't. So we only have to check
				//       when status is != 0 and handle that value.
				// TODO: Handle error codes.
				// errno = s;
				// perror("getpwnam_r");
			}

			return static_cast<char*>(posix_passwd->pw_dir);
		}
		#endif
	}
	namespace datastore {
		/**
		 * @brief - Generates a temporary data store for the application.
		 * @description - NOTE: This application uses the C Standard `std::atexit`
		 *   to register it's cleanup action, so keep in mind that you have 31
		 *   remaining `atexit` hooks left when using with `autoclean = true`.
		 * @param name_prefix - A prefix name for the datastore, so it's identifyable.
		 * @param autoclean - Whether or not the datastore should automatically be
		 *   disposed of after the application has closed.
		 * @return - The path to the new data store.
		 */
		// Using `std::atexit` shouldn't cause issues
		// https://www.learncpp.com/cpp-tutorial/halts-exiting-your-program-early/
		fs::path temporary(const char* name_prefix, bool autoclean = true) {
			// NOTE: Don't use internal temp file generation, it's inconsistent.
			std::random_device rd; //Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
			std::uniform_int_distribution<> distrib(97, 132);
			std::string dir_name = name_prefix;
			auto prefix_length = dir_name.length();

			name.resize(prefix_length + 11); // All we should need is one allocation.
			dir_name.insert(prefix_length += 1, 1, '-');

			fs::path temporaryDataStore;
			do {
				for (std::string::size_type l=0; l < 10; l++) {
					int randchar = distrib(gen);
					if (randchar > 122) randchar -= 75; // adjust for decimal coverage.
					dir_name.insert(prefix_length + l, 1, static_cast<char>(randchar));
				}
				temporaryDataStore = fs::temp_directory_path() / dir_name;
			} while (fs::exists(temporaryDataStore));

			fs::create_directory(temporaryDataStore);

			if (autoclean) {
				tracked_temporaries.insert(temporaryDataStore);

				if (!is_tracking) {
					// NOTE: may be able to do this better somehow.
					std::atexit(&clean_temporaries);
					is_tracking = true;
				}
			}

			return temporaryDataStore;
		}

		/**
		 * @brief - Generates or finds the roaming data store for the application.
		 * @param identity - A path that uniquely identifies the application.
		 *   It's recommended that the path at least contain two entries,
		 *     * The name of the author, group, or company which produced it,
		 *     * and the name of the product itself.
		 *   But you can use less or more, depending on your circumstances.
		 *   This identity will be used as a path to further nest the contents
		 *   of the datastore, especially helpful for companies that have more
		 *   than one product.
		 * @return - The path to the data store.
		 */
		fs::path roaming(const char** identity) {
			fs::path p;

			#if defined(_WIN32) || defined(_WIN64)
			if (const char* env_var = std::getenv("APPDATA"))
				p = env_var;
			else
				p = phx::util::homedir() / "AppData" / "Roaming";

			#elif defined(__APPLE__) || defined(__MACH__)
				p = phx::util::homedir() / "Library" / "Preferences";

			#elif defined(__linux__)
			// NOTE: Using XDG freedesktop.org standard
			// https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
			if (const char* env_var = std::getenv("XDG_CONFIG_HOME"))
				p = env_var;
			else
				p = phx::util::homedir() / ".config";
			#endif

			auto id_fragments = sizeof(identity)/sizeof(identity[0]);
			for (auto l=0; l < id_fragments; l++)
				p / identity[l];

			fs::create_directories(p);
			return p;
		}

		/**
		 * @brief - Generates or finds the local data store for the application.
		 * @param identity - A path that uniquely identifies the application.
		 *   It's recommended that the path at least contain two entries,
		 *     * The name of the author, group, or company which produced it,
		 *     * and the name of the product itself.
		 *   But you can use less or more, depending on your circumstances.
		 *   This identity will be used as a path to further nest the contents
		 *   of the datastore, especially helpful for companies that have more
		 *   than one product.
		 * @return - The path to the data store.
		 */
		fs::path local(const char** identity) {
			fs::path p;

			#if defined(_WIN32) || defined(_WIN64)
			if (const char* env_var = std::getenv("LOCALAPPDATA"))
				p = env_var;
			else
				p = phx::util::homedir() / "AppData" / "Local";

			#elif defined(__APPLE__) || defined(__MACH__)
				p = phx::util::homedir() / "Library" / "Application Support";

			#elif defined(__linux__)
			if (const char* env_var = std::getenv("XDG_CONFIG_HOME"))
				p = env_var;
			else
				p = phx::util::homedir() / ".local" / "share";
			#endif

			auto id_fragments = sizeof(identity)/sizeof(identity[0]);
			for (auto l=0; l < id_fragments; l++)
				p / identity[l];

			fs::create_directories(p);
			return p;
		}

		/**
		 * @brief - Generates or finds the cache for the application.
		 * @param identity - A path that uniquely identifies the application.
		 *   It's recommended that the path at least contain two entries,
		 *     * The name of the author, group, or company which produced it,
		 *     * and the name of the product itself.
		 *   But you can use less or more, depending on your circumstances.
		 *   This identity will be used as a path to further nest the contents
		 *   of the datastore, especially helpful for companies that have more
		 *   than one product.
		 *
		 *   NOTE: Windows doesn't have a dedicated cache, so it's nested
		 *         within the `LOCALAPPDATA` folder.
		 * @return - The path to the data store.
		 */
		fs::path cache(const char** identity) {
			fs::path p;

			#if defined(_WIN32) || defined(_WIN64)
			if (const char* env_var = std::getenv("LOCALAPPDATA")) {
				p = env_var; p /= "Cache";
			}
			else {
				p = phx::util::homedir() / "AppData" / "Local" / "Cache";
			}

			#elif defined(__APPLE__) || defined(__MACH__)
				p = phx::util::homedir() / "Library" / "Caches";

			#elif defined(__linux__)
			if (const char* env_var = std::getenv("XDG_CACHE_HOME"))
				p = env_var;
			else
				p = phx::util::homedir() / ".cache";
			#endif

			auto id_fragments = sizeof(identity)/sizeof(identity[0]);
			for (auto l=0; l < id_fragments; l++)
				p / identity[l];

			fs::create_directories(p);
			return p;
		}
	};
};
