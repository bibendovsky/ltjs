#include <cassert>
#include <cstddef>
#include <cstdint>
#include <array>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include "glad.h"
#include "imgui.h"
#include "SDL.h"


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Classes
//

class ImageCache final
{
public:
	ImageCache(
		ImageCache&& rhs);

	static ImageCache& get_instance();


	void initialize();

	void uninitialize();

	bool set_font(
		const std::string& file_name,
		const float font_size_pixels);

	ImFontAtlas& get_font_atlas();


private:
	using ImFontAtlasUPtr = std::unique_ptr<ImFontAtlas>;


	ImageCache();

	~ImageCache();


	ImFontAtlasUPtr font_atlas_uptr_;
}; // ImageCache


class Clipboard final
{
public:
	Clipboard(
		Clipboard&& rhs);

	static Clipboard& get_instance();

	void initialize();

	void uninitialize();

	void initialize_im_context(
		ImGuiIO& im_io);


private:
	char* sdl_clipboard_text_;


	Clipboard();

	~Clipboard();


	const char* get_clipboard_text();

	void set_clipboard_text(
		const char* text);

	static const char* get_clipboard_text_proxy(
		void* user_data);

	static void set_clipboard_text_proxy(
		void* user_data,
		const char* text);
}; // Clipboard


using SdlCursorPtr = SDL_Cursor*;

class SystemCursors final
{
public:
	SystemCursors(
		SystemCursors&& rhs);


	static SystemCursors& get_instance();

	void initialize();

	void uninitialize();


	SdlCursorPtr& operator[](
		const int index);

	const SdlCursorPtr operator[](
		const int index) const;


private:
	using Items = std::array<SdlCursorPtr, ImGuiMouseCursor_COUNT>;


	Items items_;


	SystemCursors();

	~SystemCursors();
}; // SystemCursors


class Window
{
public:
	Window(
		Window&& rhs);

	bool initialize(
		const std::string& title,
		const int width,
		const int height);

	bool is_initialized() const;

	const std::string& get_error_message() const;

	void draw();


protected:
	friend class WindowManager;


	bool is_initialized_;
	std::string error_message_;


	Window();

	Window(
		const Window& rhs) = delete;

	virtual ~Window();


	void uninitialize();

	void handle_event(
		const SDL_Event& sdl_event);


private:
	friend class WindowManager;

	using PressedMouseButtons = std::array<bool, 3>;

	SDL_Window* sdl_window_;
	SDL_GLContext sdl_gl_context_;
	Uint32 sdl_window_id_;
	GLuint ogl_font_atlas_;
	ImGuiContext* im_context_;
	PressedMouseButtons pressed_mouse_buttons_;
	Uint64 time_;


	Uint32 get_id() const;


	void im_new_frame();

	void im_update_mouse_position_and_buttons();

	void im_update_mouse_cursor();

	void im_render();

	void im_render_data(
		ImDrawData* draw_data);


	virtual void do_draw() = 0;
}; // Window

using WindowPtr = Window*;


class ImDemoWindow final :
	public Window
{
public:
	ImDemoWindow();

	ImDemoWindow(
		ImDemoWindow&& rhs);

	~ImDemoWindow() override;


private:
	bool show_demo_window_;
	bool show_another_window_;
	float f_;
	int counter_;


	void do_draw() override;
}; // ImDemoWindow


class WindowManager final
{
public:
	WindowManager(
		WindowManager&& rhs);


	static WindowManager& get_instance();

	void initialize();

	void uninitialize();

	void handle_events();

	void draw();

	bool is_quit_requested() const;


private:
	friend class Window;


	using WindowMap = std::unordered_map<Uint32, WindowPtr>;


	WindowMap window_map_;
	bool is_quit_requested_;


	WindowManager();

	~WindowManager();


	void register_window(
		const Uint32 window_id,
		WindowPtr window_ptr);

	void unregister_window(
		const Uint32 window_id);
}; // WindowManager


class Launcher final
{
public:
	Launcher();

	Launcher(
		Launcher&& rhs);

	~Launcher();


	bool initialize();

	void uninitialize();

	bool is_initialized();

	const std::string& get_error_message();

	void run();


private:
	bool is_initialized_;
	std::string error_message_;


	bool initialize_ogl_functions();

	void uninitialize_sdl();

	bool initialize_sdl();
}; // Launcher

//
// Classes
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ImageCache
//

ImageCache::ImageCache()
	:
	font_atlas_uptr_{std::make_unique<ImFontAtlas>()}
{
}

ImageCache::ImageCache(
	ImageCache&& rhs)
	:
	font_atlas_uptr_{std::move(rhs.font_atlas_uptr_)}
{
}

ImageCache::~ImageCache()
{
}

ImageCache& ImageCache::get_instance()
{
	static auto image_cache = ImageCache{};
	return image_cache;
}

void ImageCache::initialize()
{
	uninitialize();
}

void ImageCache::uninitialize()
{
	font_atlas_uptr_->Clear();
}

bool ImageCache::set_font(
	const std::string& file_name,
	const float font_size_pixels)
{
	font_atlas_uptr_->Clear();

	auto im_font_ptr = font_atlas_uptr_->AddFontFromFileTTF(file_name.c_str(), font_size_pixels);

	return im_font_ptr != nullptr;
}

ImFontAtlas& ImageCache::get_font_atlas()
{
	return *font_atlas_uptr_;
}

//
// ImageCache
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Clipboard
//

Clipboard::Clipboard()
	:
	sdl_clipboard_text_{}
{
}

Clipboard::Clipboard(
	Clipboard&& rhs)
	:
	sdl_clipboard_text_{std::move(rhs.sdl_clipboard_text_)}
{
	rhs.sdl_clipboard_text_ = nullptr;
}

Clipboard::~Clipboard()
{
	uninitialize();
}

Clipboard& Clipboard::get_instance()
{
	static auto clipboard = Clipboard{};
	return clipboard;
}

void Clipboard::initialize_im_context(
	ImGuiIO& im_io)
{
	im_io.ClipboardUserData = this;
	im_io.GetClipboardTextFn = get_clipboard_text_proxy;
	im_io.SetClipboardTextFn = set_clipboard_text_proxy;
}

void Clipboard::initialize()
{
	uninitialize();
}

void Clipboard::uninitialize()
{
	if (sdl_clipboard_text_)
	{
		::SDL_free(sdl_clipboard_text_);
		sdl_clipboard_text_ = nullptr;
	}
}

const char* Clipboard::get_clipboard_text()
{
	if (sdl_clipboard_text_)
	{
		::SDL_free(sdl_clipboard_text_);
		sdl_clipboard_text_ = nullptr;
	}

	sdl_clipboard_text_ = ::SDL_GetClipboardText();

	return sdl_clipboard_text_;
}

void Clipboard::set_clipboard_text(
	const char* text)
{
	if (!text)
	{
		return;
	}

	static_cast<void>(::SDL_SetClipboardText(text));
}

const char* Clipboard::get_clipboard_text_proxy(
	void* user_data)
{
	auto& instance = *static_cast<Clipboard*>(user_data);

	return instance.get_clipboard_text();
}

void Clipboard::set_clipboard_text_proxy(
	void* user_data,
	const char* text)
{
	auto& instance = *static_cast<Clipboard*>(user_data);

	return instance.set_clipboard_text(text);
}

//
// Clipboard
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SystemCursors
//

SystemCursors::SystemCursors()
	:
	items_{}
{
}

SystemCursors::SystemCursors(
	SystemCursors&& rhs)
	:
	items_{rhs.items_}
{
	rhs.items_ = {};
}

SystemCursors::~SystemCursors()
{
	uninitialize();
}

SystemCursors& SystemCursors::get_instance()
{
	static auto system_cursors = SystemCursors{};

	return system_cursors;
}

void SystemCursors::initialize()
{
	uninitialize();

	items_[ImGuiMouseCursor_Arrow] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	items_[ImGuiMouseCursor_TextInput] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	items_[ImGuiMouseCursor_ResizeAll] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	items_[ImGuiMouseCursor_ResizeNS] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	items_[ImGuiMouseCursor_ResizeEW] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	items_[ImGuiMouseCursor_ResizeNESW] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	items_[ImGuiMouseCursor_ResizeNWSE] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	items_[ImGuiMouseCursor_Hand] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
}

void SystemCursors::uninitialize()
{
	for (auto& sdl_cursor_ptr : items_)
	{
		if (sdl_cursor_ptr)
		{
			::SDL_FreeCursor(sdl_cursor_ptr);
			sdl_cursor_ptr = nullptr;
		}
	}
}

SdlCursorPtr& SystemCursors::operator[](
	const int index)
{
	return items_[index];
}

const SdlCursorPtr SystemCursors::operator[](
	const int index) const
{
	return items_[index];
}

//
// SystemCursors
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Window
//

Window::Window()
	:
	is_initialized_{},
	error_message_{},
	sdl_window_{},
	sdl_gl_context_{},
	sdl_window_id_{},
	ogl_font_atlas_{},
	im_context_{},
	pressed_mouse_buttons_{},
	time_{}
{
}

Window::Window(
	Window&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)},
	sdl_window_{std::move(rhs.sdl_window_)},
	sdl_gl_context_{std::move(rhs.sdl_gl_context_)},
	sdl_window_id_{std::move(rhs.sdl_window_id_)},
	ogl_font_atlas_{std::move(rhs.ogl_font_atlas_)},
	im_context_{std::move(rhs.im_context_)},
	pressed_mouse_buttons_{std::move(rhs.pressed_mouse_buttons_)},
	time_{std::move(rhs.time_)}
{
	rhs.is_initialized_ = false;
	rhs.sdl_window_ = nullptr;
	rhs.sdl_gl_context_ = nullptr;
	rhs.sdl_window_id_ = 0;
	rhs.ogl_font_atlas_ = 0;
	rhs.im_context_ = nullptr;
}

Window::~Window()
{
	uninitialize();
}

bool Window::initialize(
	const std::string& title,
	const int width,
	const int height)
{
	uninitialize();

	if (width <= 0 || height <= 0)
	{
		error_message_ = "Invalid window dimensions.";
		return false;
	}

	auto is_succeed = true;
	auto sdl_result = 0;

	if (is_succeed)
	{
		sdl_result = ::SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

		if (sdl_result)
		{
			is_succeed = false;
			error_message_ = "Failed to set OpenGL attribute: double buffering. " + std::string{::SDL_GetError()};
		}
	}

	const auto sdl_window_flags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;

	if (is_succeed)
	{
		sdl_window_ = ::SDL_CreateWindow(
			title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width,
			height,
			sdl_window_flags);

		if (!sdl_window_)
		{
			is_succeed = false;
			error_message_ = "Failed to create SDL window. " + std::string{::SDL_GetError()};
		}
	}

	if (is_succeed)
	{
		sdl_window_id_ = ::SDL_GetWindowID(sdl_window_);

		if (sdl_window_id_ == 0)
		{
			is_succeed = false;
			error_message_ = "Failed to get SDL window id. " + std::string{::SDL_GetError()};
		}
	}

	if (is_succeed)
	{
		sdl_gl_context_ = ::SDL_GL_CreateContext(sdl_window_);

		if (!sdl_gl_context_)
		{
			is_succeed = false;
			error_message_ = "Failed to create OpenGL context and make it current. " + std::string{::SDL_GetError()};
		}
	}

	auto& image_cache = ImageCache::get_instance();
	auto& im_font_atlas = image_cache.get_font_atlas();

	if (is_succeed)
	{
		if (im_font_atlas.Fonts.empty())
		{
			is_succeed = false;
			error_message_ = "No font atlas.";
		}
	}

	if (is_succeed)
	{
		im_context_ = ImGui::CreateContext(&im_font_atlas);

		if (!im_context_)
		{
			is_succeed = false;
			error_message_ = "Failed to create ImGUI context.";
		}

		ImGui::SetCurrentContext(im_context_);

		auto& im_io = ImGui::GetIO();

		im_io.IniFilename = nullptr;
	}

	if (is_succeed)
	{
		unsigned char* pixels = nullptr;
		int font_atlas_width;
		int font_atlas_height;

		im_font_atlas.GetTexDataAsAlpha8(&pixels, &font_atlas_width, &font_atlas_height);

		if (pixels)
		{
			::glGenTextures(1, &ogl_font_atlas_);
			::glBindTexture(GL_TEXTURE_2D, ogl_font_atlas_);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			::glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, font_atlas_width, font_atlas_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
		}
		else
		{
			is_succeed = false;
			error_message_ = "Failed to get font atlas data.";
		}
	}

	if (is_succeed)
	{
		auto w = 0;
		auto h = 0;

		::SDL_GetWindowSize(sdl_window_, &w, &h);

		auto display_w = 0;
		auto display_h = 0;

		::SDL_GL_GetDrawableSize(sdl_window_, &display_w, &display_h);

		auto& io = ImGui::GetIO();

		io.DisplaySize = ImVec2{static_cast<float>(w), static_cast<float>(h)};

		io.DisplayFramebufferScale = ImVec2
		{
			w > 0 ? (static_cast<float>(display_w) / w) : 0,
			h > 0 ? (static_cast<float>(display_h) / h) : 0,
		};

		// Setup back-end capabilities flags
		//
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests (optional, rarely used)

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
		//
		io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
		io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
		io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
		io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
		io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
		io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
		io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
		io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
		io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
		io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
		io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
		io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
		io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

		auto& clipboard = Clipboard::get_instance();
		clipboard.initialize_im_context(io);

		::SDL_ShowWindow(sdl_window_);
	}

	if (!is_succeed)
	{
		uninitialize();
		return false;
	}

	auto& window_mananger = WindowManager::get_instance();

	window_mananger.register_window(sdl_window_id_, this);

	is_initialized_ = true;

	return true;
}

bool Window::is_initialized() const
{
	return is_initialized_;
}

const std::string& Window::get_error_message() const
{
	return error_message_;
}

void Window::draw()
{
	if (!is_initialized_)
	{
		return;
	}


	auto sdl_result = 0;

	sdl_result = ::SDL_GL_MakeCurrent(sdl_window_, sdl_gl_context_);
	ImGui::SetCurrentContext(im_context_);

	auto& im_io = ImGui::GetIO();

	im_io.Fonts->TexID = reinterpret_cast<ImTextureID>(static_cast<std::intptr_t>(ogl_font_atlas_));

	im_new_frame();

	ImGui::NewFrame();

	do_draw();

	im_render();

	ImGui::SetCurrentContext(nullptr);
	sdl_result = ::SDL_GL_MakeCurrent(sdl_window_, nullptr);
}

void Window::uninitialize()
{
	if (im_context_)
	{
		ImGui::DestroyContext(im_context_);
		im_context_ = nullptr;
	}

	if (sdl_window_id_ > 0 && sdl_window_)
	{
		auto& window_manager = WindowManager::get_instance();
		window_manager.unregister_window(sdl_window_id_);
	}

	is_initialized_ = false;
	error_message_ = {};

	if (sdl_gl_context_)
	{
		const auto sdl_result = ::SDL_GL_MakeCurrent(sdl_window_, sdl_gl_context_);
		assert(sdl_result == 0);

		if (ogl_font_atlas_ != 0)
		{
			::glDeleteTextures(1, &ogl_font_atlas_);
			ogl_font_atlas_ = 0;
		}

		static_cast<void>(::SDL_GL_MakeCurrent(sdl_window_, nullptr));
		::SDL_GL_DeleteContext(sdl_gl_context_);

		sdl_gl_context_ = nullptr;
	}

	if (sdl_window_)
	{
		::SDL_HideWindow(sdl_window_);
		::SDL_DestroyWindow(sdl_window_);
		sdl_window_ = nullptr;
	}

	sdl_window_id_ = 0;

	pressed_mouse_buttons_ = {};
}

void Window::handle_event(
	const SDL_Event& sdl_event)
{
	ImGui::SetCurrentContext(im_context_);

	auto& io = ImGui::GetIO();

	switch (sdl_event.type)
	{
	case SDL_MOUSEWHEEL:
		if (sdl_event.wheel.x > 0)
		{
			io.MouseWheelH += 1;
		}

		if (sdl_event.wheel.x < 0)
		{
			io.MouseWheelH -= 1;
		}

		if (sdl_event.wheel.y > 0)
		{
			io.MouseWheel += 1;
		}

		if (sdl_event.wheel.y < 0)
		{
			io.MouseWheel -= 1;
		}

		break;

	case SDL_MOUSEBUTTONDOWN:
		if (sdl_event.button.button == SDL_BUTTON_LEFT)
		{
			pressed_mouse_buttons_[0] = true;
		}

		if (sdl_event.button.button == SDL_BUTTON_RIGHT)
		{
			pressed_mouse_buttons_[1] = true;
		}

		if (sdl_event.button.button == SDL_BUTTON_MIDDLE)
		{
			pressed_mouse_buttons_[2] = true;
		}

		break;

	case SDL_TEXTINPUT:
		io.AddInputCharactersUTF8(sdl_event.text.text);
		break;

	case SDL_KEYDOWN:
	case SDL_KEYUP:
	{
		const auto key = sdl_event.key.keysym.scancode;
		IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));

		const auto mod_state = ::SDL_GetModState();

		io.KeysDown[key] = (sdl_event.type == SDL_KEYDOWN);

		io.KeyShift = ((mod_state & KMOD_SHIFT) != 0);
		io.KeyCtrl = ((mod_state & KMOD_CTRL) != 0);
		io.KeyAlt = ((mod_state & KMOD_ALT) != 0);
		io.KeySuper = ((mod_state & KMOD_GUI) != 0);

		break;
	}

	default:
		break;
	}

	ImGui::SetCurrentContext(nullptr);
}

Uint32 Window::get_id() const
{
	return sdl_window_id_;
}

void Window::im_new_frame()
{
	auto& io = ImGui::GetIO();

	// Font atlas needs to be built, call renderer _NewFrame() function e.g. ImGui_ImplOpenGL3_NewFrame()
	assert(io.Fonts->IsBuilt());

	// Setup display size (every frame to accommodate for window resizing)
	//

	auto w = 0;
	auto h = 0;

	::SDL_GetWindowSize(sdl_window_, &w, &h);

	auto display_w = 0;
	auto display_h = 0;

	::SDL_GL_GetDrawableSize(sdl_window_, &display_w, &display_h);

	io.DisplaySize = ImVec2{static_cast<float>(w), static_cast<float>(h)};

	io.DisplayFramebufferScale = ImVec2
	{
		w > 0 ? (static_cast<float>(display_w) / w) : 0,
		h > 0 ? (static_cast<float>(display_h) / h) : 0,
	};

	// Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
	//

	static auto frequency = ::SDL_GetPerformanceFrequency();
	auto current_time = ::SDL_GetPerformanceCounter();

	io.DeltaTime =
		time_ > 0
		?
		static_cast<float>(static_cast<double>(current_time - time_) / static_cast<double>(frequency))
		:
		static_cast<float>(1.0F / 60.0F);

	time_ = current_time;

	im_update_mouse_position_and_buttons();
	im_update_mouse_cursor();
}

void Window::im_update_mouse_position_and_buttons()
{
	auto& io = ImGui::GetIO();

	// Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
	if (io.WantSetMousePos)
	{
		::SDL_WarpMouseInWindow(sdl_window_, static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y));
	}
	else
	{
		io.MousePos = ImVec2{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
	}

	auto mx = 0;
	auto my = 0;

	const auto mouse_buttons = ::SDL_GetMouseState(&mx, &my);

	// If a mouse press event came, always pass it as "mouse held this frame",
	// so we don't miss click-release events that are shorter than 1 frame.
	//

	io.MouseDown[0] = pressed_mouse_buttons_[0] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
	io.MouseDown[1] = pressed_mouse_buttons_[1] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
	io.MouseDown[2] = pressed_mouse_buttons_[2] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;

	pressed_mouse_buttons_ = {};

#if SDL_VERSION_ATLEAST(2,0,4)
	auto focused_window = ::SDL_GetKeyboardFocus();

	if (sdl_window_ == focused_window)
	{
		// SDL_GetMouseState() gives mouse position seemingly based on the last window entered/focused(?)
		// The creation of a new windows at runtime and SDL_CaptureMouse both seems to severely mess up with that, so we retrieve that position globally.
		//

		auto wx = 0;
		auto wy = 0;

		::SDL_GetWindowPosition(focused_window, &wx, &wy);
		::SDL_GetGlobalMouseState(&mx, &my);

		mx -= wx;
		my -= wy;

		io.MousePos = ImVec2{static_cast<float>(mx), static_cast<float>(my)};
	}

	// SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL window boundaries shouldn't e.g. trigger the OS window resize cursor.
	// The function is only supported from SDL 2.0.4 (released Jan 2016)
	//

	const auto any_mouse_button_down = ImGui::IsAnyMouseDown();

	::SDL_CaptureMouse(any_mouse_button_down ? SDL_TRUE : SDL_FALSE);
#else // SDL_VERSION_ATLEAST
	if ((::SDL_GetWindowFlags(sdl_window_) & SDL_WINDOW_INPUT_FOCUS) != 0)
	{
		io.MousePos = ImVec2{static_cast<float>(mx), static_cast<float>(my)};
	}
#endif // SDL_VERSION_ATLEAST
}

void Window::im_update_mouse_cursor()
{
	auto& io = ImGui::GetIO();

	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) != 0)
	{
		return;
	}

	auto imgui_cursor = ImGui::GetMouseCursor();

	if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		::SDL_ShowCursor(SDL_FALSE);
	}
	else
	{
		auto& cursors = SystemCursors::get_instance();

		// Show OS mouse cursor
		::SDL_SetCursor(cursors[imgui_cursor] ? cursors[imgui_cursor] : cursors[ImGuiMouseCursor_Arrow]);
		::SDL_ShowCursor(SDL_TRUE);
	}
}

void Window::im_render()
{
	ImGui::Render();

	auto draw_data = ImGui::GetDrawData();

	if (draw_data->CmdListsCount <= 0)
	{
		return;
	}

	auto& io = ImGui::GetIO();

	::glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
	::glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
	::glClear(GL_COLOR_BUFFER_BIT);

	im_render_data(draw_data);

	::SDL_GL_SwapWindow(sdl_window_);
}

void Window::im_render_data(
	ImDrawData* draw_data)
{
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	//

	auto& io = ImGui::GetIO();

	const auto fb_width = static_cast<int>(draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
	const auto fb_height = static_cast<int>(draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);

	if (fb_width == 0 || fb_height == 0)
	{
		return;
	}

	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// We are using the OpenGL fixed pipeline to make the example code simpler to read!
	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.

	GLint last_texture;
	::glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

	GLint last_polygon_mode[2];
	::glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);

	GLint last_viewport[4];
	::glGetIntegerv(GL_VIEWPORT, last_viewport);

	GLint last_scissor_box[4];
	::glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);

	::glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
	::glEnable(GL_BLEND);
	::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	::glDisable(GL_CULL_FACE);
	::glDisable(GL_DEPTH_TEST);
	::glDisable(GL_LIGHTING);
	::glDisable(GL_COLOR_MATERIAL);
	::glEnable(GL_SCISSOR_TEST);
	::glEnableClientState(GL_VERTEX_ARRAY);
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	::glEnableClientState(GL_COLOR_ARRAY);
	::glEnable(GL_TEXTURE_2D);
	::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Setup viewport, orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
	//

	::glViewport(0, 0, static_cast<GLsizei>(fb_width), static_cast<GLsizei>(fb_height));
	::glMatrixMode(GL_PROJECTION);
	::glPushMatrix();
	::glLoadIdentity();

	::glOrtho(
		draw_data->DisplayPos.x,
		draw_data->DisplayPos.x + draw_data->DisplaySize.x,
		draw_data->DisplayPos.y + draw_data->DisplaySize.y,
		draw_data->DisplayPos.y,
		-1.0,
		+1.0);

	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glLoadIdentity();

	// Render command lists
	auto pos = draw_data->DisplayPos;

	for (int n = 0; n < draw_data->CmdListsCount; ++n)
	{
		const auto cmd_list = draw_data->CmdLists[n];
		const auto vtx_buffer = cmd_list->VtxBuffer.Data;
		auto idx_buffer = cmd_list->IdxBuffer.Data;

		::glVertexPointer(
			2,
			GL_FLOAT,
			sizeof(ImDrawVert),
			(const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos)));

		::glTexCoordPointer(
			2,
			GL_FLOAT,
			sizeof(ImDrawVert),
			(const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv)));

		::glColorPointer(
			4,
			GL_UNSIGNED_BYTE,
			sizeof(ImDrawVert),
			(const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col)));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
		{
			const auto pcmd = &cmd_list->CmdBuffer[cmd_i];

			if (pcmd->UserCallback)
			{
				// User callback (registered via ImDrawList::AddCallback)
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				auto clip_rect = ImVec4
				{
					pcmd->ClipRect.x - pos.x,
					pcmd->ClipRect.y - pos.y,
					pcmd->ClipRect.z - pos.x,
					pcmd->ClipRect.w - pos.y
				};

				if (clip_rect.x < fb_width &&
					clip_rect.y < fb_height &&
					clip_rect.z >= 0.0f &&
					clip_rect.w >= 0.0f)
				{
					::glScissor(
						static_cast<int>(clip_rect.x),
						static_cast<int>(fb_height - clip_rect.w),
						static_cast<int>(clip_rect.z - clip_rect.x),
						static_cast<int>(clip_rect.w - clip_rect.y));

					::glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(reinterpret_cast<intptr_t>(pcmd->TextureId)));

					::glDrawElements(
						GL_TRIANGLES,
						static_cast<GLsizei>(pcmd->ElemCount),
						sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
						idx_buffer);
				}
			}

			idx_buffer += pcmd->ElemCount;
		}
	}

	// Restore modified state
	//

	::glDisableClientState(GL_COLOR_ARRAY);
	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);
	::glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(last_texture));
	::glMatrixMode(GL_MODELVIEW);
	::glPopMatrix();
	::glMatrixMode(GL_PROJECTION);
	::glPopMatrix();
	::glPopAttrib();
	::glPolygonMode(GL_FRONT, static_cast<GLenum>(last_polygon_mode[0]));
	::glPolygonMode(GL_BACK, static_cast<GLenum>(last_polygon_mode[1]));

	::glViewport(
		static_cast<GLsizei>(last_viewport[0]),
		static_cast<GLsizei>(last_viewport[1]),
		static_cast<GLsizei>(last_viewport[2]),
		static_cast<GLsizei>(last_viewport[3]));

	::glScissor(
		static_cast<GLsizei>(last_scissor_box[0]),
		static_cast<GLsizei>(last_scissor_box[1]),
		static_cast<GLsizei>(last_scissor_box[2]),
		static_cast<GLsizei>(last_scissor_box[3]));
}

//
// Window
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ImDemoWindow
//

ImDemoWindow::ImDemoWindow()
	:
	show_demo_window_{true},
	show_another_window_{},
	f_{},
	counter_{}
{
}

ImDemoWindow::ImDemoWindow(
	ImDemoWindow&& rhs)
	:
	Window{std::move(rhs)},
	show_demo_window_{std::move(rhs.show_demo_window_)},
	show_another_window_{std::move(rhs.show_another_window_)},
	f_{std::move(rhs.f_)},
	counter_{std::move(rhs.counter_)}
{
}

ImDemoWindow::~ImDemoWindow()
{
}

void ImDemoWindow::do_draw()
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window_)
	{
		ImGui::ShowDemoWindow(&show_demo_window_);
	}

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		auto clear_color = ImVec4{0.45F, 0.55F, 0.60F, 1.00F};

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window_);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window_);

		ImGui::SliderFloat("float", &f_, 0.0F, 1.0F);            // Edit 1 float using a slider from 0.0f to 1.0f    
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		{
			counter_++;
		}

		ImGui::SameLine();
		ImGui::Text("counter = %d", counter_);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0F / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window_)
	{
		// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Begin("Another Window", &show_another_window_);

		ImGui::Text("Hello from another window!");

		if (ImGui::Button("Close Me"))
		{
			show_another_window_ = false;
		}

		ImGui::End();
	}
}

//
// ImDemoWindow
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// WindowManager
//

WindowManager::WindowManager()
	:
	window_map_{},
	is_quit_requested_{}
{
}

WindowManager::WindowManager(
	WindowManager&& rhs)
	:
	window_map_{std::move(rhs.window_map_)},
	is_quit_requested_{std::move(rhs.is_quit_requested_)}
{
}

WindowManager::~WindowManager()
{
}

WindowManager& WindowManager::get_instance()
{
	static auto window_manager = WindowManager{};

	return window_manager;
}

void WindowManager::initialize()
{
	uninitialize();
}

void WindowManager::uninitialize()
{
	while (!window_map_.empty())
	{
		delete window_map_.begin()->second;
	}

	is_quit_requested_ = false;
}

void WindowManager::handle_events()
{
	auto sdl_event = SDL_Event{};

	while (::SDL_PollEvent(&sdl_event))
	{
		if (sdl_event.type == SDL_QUIT)
		{
			is_quit_requested_ = true;
		}

		auto map_it = window_map_.begin();

		while (map_it != window_map_.cend())
		{
			auto current_map_it = map_it;
			auto next_map_it = ++map_it;

			current_map_it->second->handle_event(sdl_event);

			map_it = next_map_it;
		}
	}
}

void WindowManager::draw()
{
	if (window_map_.empty())
	{
		return;
	}

	auto map_it = window_map_.begin();

	while (map_it != window_map_.end())
	{
		auto current_map_it = map_it;
		auto next_map_it = ++map_it;

		current_map_it->second->draw();

		map_it = next_map_it;
	}
}

bool WindowManager::is_quit_requested() const
{
	return is_quit_requested_;
}

void WindowManager::register_window(
	const Uint32 window_id,
	WindowPtr window_ptr)
{
	if (!window_ptr)
	{
		assert(!"No window.");
		return;
	}

	window_map_[window_id] = window_ptr;
}

void WindowManager::unregister_window(
	const Uint32 window_id)
{
	if (window_map_.empty())
	{
		return;
	}

	window_map_.erase(window_id);
}

//
// WindowManager
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Launcher
//

Launcher::Launcher()
	:
	is_initialized_{},
	error_message_{}
{
}

Launcher::Launcher(
	Launcher&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)}
{
}

Launcher::~Launcher()
{
	uninitialize();
}


bool Launcher::initialize()
{
	uninitialize();

	if (!initialize_sdl())
	{
		return false;
	}

	// Clipboard
	//

	auto& clipboard = Clipboard::get_instance();
	clipboard.initialize();


	// SystemCursors
	//

	auto& system_cursors = SystemCursors::get_instance();
	system_cursors.initialize();


	// WindowManager
	//

	auto& window_manager = WindowManager::get_instance();
	window_manager.initialize();


	// ImageCache
	//

	const auto font_file_name = std::string{"F:\\temp\\noto_sans_display_condensed.ttf"};

	auto& image_cache = ImageCache::get_instance();

	image_cache.initialize();

	const auto set_font_result = image_cache.set_font(font_file_name, 18);

	if (!set_font_result)
	{
		uninitialize();

		error_message_ = "Failed to cache font: \"" + font_file_name + "\".";
		return false;
	}

	//

	is_initialized_ = true;

	return true;
}

void Launcher::uninitialize()
{
	is_initialized_ = false;
	error_message_ = {};


	// Clipboard
	//

	auto& clipboard = Clipboard::get_instance();
	clipboard.uninitialize();


	// SystemCursors
	//

	auto& system_cursors = SystemCursors::get_instance();
	system_cursors.uninitialize();


	// WindowManager
	//

	auto& window_manager = WindowManager::get_instance();
	window_manager.uninitialize();


	// ImageCache
	//

	auto& image_cache = ImageCache::get_instance();
	image_cache.uninitialize();


	// SDL
	//

	uninitialize_sdl();
}

bool Launcher::is_initialized()
{
	return is_initialized_;
}

const std::string& Launcher::get_error_message()
{
	return error_message_;
}

void Launcher::run()
{
	if (!is_initialized_)
	{
		error_message_ = "Not initialized.";
		return;
	}

	auto frequency = ::SDL_GetPerformanceFrequency();

	auto& window_manager = WindowManager::get_instance();

	const auto max_delay_ms = 1000UL;
	const auto delay_bias_ms = 5;
	const auto target_delay_ms = 1000ULL / 60ULL; // 60 FPS

	while (!window_manager.is_quit_requested())
	{
		const auto begin_time = ::SDL_GetPerformanceCounter();

		window_manager.draw();
		window_manager.handle_events();

		const auto end_time = ::SDL_GetPerformanceCounter();

		const auto process_time_ms = (end_time - begin_time + frequency - 1) / frequency;

		auto delay_ms = target_delay_ms - process_time_ms - delay_bias_ms;

		if (delay_ms > max_delay_ms)
		{
			delay_ms = max_delay_ms;
		}

		if (delay_ms > 0)
		{
			::SDL_Delay(static_cast<Uint32>(delay_ms));
		}
	}
}

bool Launcher::initialize_ogl_functions()
{
	auto is_succeed = true;
	auto sdl_result = 0;

	// Set OpenGL attributes.
	//

	if (is_succeed)
	{
		sdl_result = ::SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

		if (is_succeed && sdl_result)
		{
			is_succeed = false;
			error_message_ = "Failed to set OpenGL attribute: double buffering. " + std::string{::SDL_GetError()};
		}
	}

	// Create dummy window.
	//

	SDL_Window* sdl_window = nullptr;

	if (is_succeed)
	{
		const auto sdl_window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;

		sdl_window = ::SDL_CreateWindow(
			"dummy",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			64,
			64,
			sdl_window_flags);

		if (!sdl_window)
		{
			is_succeed = false;
			error_message_ = "Failed to create dummy window. " + std::string{::SDL_GetError()};
		}
	}

	// Create OpenGL context and make it current.
	//

	auto sdl_gl_context = SDL_GLContext{};

	if (is_succeed)
	{
		sdl_gl_context = ::SDL_GL_CreateContext(sdl_window);

		if (!sdl_gl_context)
		{
			is_succeed = false;
			error_message_ = "Failed to create OpenGL context. " + std::string{::SDL_GetError()};
		}
	}

	// Initialize GLAD.
	//

	if (is_succeed)
	{
		const auto glad_result = ::gladLoadGLLoader(::SDL_GL_GetProcAddress);

		if (!glad_result)
		{
			is_succeed = false;
			error_message_ = "Failed to initialize GLAD.";
		}
	}


	// Clean up.
	//

	if (sdl_gl_context)
	{
		static_cast<void>(::SDL_GL_MakeCurrent(sdl_window, nullptr));
	}

	if (sdl_window)
	{
		::SDL_DestroyWindow(sdl_window);
	}

	return is_succeed;
}

void Launcher::uninitialize_sdl()
{
	::SDL_GL_UnloadLibrary();
	::SDL_Quit();
}

bool Launcher::initialize_sdl()
{
	auto is_succeed = true;
	auto sdl_result = 0;

	if (is_succeed)
	{
		sdl_result = ::SDL_Init(SDL_INIT_VIDEO);

		if (sdl_result)
		{
			is_succeed = false;
			error_message_ = "Failed to initialize SDL. " + std::string{::SDL_GetError()};
		}
	}

	if (is_succeed)
	{
		sdl_result = ::SDL_GL_LoadLibrary(nullptr);

		if (sdl_result)
		{
			is_succeed = false;
			error_message_ = "Failed to load OpenGL library. " + std::string{::SDL_GetError()};
		}
	}

	if (is_succeed)
	{
		if (!initialize_ogl_functions())
		{
			is_succeed = false;
		}
	}

	if (!is_succeed)
	{
		uninitialize_sdl();
	}

	return is_succeed;
}

//
// Launcher
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


int main(
	int,
	char**)
{
	auto launcher = Launcher{};

	if (!launcher.initialize())
	{
		::SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"Launcher",
			launcher.get_error_message().c_str(),
			nullptr);

		return 1;
	}

	auto im_demo_window = ImDemoWindow{};
	im_demo_window.initialize("ImGUI demo", 1280, 720);

	launcher.run();

	return 0;
}
