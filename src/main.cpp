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

// #include <Common/Logger.hpp>

/* External Includes */
#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>

#include <SDL.h>

/* Internal Includes */
#include "core-intrinsics.hpp"
#include "util.hpp"

/* Standard Includes */
#include <filesystem>
#include <iostream>
#include <cstdlib> // EXIT_FAILURE & EXIT_SUCCESS
#include <string>
#include <future> // Async
#include <set>

using namespace phx;
using fs = std::filesystem; // Similar to other languages; polyglot helper.
using json = nlohmann::json; // Similar to other languages; polyglot helper.

static const char** program_namespace = ["GentenStudios", "Phoenix"];


// Why is this here?
#undef main

int main(int argc, char** argv)
{
	argparse::ArgumentParser argparser("phoenix");
	std::set<fs::path> cli_mod_list;
	auto roaming_store = util::datastore::roaming(program_namespace);
	//auto local_store = util::datastore::local(program_namespace);
	//auto cache_store = util::datastore::cache(program_namespace);

	// NOTE: Both --save and --add-module should be optional to effect loading
	//       when skipping the default user landing. If either are specified,
	//       it should skip the initial landing.
	//
	// TODO: If this rewrite is adopted, then deliberate on whether this should
	//       be a path or not. My opinion is yes.
	argparser.add_argument("-s", "--save")
		.help("Name of the save slot to use when skipping the initial landing.");

	argparser.add_argument("-m", "--add-module")
		.help("Adds modules to be loaded by the engine on startup.")
		.action([](const std::string& value) {
			// TODO: validate this path actually contains a mod, otherwise error.
			cli_mod_list.add(value);
		});


	// TODO: Find out what this does, and if it's like the two above or not.
	argparser.add_argument("-c", "--config")
		// TODO: add default path.
		.help("Specify a path to config file to use.")
		.default(roaming_store / "config.json")
		.action([](const std::string& path) {
			if (! fs::exists(path)) {
				// TODO: throw error
			}

		});

	argparser.add_argument("--launch-mode")
		.help(
			// NOTE: This formatting should be auto converted by the compiler to
			//       be cross platform. No need to sanitize CRLF on windows.
			//       Also, without commas, these strings should natively auto concat
			"Lauches the game in specified mode. When unset, let the user decide\n"
			"from the game menu. The modes available are as follows:\n"
			"\thybrid - Single player games.\n"
			"\tclient - Connect to a server.\n"
			"\tserver - Host a server with GUI.\n"
			"\tserver-nogui - Host a server with TUI.\n"
			"\thybrid-open - Play and host from the same instance.\n"
		)
		.action([](const std::string& value) {
			if ( value == "client" || value == "server" || value == "hybrid"
				 || value == "server-nogui" || value == "hybrid-open")
			{
				return value;
			}
			else {
				// TODO log error.
				std::exit(EXIT_FAILURE);
			}
		})

	// TODO: get these working...
	//
	// argparser.add_argument("-4")
	// 	.help("Specify an IPv4 address to connect to.")
	//
	// argparser.add_argument("-6")
	// 	.help("Specify an IPv6 address to connect to.")
	//
	// argparser.add_argument("-V", "--verbose")
	// 	.help("Prints the program version.");

	try {
		argparser.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << argparser;
		// TODO: on linux systems we can actually pass a `u8int` to help
		//       external scripts. This is an engine after all.
		//           https://tldp.org/LDP/abs/html/exitcodes.html
		//           https://man7.org/linux/man-pages/man7/signal.7.html
		//       Should be similar for Unix / Darwin as well. Windows
		//       won't be able to use this. They have unique error codes
		//       in the tens of thousands, but that's only for internal
		//       systems.
		//
		// NOTE: use EXIT_... over 0 or 1, they're implementation dependent.
		std::exit(EXIT_FAILURE);
	}



	/*##########################################################################*/

	if (argparser["--launch-mode"] == "server-nogui") {
		std::exit(EXIT_SUCCESS); // TODO: make this work.
	}

	/*##########################################################################*/
	// NOTE: I know this is going to cluster the fuck out of main.cpp, but
	//       we're only ever going to need one window, and event propagation
	//       is actually a very slow and time consuming process.
	SDL_Window* window = nullptr;
	SDL_GLContext context;
	int status;
	// TODO: Use cached values or user settings / config.
	int width = 1280;
	int height = 720;
	math::vec2 cachedScreenSize = {
		static_cast<float>(width),
		static_cast<float>(height)
	};


	status = phx::graphics::init(window, context);
	if (status == EXIT_FAILURE) {
		phx::graphics::close(window, context);
		std::exit(status);
	}

	if (argparser["--server"]) {
		// TODO: implement this. Parse -4 and -6 here too.
		std::exit(EXIT_SUCCESS)
	}
	else {
		if (argparser.is_used("-4"))


	}

	phx::graphics::close(window, context);
	std::exit(EXIT_SUCCESS);
}
