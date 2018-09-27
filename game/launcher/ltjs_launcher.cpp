#include <cassert>
#include <cstddef>
#include <cstdint>
#include <array>
#include <algorithm>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "bibendovsky_spul_path_utils.h"
#include "bibendovsky_spul_scope_guard.h"
#include "glad.h"
#include "imgui.h"
#include "SDL.h"


namespace ul = bibendovsky::spul;


using SdlWindowPtr = SDL_Window*;


ImVec2 operator*(
	const ImVec2& v,
	const float scale);

ImVec2& operator*=(
	ImVec2& v,
	const float scale);


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Classes
//

enum class ImageId
{
	boxbackground,
	checkboxc,
	checkboxf,
	checkboxn,
	closed,
	closeu,
	error,
	information,
	minimized,
	minimizeu,
	warning,

	canceld,
	cancelf,
	cancelu,
	company1webd,
	company1webf,
	company1webu,
	company2webd,
	company2webf,
	company2webu,
	custombackground,
	customd,
	customf,
	customu,
	customx,
	demomainappbackground,
	detailsettingsbackground,
	displaybackground,
	displayd,
	displayf,
	displayu,
	displayx,
	highdetaild,
	highdetailf,
	highdetailu,
	installd,
	installf,
	installu,
	lowdetaild,
	lowdetailf,
	lowdetailu,
	mainappbackground,
	mediumdetaild,
	mediumdetailf,
	mediumdetailu,
	nextd,
	nextf,
	nextu,
	nextx,
	okd,
	okf,
	oku,
	optionsbackground,
	optionsd,
	optionsf,
	optionsu,
	optionsx,
	playd,
	playf,
	playu,
	previousd,
	previousf,
	previousu,
	previousx,
	publisher1webd,
	publisher1webf,
	publisher1webu,
	publisher2webd,
	publisher2webf,
	publisher2webu,
	quitd,
	quitf,
	quitu,
	serverd,
	serverf,
	serveru,
	serverx,
}; // ImageId


class Base
{
public:
	Base();

	Base(
		Base&& rhs);


	const std::string& get_error_message();


protected:
	void clear_error_message();

	void set_error_message(
		const std::string& error_message);

	void append_error_message(
		const std::string& error_message);

	void prepend_error_message(
		const std::string& error_message);


private:
	std::string error_message_;
}; // Base


struct SdlCreateWindowParam final
{
	std::string title_;

	int width_;
	int height_;

	bool is_borderless_;
	bool is_hidden_;
	bool is_double_buffering_;
	bool is_share_with_current_ogl_context_;;
}; // SdlCreateWindowParam


struct SdlOglWindow final
{
	SdlWindowPtr window_;
	Uint32 window_id_;
	SDL_GLContext ogl_context_;


	bool is_valid() const
	{
		return window_ && window_id_ != 0 && ogl_context_;
	}

	void make_context_current(
		const bool is_current)
	{
		if (!is_valid())
		{
			return;
		}

		static_cast<void>(::SDL_GL_MakeCurrent(window_, is_current ? ogl_context_ : nullptr));
	}
}; // SdlOglWindow


class Sdl final :
	public Base
{
public:
	SdlOglWindow ogl_create_window(
		const SdlCreateWindowParam& param);

	static void ogl_destroy_window(
		SdlOglWindow& sdl_ogl_window);
}; // Sdl


class FontManager final :
	public Base
{
public:
	static const float ref_font_size_pixels;


	FontManager(
		FontManager&& rhs);

	static FontManager& get_instance();


	bool initialize();

	void uninitialize();


	bool set_font(
		const float font_size_pixels);

	ImFontAtlas& get_font_atlas();


private:
	static constexpr int max_font_file_size = 1 * 1'024 * 1'024;
	static const std::string font_file_name;


	using Buffer = std::vector<std::uint8_t>;
	using ImFontAtlasUPtr = std::unique_ptr<ImFontAtlas>;


	Buffer font_data_;
	ImFontAtlasUPtr font_atlas_uptr_;


	FontManager();

	~FontManager();


	static void font_data_deleter(
		std::uint8_t* font_data);
}; // FontManager


struct OglTexture final
{
	GLuint ogl_id_;
	ImVec2 uv0_; // left-bottom
	ImVec2 uv1_; // right-top


	ImTextureID get_im_texture_id() const
	{
		return reinterpret_cast<ImTextureID>(static_cast<std::intptr_t>(ogl_id_));
	}
}; // OglTexture


class OglTextureManager final :
	public Base
{
public:
	OglTextureManager(
		OglTextureManager&& rhs);

	static OglTextureManager& get_instance();


	bool initialize();

	void uninitialize();


	bool load_font();

	const OglTexture& get_font();


	bool load_all_textures();

	const OglTexture& get(
		const ImageId image_id);


	void make_context_current(
		const bool is_current);


private:
	using ImFontAtlasUPtr = std::unique_ptr<ImFontAtlas>;
	using Strings = std::vector<std::string>;
	using OglTextures = std::vector<OglTexture>;


	static const std::string images_path;
	static const Strings image_file_names;


	SdlOglWindow sdl_ogl_window_;
	OglTextures ogl_textures_;
	OglTexture ogl_font_texture_;


	OglTextureManager();

	~OglTextureManager();


	bool initialize_context();

	void uninitialize_context();

	static int ogl_calculate_npot_dimension(
		const int dimension);

	OglTexture create_texture_from_surface(
		SDL_Surface* sdl_surface);

	void unload_font();
}; // OglTextureManager


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


struct WindowCreateParam
{
	std::string title_;
	int width_;
	int height_;
	bool is_hidden_;
}; // WindowCreateParam


class Window;
using WindowPtr = Window*;


class Window :
	public Base
{
public:
	virtual ~Window();


	bool is_initialized() const;

	void show(
		const bool is_show);

	bool is_show() const;

	void draw();


protected:
	friend class WindowManager;


	bool is_initialized_;


	Window();

	Window(
		const Window& rhs) = delete;


	bool initialize(
		const WindowCreateParam& param);

	void handle_event(
		const SDL_Event& sdl_event);

	void minimize_internal(
		const bool is_minimize);

	static int ogl_calculate_npot_dimension(
		const int dimension);

	static bool is_point_inside_rect(
		const ImVec2& point,
		const ImVec4& rect);


private:
	friend class WindowManager;

	using PressedMouseButtons = std::array<bool, 3>;

	SdlOglWindow sdl_ogl_window_;
	ImGuiContext* im_context_;
	PressedMouseButtons pressed_mouse_buttons_;
	Uint64 time_;


	void uninitialize();

	Uint32 get_id() const;

	void im_new_frame();

	void im_update_mouse_position_and_buttons();

	void im_update_mouse_cursor();

	void im_render();

	void im_render_data(
		ImDrawData* draw_data);


	virtual void do_draw() = 0;
}; // Window


class MainWindow;
using MainWindowPtr = MainWindow*;
using MainWindowUPtr = std::unique_ptr<MainWindow>;

class MainWindow final :
	public Window
{
public:
	~MainWindow() override;


	static MainWindowPtr create();


private:
	static constexpr auto window_width = 525;
	static constexpr auto window_height = 245;


	MainWindow();


	bool initialize(
		const WindowCreateParam& param);

	void uninitialize();


	void do_draw() override;
}; // MainWindow


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

	float get_scale() const;


private:
	friend class Window;


	static constexpr auto ref_scale_dimension = 600.0F;


	using WindowMap = std::unordered_map<Uint32, WindowPtr>;


	WindowMap window_map_;
	bool is_quit_requested_;
	float scale_;


	WindowManager();

	~WindowManager();


	void register_window(
		const Uint32 window_id,
		WindowPtr window_ptr);

	void unregister_window(
		const Uint32 window_id);
}; // WindowManager


class Launcher final :
	public Base
{
public:
	Launcher();

	Launcher(
		Launcher&& rhs);

	~Launcher();


	bool initialize();

	void uninitialize();

	bool is_initialized();

	void run();


private:
	bool is_initialized_;
	MainWindowUPtr main_window_uptr_;


	bool initialize_ogl_functions();

	void uninitialize_sdl();

	bool initialize_sdl();
}; // Launcher

//
// Classes
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Base
//

Base::Base()
	:
	error_message_{}
{
}

Base::Base(
	Base&& rhs)
	:
	error_message_{std::move(rhs.error_message_)}
{
}

const std::string& Base::get_error_message()
{
	return error_message_;
}

void Base::clear_error_message()
{
	error_message_.clear();
}

void Base::set_error_message(
	const std::string& error_message)
{
	error_message_ = error_message;
}

void Base::append_error_message(
	const std::string& error_message)
{
	error_message_ += error_message;
}

void Base::prepend_error_message(
	const std::string& error_message)
{
	error_message_.insert(std::string::size_type{}, error_message);
}

//
// Base
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sdl
//

SdlOglWindow Sdl::ogl_create_window(
	const SdlCreateWindowParam& param)
{
	if (param.width_ <= 0)
	{
		set_error_message("Invalid width.");
	}

	if (param.height_ <= 0)
	{
		set_error_message("Invalid height.");
	}


	bool is_succeed = true;
	int sdl_result = 0;


	// Set OpenGL attributes.
	//
	if (is_succeed)
	{
		sdl_result = ::SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, param.is_double_buffering_);

		if (sdl_result)
		{
			is_succeed = false;

			set_error_message("Failed to set OpenGL attribute: double buffering. " + std::string{::SDL_GetError()});
		}
	}

	if (is_succeed)
	{
		sdl_result = ::SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, param.is_share_with_current_ogl_context_);

		if (sdl_result)
		{
			is_succeed = false;

			set_error_message("Failed to set OpenGL attribute: share with current context. " + std::string{::SDL_GetError()});
		}
	}

	// Create window.
	//
	auto sdl_window = SdlWindowPtr{};

	if (is_succeed)
	{
		const Uint32 sdl_window_flags =
			SDL_WINDOW_OPENGL |
			SDL_WINDOW_HIDDEN |
			(param.is_borderless_ ? SDL_WINDOW_BORDERLESS : 0);

		sdl_window = ::SDL_CreateWindow(
			param.title_.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			param.width_,
			param.height_,
			sdl_window_flags);

		if (!sdl_window)
		{
			is_succeed = false;

			set_error_message("Failed to create window. " + std::string{::SDL_GetError()});
		}
	}

	// Window ID.
	//
	auto sdl_window_id = Uint32{};

	if (is_succeed)
	{
		sdl_window_id = ::SDL_GetWindowID(sdl_window);

		if (sdl_window_id == 0)
		{
			is_succeed = false;

			set_error_message("Failed to get window's id. " + std::string{::SDL_GetError()});
		}
	}

	// Create OpenGL context.
	//
	auto sdl_gl_context = SDL_GLContext{};

	if (is_succeed)
	{
		sdl_gl_context = ::SDL_GL_CreateContext(sdl_window);

		if (!sdl_gl_context)
		{
			is_succeed = false;

			set_error_message("Failed to create OpenGL context. " + std::string{::SDL_GetError()});
		}
	}

	// Show window.
	//
	if (is_succeed)
	{
		if (!param.is_hidden_)
		{
			::SDL_ShowWindow(sdl_window);
		}
	}

	// Clean up
	//
	if (!is_succeed)
	{
		if (sdl_gl_context)
		{
			::SDL_GL_DeleteContext(sdl_gl_context);
		}

		if (sdl_window)
		{
			::SDL_DestroyWindow(sdl_window);
		}

		return {};
	}

	// Return result.
	//
	auto sdl_ogl_window = SdlOglWindow{};
	sdl_ogl_window.window_ = sdl_window;
	sdl_ogl_window.window_id_ = sdl_window_id;
	sdl_ogl_window.ogl_context_ = sdl_gl_context;

	return sdl_ogl_window;
}

void Sdl::ogl_destroy_window(
	SdlOglWindow& sdl_ogl_window)
{
	if (!sdl_ogl_window.is_valid())
	{
		return;
	}

	static_cast<void>(::SDL_GL_MakeCurrent(sdl_ogl_window.window_, nullptr));

	::SDL_GL_DeleteContext(sdl_ogl_window.ogl_context_);
	sdl_ogl_window.ogl_context_ = nullptr;

	::SDL_DestroyWindow(sdl_ogl_window.window_);
	sdl_ogl_window.window_ = nullptr;
}

//
// Sdl
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// FontManager
//

const float FontManager::ref_font_size_pixels = 18.0F;
const std::string FontManager::font_file_name = "ltjs/nolf2/launcher/fonts/noto_sans_display_condensed.ttf";


FontManager::FontManager()
	:
	font_data_{},
	font_atlas_uptr_{std::make_unique<ImFontAtlas>()}
{
}

FontManager::~FontManager()
{
}

FontManager::FontManager(
	FontManager&& rhs)
	:
	font_data_{std::move(font_data_)},
	font_atlas_uptr_{std::move(font_atlas_uptr_)}
{
}

FontManager& FontManager::get_instance()
{
	static auto font_manager = FontManager{};

	return font_manager;
}

bool FontManager::initialize()
{
	uninitialize();

	const auto file_name = ul::PathUtils::normalize(font_file_name);

	auto sdl_rw = ::SDL_RWFromFile(file_name.c_str(), "rb");

	if (!sdl_rw)
	{
		set_error_message("Failed to open font file: \"" + font_file_name + "\".");

		return false;
	}


	auto guard_rw = ul::ScopeGuard{
		[&]()
		{
			::SDL_FreeRW(sdl_rw);
		}
	};

	const auto file_size = sdl_rw->size(sdl_rw);

	if (file_size > max_font_file_size)
	{
		set_error_message("Font file too big: \"" + font_file_name + "\".");

		return false;
	}

	font_data_.resize(static_cast<Buffer::size_type>(file_size));

	const auto read_size = sdl_rw->read(sdl_rw, font_data_.data(), 1, static_cast<std::size_t>(file_size));

	if (read_size != file_size)
	{
		font_data_.clear();
		set_error_message("Failed to read font file: \"" + font_file_name + "\".");

		return false;
	}

	return true;
}

void FontManager::uninitialize()
{
	font_data_ = {};
	font_atlas_uptr_->Clear();
}

bool FontManager::set_font(
	const float font_size_pixels)
{
	font_atlas_uptr_->Clear();

	auto im_font_data = static_cast<std::uint8_t*>(ImGui::MemAlloc(font_data_.size()));

	std::uninitialized_copy(
		font_data_.cbegin(),
		font_data_.cend(),
		im_font_data);

	const auto im_font = font_atlas_uptr_->AddFontFromMemoryTTF(
		im_font_data,
		static_cast<int>(font_data_.size()),
		font_size_pixels);

	if (!im_font)
	{
		font_atlas_uptr_->Clear();

		set_error_message("Failed to load font.");

		return false;
	}

	return true;
}

ImFontAtlas& FontManager::get_font_atlas()
{
	return *font_atlas_uptr_;
}

void FontManager::font_data_deleter(
	std::uint8_t* font_data)
{
	ImGui::MemFree(font_data);
}

//
// FontManager
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// OglTextureManager
//

const std::string OglTextureManager::images_path = "ltjs/nolf2/launcher/images";

const OglTextureManager::Strings OglTextureManager::image_file_names = Strings
{
	// Common
	//

	"boxbackground.bmp",
	"checkboxc.bmp",
	"checkboxf.bmp",
	"checkboxn.bmp",
	"closed.bmp",
	"closeu.bmp",
	"error.bmp",
	"information.bmp",
	"minimized.bmp",
	"minimizeu.bmp",
	"warning.bmp",

	// Language-specific
	//

	"canceld.bmp",
	"cancelf.bmp",
	"cancelu.bmp",
	"company1webd.bmp",
	"company1webf.bmp",
	"company1webu.bmp",
	"company2webd.bmp",
	"company2webf.bmp",
	"company2webu.bmp",
	"custombackground.bmp",
	"customd.bmp",
	"customf.bmp",
	"customu.bmp",
	"customx.bmp",
	"demomainappbackground.bmp",
	"detailsettingsbackground.bmp",
	"displaybackground.bmp",
	"displayd.bmp",
	"displayf.bmp",
	"displayu.bmp",
	"displayx.bmp",
	"highdetaild.bmp",
	"highdetailf.bmp",
	"highdetailu.bmp",
	"installd.bmp",
	"installf.bmp",
	"installu.bmp",
	"lowdetaild.bmp",
	"lowdetailf.bmp",
	"lowdetailu.bmp",
	"mainappbackground.bmp",
	"mediumdetaild.bmp",
	"mediumdetailf.bmp",
	"mediumdetailu.bmp",
	"nextd.bmp",
	"nextf.bmp",
	"nextu.bmp",
	"nextx.bmp",
	"okd.bmp",
	"okf.bmp",
	"oku.bmp",
	"optionsbackground.bmp",
	"optionsd.bmp",
	"optionsf.bmp",
	"optionsu.bmp",
	"optionsx.bmp",
	"playd.bmp",
	"playf.bmp",
	"playu.bmp",
	"previousd.bmp",
	"previousf.bmp",
	"previousu.bmp",
	"previousx.bmp",
	"publisher1webd.bmp",
	"publisher1webf.bmp",
	"publisher1webu.bmp",
	"publisher2webd.bmp",
	"publisher2webf.bmp",
	"publisher2webu.bmp",
	"quitd.bmp",
	"quitf.bmp",
	"quitu.bmp",
	"serverd.bmp",
	"serverf.bmp",
	"serveru.bmp",
	"serverx.bmp",
}; // OglTextureManager::image_file_names


OglTextureManager::OglTextureManager()
	:
	sdl_ogl_window_{},
	ogl_textures_{},
	ogl_font_texture_{}
{
}

OglTextureManager::OglTextureManager(
	OglTextureManager&& rhs)
	:
	sdl_ogl_window_{std::move(rhs.sdl_ogl_window_)},
	ogl_textures_{std::move(rhs.ogl_textures_)},
	ogl_font_texture_{std::move(rhs.ogl_font_texture_)}
{
}

OglTextureManager::~OglTextureManager()
{
}

OglTextureManager& OglTextureManager::get_instance()
{
	static auto image_cache = OglTextureManager{};

	return image_cache;
}

bool OglTextureManager::initialize()
{
	uninitialize();

	if (!initialize_context())
	{
		uninitialize_context();
		return false;
	}

	ogl_textures_.resize(image_file_names.size());

	return true;
}

void OglTextureManager::uninitialize()
{
	if (sdl_ogl_window_.is_valid())
	{
		sdl_ogl_window_.make_context_current(true);

		for (auto& surface : ogl_textures_)
		{
			if (surface.ogl_id_)
			{
				::glDeleteTextures(1, &surface.ogl_id_);
			}
		}

		sdl_ogl_window_.make_context_current(false);
	}

	ogl_textures_.clear();

	if (ogl_font_texture_.ogl_id_)
	{
		::glDeleteTextures(1, &ogl_font_texture_.ogl_id_);
		ogl_font_texture_.ogl_id_ = 0;
	}

	uninitialize_context();
}

bool OglTextureManager::load_all_textures()
{
	if (image_file_names.empty())
	{
		set_error_message("No images.");
		return false;
	}

	auto guard_context = ul::ScopeGuard{
		[&]()
		{
			sdl_ogl_window_.make_context_current(false);
		}
	};

	sdl_ogl_window_.make_context_current(true);

	const auto image_count = static_cast<int>(image_file_names.size());

	for (auto i = 0; i < image_count; ++i)
	{
		const auto& image_file_name = image_file_names[i];

		const auto invariant_image_path = ul::PathUtils::normalize(ul::PathUtils::append(images_path, image_file_name));

		auto image_surface = ::SDL_LoadBMP(invariant_image_path.c_str());

		if (!image_surface)
		{
			const auto specific_image_path = ul::PathUtils::normalize(
				ul::PathUtils::append(ul::PathUtils::append(images_path, "en"), image_file_name));

			image_surface = ::SDL_LoadBMP(specific_image_path.c_str());

			if (!image_surface)
			{
				set_error_message("Failed to load image: \"" + invariant_image_path + "\".");
				return false;
			}
		}

		const auto ogl_texture = create_texture_from_surface(image_surface);

		::SDL_FreeSurface(image_surface);

		ogl_textures_[i] = ogl_texture;

		if (!ogl_texture.ogl_id_)
		{
			prepend_error_message("Failed to create texure from \"" + image_file_name + "\". ");

			return false;
		}
	}

	return true;
}

const OglTexture& OglTextureManager::get(
	const ImageId image_id)
{
	static const auto invalid_ogl_texture = OglTexture{};

	const auto image_index = static_cast<int>(image_id);

	if (image_index < 0 || image_index >= static_cast<int>(ogl_textures_.size()))
	{
		return invalid_ogl_texture;
	}

	return ogl_textures_[image_index];
}

void OglTextureManager::make_context_current(
	const bool is_current)
{
	sdl_ogl_window_.make_context_current(is_current);
}

bool OglTextureManager::load_font()
{
	unload_font();

	unsigned char* pixels = nullptr;
	int font_atlas_width;
	int font_atlas_height;

	auto& font_manager = FontManager::get_instance();
	auto& font_atlas = font_manager.get_font_atlas();

	font_atlas.GetTexDataAsAlpha8(&pixels, &font_atlas_width, &font_atlas_height);

	if (!pixels || font_atlas_width <= 0 || font_atlas_height <= 0)
	{
		set_error_message("Failed to get font atlas.");

		return false;
	}

	sdl_ogl_window_.make_context_current(true);

	::glGenTextures(1, &ogl_font_texture_.ogl_id_);
	::glBindTexture(GL_TEXTURE_2D, ogl_font_texture_.ogl_id_);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	::glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, font_atlas_width, font_atlas_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

	ogl_font_texture_.uv0_ = {0.0F, 0.0F};
	ogl_font_texture_.uv1_ = {1.0F, 1.0F};

	sdl_ogl_window_.make_context_current(false);

	return true;
}

const OglTexture& OglTextureManager::get_font()
{
	return ogl_font_texture_;
}

OglTexture OglTextureManager::create_texture_from_surface(
	SDL_Surface* sdl_surface)
{
	if (!sdl_surface)
	{
		set_error_message("No surface.");
		return {};
	}

	if (sdl_surface->w <= 0 || sdl_surface->h <= 0)
	{
		set_error_message("Invalid surface dimensions.");
		return {};
	}

	if (!sdl_surface->pixels)
	{
		set_error_message("No surface pixels.");
		return {};
	}

	switch (sdl_surface->format->BitsPerPixel)
	{
	case 24:
	case 32:
		break;

	default:
		set_error_message("Unsupported bit depth.");
		return {};
	}


	static auto buffer = std::vector<std::uint8_t>{};

	auto result = OglTexture{};

	const auto alignment = 4;
	const auto bpp = sdl_surface->format->BytesPerPixel;

	const auto pot_width = ogl_calculate_npot_dimension(sdl_surface->w);
	const auto pot_height = ogl_calculate_npot_dimension(sdl_surface->h);

	const auto is_npot = (sdl_surface->w != pot_width || sdl_surface->h != pot_height);
	const auto is_make_pot = is_npot && !GLAD_GL_ARB_texture_non_power_of_two;

	auto dst_pitch = sdl_surface->w * bpp;
	dst_pitch += alignment - 1;
	dst_pitch /= alignment;
	dst_pitch *= alignment;

	auto dst_width = sdl_surface->w;
	auto dst_height = sdl_surface->h;

	const auto is_convert = (
		!GLAD_GL_EXT_bgra ||
		is_make_pot ||
		sdl_surface->pitch != dst_pitch);

	if (is_convert)
	{
		if (is_make_pot)
		{
			dst_pitch = pot_width * bpp;

			dst_width = pot_width;
			dst_height = pot_height;
		}

		const auto area = dst_pitch * dst_height;

		if (static_cast<int>(buffer.size()) < area)
		{
			buffer.resize(area);
		}

		for (auto i = 0; i < sdl_surface->h; ++i)
		{
			auto src_pixels = static_cast<const std::uint8_t*>(sdl_surface->pixels) + (i * sdl_surface->pitch);
			auto dst_pixels = buffer.data() + (i * dst_pitch);

			for (auto j = 0; j < sdl_surface->w; ++j)
			{
				if (bpp == 3)
				{
					dst_pixels[0] = src_pixels[2];
					dst_pixels[1] = src_pixels[1];
					dst_pixels[2] = src_pixels[0];
				}
				else
				{
					dst_pixels[0] = src_pixels[3];
					dst_pixels[1] = src_pixels[2];
					dst_pixels[2] = src_pixels[1];
					dst_pixels[3] = src_pixels[0];
				}

				src_pixels += bpp;
				dst_pixels += bpp;
			}
		}
	}

	auto ogl_format = GLenum{};
	auto ogl_internal_format = GLenum{};

	switch (sdl_surface->format->BitsPerPixel)
	{
	case 24:
		ogl_format = (is_convert ? GL_RGB : GL_BGR_EXT);
		ogl_internal_format = GL_RGB;
		break;

	case 32:
		ogl_format = (is_convert ? GL_RGBA : GL_BGRA_EXT);
		ogl_internal_format = GL_RGBA;
		break;

	default:
		break;
	}

	::glGenTextures(1, &result.ogl_id_);
	::glBindTexture(GL_TEXTURE_2D, result.ogl_id_);

	::glTexImage2D(
		GL_TEXTURE_2D,
		0,
		ogl_internal_format,
		is_convert ? dst_width : sdl_surface->w,
		is_convert ? dst_height : sdl_surface->h,
		0,
		ogl_format,
		GL_UNSIGNED_BYTE,
		is_convert ? buffer.data() : sdl_surface->pixels);

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	const auto u_offset = 0.5F / static_cast<float>(dst_width);
	const auto v_offset = 0.5F / static_cast<float>(dst_height);

	result.uv0_ = {u_offset, v_offset};

	result.uv1_ =
	{
		(static_cast<float>(sdl_surface->w) / static_cast<float>(dst_width)) - u_offset,
		(static_cast<float>(sdl_surface->h) / static_cast<float>(dst_height)) - v_offset,
	};

	return result;
}

void OglTextureManager::unload_font()
{
	if (!ogl_font_texture_.ogl_id_)
	{
		return;
	}

	::glDeleteTextures(1, &ogl_font_texture_.ogl_id_);
	ogl_font_texture_.ogl_id_ = 0;

	ogl_font_texture_.uv0_ = {};
	ogl_font_texture_.uv1_ = {};
}

bool OglTextureManager::initialize_context()
{
	auto sdl = Sdl{};

	auto window_param = SdlCreateWindowParam{};
	window_param.title_ = "dummy";
	window_param.width_ = 64;
	window_param.height_ = 64;
	window_param.is_hidden_ = true;
	window_param.is_double_buffering_ = true;
	window_param.is_share_with_current_ogl_context_ = false;

	sdl_ogl_window_ = sdl.ogl_create_window(window_param);

	if (!sdl_ogl_window_.is_valid())
	{
		set_error_message("Failed to create dummy window. " + sdl.get_error_message());
		return false;
	}

	return true;
}

void OglTextureManager::uninitialize_context()
{
	auto sdl = Sdl{};

	sdl.ogl_destroy_window(sdl_ogl_window_);
}

int OglTextureManager::ogl_calculate_npot_dimension(
	const int dimension)
{
	auto power = 0;

	while ((1 << power) < dimension)
	{
		power += 1;
	}

	return 1 << power;
}

//
// OglTextureManager
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
	sdl_ogl_window_{},
	im_context_{},
	pressed_mouse_buttons_{},
	time_{}
{
}

Window::~Window()
{
	uninitialize();
}

bool Window::initialize(
	const WindowCreateParam& param)
{
	auto& ogl_texture_manager = OglTextureManager::get_instance();

	ogl_texture_manager.make_context_current(true);

	auto is_succeed = true;

	if (is_succeed)
	{
		auto window_param = SdlCreateWindowParam{};
		window_param.title_ = param.title_;
		window_param.width_ = param.width_;
		window_param.height_ = param.height_;
		window_param.is_hidden_ = true;
		window_param.is_borderless_ = true;
		window_param.is_double_buffering_ = true;
		window_param.is_share_with_current_ogl_context_ = true;

		auto sdl = Sdl{};

		sdl_ogl_window_ = sdl.ogl_create_window(window_param);

		if (!sdl_ogl_window_.is_valid())
		{
			is_succeed = false;
			set_error_message("Failed to create window. " + sdl.get_error_message());
		}
	}

	auto& font_manager = FontManager::get_instance();
	auto& im_font_atlas = font_manager.get_font_atlas();

	if (is_succeed)
	{
		if (im_font_atlas.Fonts.empty())
		{
			is_succeed = false;
			set_error_message("No font atlas.");
		}
	}

	if (is_succeed)
	{
		im_context_ = ImGui::CreateContext(&im_font_atlas);

		if (!im_context_)
		{
			is_succeed = false;
			set_error_message("Failed to create ImGUI context.");
		}
	}

	if (is_succeed)
	{
		ImGui::SetCurrentContext(im_context_);

		auto& im_io = ImGui::GetIO();

		im_io.IniFilename = nullptr;

		auto& im_style = ImGui::GetStyle();
		im_style.ChildRounding = 0.0F;
		im_style.FrameBorderSize = 0.0F;
		im_style.FramePadding = {};
		im_style.FrameRounding = 0.0F;
		im_style.GrabRounding = 0.0F;
		im_style.ItemInnerSpacing = {};
		im_style.ItemSpacing = {};
		im_style.PopupRounding = 0.0F;
		im_style.ScrollbarRounding = 0.0F;
		im_style.TouchExtraPadding = {};
		im_style.WindowBorderSize = 0.0F;
		im_style.WindowPadding = {};
		im_style.WindowRounding = 0.0F;
	}

	if (is_succeed)
	{
		auto w = 0;
		auto h = 0;

		::SDL_GetWindowSize(sdl_ogl_window_.window_, &w, &h);

		auto display_w = 0;
		auto display_h = 0;

		::SDL_GL_GetDrawableSize(sdl_ogl_window_.window_, &display_w, &display_h);

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

		if (!param.is_hidden_)
		{
			::SDL_ShowWindow(sdl_ogl_window_.window_);
		}
	}

	if (!is_succeed)
	{
		return false;
	}

	auto& window_mananger = WindowManager::get_instance();

	window_mananger.register_window(sdl_ogl_window_.window_id_, this);

	is_initialized_ = true;

	return true;
}

bool Window::is_initialized() const
{
	return is_initialized_;
}

void Window::show(
	const bool is_show)
{
	if (!is_initialized_ || !sdl_ogl_window_.is_valid())
	{
		return;
	}

	if (is_show)
	{
		::SDL_ShowWindow(sdl_ogl_window_.window_);
	}
	else
	{
		::SDL_HideWindow(sdl_ogl_window_.window_);
	}
}

bool Window::is_show() const
{
	if (!is_initialized_ || !sdl_ogl_window_.is_valid())
	{
		return false;
	}

	const auto sdl_window_flags = ::SDL_GetWindowFlags(sdl_ogl_window_.window_);

	return (sdl_window_flags & SDL_WINDOW_SHOWN) != 0;
}

void Window::draw()
{
	if (!is_initialized_)
	{
		return;
	}


	sdl_ogl_window_.make_context_current(true);

	ImGui::SetCurrentContext(im_context_);

	auto& im_io = ImGui::GetIO();

	auto& ogl_texture_manager = OglTextureManager::get_instance();
	const auto& font_texture = ogl_texture_manager.get_font();

	im_io.Fonts->TexID = font_texture.get_im_texture_id();

	im_new_frame();

	ImGui::NewFrame();

	do_draw();

	im_render();

	ImGui::SetCurrentContext(nullptr);
	sdl_ogl_window_.make_context_current(false);
}

void Window::uninitialize()
{
	if (im_context_)
	{
		ImGui::DestroyContext(im_context_);
		im_context_ = nullptr;
	}

	if (sdl_ogl_window_.is_valid())
	{
		auto& window_manager = WindowManager::get_instance();
		window_manager.unregister_window(sdl_ogl_window_.window_id_);

		auto sdl = Sdl{};
		sdl.ogl_destroy_window(sdl_ogl_window_);
	}

	is_initialized_ = false;

	pressed_mouse_buttons_ = {};
}

void Window::handle_event(
	const SDL_Event& sdl_event)
{
	if (!is_initialized_)
	{
		return;
	}

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

void Window::minimize_internal(
	const bool is_minimize)
{
	if (is_minimize)
	{
		::SDL_MinimizeWindow(sdl_ogl_window_.window_);
	}
	else
	{
		::SDL_RestoreWindow(sdl_ogl_window_.window_);
	}
}

int Window::ogl_calculate_npot_dimension(
	const int dimension)
{
	auto power = 0;

	while ((1 << power) < dimension)
	{
		power += 1;
	}

	return 1 << power;
}

bool Window::is_point_inside_rect(
	const ImVec2& point,
	const ImVec4& rect)
{
	return
		point.x >= rect.x && point.x < (rect.x + rect.z) &&
		point.y >= rect.y && point.y < (rect.y + rect.w);
}

Uint32 Window::get_id() const
{
	return sdl_ogl_window_.window_id_;
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

	::SDL_GetWindowSize(sdl_ogl_window_.window_, &w, &h);

	auto display_w = 0;
	auto display_h = 0;

	::SDL_GL_GetDrawableSize(sdl_ogl_window_.window_, &display_w, &display_h);

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
		::SDL_WarpMouseInWindow(sdl_ogl_window_.window_, static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y));
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

	if (sdl_ogl_window_.window_ == focused_window)
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

	::SDL_GL_SwapWindow(sdl_ogl_window_.window_);
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
// MainWindow
//

MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{
	uninitialize();
}

MainWindowPtr MainWindow::create()
{
	auto im_demo_window_ptr = new MainWindow();

	const auto& window_manager = WindowManager::get_instance();
	const auto scale = window_manager.get_scale();

	auto im_demo_window_param = WindowCreateParam{};
	im_demo_window_param.title_ = "main";
	im_demo_window_param.width_ = static_cast<int>(window_width * scale);
	im_demo_window_param.height_ = static_cast<int>(window_height * scale);

	static_cast<void>(im_demo_window_ptr->initialize(im_demo_window_param));

	return im_demo_window_ptr;
}

bool MainWindow::initialize(
	const WindowCreateParam& param)
{
	if (!Window::initialize(param))
	{
		return false;
	}

	return true;
}

void MainWindow::uninitialize()
{
}

void MainWindow::do_draw()
{
	const auto& window_manager = WindowManager::get_instance();
	const auto scale = window_manager.get_scale();

	auto& ogl_texture_manager = OglTextureManager::get_instance();

	const auto is_mouse_button_down = ImGui::IsMouseDown(0);
	const auto is_mouse_button_up = ImGui::IsMouseReleased(0);

	const auto mouse_pos = ImGui::GetMousePos();

	const auto minimize_pos = ImVec2{487.0F, 6.0F} * scale;
	const auto minimize_size = ImVec2{16.0F, 14.0F} * scale;
	const auto minimize_rect = ImVec4{minimize_pos.x, minimize_pos.y, minimize_size.x, minimize_size.y};

	const auto close_pos = ImVec2{503.0F, 6.0F} * scale;
	const auto close_size = ImVec2{16.0F, 14.0F} * scale;
	const auto close_rect = ImVec4{close_pos.x, close_pos.y, close_size.x, close_size.y};

	const auto publisher1web_pos = ImVec2{14.0F, 187.0F} * scale;
	const auto publisher1web_size = ImVec2{52.0F, 40.0F} * scale;
	const auto publisher1web_rect = ImVec4{publisher1web_pos.x, publisher1web_pos.y, publisher1web_size.x, publisher1web_size.y};

	const auto company1web_pos = ImVec2{76.0F, 187.0F} * scale;
	const auto company1web_size = ImVec2{61.0F, 17.0F} * scale;
	const auto company1web_rect = ImVec4{company1web_pos.x, company1web_pos.y, company1web_size.x, company1web_size.y};

	const auto company2web_pos = ImVec2{76.0F, 210.0F} * scale;
	const auto company2web_size = ImVec2{62.0F, 17.0F} * scale;
	const auto company2web_rect = ImVec4{company2web_pos.x, company2web_pos.y, company2web_size.x, company2web_size.y};

	const auto publisher2web_pos = ImVec2{147.0F, 198.0F} * scale;
	const auto publisher2web_size = ImVec2{99.0F, 30.0F} * scale;
	const auto publisher2web_rect = ImVec4{publisher2web_pos.x, publisher2web_pos.y, publisher2web_size.x, publisher2web_size.y};

	const auto install_or_play_pos = ImVec2{413.0F, 25.0F} * scale;
	const auto install_or_play_size = ImVec2{100.0F, 30.0F} * scale;
	const auto install_or_play_rect = ImVec4{install_or_play_pos.x, install_or_play_pos.y, install_or_play_size.x, install_or_play_size.y};

	const auto display_pos = ImVec2{413.0F, 97.0F} * scale;
	const auto display_size = ImVec2{100.0F, 30.0F} * scale;
	const auto display_rect = ImVec4{display_pos.x, display_pos.y, display_size.x, display_size.y};

	const auto options_pos = ImVec2{413.0F, 133.0F} * scale;
	const auto options_size = ImVec2{100.0F, 30.0F} * scale;
	const auto options_rect = ImVec4{options_pos.x, options_pos.y, options_size.x, options_size.y};

	const auto quit_pos = ImVec2{413.0F, 205.0F} * scale;
	const auto quit_size = ImVec2{100.0F, 30.0F} * scale;
	const auto quit_rect = ImVec4{quit_pos.x, quit_pos.y, quit_size.x, quit_size.y};


	// Begin main window.
	//
	const auto main_flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_None;

	ImGui::Begin("main", nullptr, main_flags);
	ImGui::SetWindowPos({}, ImGuiCond_Always);

	ImGui::SetWindowSize(
		ImVec2{static_cast<float>(window_width), static_cast<float>(window_height)} * scale,
		ImGuiCond_Always);


	// Background.
	//
	ImGui::SetCursorPos(ImVec2{});

	const auto& ogl_mainappbackground_texture = ogl_texture_manager.get(ImageId::mainappbackground);

	ImGui::Image(
		ogl_mainappbackground_texture.get_im_texture_id(),
		ImVec2{static_cast<float>(window_width), static_cast<float>(window_height)} * scale,
		ogl_mainappbackground_texture.uv0_,
		ogl_mainappbackground_texture.uv1_);


	// Minimize button.
	//
	auto is_minimize_mouse_button_down = false;
	auto is_minimize_button_clicked = false;

	if (is_mouse_button_down || is_mouse_button_up)
	{
		if (is_point_inside_rect(mouse_pos, minimize_rect))
		{
			if (is_mouse_button_down)
			{
				is_minimize_mouse_button_down = true;
			}

			if (is_mouse_button_up)
			{
				is_minimize_button_clicked = true;
			}
		}
	}

	const auto ogl_minimize_image_id = (is_minimize_mouse_button_down ? ImageId::minimized : ImageId::minimizeu);
	const auto& ogl_minimize_texture = ogl_texture_manager.get(ogl_minimize_image_id);

	ImGui::SetCursorPos(minimize_pos);

	ImGui::Image(
		ogl_minimize_texture.get_im_texture_id(),
		minimize_size,
		ogl_minimize_texture.uv0_,
		ogl_minimize_texture.uv1_);


	// Close button.
	//
	auto is_close_mouse_button_down = false;
	auto is_close_button_clicked = false;

	if (is_mouse_button_down || is_mouse_button_up)
	{
		if (is_point_inside_rect(mouse_pos, close_rect))
		{
			if (is_mouse_button_down)
			{
				is_close_mouse_button_down = true;
			}

			if (is_mouse_button_up)
			{
				is_close_button_clicked = true;
			}
		}
	}

	const auto ogl_close_image_id = (is_close_mouse_button_down ? ImageId::closed : ImageId::closeu);
	const auto& ogl_close_texture = ogl_texture_manager.get(ogl_close_image_id);

	ImGui::SetCursorPos(close_pos);

	ImGui::Image(
		ogl_close_texture.get_im_texture_id(),
		close_size,
		ogl_close_texture.uv0_,
		ogl_close_texture.uv1_);


	// Fox Interactive button.
	//
	auto is_publisher1web_mouse_button_down = false;
	auto is_publisher1web_button_clicked = false;

	const auto is_publisher1web_button_hightlighted = is_point_inside_rect(mouse_pos, publisher1web_rect);

	if (is_publisher1web_button_hightlighted && (is_mouse_button_down || is_mouse_button_up))
	{
		if (is_mouse_button_down)
		{
			is_publisher1web_mouse_button_down = true;
		}

		if (is_mouse_button_up)
		{
			is_publisher1web_button_clicked = true;
		}
	}

	auto ogl_publisher1web_image_id = ImageId{};

	if (is_publisher1web_mouse_button_down)
	{
		ogl_publisher1web_image_id = ImageId::publisher1webd;
	}
	else if (is_publisher1web_button_hightlighted)
	{
		ogl_publisher1web_image_id = ImageId::publisher1webf;
	}
	else
	{
		ogl_publisher1web_image_id = ImageId::publisher1webu;
	}

	const auto& ogl_publisher1web_texture = ogl_texture_manager.get(ogl_publisher1web_image_id);

	ImGui::SetCursorPos(publisher1web_pos);

	ImGui::Image(
		ogl_publisher1web_texture.get_im_texture_id(),
		publisher1web_size,
		ogl_publisher1web_texture.uv0_,
		ogl_publisher1web_texture.uv1_);


	// Monolith Productions button.
	//
	auto is_company1web_mouse_button_down = false;
	auto is_company1web_button_clicked = false;

	const auto is_company1web_button_hightlighted = is_point_inside_rect(mouse_pos, company1web_rect);

	if (is_company1web_button_hightlighted && (is_mouse_button_down || is_mouse_button_up))
	{
		if (is_mouse_button_down)
		{
			is_company1web_mouse_button_down = true;
		}

		if (is_mouse_button_up)
		{
			is_company1web_button_clicked = true;
		}
	}

	auto ogl_company1web_image_id = ImageId{};

	if (is_company1web_mouse_button_down)
	{
		ogl_company1web_image_id = ImageId::company1webd;
	}
	else if (is_company1web_button_hightlighted)
	{
		ogl_company1web_image_id = ImageId::company1webf;
	}
	else
	{
		ogl_company1web_image_id = ImageId::company1webu;
	}

	const auto& ogl_company1web_texture = ogl_texture_manager.get(ogl_company1web_image_id);

	ImGui::SetCursorPos(company1web_pos);

	ImGui::Image(
		ogl_company1web_texture.get_im_texture_id(),
		company1web_size,
		ogl_company1web_texture.uv0_,
		ogl_company1web_texture.uv1_);


	// LithTech button.
	//
	auto is_company2web_mouse_button_down = false;
	auto is_company2web_button_clicked = false;

	const auto is_company2web_button_hightlighted = is_point_inside_rect(mouse_pos, company2web_rect);

	if (is_company2web_button_hightlighted && (is_mouse_button_down || is_mouse_button_up))
	{
		if (is_mouse_button_down)
		{
			is_company2web_mouse_button_down = true;
		}

		if (is_mouse_button_up)
		{
			is_company2web_button_clicked = true;
		}
	}

	auto ogl_company2web_image_id = ImageId{};

	if (is_company2web_mouse_button_down)
	{
		ogl_company2web_image_id = ImageId::company2webd;
	}
	else if (is_company2web_button_hightlighted)
	{
		ogl_company2web_image_id = ImageId::company2webf;
	}
	else
	{
		ogl_company2web_image_id = ImageId::company2webu;
	}

	const auto& ogl_company2web_texture = ogl_texture_manager.get(ogl_company2web_image_id);

	ImGui::SetCursorPos(company2web_pos);

	ImGui::Image(
		ogl_company2web_texture.get_im_texture_id(),
		company2web_size,
		ogl_company2web_texture.uv0_,
		ogl_company2web_texture.uv1_);


	// Sierra Entertainment button.
	//
	auto is_publisher2web_mouse_button_down = false;
	auto is_publisher2web_button_clicked = false;

	const auto is_publisher2web_button_hightlighted = is_point_inside_rect(mouse_pos, publisher2web_rect);

	if (is_publisher2web_button_hightlighted && (is_mouse_button_down || is_mouse_button_up))
	{
		if (is_mouse_button_down)
		{
			is_publisher2web_mouse_button_down = true;
		}

		if (is_mouse_button_up)
		{
			is_publisher2web_button_clicked = true;
		}
	}

	auto ogl_publisher2web_image_id = ImageId{};

	if (is_publisher2web_mouse_button_down)
	{
		ogl_publisher2web_image_id = ImageId::publisher2webd;
	}
	else if (is_publisher2web_button_hightlighted)
	{
		ogl_publisher2web_image_id = ImageId::publisher2webf;
	}
	else
	{
		ogl_publisher2web_image_id = ImageId::publisher2webu;
	}

	const auto& ogl_publisher2web_texture = ogl_texture_manager.get(ogl_publisher2web_image_id);

	ImGui::SetCursorPos(publisher2web_pos);

	ImGui::Image(
		ogl_publisher2web_texture.get_im_texture_id(),
		publisher2web_size,
		ogl_publisher2web_texture.uv0_,
		ogl_publisher2web_texture.uv1_);


	// Install/Play button.
	//

	// TODO
	auto is_install = false;

	auto is_install_or_play_mouse_button_down = false;
	auto is_install_or_play_button_clicked = false;

	const auto is_install_or_play_button_hightlighted = is_point_inside_rect(mouse_pos, install_or_play_rect);

	if (is_install_or_play_button_hightlighted && (is_mouse_button_down || is_mouse_button_up))
	{
		if (is_mouse_button_down)
		{
			is_install_or_play_mouse_button_down = true;
		}

		if (is_mouse_button_up)
		{
			is_install_or_play_button_clicked = true;
		}
	}

	auto ogl_install_or_play_image_id = ImageId{};

	if (is_install_or_play_mouse_button_down)
	{
		ogl_install_or_play_image_id = (is_install ? ImageId::installd : ImageId::playd);
	}
	else if (is_install_or_play_button_hightlighted)
	{
		ogl_install_or_play_image_id = (is_install ? ImageId::installf : ImageId::playf);
	}
	else
	{
		ogl_install_or_play_image_id = (is_install ? ImageId::installu : ImageId::playu);
	}

	const auto& ogl_install_or_play_texture = ogl_texture_manager.get(ogl_install_or_play_image_id);

	ImGui::SetCursorPos(install_or_play_pos);

	ImGui::Image(
		ogl_install_or_play_texture.get_im_texture_id(),
		install_or_play_size,
		ogl_install_or_play_texture.uv0_,
		ogl_install_or_play_texture.uv1_);


	// Display button.
	//

	// TODO
	auto is_display_enabled = false;
	auto is_display_mouse_button_down = false;
	auto is_display_button_clicked = false;

	auto is_display_button_hightlighted = false;

	if (is_display_enabled)
	{
		is_display_button_hightlighted = is_point_inside_rect(mouse_pos, display_rect);
	}

	if (is_display_button_hightlighted && (is_mouse_button_down || is_mouse_button_up))
	{
		if (is_mouse_button_down)
		{
			is_display_mouse_button_down = true;
		}

		if (is_mouse_button_up)
		{
			is_display_button_clicked = true;
		}
	}

	auto ogl_display_image_id = ImageId{};

	if (!is_display_enabled)
	{
		ogl_display_image_id = ImageId::displayx;
	}
	else if (is_display_mouse_button_down)
	{
		ogl_display_image_id = ImageId::displayd;
	}
	else if (is_display_button_hightlighted)
	{
		ogl_display_image_id = ImageId::displayf;
	}
	else
	{
		ogl_display_image_id = ImageId::displayu;
	}

	const auto& ogl_display_texture = ogl_texture_manager.get(ogl_display_image_id);

	ImGui::SetCursorPos(display_pos);

	ImGui::Image(
		ogl_display_texture.get_im_texture_id(),
		display_size,
		ogl_display_texture.uv0_,
		ogl_display_texture.uv1_);


	// Options button.
	//

	// TODO
	auto is_options_enabled = false;
	auto is_options_mouse_button_down = false;
	auto is_options_button_clicked = false;

	auto is_options_button_hightlighted = false;

	if (is_options_enabled)
	{
		is_options_button_hightlighted = is_point_inside_rect(mouse_pos, options_rect);
	}

	if (is_options_button_hightlighted && (is_mouse_button_down || is_mouse_button_up))
	{
		if (is_mouse_button_down)
		{
			is_options_mouse_button_down = true;
		}

		if (is_mouse_button_up)
		{
			is_options_button_clicked = true;
		}
	}

	auto ogl_options_image_id = ImageId{};

	if (!is_options_enabled)
	{
		ogl_options_image_id = ImageId::optionsx;
	}
	else if (is_options_mouse_button_down)
	{
		ogl_options_image_id = ImageId::optionsd;
	}
	else if (is_options_button_hightlighted)
	{
		ogl_options_image_id = ImageId::optionsf;
	}
	else
	{
		ogl_options_image_id = ImageId::optionsu;
	}

	const auto& ogl_options_texture = ogl_texture_manager.get(ogl_options_image_id);

	ImGui::SetCursorPos(options_pos);

	ImGui::Image(
		ogl_options_texture.get_im_texture_id(),
		options_size,
		ogl_options_texture.uv0_,
		ogl_options_texture.uv1_);


	// Quit button.
	//

	auto is_quit_mouse_button_down = false;
	auto is_quit_button_clicked = false;

	const auto is_quit_button_hightlighted = is_point_inside_rect(mouse_pos, quit_rect);

	if (is_quit_button_hightlighted && (is_mouse_button_down || is_mouse_button_up))
	{
		if (is_mouse_button_down)
		{
			is_quit_mouse_button_down = true;
		}

		if (is_mouse_button_up)
		{
			is_quit_button_clicked = true;
		}
	}

	auto ogl_quit_image_id = ImageId{};

	if (is_quit_mouse_button_down)
	{
		ogl_quit_image_id = ImageId::quitd;
	}
	else if (is_quit_button_hightlighted)
	{
		ogl_quit_image_id = ImageId::quitf;
	}
	else
	{
		ogl_quit_image_id = ImageId::quitu;
	}

	const auto& ogl_quit_texture = ogl_texture_manager.get(ogl_quit_image_id);

	ImGui::SetCursorPos(quit_pos);

	ImGui::Image(
		ogl_quit_texture.get_im_texture_id(),
		quit_size,
		ogl_quit_texture.uv0_,
		ogl_quit_texture.uv1_);


	// End main window.
	//
	ImGui::End();


	// Handle events.
	//

	if (is_minimize_button_clicked)
	{
		minimize_internal(true);
	}

	if (is_close_button_clicked || is_quit_button_clicked)
	{
		auto sdl_event = SDL_Event{};

		sdl_event.type = SDL_QUIT;
		::SDL_PushEvent(&sdl_event);
	}
}

//
// MainWindow
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

	auto sdl_display_mode = SDL_DisplayMode{};

	const auto sdl_result = ::SDL_GetCurrentDisplayMode(0, &sdl_display_mode);

	if (sdl_result == 0)
	{
		const auto dimension = static_cast<float>(std::min(sdl_display_mode.w, sdl_display_mode.h));

		scale_ = dimension / ref_scale_dimension;
	}

	if (scale_ < 1.0F)
	{
		scale_ = 1.0F;
	}
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

float WindowManager::get_scale() const
{
	return scale_;
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
	main_window_uptr_{}
{
}

Launcher::Launcher(
	Launcher&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	main_window_uptr_{std::move(rhs.main_window_uptr_)}
{
	rhs.is_initialized_ = false;
}

Launcher::~Launcher()
{
	uninitialize();
}

bool Launcher::initialize()
{
	uninitialize();

	auto is_succeed = true;

	if (is_succeed)
	{
		if (!initialize_sdl())
		{
			is_succeed = false;
		}
	}

	if (is_succeed)
	{
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
	}

	// FontManager
	//
	auto& font_manager = FontManager::get_instance();

	if (is_succeed)
	{
		if (!font_manager.initialize())
		{
			is_succeed = false;

			set_error_message(font_manager.get_error_message());
		}
	}

	if (is_succeed)
	{
		const auto& window_manager = WindowManager::get_instance();
		const auto scale = window_manager.get_scale();

		const auto font_size_pixels = scale * FontManager::ref_font_size_pixels;

		if (!font_manager.set_font(font_size_pixels))
		{
			is_succeed = false;

			set_error_message(font_manager.get_error_message());
		}
	}

	// OglTextureManager
	//
	auto& ogl_texture_manager = OglTextureManager::get_instance();

	if (is_succeed)
	{
		if (!ogl_texture_manager.initialize())
		{
			is_succeed = false;

			set_error_message(ogl_texture_manager.get_error_message());
		}
	}

	if (is_succeed)
	{
		if (!ogl_texture_manager.load_font())
		{
			is_succeed = false;

			set_error_message(ogl_texture_manager.get_error_message());
		}
	}

	if (is_succeed)
	{
		if (!ogl_texture_manager.load_all_textures())
		{
			is_succeed = false;

			set_error_message(ogl_texture_manager.get_error_message());
		}
	}

	if (is_succeed)
	{
		main_window_uptr_.reset(MainWindow::create());

		if (!main_window_uptr_->is_initialized())
		{
			is_succeed = false;
			set_error_message("Failed to initialize main window. " + main_window_uptr_->get_error_message());
		}
	}

	if (!is_succeed)
	{
		uninitialize();
		return false;
	}

	is_initialized_ = true;

	return true;
}

void Launcher::uninitialize()
{
	is_initialized_ = false;


	main_window_uptr_ = {};


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


	// FontManager
	//
	auto& font_manager = FontManager::get_instance();
	font_manager.uninitialize();


	// OglTextureManager
	//
	auto& image_cache = OglTextureManager::get_instance();
	image_cache.uninitialize();


	// SDL
	//
	uninitialize_sdl();
}

bool Launcher::is_initialized()
{
	return is_initialized_;
}

void Launcher::run()
{
	if (!is_initialized_)
	{
		set_error_message("Not initialized.");
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
			set_error_message("Failed to set OpenGL attribute: double buffering. " + std::string{::SDL_GetError()});
		}

		sdl_result = ::SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, SDL_FALSE);

		if (is_succeed && sdl_result)
		{
			is_succeed = false;
			set_error_message("Failed to set OpenGL attribute: share with current context. " + std::string{::SDL_GetError()});
		}
	}

	// Create dummy window.
	//

	auto window_param = SdlCreateWindowParam{};
	window_param.title_ = "dummy";
	window_param.width_ = 64;
	window_param.height_ = 64;
	window_param.is_hidden_ = true;
	window_param.is_double_buffering_ = true;

	auto sdl = Sdl{};
	auto ogl_window = sdl.ogl_create_window(window_param);

	if (!ogl_window.is_valid())
	{
		is_succeed = false;
		set_error_message("Failed to create dummy window. " + sdl.get_error_message());
	}

	ogl_window.make_context_current(true);

	// Initialize GLAD.
	//

	if (is_succeed)
	{
		const auto glad_result = ::gladLoadGLLoader(::SDL_GL_GetProcAddress);

		if (!glad_result)
		{
			is_succeed = false;
			set_error_message("Failed to initialize GLAD.");
		}
	}

	// Clean up.
	//

	sdl.ogl_destroy_window(ogl_window);

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
			set_error_message("Failed to initialize SDL. " + std::string{::SDL_GetError()});
		}
	}

	if (is_succeed)
	{
		sdl_result = ::SDL_GL_LoadLibrary(nullptr);

		if (sdl_result)
		{
			is_succeed = false;
			set_error_message("Failed to load OpenGL library. " + std::string{::SDL_GetError()});
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


ImVec2 operator*(
	const ImVec2& v,
	const float scale)
{
	return {v.x * scale, v.y * scale};
}

ImVec2& operator*=(
	ImVec2& v,
	const float scale)
{
	v.x *= scale;
	v.y *= scale;

	return v;
}


int main(
	int,
	char**)
{
	auto launcher = Launcher{};

	if (!launcher.initialize())
	{
		auto error_message = launcher.get_error_message();

		if (error_message.empty())
		{
			error_message = "Generic failure.";
		}

		::SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"Launcher",
			error_message.c_str(),
			nullptr);

		return 1;
	}

	launcher.run();

	return 0;
}
