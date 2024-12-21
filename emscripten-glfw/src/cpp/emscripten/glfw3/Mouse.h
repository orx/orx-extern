/*
 * Copyright (c) 2023 pongasoft
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 *
 * @author Yan Pujante
 */

#ifndef EMSCRIPTEN_GLFW_MOUSE_H
#define EMSCRIPTEN_GLFW_MOUSE_H

#include <GLFW/glfw3.h>
#include <array>
#include "Types.h"
#include "Cursor.h"

using glfw_mouse_button_state_t = int; // ex: GLFW_RELEASE
using glfw_mouse_button_t = int; // ex: GLFW_MOUSE_BUTTON_LEFT
using glfw_cursor_mode_t = int; // ex: GLFW_CURSOR_NORMAL

namespace emscripten::glfw3 {

class Window;


class Mouse
{
public:

  constexpr bool isPointerLock() const { return fCursorMode == GLFW_CURSOR_DISABLED; }

  inline std::shared_ptr<Cursor> hideCursor() {
    fVisibleCursor = fCursor;
    fCursor = StandardCursor::getHiddenCursor();
    return fCursor;
  }

  inline std::shared_ptr<Cursor> showCursor() {
    fCursor = fVisibleCursor;
    return fCursor;
  }

  inline bool isCursorHidden() const { return fCursor == StandardCursor::getHiddenCursor(); }

  friend class Window;

public:
  glfw_mouse_button_state_t fLastButtonState{GLFW_RELEASE};
  glfw_mouse_button_t fLastButton{-1};
  std::array<glfw_mouse_button_state_t, GLFW_MOUSE_BUTTON_LAST + 1> fButtonStates{GLFW_RELEASE};

  glfw_cursor_mode_t fCursorMode{GLFW_CURSOR_NORMAL};
  glfw_bool_t fStickyMouseButtons{GLFW_FALSE};

  Vec2<double> fCursorPos{};
  Vec2<double> fCursorPosBeforePointerLock{};
  Vec2<double> fCursorLockResidual{};

  GLFWmousebuttonfun fButtonCallback{};
  GLFWscrollfun fScrollCallback{};
  GLFWcursorenterfun fCursorEnterCallback{};
  GLFWcursorposfun fCursorPosCallback{};

private:
  static constexpr glfw_mouse_button_state_t kStickyPress = 3;

private:
  std::shared_ptr<Cursor> fCursor{StandardCursor::getDefault()};
  std::shared_ptr<Cursor> fVisibleCursor{fCursor};
};

}

#endif //EMSCRIPTEN_GLFW_MOUSE_H
