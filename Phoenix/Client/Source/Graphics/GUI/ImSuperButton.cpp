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

#include <Client/Graphics/GUI/ImSuperButton.hpp>

#include <imgui.h>
#include <imgui_internal.h>

using namespace ImGui;

ImButtonStateMap ImGui::SuperButtonBehavior(
	const ImRect& bb,
	ImGuiID id,
	bool* out_hovered,
	std::optional<ImButtonStateMap> out_held = std::nullopt,
	ImGuiButtonFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();

	if (flags & ImGuiButtonFlags_Disabled)
	{
		if (out_hovered) *out_hovered = false;
		if (out_held.has_value()) out_held->set(0);
		if (g.ActiveId == id) ClearActiveID();
		return ImGui::ImButtonStateMap(0);
	}

	// Default behavior requires click+release on same spot
	if ((flags & (ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnRelease | ImGuiButtonFlags_PressedOnDoubleClick)) == 0)
		flags |= ImGuiButtonFlags_PressedOnClickRelease;

	ImGuiWindow* backup_hovered_window = g.HoveredWindow;
	const bool flatten_hovered_children = (flags & ImGuiButtonFlags_FlattenChildren) && g.HoveredRootWindow == window;
	if (flatten_hovered_children)
		g.HoveredWindow = window;

#ifdef IMGUI_ENABLE_TEST_ENGINE
	if (id != 0 && window->DC.LastItemId != id)
		ImGuiTestEngineHook_ItemAdd(&g, bb, id);
#endif

	ImButtonStateMap pressed = 0;
	bool hovered = ItemHoverable(bb, id);

	// Drag source doesn't report as hovered
	if (hovered && g.DragDropActive && g.DragDropPayload.SourceId == id && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoDisableHover))
		hovered = false;

	// Special mode for Drag and Drop where holding button pressed for a long time while dragging another item triggers the button
	if (g.DragDropActive && (flags & ImGuiButtonFlags_PressedOnDragDropHold) && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
		if (IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		{
			hovered = true;
			SetHoveredID(id);
			if (CalcTypematicRepeatAmount(g.HoveredIdTimer + 0.0001f - g.IO.DeltaTime, g.HoveredIdTimer + 0.0001f, 0.70f, 0.00f))
			{
				pressed[0] = true;
				FocusWindow(window);
			}
		}

	if (flatten_hovered_children)
		g.HoveredWindow = backup_hovered_window;

	// AllowOverlap mode (rarely used) requires previous frame HoveredId to be null or to match. This allows using patterns where a later submitted widget overlaps a previous one.
	if (hovered && (flags & ImGuiButtonFlags_AllowItemOverlap) && (g.HoveredIdPreviousFrame != id && g.HoveredIdPreviousFrame != 0))
		hovered = false;

	// Mouse
	if (hovered)
	{
		if (!(flags & ImGuiButtonFlags_NoKeyModifiers) || (!g.IO.KeyCtrl && !g.IO.KeyShift && !g.IO.KeyAlt))
		{
			for (std::size_t e=0; e < 5; e++) {
				if ((flags & ImGuiButtonFlags_PressedOnClickRelease) && g.IO.MouseClicked[e])
				{
					SetActiveID(id, window);
					if (!(flags & ImGuiButtonFlags_NoNavFocus))
						SetFocusID(id, window);
					FocusWindow(window);
				}
				if (((flags & ImGuiButtonFlags_PressedOnClick) && g.IO.MouseClicked[e]) || ((flags & ImGuiButtonFlags_PressedOnDoubleClick) && g.IO.MouseDoubleClicked[e]))
				{
					pressed[e] = true;
					if (flags & ImGuiButtonFlags_NoHoldingActiveID)
						ClearActiveID();
					else
						SetActiveID(id, window); // Hold on ID
					FocusWindow(window);
				}
				if ((flags & ImGuiButtonFlags_PressedOnRelease) && g.IO.MouseReleased[e])
				{
					if (!((flags & ImGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[e] >= g.IO.KeyRepeatDelay))  // Repeat mode trumps <on release>
						pressed[e] = true;
					ClearActiveID();
				}

				// 'Repeat' mode acts when held regardless of _PressedOn flags (see table above).
				// Relies on repeat logic of IsMouseClicked() but we may as well do it ourselves if we end up exposing finer RepeatDelay/RepeatRate settings.
				if ((flags & ImGuiButtonFlags_Repeat) && g.ActiveId == id && g.IO.MouseDownDuration[e] > 0.0f && IsMouseClicked(e, true))
					pressed[e] = true;
			}
		}

		if (pressed.any()) // auto cast to bool 1 == true
			g.NavDisableHighlight = true;
	}

	// Gamepad/Keyboard navigation
	// We report navigated item as hovered but we don't set g.HoveredId to not interfere with mouse.
	if (g.NavId == id && !g.NavDisableHighlight && g.NavDisableMouseHover && (g.ActiveId == 0 || g.ActiveId == id || g.ActiveId == window->MoveId))
		if (!(flags & ImGuiButtonFlags_NoHoveredOnNav))
			hovered = true;

	if (g.NavActivateDownId == id)
	{
		bool nav_activated_by_code = (g.NavActivateId == id);
		bool nav_activated_by_inputs = IsNavInputTest(ImGuiNavInput_Activate, (flags & ImGuiButtonFlags_Repeat) ? ImGuiInputReadMode_Repeat : ImGuiInputReadMode_Pressed);
		if (nav_activated_by_code || nav_activated_by_inputs)
			pressed = true;
		if (nav_activated_by_code || nav_activated_by_inputs || g.ActiveId == id)
		{
			// Set active id so it can be queried by user via IsItemActive(), equivalent of holding the mouse button.
			g.NavActivateId = id; // This is so SetActiveId assign a Nav source
			SetActiveID(id, window);
			if ((nav_activated_by_code || nav_activated_by_inputs) && !(flags & ImGuiButtonFlags_NoNavFocus))
				SetFocusID(id, window);
		}
	}

	ImButtonStateMap held = 0;
	for (std::size_t e; e < 5; e++) {
		if (g.ActiveId == id)
		{
			if (pressed.any())
				g.ActiveIdHasBeenPressedBefore = true;
			if (g.ActiveIdSource == ImGuiInputSource_Mouse)
			{
				if (g.ActiveIdIsJustActivated)
					g.ActiveIdClickOffset = g.IO.MousePos - bb.Min;

				if (g.IO.MouseDown[e])
				{
					held[e] = true;
				}
				else
				{
					if (hovered && (flags & ImGuiButtonFlags_PressedOnClickRelease) && !g.DragDropActive)
					{
						bool is_double_click_release = (flags & ImGuiButtonFlags_PressedOnDoubleClick) && g.IO.MouseDownWasDoubleClick[e];
						bool is_repeating_already = (flags & ImGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[e] >= g.IO.KeyRepeatDelay; // Repeat mode trumps <on release>
						if (!is_double_click_release && !is_repeating_already)
							pressed[e] = true;
					}
					ClearActiveID();
				}

				if (!(flags & ImGuiButtonFlags_NoNavFocus))
					g.NavDisableHighlight = true;
			}
			else if (g.ActiveIdSource == ImGuiInputSource_Nav)
			{
				if (g.NavActivateDownId != id)
					ClearActiveID();
			}
		}
	}

	if (out_hovered) *out_hovered = hovered;
	if (out_held.has_value()) out_held = held;

	return pressed;
}

ImButtonStateMap ImGui::SuperButtonEx(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return ImGui::ImButtonStateMap(0);

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered;
	ImButtonStateMap held = 0;
	ImButtonStateMap pressed = SuperButtonBehavior(bb, id, &hovered, held, flags);

	// Render
	const ImU32 col = GetColorU32((held.any() && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
	return pressed;
}

ImButtonStateMap ImGui::SuperButton(const char* label, const ImVec2& size_arg)
{
	return SuperButtonEx(label, size_arg, 0);
}

// Small buttons fits within text without additional vertical spacing.
ImButtonStateMap ImGui::SmallSuperButton(const char* label)
{
	ImGuiContext& g = *GImGui;
	float backup_padding_y = g.Style.FramePadding.y;
	g.Style.FramePadding.y = 0.0f;
	ImButtonStateMap pressed = SuperButtonEx(label, ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine);
	g.Style.FramePadding.y = backup_padding_y;
	return pressed;
}

// Tip: use ImGui::PushID()/PopID() to push indices or pointers in the ID stack.
// Then you can keep 'str_id' empty or the same for all your buttons (instead of creating a string based on a non-string id)
ImButtonStateMap ImGui::InvisibleSuperButton(const char* str_id, const ImVec2& size_arg)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	// Cannot use zero-size for InvisibleButton(). Unlike Button() there is not way to fallback using the label size.
	IM_ASSERT(size_arg.x != 0.0f && size_arg.y != 0.0f);

	const ImGuiID id = window->GetID(str_id);
	ImVec2 size = CalcItemSize(size_arg, 0.0f, 0.0f);
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	ItemSize(size);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered;
	ImButtonStateMap held = 0;
	ImButtonStateMap pressed = SuperButtonBehavior(bb, id, &hovered, held);

	return pressed;
}

ImButtonStateMap ImGui::ArrowSuperButtonEx(const char* str_id, ImGuiDir dir, ImVec2 size, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiID id = window->GetID(str_id);
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	const float default_size = GetFrameHeight();
	ItemSize(size, (size.y >= default_size) ? g.Style.FramePadding.y : -1.0f);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;

	bool hovered;
	ImButtonStateMap held = 0;
	ImButtonStateMap pressed = SuperButtonBehavior(bb, id, &hovered, held, flags);

	// Render
	const ImU32 bg_col = GetColorU32((held.any() && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	const ImU32 text_col = GetColorU32(ImGuiCol_Text);
	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, bg_col, true, g.Style.FrameRounding);
	RenderArrow(window->DrawList, bb.Min + ImVec2(ImMax(0.0f, (size.x - g.FontSize) * 0.5f), ImMax(0.0f, (size.y - g.FontSize) * 0.5f)), text_col, dir);

	return pressed;
}

ImButtonStateMap ImGui::ArrowSuperButton(const char* str_id, ImGuiDir dir)
{
	float sz = GetFrameHeight();
	return ArrowSuperButtonEx(str_id, dir, ImVec2(sz, sz), 0);
}
