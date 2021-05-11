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

#include <Client/Graphics/OpenGLTools.hpp>
#include <Client/Graphics/Window.hpp>

#include <Common/CoreIntrinsics.hpp>

#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include <algorithm>
#include <iostream>

using namespace phx;
using namespace gfx;

Window::Window(const std::string& title, int width, int height)
{

}

Window::~Window()
{

}

void Window::pollEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) > 0)
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		ImGuiIO& io = ImGui::GetIO();

		using namespace events;
		Event e;

		switch (event.type)
		{
		case SDL_QUIT:
			e.type = EventType::WINDOW_CLOSED;
			dispatchToListeners(e);
			m_running = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (io.WantCaptureMouse)
				break;
			e.type         = EventType::MOUSE_BUTTON_PRESSED;
			e.mouse.button = static_cast<MouseButtons>(event.button.button);
			e.mouse.x      = event.button.x;
			e.mouse.y      = event.button.y;
			e.mouse.mods   = static_cast<Mods>(SDL_GetModState());
			dispatchToListeners(e);
			break;
		case SDL_MOUSEBUTTONUP:
			if (io.WantCaptureMouse)
				break;
			e.type         = EventType::MOUSE_BUTTON_RELEASED;
			e.mouse.button = static_cast<MouseButtons>(event.button.button);
			e.mouse.mods   = static_cast<Mods>(SDL_GetModState());
			dispatchToListeners(e);
			break;
		case SDL_MOUSEMOTION:
			if (io.WantCaptureMouse)
				break;
			e.type       = EventType::CURSOR_MOVED;
			e.position.x = event.motion.x;
			e.position.y = event.motion.y;
			dispatchToListeners(e);
			break;
		case SDL_KEYDOWN:
			if (io.WantCaptureKeyboard)
			{
				break;
			}
			e.type          = EventType::KEY_PRESSED;
			e.keyboard.key  = static_cast<Keys>(event.key.keysym.scancode);
			e.keyboard.mods = static_cast<Mods>(
			    event.key.keysym.mod); // access these with bitwise operators
			                           // like AND (&) and OR (|)
			dispatchToListeners(e);
			break;
		case SDL_KEYUP:
			if (io.WantCaptureKeyboard)
			{
				break;
			}
			e.type          = EventType::KEY_RELEASED;
			e.keyboard.key  = static_cast<Keys>(event.key.keysym.scancode);
			e.keyboard.mods = static_cast<Mods>(
			    event.key.keysym.mod); // access these with bitwise operators
			                           // like AND (&) and OR (|)
			dispatchToListeners(e);
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
				//			case SDL_WINDOWEVENT_RESIZED: <- we don't want this
				//-> https://wiki.libsdl.org/SDL_WindowEventID
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				e.type        = EventType::WINDOW_RESIZED;
				e.size.width  = event.window.data1;
				e.size.height = event.window.data2;
				glViewport(0, 0, e.size.width, e.size.height);
				dispatchToListeners(e);
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				e.type = EventType::WINDOW_FOCUSED;
				dispatchToListeners(e);
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				e.type = EventType::WINDOW_DEFOCUSED;
				dispatchToListeners(e);
				break;
			case SDL_WINDOWEVENT_CLOSE:
				e.type = EventType::WINDOW_CLOSED;
				dispatchToListeners(e);
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				e.type = EventType::WINDOW_MINIMIZED;
				dispatchToListeners(e);
				break;
			case SDL_WINDOWEVENT_MAXIMIZED:
				e.type = EventType::WINDOW_MAXIMIZED;
				dispatchToListeners(e);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				e.type = EventType::WINDOW_RESTORED;
				dispatchToListeners(e);
				break;
			case SDL_WINDOWEVENT_LEAVE:
				e.type = EventType::CURSOR_LEFT;
				dispatchToListeners(e);
				break;
			case SDL_WINDOWEVENT_ENTER:
				e.type = EventType::CURSOR_ENTERED;
				dispatchToListeners(e);
				break;
			case SDL_WINDOWEVENT_MOVED:
				e.type       = EventType::WINDOW_MOVED;
				e.position.x = event.window.data1;
				e.position.y = event.window.data2;
				dispatchToListeners(e);
				break;
			default:
				break;
			}
		default:
			break;
		}
	}
}

void Window::swapBuffers() const {  }
bool Window::isRunning() const { return m_running; }

void Window::startFrame()
{

}

void Window::endFrame()
{

}


void Window::close() { m_running = false; }

void Window::resize(math::vec2 size)
{
	SDL_SetWindowSize(m_window, static_cast<int>(size.x),
	                  static_cast<int>(size.y));
}

math::vec2 Window::getSize() const
{
	math::vec2i size;
	SDL_GetWindowSize(m_window, &size.x, &size.y);

	return size;
}

void Window::setCursorPosition(math::vec2i pos)
{
	SDL_WarpMouseInWindow(m_window, pos.x, pos.y);
}

math::vec2i Window::getCursorPosition() const
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	return {static_cast<float>(x), static_cast<float>(y)};
}

void Window::dispatchToListeners(events::Event& event)
{
	for (events::IEventListener* eventListener : m_eventListeners)
	{
		eventListener->onEvent(event);
	}
}

void Window::setResizable(bool enabled)
{
	SDL_SetWindowResizable(m_window, enabled ? SDL_TRUE : SDL_FALSE);
}

void Window::setVSync(bool enabled)
{
	m_vsync = enabled;
	SDL_GL_SetSwapInterval(m_vsync ? 1 : 0);
}

bool Window::isVSync() const { return m_vsync; }
void Window::setTitle(const std::string& title) const {}

void Window::setFullscreen(bool enabled)
{
	m_fullscreen = enabled;

	if (enabled)
	{
		SDL_DisplayMode current;
		const int       check = SDL_GetCurrentDisplayMode(0, &current);

		if (check != 0)
		{
			std::cout << "[ERROR][WINDOW] In \"q2::Window::setFullscreen(bool "
			             "enabled)\" - "
			          << SDL_GetError() << std::endl;
		}
		else
		{
			m_cachedScreenSize = getSize();
			resize(
			    {static_cast<float>(current.w), static_cast<float>(current.h)});
			SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);

			glViewport(0, 0, current.w, current.h);
		}
	}
	else
	{
		SDL_SetWindowFullscreen(m_window, 0);
		resize(m_cachedScreenSize);

		glViewport(0, 0, static_cast<int>(m_cachedScreenSize.x),
		           static_cast<int>(m_cachedScreenSize.y));
	}
}

bool Window::isFullscreen() const { return m_fullscreen; }

void Window::registerEventListener(events::IEventListener* listener)
{
	m_eventListeners.emplace_back(listener);
}

void Window::nullifyEventListener(events::IEventListener* listener)
{
	const auto iterPos =
	    std::find(m_eventListeners.begin(), m_eventListeners.end(), listener);

	if (iterPos == m_eventListeners.end())
	{
		std::cout
		    << "[WARNING] In "
		       "\"Window::nullifyEventListener(events::IEventListener* "
		       "listener)\" - An event listener that has not been registered "
		       "has requested to be removed from a window - please check."
		    << std::endl;
		return;
	}

	m_eventListeners.erase(iterPos);
}

void Window::dispatchCustomEvent(events::Event e) { dispatchToListeners(e); }

bool Window::isKeyDown(events::Keys key) const
{
	return SDL_GetKeyboardState(nullptr)[static_cast<SDL_Scancode>(key)];
}

void Window::setCursorState(gfx::CursorState state)
{
	const bool on = (state == gfx::CursorState::NORMAL);
	SDL_ShowCursor(on);
}
