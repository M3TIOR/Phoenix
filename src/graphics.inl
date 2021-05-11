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

#pragma once

#define SDL_MAIN_HANDLED // Needed to "keep main unmodified."
#include <SDL.h>

namespace phx::graphics {
	inline int init(SDL_Window* window, SDL_GLContext context, int width, int height)
	{
		SDL_SetMainReady(); // because we've defined SDL_MAIN_HANDLED, we want SDL
												// to know our main function is perfectly capable.
												// Capable of what exactly?
		SDL_Init(SDL_INIT_EVERYTHING);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
												SDL_GL_CONTEXT_PROFILE_CORE);

		#ifdef ENGINE_DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		#endif

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

		// The window name can be reset later when the user chooses to use
		// either the server or client.
		window = SDL_CreateWindow("Phoenix", SDL_WINDOWPOS_CENTERED,
															SDL_WINDOWPOS_CENTERED, width, height,
															SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (window == nullptr)
		{
			SDL_Quit();
			// TODO: Use internal logging system.
			std::cout << "[ERROR1] Couldn't create window, need OpenGL >= 3.3"
								<< std::endl;
			exit(EXIT_FAILURE);
		}

		context = SDL_GL_CreateContext(window);
		SDL_GL_MakeCurrent(window, context);

		if (!gladLoadGLLoader(static_cast<GLADloadproc>(SDL_GL_GetProcAddress)))
		{
			// TODO: use internal logging mechanism.
			std::cout << "[ERROR2] Couldn't create window, need OpenGL >= 3.3"
								<< std::endl;
			exit(EXIT_FAILURE);
		}

		#ifdef ENGINE_DEBUG
			GLint flags;
			glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
			if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
			{
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				glDebugMessageCallback(glDebugOutput, nullptr);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
				                      nullptr, GL_TRUE);
			}
		#endif

			SDL_ShowWindow(window);

			GLCheck(glEnable(GL_DEPTH_TEST));
			GLCheck(glEnable(GL_MULTISAMPLE));
			GLCheck(glEnable(GL_BLEND));
			GLCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			GLCheck(glEnable(GL_CULL_FACE));
			GLCheck(glCullFace(GL_BACK));
			GLCheck(glFrontFace(GL_CW));

			SDL_SetRelativeMouseMode(SDL_FALSE);

			glViewport(0, 0, width, height);

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& imguiIO = ImGui::GetIO();
			imguiIO.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

			ImGui_ImplSDL2_InitForOpenGL(window, context);
			ImGui_ImplOpenGL3_Init("#version 330 core");
	}

	inline void start_frame(SDL_Window* window) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.52f, 0.64f, 0.85f, 1.0f);
	}

	inline void swapBuffers(SDL_Window* window) const { SDL_GL_SwapWindow(window); }
	inline void show(SLD_Window* window) const { SDL_ShowWindow(window); }
	inline void hide(SLD_Window* window) const { SDL_HideWindow(window); }
	inline void maximize(SLD_Window* window) const { SDL_MaximizeWindow(window); }
	inline void minimize(SLD_Window* window) const { SDL_MinimizeWindow(window); }
	inline void focus(SLD_Window* window) const { SDL_SetWindowInputFocus(window); }

	inline void end_frame(){
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		swapBuffers();
		pollEvents();
	}

	inline close(SDL_Window* window, SDL_GLContext context)
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);

		// NOTE: Tears down SDL device handlers. May want to used std::atexit
		//       in the future instead of having this directly before exit.
		SDL_Quit();
	}
};
