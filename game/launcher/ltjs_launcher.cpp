#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "SDL3/SDL_main.h"
#include "ltjs_language_mgr.h"
#include "ltjs_script_tokenizer.h"
#include "ltjs_launcher_resource_strings.h"
#include "ltjs_launcher_search_paths.h"
#include "ltjs_launcher_utility.h"
#include "ltjs_logger.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <charconv>
#include <exception>
#include <format>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(LTJS_NOLF2)
#define LTJS_GAME_ID_STRING "nolf2"
#define LTJS_GAME_ID_STRING_UC "NOLF2"
#else // LTJS_NOLF2
#error Unsupported game.
#endif // LTJS_NOLF2

namespace ltjs::launcher {

namespace {

ImVec2 operator*(const ImVec2& v, float scale);
ImVec2& operator*=(ImVec2& v, float scale);

// ======================================

[[noreturn]] void fail(const std::string& error_message)
{
	throw std::runtime_error{error_message};
}

template<typename... TArgs>
[[noreturn]] void fail(std::format_string<TArgs...> format_string, TArgs... args)
{
	std::string error_message{};
	error_message.reserve(1024);
	std::vformat_to(std::back_inserter(error_message), format_string.get(), std::make_format_args(args...));
	fail(error_message);
}

// ======================================

[[noreturn]] void fail_sdl_function(const std::string& sdl_function_name)
{
	fail("[{}] {}", sdl_function_name, SDL_GetError());
}

// ======================================

void ensure_sdl_bool_result(const std::string& sdl_function_name, bool result)
{
	if (!result)
	{
		fail_sdl_function(sdl_function_name);
	}
}

// ======================================

class ImguiContextUPtr
{
public:
	ImguiContextUPtr() = default;
	explicit ImguiContextUPtr(ImGuiContext* imgui_context);
	void operator=(std::nullptr_t);
	ImguiContextUPtr(ImguiContextUPtr&& rhs) noexcept;
	ImguiContextUPtr& operator=(ImguiContextUPtr&&) noexcept = delete;
	~ImguiContextUPtr();

	ImGuiContext* get() const;
	void reset();
	void reset(ImGuiContext* imgui_context);
	ImGuiContext* operator->() const;

private:
	ImGuiContext* imgui_context_{};
};

// --------------------------------------

ImguiContextUPtr::ImguiContextUPtr(ImGuiContext* imgui_context)
	:
	imgui_context_{imgui_context}
{}

void ImguiContextUPtr::operator=(std::nullptr_t)
{
	reset();
}

ImguiContextUPtr::ImguiContextUPtr(ImguiContextUPtr&& rhs) noexcept
	:
	imgui_context_{rhs.imgui_context_}
{
	rhs.imgui_context_ = nullptr;
}

ImguiContextUPtr::~ImguiContextUPtr()
{
	reset();
}

ImGuiContext* ImguiContextUPtr::get() const
{
	return imgui_context_;
}

void ImguiContextUPtr::reset()
{
	reset(nullptr);
}

void ImguiContextUPtr::reset(ImGuiContext* imgui_context)
{
	if (get() != nullptr)
	{
		ImGui::SetCurrentContext(get());
		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext(get());
	}
	imgui_context_ = imgui_context;
}

ImGuiContext* ImguiContextUPtr::operator->() const
{
	return get();
}

// ======================================

bool operator==(const ImguiContextUPtr& a, std::nullptr_t);
bool operator==(std::nullptr_t, const ImguiContextUPtr& b);

bool operator!=(const ImguiContextUPtr& a, std::nullptr_t);
bool operator!=(std::nullptr_t, const ImguiContextUPtr& b);

// --------------------------------------

bool operator==(const ImguiContextUPtr& a, std::nullptr_t)
{
	return a.get() == nullptr;
}

bool operator==(std::nullptr_t, const ImguiContextUPtr& b)
{
	return b.get() == nullptr;
}

bool operator!=(const ImguiContextUPtr& a, std::nullptr_t)
{
	return !(a == nullptr);
}

bool operator!=(std::nullptr_t, const ImguiContextUPtr& b)
{
	return !(b == nullptr);
}

// ======================================

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
};

enum class MessageBoxType
{
	information,
	warning,
	error,
};

enum class MessageBoxButtons
{
	ok,
	ok_cancel,
};

enum class DialogResult
{
	none = 0,
	ok,
	cancel,
};

enum class DetailLevel
{
	none = 0,
	low,
	medium,
	high,
};

struct ResourceStringId
{
	static constexpr int ids_appname = 1;
	static constexpr int ids_appexe = 2;
	static constexpr int ids_display_warning = 3;
	static constexpr int ids_options_warning = 4;
	static constexpr int ids_appcd1check = 5;
	static constexpr int ids_appcd2check = 6;
	static constexpr int ids_rezbase = 7;
	static constexpr int ids_setupexe = 8;
	static constexpr int ids_serverexe = 9;
	static constexpr int ids_language = 10;
	static constexpr int ids_insertcd2 = 11;
	static constexpr int ids_insertcd = 12;
	static constexpr int ids_cantlaunchsetup = 13;
	static constexpr int ids_norens = 14;
	static constexpr int ids_help_disablesound = 15;
	static constexpr int ids_help_disablemusic = 16;
	static constexpr int ids_help_disablemovies = 17;
	static constexpr int ids_help_disablefog = 18;
	static constexpr int ids_help_disablejoysticks = 19;
	static constexpr int ids_help_disabletriplebuffering = 20;
	static constexpr int ids_help_disablehardwarecursor = 21;
	static constexpr int ids_help_disableanimatedloadscreen = 22;
	static constexpr int ids_help_restoredefaults = 23;
	static constexpr int ids_help_alwaysspecify = 24;
	static constexpr int ids_cantfindrezfile = 25;
	static constexpr int ids_cantlaunchclientexe = 26;
	static constexpr int ids_detail_header = 27;
	static constexpr int ids_detail_low = 28;
	static constexpr int ids_detail_medium = 29;
	static constexpr int ids_detail_high = 30;
	static constexpr int ids_cantlaunchserver = 31;
	static constexpr int ids_appversion = 32;
	static constexpr int ids_cantuninstall = 33;
	static constexpr int ids_companywebpage = 34;
	static constexpr int ids_cantopenavi = 35;
	static constexpr int ids_publisherwebpage = 36;
	static constexpr int ids_od_disablesound = 37;
	static constexpr int ids_od_disablemusic = 38;
	static constexpr int ids_od_disablemovies = 39;
	static constexpr int ids_od_disablefog = 40;
	static constexpr int ids_appname_demo = 40;
	static constexpr int ids_od_disablejoysticks = 41;
	static constexpr int ids_od_disabletriplebuffering = 42;
	static constexpr int ids_od_disablehardwarecursor = 43;
	static constexpr int ids_od_disableanimatedloadscreens = 44;
	static constexpr int ids_od_restoredefaults = 45;
	static constexpr int ids_od_alwaysspecify = 46;
	static constexpr int ids_debug_regcreateerror = 47;
	static constexpr int ids_debug_installsuccess = 48;
	static constexpr int ids_debug_uninstallsuccess = 49;
	static constexpr int ids_launchbrowsererror = 50;
	static constexpr int ids_help_default = 51;
	static constexpr int ids_help_disablehardwaresound = 52;
	static constexpr int ids_help_disablesoundfilters = 53;
	static constexpr int ids_od_disablehardwaresound = 54;
	static constexpr int ids_od_disablesoundfilters = 55;
	static constexpr int ids_cantopencommandfile = 56;
	static constexpr int ids_lithtechwebpage = 57;
	static constexpr int ids_sierrawebpage = 58;
	static constexpr int ids_nocustomdir = 59;
};

// ======================================

template<typename T>
class SettingValue
{
public:
	SettingValue() = default;

	explicit SettingValue(const T& value)
		:
		accepted_value_{value},
		current_value_{value}
	{}

	SettingValue(const SettingValue& that)
		:
		accepted_value_{that.accepted_value_},
		current_value_{that.current_value_}
	{}

	SettingValue& operator=(const T& value)
	{
		current_value_ = value;
		return *this;
	}

	T& get_ref()
	{
		return current_value_;
	}

	const T& get_ref() const
	{
		return current_value_;
	}

	T* get_ptr()
	{
		return &current_value_;
	}

	const T* get_ptr() const
	{
		return &current_value_;
	}

	void accept()
	{
		accepted_value_ = current_value_;
	}

	void reject()
	{
		current_value_ = accepted_value_;
	}

	void set_and_accept(const T& value)
	{
		accepted_value_ = value;
		current_value_ = value;
	}

	operator T&()
	{
		return get_ref();
	}

	operator const T&() const
	{
		return get_ref();
	}

	operator T*()
	{
		return get_ptr();
	}

	operator const T*() const
	{
		return get_ptr();
	}

private:
	T accepted_value_{};
	T current_value_{};
};

// ======================================

class Direct3d9
{
public:
	static bool has_direct3d9();
	static const std::string& get_renderer_name();
	static const std::string& get_display_name();

private:
	static const bool has_direct3d9_;

	static bool check_for_direct3d9();
};

// --------------------------------------

const bool Direct3d9::has_direct3d9_ = check_for_direct3d9();

// --------------------------------------

bool Direct3d9::has_direct3d9()
{
	return has_direct3d9_;
}

const std::string& Direct3d9::get_renderer_name()
{
	constinit static const std::string renderer_name{"Direct3D 9"};
	return renderer_name;
}

const std::string& Direct3d9::get_display_name()
{
	constinit static const std::string display_name{"Default"};
	return display_name;
}

bool Direct3d9::check_for_direct3d9()
{
#ifdef _WIN32
	if (const SdlSharedObjectUPtr d3d9_dll_uptr{SDL_LoadObject("d3d9.dll")};
		d3d9_dll_uptr != nullptr)
	{
		if (const SDL_FunctionPointer func = SDL_LoadFunction(d3d9_dll_uptr.get(), "Direct3DCreate9");
			func != nullptr)
		{
			return true;
		}
	}
#endif // _WIN32
	return false;
}

// ======================================

struct DisplayMode
{
	int width;
	int height;
	std::string as_string;
};

// ======================================

class DisplayModeMgr
{
public:
	static constexpr int min_display_mode_width = 640;
	static constexpr int min_display_mode_height = 480;

	using DisplayModes = std::vector<DisplayMode>;

	static const DisplayModeMgr& get_singleton();
	int get_mode_count() const;
	std::span<const DisplayMode> get_modes() const;
	const DisplayMode& get_mode_by_index(int index) const;
	const DisplayMode& get_native_mode() const;
	int find_mode_index(int width, int height) const;
	int get_native_display_mode_index() const;

private:
	DisplayModes display_modes_{};
	DisplayMode native_display_mode_{};
	int native_display_mode_index_{};

	DisplayModeMgr();
	~DisplayModeMgr() = default;

	static bool is_sdl_pixel_format_valid(SDL_PixelFormat sdl_pixel_format);
};

// --------------------------------------

const DisplayModeMgr& DisplayModeMgr::get_singleton()
{
	static const DisplayModeMgr display_mode_mgr{};
	return display_mode_mgr;
}

int DisplayModeMgr::get_mode_count() const
{
	return narrow_cast<int>(display_modes_.size());
}

std::span<const DisplayMode> DisplayModeMgr::get_modes() const
{
	return display_modes_;
}

const DisplayMode& DisplayModeMgr::get_mode_by_index(int index) const
{
	return display_modes_[index];
}

const DisplayMode& DisplayModeMgr::get_native_mode() const
{
	return native_display_mode_;
}

int DisplayModeMgr::find_mode_index(int width, int height) const
{
	if (width <= 0 || height <= 0)
	{
		return -1;
	}
	const auto display_mode_begin_iter = display_modes_.cbegin();
	const auto display_mode_end_iter = display_modes_.cend();
	const auto display_mode_iter = std::find_if(
		display_mode_begin_iter,
		display_mode_end_iter,
		[width, height](const DisplayMode& item)
		{
			return item.width == width && item.height == height;
		}
	);
	if (display_mode_iter == display_mode_end_iter)
	{
		return -1;
	}
	const int index = narrow_cast<int>(display_mode_iter - display_mode_begin_iter);
	return index;
}

int DisplayModeMgr::get_native_display_mode_index() const
{
	return native_display_mode_index_;
}

DisplayModeMgr::DisplayModeMgr()
{
	using SdlDisplayModeListUPtr = std::unique_ptr<SDL_DisplayMode*, SdlRawDeleter>;
	const SDL_DisplayID sdl_primary_display_id = SDL_GetPrimaryDisplay();
	if (sdl_primary_display_id == 0)
	{
		return;
	}
	int mode_count = 0;
	SDL_DisplayMode** const sdl_display_mode_list = SDL_GetFullscreenDisplayModes(sdl_primary_display_id, &mode_count);
	if (sdl_display_mode_list == nullptr || mode_count == 0)
	{
		return;
	}
	SdlDisplayModeListUPtr display_mode_list{sdl_display_mode_list};
	DisplayMode native_display_mode{};
	const SDL_DisplayMode* const sdl_native_display_mode = SDL_GetCurrentDisplayMode(sdl_primary_display_id);
	if (sdl_native_display_mode != nullptr)
	{
		if (!is_sdl_pixel_format_valid(sdl_native_display_mode->format))
		{
			return;
		}
		native_display_mode.width = sdl_native_display_mode->w;
		native_display_mode.height = sdl_native_display_mode->h;
	}
	DisplayModes display_modes{};
	display_modes.reserve(mode_count);
	int native_display_mode_index = -1;
	for (int i = 0; i < mode_count; ++i)
	{
		const SDL_DisplayMode& sdl_display_mode = *(sdl_display_mode_list[i]);
		if (!is_sdl_pixel_format_valid(sdl_display_mode.format))
		{
			continue;
		}
		if (sdl_display_mode.w < min_display_mode_width || sdl_display_mode.h < min_display_mode_height)
		{
			continue;
		}
		if (const auto item_iter = std::find_if(
				display_modes.cbegin(),
				display_modes.cend(),
				[&sdl_display_mode](const DisplayMode& item)
				{
					return item.width == sdl_display_mode.w && item.height == sdl_display_mode.h;
				});
			item_iter != display_modes.cend())
		{
			continue;
		}
		display_modes.emplace_back(DisplayMode{
			.width = sdl_display_mode.w,
			.height = sdl_display_mode.h,
			.as_string = std::format("{} x {}", sdl_display_mode.w, sdl_display_mode.h),
		});
	}
	const auto native_display_mode_begin_iter = display_modes.cbegin();
	const auto native_display_mode_end_iter = display_modes.cend();
	const auto native_display_mode_iter = std::find_if(
		native_display_mode_begin_iter,
		native_display_mode_end_iter,
		[&native_display_mode](const DisplayMode& item)
		{
			return
				native_display_mode.width == item.width &&
				native_display_mode.height == item.height;
		}
	);
	if (native_display_mode_iter != native_display_mode_end_iter)
	{
		native_display_mode_index = narrow_cast<int>(native_display_mode_iter - native_display_mode_begin_iter);
	}
	native_display_mode_ = native_display_mode;
	display_modes_ = display_modes;
	native_display_mode_index_ = native_display_mode_index;
}

bool DisplayModeMgr::is_sdl_pixel_format_valid(SDL_PixelFormat sdl_pixel_format)
{
	return
		(SDL_BITSPERPIXEL(sdl_pixel_format) == 24 || SDL_BITSPERPIXEL(sdl_pixel_format) == 32) &&
		!SDL_ISPIXELFORMAT_INDEXED(sdl_pixel_format) ||
		!SDL_ISPIXELFORMAT_ALPHA(sdl_pixel_format);
}

// ======================================

class Configuration
{
public:
	static constexpr int max_file_size = 4 * 1024;

	SettingValue<std::string> language{};
	SettingValue<bool> is_disable_display_settings_warning{};
	SettingValue<bool> is_disable_advanced_settings_warning{};
	SettingValue<bool> is_disable_sound_effects{};
	SettingValue<bool> is_disable_music{};
	SettingValue<bool> is_disable_fmvs{};
	SettingValue<bool> is_disable_controllers{};
	SettingValue<bool> is_disable_triple_buffering{};
	SettingValue<bool> is_disable_hardware_cursor{};
	SettingValue<bool> is_disable_animated_loading_screen{};
	SettingValue<bool> is_disable_hardware_sound{};
	SettingValue<bool> is_disable_sound_filters{};
	SettingValue<bool> is_pass_custom_arguments{};
	SettingValue<std::string> custom_arguments{};
	SettingValue<int> screen_width{};
	SettingValue<int> screen_height{};
	SettingValue<bool> is_restore_defaults{}; // Not serializable.

	Configuration();
	~Configuration() = default;
	static Configuration& get_singleton();
	ltjs::LanguageMgr* get_language_mgr() const;
	static const std::string& get_base_path();
	static const std::string& get_game_base_path();
	static const std::string& get_resources_base_path();
	static const std::string& get_config_path();
	const std::string& get_arguments_file_name() const;
	const std::string& get_log_file_name() const;
	void reset();
	void reload();
	void save();

private:
	static inline const std::string configuration_file_name = "config.txt";
	static inline const std::string arguments_file_name = "arguments.txt";
	static inline const std::string log_file_name = "log.txt";

	static inline const std::string default_language = "en";
	static inline const bool default_is_warned_about_display = false;
	static inline const bool default_is_disable_advanced_settings_warning = false;
	static inline const bool default_is_disable_sound_effects = false;
	static inline const bool default_is_disable_music = false;
	static inline const bool default_is_disable_fmvs = false;
	static inline const bool default_is_disable_controllers = false;
	static inline const bool default_is_disable_triple_buffering = true;
	static inline const bool default_is_disable_hardware_cursor = false;
	static inline const bool default_is_disable_animated_loading_screen = false;
	static inline const bool default_is_detail_level_selected = false;
	static inline const bool default_is_disable_hardware_sound = false;
	static inline const bool default_is_disable_sound_filters = false;
	static inline const bool default_is_pass_custom_arguments = false;
	static inline const std::string default_custom_arguments{};
	static inline const int default_screen_width = 0;
	static inline const int default_screen_height = 0;

	static inline const std::string language_setting_name = "language";
	static inline const std::string is_disable_display_settings_warning_setting_name = "disable_display_settings_warning";
	static inline const std::string is_disable_advanced_settings_warning_setting_name = "disable_advanced_settings_warning";
	static inline const std::string is_disable_sound_effects_setting_name = "disable_sound_effects";
	static inline const std::string is_disable_music_setting_name = "disable_music";
	static inline const std::string is_disable_fmvs_setting_name = "disable_fmvs";
	static inline const std::string is_disable_controllers_setting_name = "disable_controllers";
	static inline const std::string is_disable_triple_buffering_setting_name = "disable_triple_buffering";
	static inline const std::string is_disable_hardware_cursor_setting_name = "disable_hardware_cursor";
	static inline const std::string is_disable_animated_loading_screen_setting_name = "disable_animated_loading_screen";
	static inline const std::string is_disable_hardware_sound_setting_name = "disable_hardware_sound";
	static inline const std::string is_disable_sound_filters_setting_name = "disable_sound_filters";
	static inline const std::string is_pass_custom_arguments_setting_name = "pass_custom_arguments";
	static inline const std::string custom_arguments_setting_name = "custom_arguments";
	static inline const std::string screen_width_setting_name = "screen_width";
	static inline const std::string screen_height_setting_name = "screen_height";

	std::string configuration_path_{};
	ltjs::LanguageMgrUPtr language_mgr_{};

	void initialize();
	static std::string serialize_cl_args(const std::string& custom_command_line);
	static std::string deserialize_cl_args(const ScriptTokenizerToken& token);
};

// --------------------------------------

Configuration::Configuration()
{
	language_mgr_ = ltjs::make_language_mgr();
	initialize();
}

Configuration& Configuration::get_singleton()
{
	static Configuration configuration{};
	return configuration;
}

void Configuration::initialize()
{
	configuration_path_ = combine_and_normalize_file_paths(get_config_path(), configuration_file_name);
	reset();
}

ltjs::LanguageMgr* Configuration::get_language_mgr() const
{
	return language_mgr_.get();
}

const std::string& Configuration::get_base_path()
{
	constinit static const std::string result{"ltjs"};
	return result;
}

const std::string& Configuration::get_game_base_path()
{
	static const std::string result{get_base_path() + "/" LTJS_GAME_ID_STRING};
	return result;
}

const std::string& Configuration::get_config_path()
{
	static const std::string result = get_resources_base_path() + "/config";
	return result;
}

const std::string& Configuration::get_arguments_file_name() const
{
	return arguments_file_name;
}

const std::string& Configuration::get_log_file_name() const
{
	return log_file_name;
}

const std::string& Configuration::get_resources_base_path()
{
	static const std::string result = get_game_base_path() + "/launcher";
	return result;
}

void Configuration::reload()
{
	const SdlIoStreamUPtr sdl_io_stream_uptr{SDL_IOFromFile(configuration_path_.c_str(), "rb")};
	if (sdl_io_stream_uptr == nullptr)
	{
		return;
	}
	const Sint64 stream_size = SDL_GetIOSize(sdl_io_stream_uptr.get());
	if (stream_size < 0)
	{
		fail("Unknown size. (file_path={}; error_message={})", configuration_path_, SDL_GetError());
	}
	if (stream_size > max_file_size)
	{
		fail("Size too big. (file_size={}; max_file_size={}; file_path={})", stream_size, max_file_size, configuration_path_);
	}
	const int file_size = narrow_cast<int>(stream_size);
	std::vector<char> string_buffer{};
	string_buffer.resize(file_size);
	const std::size_t read_result = SDL_ReadIO(sdl_io_stream_uptr.get(), string_buffer.data(), file_size);
	if (read_result != file_size)
	{
		fail("Failed to read a file. (file_path={}; error_message={})", configuration_path_, SDL_GetError());
	}
	ScriptTokenizer script_tokenizer{};
	ScriptTokenizerInitParam script_tokenizer_init_param{.data = string_buffer.data(), .size = file_size};
	try
	{
		script_tokenizer.initialize(script_tokenizer_init_param);
	}
	catch (const std::exception& exception)
	{
		fail("Invalid configuration. (file_path={}; error_message={})", configuration_path_, exception.what());
	}
	class SettingValueParser
	{
	public:
		static void parse(const std::string& value_string, SettingValue<bool>& setting_value)
		{
			if (const OptionalInt optional_int = parse_int(value_string);
				optional_int.has_value())
			{
				setting_value.set_and_accept(optional_int.value() != 0);
			}
		}

		static void parse(const std::string& value_string, SettingValue<int>& setting_value)
		{
			if (const OptionalInt optional_int = parse_int(value_string);
				optional_int.has_value())
			{
				setting_value.set_and_accept(optional_int.value());
			}
		}

	private:
		using OptionalInt = std::optional<int>;

		static OptionalInt parse_int(const std::string& int_string)
		{
			int value = 0;
			if (const auto [ptr, ec] = std::from_chars(int_string.data(), int_string.data() + int_string.size(), value);
				ec == std::errc{})
			{
				return OptionalInt{value};
			}
			return OptionalInt{};
		};
	};
	ScriptTokenizerToken tokens[2];
	for (;;)
	{
		ScriptTokenizerLine script_line{};
		try
		{
			script_line = script_tokenizer.tokenize_line(tokens, 2, 2, 2);
		}
		catch (const std::exception& exception)
		{
			fail("Failed to tokenize. (file_path={}; error_message={})", configuration_path_, exception.what());
		}
		if (script_line.is_end_of_data)
		{
			break;
		}
		if (script_line.is_empty())
		{
			continue;
		}
		if (script_line.size != 2)
		{
			fail("Expected two tokens. (file_path={}; line_number={})", configuration_path_, script_line.get_line_number());
		}
		const std::string key_string{tokens[0].data, narrow_cast<std::size_t>(tokens[0].size)};
		const std::string value_string{tokens[1].data, narrow_cast<std::size_t>(tokens[1].size)};
		if (false)
		{}
		else if (key_string == language_setting_name)
		{
			language.set_and_accept(value_string);
		}
		else if (key_string == is_disable_display_settings_warning_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_display_settings_warning);
		}
		else if (key_string == is_disable_advanced_settings_warning_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_advanced_settings_warning);
		}
		else if (key_string == is_disable_sound_effects_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_sound_effects);
		}
		else if (key_string == is_disable_music_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_music);
		}
		else if (key_string == is_disable_fmvs_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_fmvs);
		}
		else if (key_string == is_disable_controllers_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_controllers);
		}
		else if (key_string == is_disable_triple_buffering_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_triple_buffering);
		}
		else if (key_string == is_disable_hardware_cursor_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_hardware_cursor);
		}
		else if (key_string == is_disable_animated_loading_screen_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_animated_loading_screen);
		}
		else if (key_string == is_disable_hardware_sound_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_hardware_sound);
		}
		else if (key_string == is_disable_sound_filters_setting_name)
		{
			SettingValueParser::parse(value_string, is_disable_sound_filters);
		}
		else if (key_string == is_pass_custom_arguments_setting_name)
		{
			SettingValueParser::parse(value_string, is_pass_custom_arguments);
		}
		else if (key_string == custom_arguments_setting_name)
		{
			custom_arguments.set_and_accept(deserialize_cl_args(tokens[1]));
		}
		else if (key_string == screen_width_setting_name)
		{
			SettingValueParser::parse(value_string, screen_width);
		}
		else if (key_string == screen_height_setting_name)
		{
			SettingValueParser::parse(value_string, screen_height);
		}
	}
	language_mgr_->load();
	const ltjs::Language* const current_language = language_mgr_->get_current();
	language.set_and_accept(current_language->id_string.data);
}

void Configuration::save()
{
	const SdlIoStreamUPtr sdl_io_stream_uptr{SDL_IOFromFile(configuration_path_.c_str(), "wb")};
	if (sdl_io_stream_uptr == nullptr)
	{
		fail("Failed to open a file. (file_path={}; error_message={})", configuration_path_, SDL_GetError());
	}
	// Commit language changes.
	language.accept();
	language_mgr_->set_current_by_id_string(language.get_ref().c_str());
	language_mgr_->save();
	// Make content.
	class StringBuilder
	{
	public:
		explicit StringBuilder(std::size_t capacity)
		{
			string_.reserve(capacity);
		}

		const std::string& get_string() const
		{
			return string_;
		}

		void append(const std::string& value)
		{
			string_ += value;
		}

		void append(const std::string& key, int value)
		{
			std::format_to(std::back_inserter(string_), "{} {}\n", key, value);
		}

		void append(const std::string& key, const std::string& value)
		{
			std::format_to(std::back_inserter(string_), "{} \"{}\"\n", key, value);
		}

	private:
		std::string string_{};
	};
	StringBuilder string_builder{max_file_size};
	string_builder.append(
		"/*\n"
		"LTJS\n"
		LTJS_GAME_ID_STRING_UC " LAUNCHER CONFIGURATION\n"
		"WARNING This is auto-generated file.\n"
		"*/\n"
		"\n");
	//
	is_disable_display_settings_warning.accept();
	string_builder.append(is_disable_display_settings_warning_setting_name, is_disable_display_settings_warning);
	//
	is_disable_advanced_settings_warning.accept();
	string_builder.append(is_disable_advanced_settings_warning_setting_name, is_disable_advanced_settings_warning);
	//
	is_disable_sound_effects.accept();
	string_builder.append(is_disable_sound_effects_setting_name, is_disable_sound_effects);
	//
	is_disable_music.accept();
	string_builder.append(is_disable_music_setting_name, is_disable_music);
	//
	is_disable_fmvs.accept();
	string_builder.append(is_disable_fmvs_setting_name, is_disable_fmvs);
	//
	is_disable_controllers.accept();
	string_builder.append(is_disable_controllers_setting_name, is_disable_controllers);
	//
	is_disable_triple_buffering.accept();
	string_builder.append(is_disable_triple_buffering_setting_name, is_disable_triple_buffering);
	//
	is_disable_hardware_cursor.accept();
	string_builder.append(is_disable_hardware_cursor_setting_name, is_disable_hardware_cursor);
	//
	is_disable_animated_loading_screen.accept();
	string_builder.append(is_disable_animated_loading_screen_setting_name, is_disable_animated_loading_screen);
	//
	is_disable_hardware_sound.accept();
	string_builder.append(is_disable_hardware_sound_setting_name, is_disable_hardware_sound);
	//
	is_disable_sound_filters.accept();
	string_builder.append(is_disable_sound_filters_setting_name, is_disable_sound_filters);
	//
	is_pass_custom_arguments.accept();
	string_builder.append(is_pass_custom_arguments_setting_name, is_pass_custom_arguments);
	//
	custom_arguments.accept();
	string_builder.append(custom_arguments_setting_name, is_pass_custom_arguments ? serialize_cl_args(custom_arguments) : "");
	//
	screen_width.accept();
	string_builder.append(screen_width_setting_name, screen_width);
	//
	screen_height.accept();
	string_builder.append(screen_height_setting_name, screen_height);
	// Dump the string buffer into file stream.
	const std::string& content = string_builder.get_string();
	const int content_size = narrow_cast<int>(content.size());
	if (content_size > max_file_size)
	{
		fail("Configuration data too big. (content_size={}; max_content_size={})", content_size, max_file_size);
	}
	const std::size_t write_result = SDL_WriteIO(sdl_io_stream_uptr.get(), content.data(), content_size);
	if (write_result != content_size)
	{
		fail("Failed to write settings. (file_path={}; error_message={})", configuration_path_, SDL_GetError());
	}
}

void Configuration::reset()
{
	language.set_and_accept(default_language);
	is_disable_display_settings_warning.set_and_accept(default_is_warned_about_display);
	is_disable_advanced_settings_warning.set_and_accept(default_is_disable_advanced_settings_warning);
	is_disable_sound_effects.set_and_accept(default_is_disable_sound_effects);
	is_disable_music.set_and_accept(default_is_disable_music);
	is_disable_fmvs.set_and_accept(default_is_disable_fmvs);
	is_disable_controllers.set_and_accept(default_is_disable_controllers);
	is_disable_triple_buffering.set_and_accept(default_is_disable_triple_buffering);
	is_disable_hardware_cursor.set_and_accept(default_is_disable_hardware_cursor);
	is_disable_animated_loading_screen.set_and_accept(default_is_disable_animated_loading_screen);
	is_disable_hardware_sound.set_and_accept(default_is_disable_hardware_sound);
	is_disable_sound_filters.set_and_accept(default_is_disable_sound_filters);
	is_pass_custom_arguments.set_and_accept(default_is_pass_custom_arguments);
	custom_arguments.set_and_accept(default_custom_arguments);
	screen_width.set_and_accept(default_screen_width);
	screen_height.set_and_accept(default_screen_height);

	language_mgr_->initialize(get_base_path().c_str());
}

std::string Configuration::serialize_cl_args(const std::string& custom_command_line)
{
	const int src_string_size = narrow_cast<int>(custom_command_line.size());
	const int dst_string_size = src_string_size * 2;
	std::string result{};
	result.resize(dst_string_size);
	const int escaped_size = narrow_cast<int>(ScriptTokenizer::escape_string(
		custom_command_line.c_str(),
		src_string_size,
		result.data(),
		2 * src_string_size));
	result.resize(escaped_size);
	return result;
}

std::string Configuration::deserialize_cl_args(const ScriptTokenizerToken& token)
{
	if (token.is_escaped())
	{
		std::string result{};
		result.resize(token.unescaped_size);
		ScriptTokenizer::unescape_string(token, result.data(), token.unescaped_size);
		return result;
	}
	else
	{
		return std::string{token.data, narrow_cast<std::size_t>(token.size)};
	}
}

// ======================================

struct Language
{
	std::string id;
	std::string name;
};

using Languages = std::vector<Language>;

// ======================================

class SupportedLanguages
{
public:
	static SupportedLanguages& get_singleton();
	void load();
	const Languages& get() const;
	bool has_id(const std::string& id) const;
	int find_index_by_id(const std::string& id) const;

private:
	static constexpr int max_file_size = 4096;

	Languages languages_{};

	SupportedLanguages() = default;
	~SupportedLanguages() = default;
};

// --------------------------------------

SupportedLanguages& SupportedLanguages::get_singleton()
{
	static SupportedLanguages supported_languages{};
	return supported_languages;
}

void SupportedLanguages::load()
{
	languages_.clear();
	Configuration& configuration = Configuration::get_singleton();
	LanguageMgr* const language_mgr = configuration.get_language_mgr();
	language_mgr->load();
	const LanguageMgrLanguages languages = language_mgr->get_languages();
	const ltjs::Index language_count = languages.size;
	languages_.resize(language_count);
	for (ltjs::Index i = 0; i < language_count; ++i)
	{
		const ltjs::Language& src_language = languages.data[i];
		Language& dst_language = languages_[i];
		dst_language.id.assign(src_language.id_string.data, narrow_cast<std::size_t>(src_language.id_string.size));
		dst_language.name.assign(src_language.name.data, narrow_cast<std::size_t>(src_language.name.size));
	}
}

const Languages& SupportedLanguages::get() const
{
	return languages_;
}

bool SupportedLanguages::has_id(const std::string& id) const
{
	const auto language_end_iter = languages_.cend();
	const auto language_iter = std::find_if(
		languages_.cbegin(),
		language_end_iter,
		[&id](const Language& item)
		{
			return item.id == id;
		});
	return language_iter != language_end_iter;
}

int SupportedLanguages::find_index_by_id(const std::string& id) const
{
	const auto language_begin_iter = languages_.cbegin();
	const auto language_end_iter = languages_.cend();
	const auto language_iter = std::find_if(
		language_begin_iter,
		language_end_iter,
		[&id](const Language& item)
		{
			return item.id == id;
		});
	if (language_iter == language_end_iter)
	{
		return -1;
	}
	return narrow_cast<int>(language_iter - language_begin_iter);
}

// ======================================

using FontData = std::vector<std::uint8_t>;

class FontMgr
{
public:
	static FontMgr& get_singleton();
	const FontData& get_regular_data() const;
	const FontData& get_bold_data() const;
	float get_regular_size() const;
	float get_bold_size() const;

private:
	static constexpr int max_font_file_size = 1 * 1024 * 1024;
	static const std::string regular_font_file_name;
	static const std::string bold_font_file_name;

	FontData regular_font_data_{};
	FontData bold_font_data_{};

	FontMgr();
	~FontMgr() = default;

	void load_font_data(const std::string& file_name, FontData& font_data);
};

// --------------------------------------

const std::string FontMgr::regular_font_file_name = "ltjs/" LTJS_GAME_ID_STRING "/launcher/noto_sans_display_condensed.ttf";
const std::string FontMgr::bold_font_file_name = "ltjs/" LTJS_GAME_ID_STRING "/launcher/noto_sans_display_condensed_bold.ttf";

// --------------------------------------

FontMgr::FontMgr()
{
	load_font_data(regular_font_file_name, regular_font_data_);
	load_font_data(bold_font_file_name, bold_font_data_);
}

FontMgr& FontMgr::get_singleton()
{
	static FontMgr font_mgr{};
	return font_mgr;
}

const FontData& FontMgr::get_regular_data() const
{
	return regular_font_data_;
}

const FontData& FontMgr::get_bold_data() const
{
	return bold_font_data_;
}

float FontMgr::get_regular_size() const
{
	return 17.0F;
}

float FontMgr::get_bold_size() const
{
	return 27.0F;
}

void FontMgr::load_font_data(const std::string& file_name, FontData& font_data)
{
	const std::string normalized_file_name = normalize_file_path(file_name);
	const SdlIoStreamUPtr sdl_io_stream_uptr{SDL_IOFromFile(normalized_file_name.c_str(), "rb")};
	if (sdl_io_stream_uptr == nullptr)
	{
		fail("Failed to open a file. (file_path={}; error_message={})", normalized_file_name, SDL_GetError());
	}
	const Sint64 file_size = SDL_GetIOSize(sdl_io_stream_uptr.get());
	if (file_size < 0)
	{
		fail("Unknown file size. (file_path={}; error_message={})", normalized_file_name, SDL_GetError());
	}
	if (file_size > max_font_file_size)
	{
		fail(
			"Font file too big. (size={}; max_size={}; file_path={})",
			file_size,
			max_font_file_size,
			normalized_file_name);
	}
	font_data.resize(narrow_cast<std::size_t>(file_size));
	const std::size_t read_size = SDL_ReadIO(sdl_io_stream_uptr.get(), font_data.data(), narrow_cast<std::size_t>(file_size));
	if (read_size != narrow_cast<std::size_t>(file_size))
	{
		fail(
			"Failed to read font file. (file_path={}; error_message={})",
			normalized_file_name,
			SDL_GetError());
	}
}

// ======================================

struct TextureMgrCreateParam
{
	SDL_Renderer* sdl_renderer;
};

class TextureMgr
{
public:
	explicit TextureMgr(const TextureMgrCreateParam& param);

	void clear();
	ImTextureID get(ImageId image_id);

private:
	using Strings = std::vector<std::string>;
	using TextureMap = std::unordered_map<ImageId, SdlTextureUPtr>;

	static const std::string images_path;
	static const Strings image_file_names;

	SDL_Renderer* sdl_renderer_{};
	TextureMap texture_map_{};
};

using TextureMgrUPtr = std::unique_ptr<TextureMgr>;

TextureMgrUPtr make_texture_mgr(const TextureMgrCreateParam& param)
{
	return std::make_unique<TextureMgr>(param);
}

// --------------------------------------

const std::string TextureMgr::images_path = "ltjs/" LTJS_GAME_ID_STRING "/launcher";

const TextureMgr::Strings TextureMgr::image_file_names{
	// Shared.

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

	// Localized.

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
};

// --------------------------------------

TextureMgr::TextureMgr(const TextureMgrCreateParam& param)
{
	sdl_renderer_ = param.sdl_renderer;
	texture_map_.reserve(image_file_names.size());
}

void TextureMgr::clear()
{
	texture_map_.clear();
}

ImTextureID TextureMgr::get(ImageId image_id)
{
	const std::size_t image_id_u = static_cast<std::size_t>(image_id);
	if (image_id_u >= image_file_names.size())
	{
		return ImTextureID{};
	}
	const auto cast = [](SDL_Texture* sdl_texture) -> ImTextureID
	{
		return narrow_cast<ImTextureID>(reinterpret_cast<std::size_t>(sdl_texture));
	};
	if (const auto texture_iter = texture_map_.find(image_id);
		texture_iter != texture_map_.cend())
	{
		return cast(texture_iter->second.get());
	}
	const Configuration& configuration = Configuration::get_singleton();
	const std::string& image_file_name = image_file_names[image_id_u];
	const std::string shared_image_path = combine_and_normalize_file_paths(images_path, image_file_name);
	SdlSurfaceUPtr image_surface{SDL_LoadBMP(shared_image_path.c_str())};
	if (image_surface == nullptr)
	{
		const std::string localized_image_path =
			combine_and_normalize_file_paths(images_path, configuration.language, image_file_name);
		image_surface.reset(SDL_LoadBMP(localized_image_path.c_str()));
	}
	if (image_surface == nullptr)
	{
		fail("Missing image. (file_name={})", image_file_name);
	}
	if (SDL_Texture* const sdl_texture = SDL_CreateTextureFromSurface(sdl_renderer_, image_surface.get());
		sdl_texture != nullptr)
	{
		texture_map_.emplace(image_id, SdlTextureUPtr{sdl_texture});
		return cast(sdl_texture);
	}
	fail("Failed to create a texture. (file_name={}; error_message={})", image_file_name, SDL_GetError());
}

// ======================================

class ScaleMgr
{
public:
	static ScaleMgr& get_singleton();
	float get_scale() const;

private:
	static constexpr float ref_scale_dimension = 600.0F;

	float scale_{};

	ScaleMgr();
	~ScaleMgr() = default;
};

// --------------------------------------

ScaleMgr::ScaleMgr()
{
	const SDL_DisplayMode* const sdl_display_mode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
	if (sdl_display_mode != nullptr)
	{
		const float dimension = narrow_cast<float>(std::min(sdl_display_mode->w, sdl_display_mode->h));
		scale_ = dimension / ref_scale_dimension;
	}
	if (scale_ < 1.0F)
	{
		scale_ = 1.0F;
	}
}

ScaleMgr& ScaleMgr::get_singleton()
{
	static ScaleMgr scale_mgr{};
	return scale_mgr;
}

float ScaleMgr::get_scale() const
{
	return scale_;
}

// ======================================

class BasicWindow
{
public:
	BasicWindow() = default;
	virtual ~BasicWindow() = default;

	virtual void hide() = 0;
	virtual void show() = 0;
};

// ======================================

struct WindowCreateParam
{
	int width;
	int height;
	std::string title;
};

class Window : public BasicWindow
{
public:
	explicit Window(const WindowCreateParam& param);
	virtual ~Window() = default;

	void hide() override;
	void show() override;

	void show(bool is_show);
	bool is_visible() const;
	void draw();
	void handle_event(const SDL_Event& sdl_event);
	void imgui_handle_events();
	void set_icon(SDL_Surface* sdl_surface);
	void set_parent(Window* parent);

protected:
	ImFont* imgui_regular_font_{};
	ImFont* imgui_bold_font_{};
	Window* parent_window_{};

	virtual void on_dialog_result(DialogResult dialog_result);

	void get_size(int& width, int& height) const;
	void get_size_in_pixels(int& width, int& height) const;
	void minimize_internal(bool is_minimize);
	DialogResult get_dialog_result() const;
	void reset_dialog_result();
	void set_dialog_result(DialogResult dialog_result);
	static bool is_point_inside_rect(const ImVec2& point, const ImVec4& rect);
	static ImVec2 center_size_inside_rect(const ImVec4& outer_rect, const ImVec2& inner_size);
	TextureMgr& get_texture_manager();
	ImFont* get_regular_imgui_font();
	ImFont* get_bold_imgui_font();
	static void imgui_add_button(
		const char* id_string,
		const ImVec2& position,
		const ImVec2& size,
		ImageId down_image_id,
		ImageId hover_image_id,
		ImageId up_image_id,
		TextureMgr& texture_mgr,
		bool& out_is_clicked);
	static void imgui_add_title_minimize_button(
		const ImVec2& position,
		const ImVec2& size,
		TextureMgr& texture_mgr,
		bool& out_is_clicked);
	static void imgui_add_title_close_button(
		const ImVec2& position,
		const ImVec2& size,
		TextureMgr& texture_mgr,
		bool& out_is_clicked);
	static void imgui_add_ok_button(
		const ImVec2& position,
		const ImVec2& size,
		TextureMgr& texture_mgr,
		bool& out_is_clicked);
	static void imgui_add_cancel_button(
		const ImVec2& position,
		const ImVec2& size,
		TextureMgr& texture_mgr,
		bool& out_is_clicked);

private:
	SdlWindowUPtr sdl_window_uptr_{};
	SdlRendererUPtr sdl_renderer_uptr_{};
	TextureMgrUPtr texture_mgr_uptr_{};
	ImguiContextUPtr imgui_context_uptr_{};
	Uint64 time_{};
	DialogResult dialog_result_{};

	void initialize_sdl_window_and_renderer(const WindowCreateParam& param);
	void initialize(const WindowCreateParam& param);
	void imgui_new_frame();
	void imgui_render();

	virtual void do_draw() = 0;
	virtual void do_imgui_handle_events() = 0;
};

// --------------------------------------

Window::Window(const WindowCreateParam& param)
{
	initialize(param);
}

void Window::on_dialog_result(DialogResult dialog_result)
{}

void Window::initialize(const WindowCreateParam& param)
{
	initialize_sdl_window_and_renderer(param);
	FontMgr& font_mgr = FontMgr::get_singleton();
	texture_mgr_uptr_ = make_texture_mgr(TextureMgrCreateParam{.sdl_renderer = sdl_renderer_uptr_.get()});
	if (imgui_context_uptr_.reset(ImGui::CreateContext());
		imgui_context_uptr_ == nullptr)
	{
		fail("Failed to create ImGUI context.");
	}
	ImGui::SetCurrentContext(imgui_context_uptr_.get());
	ImGuiIO& im_io = ImGui::GetIO();
	//
	im_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	im_io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	//
	im_io.IniFilename = nullptr;
	//
	ImGuiStyle& im_style = ImGui::GetStyle();
	im_style.ChildRounding = 0.0F;
	im_style.FrameBorderSize = 0.0F;
	im_style.FramePadding = ImVec2{};
	im_style.FrameRounding = 0.0F;
	im_style.GrabRounding = 0.0F;
	im_style.ItemInnerSpacing = ImVec2{};
	im_style.ItemSpacing = ImVec2{};
	im_style.PopupRounding = 0.0F;
	im_style.ScrollbarRounding = 0.0F;
	im_style.TouchExtraPadding = ImVec2{};
	im_style.WindowBorderSize = 0.0F;
	im_style.WindowPadding = ImVec2{};
	im_style.WindowRounding = 0.0F;
	constinit static ImWchar glyph_ranges[] =
	{
		L'\x0020', L'\x024F', // Basic Latin + Latin Supplement + Latin Extended-A + Latin Extended-B
		L'\x0400', L'\x052F', // Cyrillic + Cyrillic Supplement
		L'\0',
	};
	im_io.Fonts->Clear();
	{
		const FontData& font_data = font_mgr.get_regular_data();
		ImFontConfig imgui_font_config{};
		imgui_font_config.FontDataOwnedByAtlas = false;
		imgui_regular_font_ = im_io.Fonts->AddFontFromMemoryTTF(
			const_cast<std::uint8_t*>(font_data.data()),
			narrow_cast<int>(font_data.size()),
			0.0F,
			&imgui_font_config,
			glyph_ranges);
		if (imgui_regular_font_ == nullptr)
		{
			fail("[{}] {}", "ImGui", "Failed to load a regular font.");
		}
	}
	{
		const FontData& font_data = font_mgr.get_bold_data();
		ImFontConfig imgui_font_config{};
		imgui_font_config.FontDataOwnedByAtlas = false;
		imgui_bold_font_ = im_io.Fonts->AddFontFromMemoryTTF(
			const_cast<std::uint8_t*>(font_data.data()),
			narrow_cast<int>(font_data.size()),
			0.0F,
			&imgui_font_config,
			glyph_ranges);
		if (imgui_bold_font_ == nullptr)
		{
			fail("[{}] {}", "ImGui", "Failed to load a bold font.");
		}
	}
	SDL_Window* const sdl_window = sdl_window_uptr_.get();
	SDL_Renderer* const sdl_renderer = sdl_renderer_uptr_.get();
	ImGui_ImplSDL3_InitForSDLRenderer(sdl_window, sdl_renderer);
	ImGui_ImplSDLRenderer3_Init(sdl_renderer);
	{
		int w = 0;
		int h = 0;
		get_size(w, h);
		int display_w = 0;
		int display_h = 0;
		get_size_in_pixels(display_w, display_h);
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2{narrow_cast<float>(w), narrow_cast<float>(h)};
		io.DisplayFramebufferScale = ImVec2{
			w > 0 ? (narrow_cast<float>(display_w) / w) : 0.0F,
			h > 0 ? (narrow_cast<float>(display_h) / h) : 0.0F,
		};
	}
}

void Window::hide()
{
	show(false);
}

void Window::show()
{
	show(true);
}

void Window::show(bool is_show)
{
	[[maybe_unused]] bool sdl_result;
	SDL_Window* const sdl_window = sdl_window_uptr_.get();
	if (is_show)
	{
		sdl_result = SDL_ShowWindow(sdl_window);
	}
	else
	{
		sdl_result = SDL_HideWindow(sdl_window);
	}
	assert(sdl_result);
}

bool Window::is_visible() const
{
	const SDL_WindowFlags sdl_window_flags = SDL_GetWindowFlags(sdl_window_uptr_.get());
	return (sdl_window_flags & SDL_WINDOW_HIDDEN) == 0;
}

void Window::draw()
{
	if (!is_visible())
	{
		return;
	}
	ImGui::SetCurrentContext(imgui_context_uptr_.get());
	imgui_new_frame();
	ImGui::NewFrame();
	do_draw();
	ImGui::EndFrame();
	imgui_render();
	ImGui::SetCurrentContext(nullptr);
}

void Window::handle_event(const SDL_Event& sdl_event)
{
	ImGui::SetCurrentContext(imgui_context_uptr_.get());
	ImGui_ImplSDL3_ProcessEvent(&sdl_event);
	ImGui::SetCurrentContext(nullptr);
}

void Window::imgui_handle_events()
{
	do_imgui_handle_events();
}

void Window::set_icon(SDL_Surface* sdl_surface)
{
	[[maybe_unused]] const bool sdl_result = SDL_SetWindowIcon(sdl_window_uptr_.get(), sdl_surface);
	assert(sdl_result);
}

void Window::set_parent(Window* parent)
{
	parent_window_ = parent;
	[[maybe_unused]] const bool sdl_result = SDL_SetWindowParent(
		sdl_window_uptr_.get(),
		parent != nullptr ? parent->sdl_window_uptr_.get() : nullptr);
	assert(sdl_result);
}

DialogResult Window::get_dialog_result() const
{
	return dialog_result_;
}

void Window::reset_dialog_result()
{
	dialog_result_ = DialogResult::none;
}

void Window::set_dialog_result(DialogResult dialog_result)
{
	assert(dialog_result != DialogResult::none);
	dialog_result_ = dialog_result;
	if (parent_window_ != nullptr)
	{
		parent_window_->on_dialog_result(dialog_result_);
	}
}

void Window::initialize_sdl_window_and_renderer(const WindowCreateParam& param)
{
	SDL_Window* sdl_window = nullptr;
	SDL_Renderer* sdl_renderer = nullptr;
	ensure_sdl_bool_result("SDL_CreateWindowAndRenderer", SDL_CreateWindowAndRenderer(
		param.title.c_str(),
		param.width,
		param.height,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS,
		&sdl_window,
		&sdl_renderer));
	sdl_window_uptr_.reset(sdl_window);
	sdl_renderer_uptr_.reset(sdl_renderer);
	ensure_sdl_bool_result(
		"SDL_SetWindowPosition",
		SDL_SetWindowPosition(sdl_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED));
}

void Window::get_size(int& width, int& height) const
{
	[[maybe_unused]] const bool sdl_result = SDL_GetWindowSize(sdl_window_uptr_.get(), &width, &height);
	assert(sdl_result);
}

void Window::get_size_in_pixels(int& width, int& height) const
{
	[[maybe_unused]] const bool sdl_result = SDL_GetWindowSizeInPixels(sdl_window_uptr_.get(), &width, &height);
	assert(sdl_result);
}

void Window::minimize_internal(bool is_minimize)
{
	const auto sdl_func = (is_minimize ? SDL_MinimizeWindow : SDL_RestoreWindow);
	[[maybe_unused]] const bool sdl_result = sdl_func(sdl_window_uptr_.get());
	assert(sdl_result);
}

bool Window::is_point_inside_rect(const ImVec2& point, const ImVec4& rect)
{
	return
		point.x >= rect.x && point.x < (rect.x + rect.z) &&
		point.y >= rect.y && point.y < (rect.y + rect.w);
}

ImVec2 Window::center_size_inside_rect(const ImVec4& outer_rect, const ImVec2& inner_size)
{
	return {
		outer_rect.x + (0.5F * (outer_rect.z - inner_size.x)),
		outer_rect.y + (0.5F * (outer_rect.w - inner_size.y))};
}

TextureMgr& Window::get_texture_manager()
{
	return *texture_mgr_uptr_;
}

ImFont* Window::get_regular_imgui_font()
{
	return imgui_regular_font_;
}

ImFont* Window::get_bold_imgui_font()
{
	return imgui_bold_font_;
}

void Window::imgui_add_button(
	const char* id_string,
	const ImVec2& position,
	const ImVec2& size,
	ImageId down_image_id,
	ImageId hover_image_id,
	ImageId up_image_id,
	TextureMgr& texture_mgr,
	bool& out_is_clicked)
{
	ImGui::SetCursorPos(position);
	out_is_clicked = ImGui::InvisibleButton(id_string, size);
	const ImageId image_id = ImGui::IsItemActive() ? down_image_id : (ImGui::IsItemHovered() ? hover_image_id : up_image_id);
	ImGui::SetCursorPos(position);
	ImGui::Image(texture_mgr.get(image_id), size);
}

void Window::imgui_add_title_minimize_button(
	const ImVec2& position,
	const ImVec2& size,
	TextureMgr& texture_mgr,
	bool& out_is_clicked)
{
	imgui_add_button(
		/* id_string      */ "title minimize",
		/* position       */ position,
		/* size           */ size,
		/* down_image_id  */ ImageId::minimized,
		/* hover_image_id */ ImageId::minimizeu,
		/* up_image_id    */ ImageId::minimizeu,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ out_is_clicked);
}

void Window::imgui_add_title_close_button(
	const ImVec2& position,
	const ImVec2& size,
	TextureMgr& texture_mgr,
	bool& out_is_clicked)
{
	imgui_add_button(
		/* id_string      */ "title close",
		/* position       */ position,
		/* size           */ size,
		/* down_image_id  */ ImageId::closed,
		/* hover_image_id */ ImageId::closeu,
		/* up_image_id    */ ImageId::closeu,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ out_is_clicked);
}

void Window::imgui_add_ok_button(
	const ImVec2& position,
	const ImVec2& size,
	TextureMgr& texture_mgr,
	bool& out_is_clicked)
{
	imgui_add_button(
		/* id_string      */ "ok",
		/* position       */ position,
		/* size           */ size,
		/* down_image_id  */ ImageId::okd,
		/* hover_image_id */ ImageId::okf,
		/* up_image_id    */ ImageId::oku,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ out_is_clicked);
}

void Window::imgui_add_cancel_button(
	const ImVec2& position,
	const ImVec2& size,
	TextureMgr& texture_mgr,
	bool& out_is_clicked)
{
	imgui_add_button(
		/* id_string      */ "cancel",
		/* position       */ position,
		/* size           */ size,
		/* down_image_id  */ ImageId::canceld,
		/* hover_image_id */ ImageId::cancelf,
		/* up_image_id    */ ImageId::cancelu,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ out_is_clicked);
}

void Window::imgui_new_frame()
{
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
}

void Window::imgui_render()
{
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	const SDL_Rect sdl_viewport{0, 0, narrow_cast<int>(io.DisplaySize.x), narrow_cast<int>(io.DisplaySize.y)};
	SDL_Renderer* const sdl_renderer = sdl_renderer_uptr_.get();
	SDL_SetRenderViewport(sdl_renderer, &sdl_viewport);
	SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(sdl_renderer);
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdl_renderer);
	SDL_RenderPresent(sdl_renderer);
}

// ======================================

class MessageBoxWindow final : public Window
{
public:
	MessageBoxWindow();
	~MessageBoxWindow() override = default;

	void set_message_box_title(const std::string& title);
	using Window::show;
	void show(MessageBoxType type, MessageBoxButtons buttons, const std::string& text);
	void show(MessageBoxType type, MessageBoxButtons buttons, const std::string& title, const std::string& text);

private:
	static constexpr int window_width = 600;
	static constexpr int window_height = 250;

	MessageBoxType type_{};
	MessageBoxButtons buttons_{};
	std::string title_{};
	std::string message_{};
	bool is_title_position_calculated_{};
	ImVec2 calculated_title_position_{};
	bool is_close_button_clicked_{};
	bool is_ok_button_clicked_{};
	bool is_cancel_button_clicked_{};
	bool is_escape_down_{};

	void do_draw() override;
	void do_imgui_handle_events() override;
};

// --------------------------------------

MessageBoxWindow::MessageBoxWindow()
	:
	Window{
		WindowCreateParam{
			.width = narrow_cast<int>(window_width * ScaleMgr::get_singleton().get_scale()),
			.height = narrow_cast<int>(window_height * ScaleMgr::get_singleton().get_scale()),
			.title = "LTJS " LTJS_GAME_ID_STRING_UC " message box"}}
{}

void MessageBoxWindow::set_message_box_title(const std::string& title)
{
	title_ = title;
	is_title_position_calculated_ = false;
}

void MessageBoxWindow::show(MessageBoxType type, MessageBoxButtons buttons, const std::string& text)
{
	type_ = type;
	buttons_ = buttons;
	message_ = text;
	show(true);
}

void MessageBoxWindow::show(MessageBoxType type, MessageBoxButtons buttons, const std::string& title, const std::string& text)
{
	type_ = type;
	buttons_ = buttons;
	title_ = title;
	message_ = text;
	is_title_position_calculated_ = false;
	show(true);
}

void MessageBoxWindow::do_draw()
{
	const bool has_cancel_button = (buttons_ == MessageBoxButtons::ok_cancel);
	const ScaleMgr& scale_mgr = ScaleMgr::get_singleton();
	const float scale = scale_mgr.get_scale();
	FontMgr& font_mgr = FontMgr::get_singleton();
	const float regular_font_size = font_mgr.get_regular_size() * scale;
	const float bold_font_size = font_mgr.get_bold_size() * scale;
	TextureMgr& texture_mgr = get_texture_manager();
	is_escape_down_ = ImGui::IsKeyPressed(ImGuiKey_Escape);
	// Begin message box window.
	constexpr ImGuiWindowFlags main_flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_None;
	ImGui::Begin("message_box", nullptr, main_flags);
	ImGui::SetWindowPos(ImVec2{}, ImGuiCond_Always);
	ImGui::SetWindowSize(
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale,
		ImGuiCond_Always);
	// Background image.
	ImGui::SetCursorPos(ImVec2{});
	const ImTextureID boxbackground_tex_id = texture_mgr.get(ImageId::boxbackground);
	ImGui::Image(
		boxbackground_tex_id,
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale);
	// "Close" button.
	imgui_add_title_close_button(
		/* position       */ ImVec2{578.0F, 6.0F} * scale,
		/* size           */ ImVec2{17.0F, 14.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_close_button_clicked_);
	// "Icon" image.
	const ImVec2 icon_pos = ImVec2{6.0F, 15.0F} * scale;
	const ImVec2 icon_size = ImVec2{38.0F, 38.0F} * scale;
	const ImVec4 icon_rect{icon_pos.x, icon_pos.y, icon_size.x, icon_size.y};
	ImageId icon_image_id;
	switch (type_)
	{
		case MessageBoxType::error:
			icon_image_id = ImageId::error;
			break;
		case MessageBoxType::warning:
			icon_image_id = ImageId::warning;
			break;
		case MessageBoxType::information:
		default:
			icon_image_id = ImageId::information;
			break;
	}
	const ImTextureID icon_tex_id = texture_mgr.get(icon_image_id);
	ImGui::SetCursorPos(icon_pos);
	ImGui::Image(icon_tex_id, icon_size);
	// Title.
	if (!title_.empty())
	{
		const ImVec2 title_pos = ImVec2{52.0F, 14.0F} * scale;
		const ImVec2 title_size = ImVec2{518.0F, 31.0F} * scale;
		const ImVec4 title_rect{title_pos.x, title_pos.y, title_size.x, title_size.y};
		ImGui::PushFont(get_bold_imgui_font(), bold_font_size);
		const ImVec2 centered_title_position = calculated_title_position_;
		if (!is_title_position_calculated_)
		{
			is_title_position_calculated_ = true;
			const ImVec2 title_text_size = ImGui::CalcTextSize(title_.data(), title_.data() + title_.size());
			calculated_title_position_ = center_size_inside_rect(title_rect, title_text_size);
		}
		ImGui::SetCursorPos(calculated_title_position_);
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
		ImGui::Text("%s", title_.c_str());
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
	// Message.
	if (!message_.empty())
	{
		const ImVec2 message_pos = ImVec2{14.0F, 64.0F} * scale;
		const ImVec2 message_size = ImVec2{573.0F, 121.0F} * scale;
		const ImVec4 message_rect{message_pos.x, message_pos.y, message_size.x, message_size.y};
		ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
		ImGui::SetCursorPos(message_pos);
		ImGui::BeginChild("message", message_size);
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
		ImGui::TextWrapped("%s", message_.c_str());
		ImGui::PopStyleColor();
		ImGui::EndChild();
		ImGui::PopFont();
	}
	// "Ok" button.
	imgui_add_ok_button(
		/* position       */ ImVec2{has_cancel_button ? 189.0F : 250.0F, 206.0F} * scale,
		/* size           */ ImVec2{100.0F, 30.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_ok_button_clicked_);
	// "Cancel" button.
	is_cancel_button_clicked_ = false;
	if (has_cancel_button)
	{
		imgui_add_cancel_button(
			/* position       */ ImVec2{312.0F, 206.0F} * scale,
			/* size           */ ImVec2{100.0F, 30.0F} * scale,
			/* texture_mgr    */ texture_mgr,
			/* out_is_clicked */ is_cancel_button_clicked_);
	}
	// End message box window.
	ImGui::End();
}

void MessageBoxWindow::do_imgui_handle_events()
{
	const bool has_cancel_button = (buttons_ == MessageBoxButtons::ok_cancel);
	DialogResult dialog_result = DialogResult::none;
	if (is_close_button_clicked_)
	{
		dialog_result = (has_cancel_button ? DialogResult::cancel : DialogResult::ok);
	}
	else if (is_ok_button_clicked_)
	{
		dialog_result = DialogResult::ok;
	}
	else if (is_cancel_button_clicked_ || is_escape_down_)
	{
		dialog_result = DialogResult::cancel;
	}
	is_close_button_clicked_ = false;
	is_ok_button_clicked_ = false;
	is_cancel_button_clicked_ = false;
	is_escape_down_ = false;
	if (dialog_result != DialogResult::none)
	{
		show(false);
		set_dialog_result(dialog_result);
	}
}

// ======================================

using MessageBoxWindowUPtr = std::unique_ptr<MessageBoxWindow>;

// ======================================

class DetailSettingsWindow final : public Window
{
public:
	DetailSettingsWindow();
	~DetailSettingsWindow() override = default;

	DetailLevel get_detail_level() const;

private:
	static constexpr int window_width = 456;
	static constexpr int window_height = 480;

	DetailLevel detail_level_{};
	bool is_escape_down_{};
	bool is_close_button_clicked_{};
	bool is_cancel_button_clicked_{};
	bool is_high_button_clicked_{};
	bool is_medium_button_clicked_{};
	bool is_low_button_clicked_{};

	void do_draw() override;
	void do_imgui_handle_events() override;
};

// ======================================

using DetailSettingsWindowUPtr = std::unique_ptr<DetailSettingsWindow>;

// ======================================

class DisplaySettingsWindow final : public Window
{
public:
	DisplaySettingsWindow();
	~DisplaySettingsWindow() override = default;

private:
	static constexpr int window_width = 600;
	static constexpr int window_height = 394;

	using Strings = std::vector<std::string>;

	SettingValue<int> selected_resolution_index_{-1};
	bool is_escape_down_{};
	bool has_display_modes_{};
	bool is_close_button_clicked_{};
	bool is_ok_button_clicked_{};
	bool is_cancel_button_clicked_{};

	static const char* im_display_modes_getter(void* data, int idx);

	void do_draw() override;
	void do_imgui_handle_events() override;
};

// ======================================

using DisplaySettingsWindowUPtr = std::unique_ptr<DisplaySettingsWindow>;

// ======================================

class AdvancedSettingsWindow final : public Window
{
public:
	AdvancedSettingsWindow();
	~AdvancedSettingsWindow() override = default;

private:
	struct CheckBoxContext
	{
		ImVec2 position;
		bool checked_value;
		bool is_pressed;
		int resource_string_id;
		std::string resource_string_default;
		int hint_resource_string_id;
		std::string hint_resource_string_default;
		SettingValue<bool>* setting_value_ptr;
	};

	using CheckBoxContexts = std::vector<CheckBoxContext>;

	static constexpr int window_width = 456;
	static constexpr int window_height = 480;

	CheckBoxContexts check_box_contexts_{};
	std::string hint_{};

	bool is_escape_down_{};
	bool is_close_button_clicked_{};
	bool is_ok_button_clicked_{};
	bool is_cancel_button_clicked_{};

	void initialize_check_box_contents();
	void draw_check_box(int index);
	void update_check_box_configuration(bool is_accept);

	void do_draw() override;
	void do_imgui_handle_events() override;
};

using AdvancedSettingsWindowUPtr = std::unique_ptr<AdvancedSettingsWindow>;

// ======================================

class MainWindow final : public Window
{
public:
	MainWindow();
	~MainWindow() override = default;

private:
	static constexpr int window_width = 525;
	static constexpr int window_height = 245;

	using Strings = std::vector<std::string>;
	using StringPtrs = std::vector<const char*>;

	enum class State
	{
		main_window,
		main_window_message_box,
		display_settings_warning,
		display_settings_no_renderers,
		display_settings_window,
		advanced_settings_warning,
		advanced_settings_window,
		detail_settings_window,
	};

	static const std::string lithtech_executable;

	bool is_minimize_button_clicked_{};
	bool is_close_button_clicked_{};
	bool is_quit_button_clicked_{};
	bool is_escape_down_{};
	bool is_play_button_clicked_{};
	bool is_display_button_clicked_{};
	bool is_options_button_clicked_{};
	bool is_language_selected_{};
	bool is_publisher1web_button_clicked_{};
	bool is_company1web_button_clicked_{};
	bool is_company2web_button_clicked_{};
	bool is_publisher2web_button_clicked_{};
	int language_index_{};
	State state_{};
	SdlProcessUPtr lithtech_sdl_process_uptr_{};

	void initialize_language();
	bool is_lithtech_executable_exists() const;
	void handle_check_for_renderers();
	void run_the_game();
	void no_lithtech_exe_error();
	void change_language();
	static void append_custom_command_line(const std::string& custom_string, Strings& args);
	Strings build_command_line(bool has_detail_settings) const;
	static StringPtrs make_string_ptrs(const Strings& strings);
	bool save_arguments(const Strings& args) const;

	void on_dialog_result(DialogResult dialog_result) override;
	void do_draw() override;
	void do_imgui_handle_events() override;
};

// ======================================

using MainWindowUPtr = std::unique_ptr<MainWindow>;

// ======================================

class WindowMgr
{
public:
	static WindowMgr& get_singleton();
	MainWindow* get_main_window();
	DetailSettingsWindow* get_detail_settings_window();
	DisplaySettingsWindow* get_display_settings_window();
	AdvancedSettingsWindow* get_advanced_settings_window();
	MessageBoxWindow* get_message_box_window();
	const ResourceStrings& get_resource_strings() const;
	ResourceStrings& get_resource_strings();
	const SearchPaths& get_search_paths() const;
	void setup_search_paths(const std::string& language_id_string);
	void hide_all();
	void handle_event(const SDL_Event& sdl_event);
	void imgui_handle_events();
	void draw();

private:
	using Windows = std::vector<Window*>;

	MainWindowUPtr main_window_{};
	DetailSettingsWindowUPtr detail_settings_window_{};
	DisplaySettingsWindowUPtr display_settings_window_{};
	AdvancedSettingsWindowUPtr advanced_settings_window_{};
	MessageBoxWindowUPtr message_box_window_{};
	Windows windows_{};
	ResourceStrings resource_strings_{};
	SearchPaths search_paths_{};

	WindowMgr();
	~WindowMgr() = default;
};

// ======================================

class Launcher
{
public:
	static std::string launcher_commands_file_name;
	static std::string resource_strings_directory;
	static std::string resource_strings_file_name;

	Launcher();
	static Launcher& get_singleton();
	Logger* get_logger();
	void run();

private:
	using Windows = std::vector<Window*>;

	static std::string icon_path;
	bool is_quit_requested_{};
	LoggerUPtr logger_{};
	SdlSurfaceUPtr icon_sdl_surface_uptr_{};

	~Launcher() = default;

	void initialize_sdl();
	void initialize();
	void handle_events();
	void draw();
};

// ======================================

const std::string MainWindow::lithtech_executable =
#ifdef _WIN32
	"ltjs_lithtech.exe"
#else // _WIN32
	"ltjs_lithtech"
#endif // _WIN32
;

MainWindow::MainWindow()
	:
	Window{
		WindowCreateParam{
			.width = narrow_cast<int>(window_width * ScaleMgr::get_singleton().get_scale()),
			.height = narrow_cast<int>(window_height * ScaleMgr::get_singleton().get_scale()),
			.title = "LTJS " LTJS_GAME_ID_STRING_UC " launcher"}}
{
	initialize_language();
}

void MainWindow::initialize_language()
{
	const SupportedLanguages& supported_languages = SupportedLanguages::get_singleton();
	const Configuration& configuration = Configuration::get_singleton();
	language_index_ = supported_languages.find_index_by_id(configuration.language);
}

bool MainWindow::is_lithtech_executable_exists() const
{
	const std::string normalized_file_name = normalize_file_path(lithtech_executable);
	if (const SdlIoStreamUPtr sdl_io_stream_uptr{SDL_IOFromFile(normalized_file_name.c_str(), "rb")};
		sdl_io_stream_uptr != nullptr)
	{
		if (const Sint64 file_size = SDL_GetIOSize(sdl_io_stream_uptr.get());
			file_size > 0)
		{
			return true;
		}
	}
	return false;
}

void MainWindow::handle_check_for_renderers()
{
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	if (Direct3d9::has_direct3d9())
	{
		state_ = State::display_settings_window;
		DisplaySettingsWindow* const display_settings_window = window_mgr.get_display_settings_window();
		display_settings_window->show(true);
	}
	else
	{
		state_ = State::display_settings_no_renderers;
		const ResourceStrings& resource_strings = window_mgr.get_resource_strings();
		const std::string& message = resource_strings.get(ResourceStringId::ids_norens, "IDS_NORENS");
		MessageBoxWindow* const message_box_window = window_mgr.get_message_box_window();
		message_box_window->show(MessageBoxType::error, MessageBoxButtons::ok, message);
	}
}

void MainWindow::run_the_game()
{
	Configuration& configuration = Configuration::get_singleton();
	show(false);
	configuration.save();
	Strings arguments = build_command_line(configuration.is_restore_defaults);
	save_arguments(arguments);
	if (!is_lithtech_executable_exists())
	{
		show(true);
		no_lithtech_exe_error();
		return;
	}
	arguments.emplace(arguments.cbegin(), lithtech_executable);
	const StringPtrs argument_ptrs = make_string_ptrs(arguments);
	if (const SdlPropertiesIdURes sdl_properties_id_ures{SDL_CreateProperties()};
		sdl_properties_id_ures.get() != 0)
	{
		constexpr SDL_ProcessIO sdl_process_io =
#ifdef _WIN32
			SDL_PROCESS_STDIO_NULL
#else
			SDL_PROCESS_STDIO_INHERITED
#endif
		;
		const SDL_PropertiesID sdl_properties_id = sdl_properties_id_ures.get();
		SDL_SetPointerProperty(
			sdl_properties_id,
			SDL_PROP_PROCESS_CREATE_ARGS_POINTER,
			const_cast<char**>(argument_ptrs.data()));
		SDL_SetNumberProperty(
			sdl_properties_id,
			SDL_PROP_PROCESS_CREATE_STDIN_NUMBER,
			SDL_PROCESS_STDIO_NULL);
		SDL_SetNumberProperty(
			sdl_properties_id,
			SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER,
			sdl_process_io);
		SDL_SetNumberProperty(
			sdl_properties_id,
			SDL_PROP_PROCESS_CREATE_STDERR_NUMBER,
			sdl_process_io);
		lithtech_sdl_process_uptr_.reset(SDL_CreateProcessWithProperties(sdl_properties_id));
	}
	if (lithtech_sdl_process_uptr_ == nullptr)
	{
		show(true);
		no_lithtech_exe_error();
	}
	else
	{
		configuration.is_restore_defaults.set_and_accept(false);
	}
}

void MainWindow::no_lithtech_exe_error()
{
	state_ = State::main_window_message_box;
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	const ResourceStrings& resource_strings = window_mgr.get_resource_strings();
	const std::string& message = resource_strings.get(ResourceStringId::ids_cantlaunchclientexe, "IDS_CANTLAUNCHCLIENTEXE");
	window_mgr.get_message_box_window()->show(MessageBoxType::error, MessageBoxButtons::ok, message);
}

void MainWindow::change_language()
{
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	ResourceStrings& resource_strings = window_mgr.get_resource_strings();
	Configuration& configuration = Configuration::get_singleton();
	TextureMgr& texture_mgr = get_texture_manager();
	window_mgr.setup_search_paths(configuration.language);
	{
		const bool resource_strings_result = resource_strings.initialize(
			window_mgr.get_search_paths(),
			Launcher::resource_strings_file_name);
		if (!resource_strings_result)
		{
			fail("Failed to reload resource strings. (error_message={})", resource_strings.get_error_message());
		}
	}
	texture_mgr.clear();
	configuration.language.accept();
}

void MainWindow::append_custom_command_line(const std::string& custom_string, Strings& args)
{
	constexpr std::size_t npos = std::string::npos;
	for (std::size_t begin_pos = 0; ; )
	{
		begin_pos = custom_string.find_first_not_of(" \t", begin_pos);
		if (begin_pos == npos)
		{
			break;
		}
		if (custom_string[begin_pos] == '"')
		{
			++begin_pos;
			const std::size_t end_pos = custom_string.find('"', begin_pos);
			if (end_pos == npos)
			{
				// Unclosed string.
				break;
			}
			args.emplace_back(custom_string.substr(begin_pos, end_pos - begin_pos));
			begin_pos = end_pos + 1;
		}
		else
		{
			const std::size_t end_pos = custom_string.find_first_of(" \t", begin_pos);
			args.emplace_back(custom_string.substr(begin_pos, end_pos - begin_pos));
			begin_pos = end_pos;
		}
	}
}

auto MainWindow::build_command_line(bool has_detail_settings) const -> Strings
{
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	const ResourceStrings& resource_strings = window_mgr.get_resource_strings();
	const Configuration& configuration = Configuration::get_singleton();
	Strings args{};
	constinit static const std::string zero_string{"0"};
	constinit static const std::string one_string{"1"};
	const auto bool_to_string = [](bool value) -> const std::string&
	{
		if (value)
		{
			return one_string;
		}
		else
		{
			return zero_string;
		}
	};
	// Window title.
	const std::string& app_name = resource_strings.get(ResourceStringId::ids_appname, "IDS_APPNAME");
	args.emplace_back("-windowtitle");
	args.emplace_back(app_name);
	// .REZ files.
	const std::string& rez_base = resource_strings.get(ResourceStringId::ids_rezbase, "IDS_REZBASE");
	args.emplace_back("-rez");
	args.emplace_back(rez_base + ".REZ");
	args.emplace_back("-rez");
	args.emplace_back(rez_base + "2.REZ");
	args.emplace_back("-rez");
	args.emplace_back(rez_base + "DLL.REZ");
	args.emplace_back("-rez");
	args.emplace_back("SOUND.REZ");
	args.emplace_back("-rez");
	args.emplace_back(rez_base + "L.REZ");
	args.emplace_back("-rez");
	args.emplace_back("custom");
	args.emplace_back("-rez");
	args.emplace_back(rez_base + "P.REZ");
	args.emplace_back("-rez");
	args.emplace_back(rez_base + "P2.REZ");
#ifdef LTJS_NOLF2
	args.emplace_back("-rez");
	args.emplace_back("Update_v1x3.rez");
#endif // LTJS_NOLF2
	// Advanced settings.
	args.emplace_back("+DisableSound");
	args.emplace_back(bool_to_string(configuration.is_disable_sound_effects));
	args.emplace_back("+DisableMusic");
	args.emplace_back(bool_to_string(configuration.is_disable_music));
	args.emplace_back("+DisableMovies");
	args.emplace_back(bool_to_string(configuration.is_disable_fmvs));
	args.emplace_back("+DisableJoystick");
	args.emplace_back(bool_to_string(configuration.is_disable_controllers));
	args.emplace_back("+DisableTripBuf");
	args.emplace_back(bool_to_string(configuration.is_disable_triple_buffering));
	args.emplace_back("+DisableHardwareCursor");
	args.emplace_back(bool_to_string(configuration.is_disable_hardware_cursor));
	args.emplace_back("+DynamicLoadScreen");
	args.emplace_back(bool_to_string(configuration.is_disable_animated_loading_screen));
	args.emplace_back("+DisableHardwareSound");
	args.emplace_back(bool_to_string(configuration.is_disable_hardware_sound));
	args.emplace_back("+DisableSoundFilters");
	args.emplace_back(bool_to_string(configuration.is_disable_sound_filters));
	// Display settings.
	const DisplayModeMgr& display_mode_mgr = DisplayModeMgr::get_singleton();
	const DisplayMode& native_display_mode = display_mode_mgr.get_native_mode();
	int screen_width = 0;
	if (configuration.screen_width <= 0)
	{
		screen_width = native_display_mode.width;
	}
	else
	{
		screen_width = configuration.screen_width.get_ref();
	}
	screen_width = std::max(screen_width, DisplayModeMgr::min_display_mode_width);
	args.emplace_back("+ScreenWidth");
	args.emplace_back(std::format("{}", screen_width));
	int screen_height = 0;
	if (configuration.screen_height <= 0)
	{
		screen_height = native_display_mode.height;
	}
	else
	{
		screen_height = configuration.screen_height.get_ref();
	}
	screen_height = std::max(screen_height, DisplayModeMgr::min_display_mode_height);
	args.emplace_back("+ScreenHeight");
	args.emplace_back(std::format("{}", screen_height));
	args.emplace_back("+BitDepth");
	args.emplace_back("32");
	// Restore defaults.
	if (configuration.is_restore_defaults)
	{
		args.emplace_back("+RestoreDefaults");
		args.emplace_back("1");
		args.emplace_back("+HardwareCursor");
		args.emplace_back("1");
		args.emplace_back("+VSyncOnFlip");
		args.emplace_back("1");
		args.emplace_back("+GammaR");
		args.emplace_back("1.0");
		args.emplace_back("+GammaG");
		args.emplace_back("1.0");
		args.emplace_back("+GammaB");
		args.emplace_back("1.0");
	}
	// Detail settings.
	if (has_detail_settings)
	{
		const DetailLevel detail_level_id = window_mgr.get_detail_settings_window()->get_detail_level();
		std::string detail_level_string{};
		switch (detail_level_id)
		{
			case DetailLevel::low:
				detail_level_string = ".DefaultLow";
				break;
			case DetailLevel::medium:
				detail_level_string = ".DefaultMid";
				break;
			case DetailLevel::high:
				detail_level_string = ".DefaultHigh";
				break;
		}
		if (!detail_level_string.empty())
		{
			args.emplace_back("+SetPerformanceLevel");
			args.emplace_back(detail_level_string);
		}
	}
	// Custom command line.
	append_custom_command_line(configuration.custom_arguments.get_ref(), args);
	return args;
}

auto MainWindow::make_string_ptrs(const Strings& strings) -> StringPtrs
{
	StringPtrs string_ptrs{};
	string_ptrs.reserve(strings.size() + 1);
	for (const std::string& string : strings)
	{
		string_ptrs.emplace_back(string.data());
	}
	string_ptrs.emplace_back(nullptr);
	return string_ptrs;
}

bool MainWindow::save_arguments(const Strings& args) const
{
	const auto need_quotation = [](const std::string arg)
	{
		return arg.find_first_of(" \t", 0) != std::string::npos;
	};
	std::string command_line{};
	command_line.reserve(4096);
	for (const std::string& arg : args)
	{
		if (!command_line.empty())
		{
			command_line += ' ';
		}
		const bool requires_quotes = need_quotation(arg);
		if (requires_quotes)
		{
			command_line += '"';
		}
		command_line += arg;
		if (requires_quotes)
		{
			command_line += '"';
		}
	}
	const Configuration& configuration = Configuration::get_singleton();
	const std::string& config_path = configuration.get_config_path();
	const std::string& arguments_file_name = configuration.get_arguments_file_name();
	const std::string file_name = combine_and_normalize_file_paths(config_path, arguments_file_name);
	const SdlIoStreamUPtr sdl_io_stream_uptr{SDL_IOFromFile(file_name.c_str(), "wb")};
	if (sdl_io_stream_uptr == nullptr)
	{
		return false;
	}
	const std::size_t data_size = command_line.size();
	const std::size_t write_result = SDL_WriteIO(sdl_io_stream_uptr.get(), command_line.c_str(), data_size);
	if (write_result != data_size)
	{
		return false;
	}
	return true;
}

void MainWindow::on_dialog_result(DialogResult dialog_result)
{
	Configuration& configuration = Configuration::get_singleton();
	switch (state_)
	{
		case State::main_window_message_box:
			state_ = State::main_window;
			break;
		case State::display_settings_warning:
			state_ = State::main_window;
			if (dialog_result == DialogResult::ok)
			{
				configuration.is_disable_display_settings_warning = true;
				handle_check_for_renderers();
			}
			break;
		case State::display_settings_no_renderers:
			state_ = State::main_window;
			break;
		case State::display_settings_window:
			state_ = State::main_window;
			break;
		case State::advanced_settings_warning:
			if (dialog_result == DialogResult::cancel)
			{
				state_ = State::main_window;
			}
			else
			{
				state_ = State::advanced_settings_window;
				WindowMgr& window_mgr = WindowMgr::get_singleton();
				AdvancedSettingsWindow* const advanced_options_window = window_mgr.get_advanced_settings_window();
				configuration.is_disable_advanced_settings_warning = true;
				advanced_options_window->show(true);
			}
			break;
		case State::advanced_settings_window:
			state_ = State::main_window;
			break;
		case State::detail_settings_window:
			state_ = State::main_window;
			if (dialog_result != DialogResult::cancel)
			{
				run_the_game();
			}
			break;
		default:
			throw std::runtime_error{"Invalid state."};
	}
}

void MainWindow::do_draw()
{
	const ScaleMgr& scale_mgr = ScaleMgr::get_singleton();
	FontMgr& font_mgr = FontMgr::get_singleton();
	const float scale = scale_mgr.get_scale();
	const float regular_font_size = font_mgr.get_regular_size() * scale;
	TextureMgr& texture_mgr = get_texture_manager();
	is_escape_down_ = ImGui::IsKeyPressed(ImGuiKey_Escape);
	const bool is_modal = (state_ != State::main_window);
	const ImVec2 mouse_pos = ImGui::GetMousePos();
	// Begin main window.
	const ImGuiWindowFlags main_flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoSavedSettings |
		(is_modal ? ImGuiWindowFlags_NoInputs : ImGuiWindowFlags_None) |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_None;
	ImGui::Begin("main", nullptr, main_flags);
	ImGui::SetWindowPos(ImVec2{}, ImGuiCond_Always);
	ImGui::SetWindowSize(
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale,
		ImGuiCond_Always);
	// Background.
	ImGui::SetCursorPos(ImVec2{});
	const ImTextureID mainappbackground_tex_id = texture_mgr.get(ImageId::mainappbackground);
	ImGui::Image(
		mainappbackground_tex_id,
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale);
	// Minimize button.
	imgui_add_title_minimize_button(
		/* position       */ ImVec2{487.0F, 6.0F} * scale,
		/* size           */ ImVec2{16.0F, 14.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_minimize_button_clicked_);
	// Close button.
	imgui_add_title_close_button(
		/* position       */ ImVec2{503.0F, 6.0F} * scale,
		/* size           */ ImVec2{16.0F, 14.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_close_button_clicked_);
	// Language.
	const ImVec2 language_pos = ImVec2{14.0F, 161.0F} * scale;
	const SupportedLanguages& supported_languages = SupportedLanguages::get_singleton();
	const Languages& languages = supported_languages.get();
	const char* language_preview = nullptr;
	if (language_index_ >= 0)
	{
		language_preview = languages[language_index_].name.c_str();
	}
	else
	{
		language_preview = "???";
	}
	ImGui::SetCursorPos(language_pos);
	ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
	ImGui::PushItemWidth(100.0F * scale);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0xD0));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0xB0));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0x80));
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0xD0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0xB0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 0x80));
	is_language_selected_ = false;
	if (ImGui::BeginCombo("##language", language_preview))
	{
		const ImGuiSelectableFlags selectable_flags = (is_modal ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None);
		const int language_count = narrow_cast<int>(languages.size());
		for (int i = 0; i < language_count; ++i)
		{
			const Language& language = languages[i];
			const bool is_selected = ImGui::Selectable(
				language.name.c_str(),
				i == language_index_,
				selectable_flags);
			if (!is_modal && is_selected)
			{
				is_language_selected_ = true;
				language_index_ = i;
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopItemWidth();
	ImGui::PopFont();
	// Fox Interactive button.
	imgui_add_button(
		/* id_string      */ "publisher 1 web button",
		/* position       */ ImVec2{14.0F, 187.0F} * scale,
		/* size           */ ImVec2{52.0F, 40.0F} * scale,
		/* down_image_id  */ ImageId::publisher1webd,
		/* hover_image_id */ ImageId::publisher1webf,
		/* up_image_id    */ ImageId::publisher1webu,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_publisher1web_button_clicked_);
	// Monolith Productions button.
	imgui_add_button(
		/* id_string      */ "company 1 web button",
		/* position       */ ImVec2{76.0F, 187.0F} * scale,
		/* size           */ ImVec2{61.0F, 17.0F} * scale,
		/* down_image_id  */ ImageId::company1webd,
		/* hover_image_id */ ImageId::company1webf,
		/* up_image_id    */ ImageId::company1webu,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_company1web_button_clicked_);
	// LithTech button.
	imgui_add_button(
		/* id_string      */ "company 2 web button",
		/* position       */ ImVec2{76.0F, 210.0F} * scale,
		/* size           */ ImVec2{62.0F, 17.0F} * scale,
		/* down_image_id  */ ImageId::company2webd,
		/* hover_image_id */ ImageId::company2webf,
		/* up_image_id    */ ImageId::company2webu,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_company2web_button_clicked_);
	// Sierra Entertainment button.
	imgui_add_button(
		/* id_string      */ "publisher 2 web button",
		/* position       */ ImVec2{147.0F, 198.0F} * scale,
		/* size           */ ImVec2{99.0F, 30.0F} * scale,
		/* down_image_id  */ ImageId::publisher2webd,
		/* hover_image_id */ ImageId::publisher2webf,
		/* up_image_id    */ ImageId::publisher2webu,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_publisher2web_button_clicked_);
	// Play button.
	const ImVec2 play_pos = ImVec2{413.0F, 25.0F} * scale;
	const ImVec2 play_size = ImVec2{100.0F, 30.0F} * scale;
	ImGui::SetCursorPos(play_pos);
	is_play_button_clicked_ = ImGui::InvisibleButton("play", play_size);
	const ImageId play_image_id =
		ImGui::IsItemActive() ? ImageId::playd :
			(ImGui::IsItemHovered() ? ImageId::playf : ImageId::playu);
	ImGui::SetCursorPos(play_pos);
	ImGui::Image(texture_mgr.get(play_image_id), play_size);
	// Display button.
	const ImVec2 display_pos = ImVec2{413.0F, 97.0F} * scale;
	const ImVec2 display_size = ImVec2{100.0F, 30.0F} * scale;
	ImageId display_image_id;
	ImGui::SetCursorPos(display_pos);
	is_display_button_clicked_ = ImGui::InvisibleButton("display", display_size);
	display_image_id =
		ImGui::IsItemActive() ? ImageId::displayd : (ImGui::IsItemHovered() ? ImageId::displayf : ImageId::displayu);
	ImGui::SetCursorPos(display_pos);
	ImGui::Image(texture_mgr.get(display_image_id), display_size);
	// Options button.
	const ImVec2 options_pos = ImVec2{413.0F, 133.0F} * scale;
	const ImVec2 options_size = ImVec2{100.0F, 30.0F} * scale;
	ImageId options_image_id;
	ImGui::SetCursorPos(options_pos);
	is_options_button_clicked_ = ImGui::InvisibleButton("options", options_size);
	options_image_id =
		ImGui::IsItemActive() ? ImageId::optionsd : (ImGui::IsItemHovered() ? ImageId::optionsf : ImageId::optionsu);
	ImGui::SetCursorPos(options_pos);
	ImGui::Image(texture_mgr.get(options_image_id), options_size);
	// Quit button.
	const ImVec2 quit_pos = ImVec2{413.0F, 205.0F} * scale;
	const ImVec2 quit_size = ImVec2{100.0F, 30.0F} * scale;
	ImGui::SetCursorPos(quit_pos);
	is_quit_button_clicked_ = ImGui::InvisibleButton("quit", quit_size);
	const ImageId quit_image_id =
		ImGui::IsItemActive() ? ImageId::quitd : (ImGui::IsItemHovered() ? ImageId::quitf : ImageId::quitu);
	ImGui::SetCursorPos(quit_pos);
	ImGui::Image(texture_mgr.get(quit_image_id), quit_size);
	// End main window.
	ImGui::End();
}

void MainWindow::do_imgui_handle_events()
{
	Configuration& configuration = Configuration::get_singleton();
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	ResourceStrings& resource_strings = window_mgr.get_resource_strings();
	if (is_minimize_button_clicked_)
	{
		minimize_internal(true);
	}
	else if (is_close_button_clicked_ || is_quit_button_clicked_ || is_escape_down_)
	{
		SDL_Event sdl_event{.type = SDL_EVENT_QUIT};
		SDL_PushEvent(&sdl_event);
	}
	else if (is_play_button_clicked_)
	{
		if (configuration.is_restore_defaults)
		{
			state_ = State::detail_settings_window;
			DetailSettingsWindow* const detail_settings_window_ptr = window_mgr.get_detail_settings_window();
			detail_settings_window_ptr->show(true);
		}
		else
		{
			run_the_game();
		}
	}
	else if (is_display_button_clicked_)
	{
		if (!configuration.is_disable_display_settings_warning)
		{
			state_ = State::display_settings_warning;
			const std::string& message = resource_strings.get(ResourceStringId::ids_display_warning, "IDS_DISPLAY_WARNING");
			MessageBoxWindow* const message_box_window_ptr = window_mgr.get_message_box_window();
			message_box_window_ptr->show(MessageBoxType::warning, MessageBoxButtons::ok_cancel, message);
		}
		else
		{
			handle_check_for_renderers();
		}
	}
	else if (is_options_button_clicked_)
	{
		if (!configuration.is_disable_advanced_settings_warning)
		{
			state_ = State::advanced_settings_warning;
			const std::string& message = resource_strings.get(ResourceStringId::ids_options_warning, "IDS_OPTIONS_WARNING");
			MessageBoxWindow* const message_box_window_ptr = window_mgr.get_message_box_window();
			message_box_window_ptr->show(MessageBoxType::warning, MessageBoxButtons::ok_cancel, message);
		}
		else
		{
			state_ = State::advanced_settings_window;
			AdvancedSettingsWindow* const advanced_settings_window_ptr = window_mgr.get_advanced_settings_window();
			advanced_settings_window_ptr->show(true);
		}
	}
	else if (is_language_selected_)
	{
		const SupportedLanguages& supported_languages = SupportedLanguages::get_singleton();
		const Languages& languages = supported_languages.get();
		configuration.language = languages[language_index_].id;
		change_language();
	}
	is_minimize_button_clicked_ = false;
	is_close_button_clicked_ = false;
	is_quit_button_clicked_ = false;
	is_escape_down_ = false;
	is_play_button_clicked_ = false;
	is_display_button_clicked_ = false;
	is_options_button_clicked_ = false;
	is_language_selected_ = false;
	is_publisher1web_button_clicked_ = false;
	is_company1web_button_clicked_ = false;
	is_company2web_button_clicked_ = false;
	is_publisher2web_button_clicked_ = false;
	if (lithtech_sdl_process_uptr_ != nullptr)
	{
		int exit_code;
		if (SDL_WaitProcess(lithtech_sdl_process_uptr_.get(), false, &exit_code))
		{
			lithtech_sdl_process_uptr_ = nullptr;
			show(true);
		}
	}
}

// ======================================

DetailSettingsWindow::DetailSettingsWindow()
	:
	Window{
		WindowCreateParam{
			.width = narrow_cast<int>(window_width * ScaleMgr::get_singleton().get_scale()),
			.height = narrow_cast<int>(window_height * ScaleMgr::get_singleton().get_scale()),
			.title = "LTJS " LTJS_GAME_ID_STRING_UC " detail settings"}}
{}

DetailLevel DetailSettingsWindow::get_detail_level() const
{
	return detail_level_;
}

void DetailSettingsWindow::do_draw()
{
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	const ResourceStrings& resource_strings = window_mgr.get_resource_strings();
	const ScaleMgr& scale_mgr = ScaleMgr::get_singleton();
	FontMgr& font_mgr = FontMgr::get_singleton();
	const float scale = scale_mgr.get_scale();
	const float regular_font_size = font_mgr.get_regular_size() * scale;
	TextureMgr& texture_mgr = get_texture_manager();
	is_escape_down_ = ImGui::IsKeyPressed(ImGuiKey_Escape);
	// Begin detail settings window.
	constexpr ImGuiWindowFlags main_flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_None;
	ImGui::Begin("detail_settings", nullptr, main_flags);
	ImGui::SetWindowPos(ImVec2{}, ImGuiCond_Always);
	ImGui::SetWindowSize(
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale,
		ImGuiCond_Always);
	// Background.
	ImGui::SetCursorPos(ImVec2{});
	const ImTextureID displaybackground_tex_id = texture_mgr.get(ImageId::detailsettingsbackground);
	ImGui::Image(
		displaybackground_tex_id,
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale);
	// Close button.
	imgui_add_title_close_button(
		/* position       */ ImVec2{434.0F, 6.0F} * scale,
		/* size           */ ImVec2{16.0F, 14.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_close_button_clicked_);
	// "Cancel" button.
	imgui_add_cancel_button(
		/* position       */ ImVec2{178.0F, 440.0F} * scale,
		/* size           */ ImVec2{100.0F, 30.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_cancel_button_clicked_);
	// Header.
	const ImVec2 header_pos = ImVec2{14.0F, 45.0F} * scale;
	const ImVec2 header_size = ImVec2{428.0F, 51.0F} * scale;
	const std::string& header_text = resource_strings.get(ResourceStringId::ids_detail_header, "IDS_DETAIL_HEADER");
	ImGui::SetCursorPos(header_pos);
	ImGui::BeginChild("##header_child", header_size);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
	//
	ImGui::PushItemWidth(-1);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
	ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
	ImGui::TextWrapped("%s", header_text.c_str());
	ImGui::PopFont();
	ImGui::PopStyleColor();
	ImGui::PopItemWidth();
	//
	ImGui::PopStyleColor();
	ImGui::EndChild();
	// High detail header.
	const ImVec2 high_header_pos = ImVec2{14.0F, 119.0F} * scale;
	const ImVec2 high_header_size = ImVec2{428.0F, 40.0F} * scale;
	const std::string& high_header_text = resource_strings.get(ResourceStringId::ids_detail_high, "IDS_DETAIL_HIGH");
	ImGui::SetCursorPos(high_header_pos);
	ImGui::BeginChild("##high_header_child", high_header_size);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
	//
	ImGui::PushItemWidth(-1);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
	ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
	ImGui::TextWrapped("%s", high_header_text.c_str());
	ImGui::PopFont();
	ImGui::PopStyleColor();
	ImGui::PopItemWidth();
	//
	ImGui::PopStyleColor();
	ImGui::EndChild();
	// High detail button.
	const ImVec2 high_button_pos = ImVec2{140.0F, 165.0F} * scale;
	const ImVec2 high_button_size = ImVec2{177.0F, 38.0F} * scale;
	ImGui::SetCursorPos(high_button_pos);
	is_high_button_clicked_ = ImGui::InvisibleButton("high", high_button_size);
	const ImageId high_image_id =
		ImGui::IsItemActive() ? ImageId::highdetaild : (ImGui::IsItemHovered() ? ImageId::highdetailf : ImageId::highdetailu);
	ImGui::SetCursorPos(high_button_pos);
	ImGui::Image(texture_mgr.get(high_image_id), high_button_size);
	// Medium detail header.
	const ImVec2 medium_header_pos = ImVec2{14.0F, 228.0F} * scale;
	const ImVec2 medium_header_size = ImVec2{428.0F, 43.0F} * scale;
	const std::string& medium_header_text = resource_strings.get(ResourceStringId::ids_detail_medium, "IDS_DETAIL_MEDIUM");
	ImGui::SetCursorPos(medium_header_pos);
	ImGui::BeginChild("##medium_header_child", medium_header_size);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
	//
	ImGui::PushItemWidth(-1);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
	ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
	ImGui::TextWrapped("%s", medium_header_text.c_str());
	ImGui::PopFont();
	ImGui::PopStyleColor();
	ImGui::PopItemWidth();
	//
	ImGui::PopStyleColor();
	ImGui::EndChild();
	// Medium detail button.
	const ImVec2 medium_button_pos = ImVec2{140.0F, 274.0F} * scale;
	const ImVec2 medium_button_size = ImVec2{178.0F, 36.0F} * scale;
	ImGui::SetCursorPos(medium_button_pos);
	is_medium_button_clicked_ = ImGui::InvisibleButton("medium", medium_button_size);
	const ImageId medium_image_id =
		ImGui::IsItemActive() ? ImageId::mediumdetaild : (ImGui::IsItemHovered() ? ImageId::mediumdetailf : ImageId::mediumdetailu);
	ImGui::SetCursorPos(medium_button_pos);
	ImGui::Image(texture_mgr.get(medium_image_id), medium_button_size);
	// Low detail header.
	const ImVec2 low_header_pos = ImVec2{14.0F, 337.0F} * scale;
	const ImVec2 low_header_size = ImVec2{428.0F, 43.0F} * scale;
	const std::string& low_header_text = resource_strings.get(ResourceStringId::ids_detail_low, "IDS_DETAIL_LOW");
	ImGui::SetCursorPos(low_header_pos);
	ImGui::BeginChild("##low_header_child", low_header_size);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
	//
	ImGui::PushItemWidth(-1);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
	ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
	ImGui::TextWrapped("%s", low_header_text.c_str());
	ImGui::PopFont();
	ImGui::PopStyleColor();
	ImGui::PopItemWidth();
	//
	ImGui::PopStyleColor();
	ImGui::EndChild();
	// Low detail button.
	const ImVec2 low_button_pos = ImVec2{140.0F, 383.0F} * scale;
	const ImVec2 low_button_size = ImVec2{178.0F, 35.0F} * scale;
	ImGui::SetCursorPos(low_button_pos);
	is_low_button_clicked_ = ImGui::InvisibleButton("low", low_button_size);
	const ImageId low_image_id =
		ImGui::IsItemActive() ? ImageId::lowdetaild : (ImGui::IsItemHovered() ? ImageId::lowdetailf : ImageId::lowdetailu);
	ImGui::SetCursorPos(low_button_pos);
	ImGui::Image(texture_mgr.get(low_image_id), low_button_size);
	// End detail settings window.
	ImGui::End();
}

void DetailSettingsWindow::do_imgui_handle_events()
{
	DialogResult dialog_result = DialogResult::none;
	if (is_close_button_clicked_ || is_cancel_button_clicked_ || is_escape_down_)
	{
		detail_level_ = DetailLevel{};
		dialog_result = DialogResult::cancel;
	}
	else
	{
		bool has_level = false;
		if (is_high_button_clicked_)
		{
			has_level = true;
			detail_level_ = DetailLevel::high;
		}
		else if (is_medium_button_clicked_)
		{
			has_level = true;
			detail_level_ = DetailLevel::medium;
		}
		else if (is_low_button_clicked_)
		{
			has_level = true;
			detail_level_ = DetailLevel::low;
		}
		if (has_level)
		{
			dialog_result = DialogResult::ok;
		}
	}
	is_close_button_clicked_ = false;
	is_cancel_button_clicked_ = false;
	is_high_button_clicked_ = false;
	is_medium_button_clicked_ = false;
	is_low_button_clicked_ = false;
	if (dialog_result != DialogResult::none)
	{
		show(false);
		set_dialog_result(dialog_result);
	}
}

// ======================================

DisplaySettingsWindow::DisplaySettingsWindow()
	:
	Window{
		WindowCreateParam{
			.width = narrow_cast<int>(window_width * ScaleMgr::get_singleton().get_scale()),
			.height = narrow_cast<int>(window_height * ScaleMgr::get_singleton().get_scale()),
			.title = "LTJS " LTJS_GAME_ID_STRING_UC " display settings"}}
{
	const DisplayModeMgr& display_mode_mgr = DisplayModeMgr::get_singleton();
	Configuration& configuration = Configuration::get_singleton();
	selected_resolution_index_ = display_mode_mgr.find_mode_index(configuration.screen_width, configuration.screen_height);
	if (selected_resolution_index_ < 0)
	{
		selected_resolution_index_ = display_mode_mgr.get_native_display_mode_index();
	}
	selected_resolution_index_.accept();
}

const char* DisplaySettingsWindow::im_display_modes_getter(void* data, int idx)
{
	const DisplayMode* const display_modes = static_cast<const DisplayMode*>(data);
	const DisplayMode& display_mode = display_modes[idx];
	return display_mode.as_string.c_str();
}

void DisplaySettingsWindow::do_draw()
{
	const DisplayModeMgr& display_mode_mgr = DisplayModeMgr::get_singleton();
	const ScaleMgr& scale_mgr = ScaleMgr::get_singleton();
	FontMgr& font_mgr = FontMgr::get_singleton();
	const float scale = scale_mgr.get_scale();
	const float regular_font_size = font_mgr.get_regular_size() * scale;
	TextureMgr& texture_mgr = get_texture_manager();
	is_escape_down_ = ImGui::IsKeyPressed(ImGuiKey_Escape);
	has_display_modes_ = (
		selected_resolution_index_ >= 0 &&
		display_mode_mgr.get_mode_count() > 0 &&
		display_mode_mgr.get_native_display_mode_index() >= 0);
	// Begin main window.
	constexpr ImGuiWindowFlags main_flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_None;
	ImGui::Begin("display_settings", nullptr, main_flags);
	ImGui::SetWindowPos(ImVec2{}, ImGuiCond_Always);
	ImGui::SetWindowSize(
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale,
		ImGuiCond_Always);
	// Background.
	ImGui::SetCursorPos(ImVec2{});
	const ImTextureID displaybackground_tex_id = texture_mgr.get(ImageId::displaybackground);
	ImGui::Image(
		displaybackground_tex_id,
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale);
	// Close button.
	imgui_add_title_close_button(
		/* position       */ ImVec2{578.0F, 6.0F} * scale,
		/* size           */ ImVec2{16.0F, 14.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_close_button_clicked_);
	// Resolutions list.
	if (has_display_modes_)
	{
		const ImVec2 resolutions_pos = ImVec2{16.0F, 73.0F} * scale;
		const ImVec2 resolutions_size = ImVec2{133.0F, 248.0F} * scale;
		const ImVec4 resolutions_rect{resolutions_pos.x, resolutions_pos.y, resolutions_size.x, resolutions_size.y};
		ImGui::SetCursorPos(resolutions_pos);
		ImGui::BeginChild("resolutions", resolutions_size);
		ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
		ImGui::PushItemWidth(-1.0F);
		const float resolutions_list_box_item_height = ImGui::GetTextLineHeightWithSpacing();
		const float resolutions_height_in_items = resolutions_size.y / resolutions_list_box_item_height;
		const std::span<const DisplayMode> display_modes = display_mode_mgr.get_modes();
		ImGui::ListBox(
			"##resolutions",
			selected_resolution_index_,
			&DisplaySettingsWindow::im_display_modes_getter,
			const_cast<DisplayMode*>(display_modes.data()),
			narrow_cast<int>(display_modes.size()),
			narrow_cast<int>(resolutions_height_in_items));
		ImGui::PopItemWidth();
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::EndChild();
	}
	// Renderers list.
	if (has_display_modes_)
	{
		const ImVec2 renderers_pos = ImVec2{181.0F, 73.0F} * scale;
		const ImVec2 renderers_size = ImVec2{405.0F, 106.0F} * scale;
		const ImVec4 renderers_rect{renderers_pos.x, renderers_pos.y, renderers_size.x, renderers_size.y};
		ImGui::SetCursorPos(renderers_pos);
		ImGui::BeginChild("renderers", renderers_size);
		ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
		ImGui::PushItemWidth(-1.0F);
		const float renderers_list_box_item_height = ImGui::GetTextLineHeightWithSpacing();
		const float renderers_height_in_items = renderers_size.y / renderers_list_box_item_height;
		const char* const renderers_list[] = {Direct3d9::get_renderer_name().c_str()};
		int selected_renderer_index = 0;
		ImGui::ListBox(
			"##renderers",
			&selected_renderer_index,
			renderers_list,
			1,
			narrow_cast<int>(renderers_height_in_items));
		ImGui::PopItemWidth();
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::EndChild();
	}
	// Displays list.
	if (has_display_modes_)
	{
		const ImVec2 displays_pos = ImVec2{181.0F, 205.0F} * scale;
		const ImVec2 displays_size = ImVec2{405.0F, 116.0F} * scale;
		const ImVec4 displays_rect{displays_pos.x, displays_pos.y, displays_size.x, displays_size.y};
		ImGui::SetCursorPos(displays_pos);
		ImGui::BeginChild("displays", displays_size);
		ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
		ImGui::PushItemWidth(-1.0F);
		const float displays_list_box_item_height = ImGui::GetTextLineHeightWithSpacing();
		const float displays_height_in_items = displays_size.y / displays_list_box_item_height;
		const char* const displays_list[] = {Direct3d9::get_display_name().c_str()};
		int selected_displays_index = 0;
		ImGui::ListBox(
			"##displays",
			&selected_displays_index,
			displays_list,
			1,
			narrow_cast<int>(displays_height_in_items));
		ImGui::PopItemWidth();
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::EndChild();
	}
	// "Ok" button.
	imgui_add_ok_button(
		/* position       */ ImVec2{194.0F, 349.0F} * scale,
		/* size           */ ImVec2{100.0F, 30.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_ok_button_clicked_);
	// "Cancel" button.
	imgui_add_cancel_button(
		/* position       */ ImVec2{306.0F, 349.0F} * scale,
		/* size           */ ImVec2{100.0F, 30.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_cancel_button_clicked_);
	// End display settings window.
	ImGui::End();
}

void DisplaySettingsWindow::do_imgui_handle_events()
{
	DialogResult dialog_result = DialogResult::none;
	if (is_close_button_clicked_ || is_cancel_button_clicked_ || is_escape_down_)
	{
		selected_resolution_index_.reject();
		dialog_result = DialogResult::cancel;
	}
	else if (is_ok_button_clicked_)
	{
		selected_resolution_index_.accept();

		if (has_display_modes_)
		{
			Configuration& configuration = Configuration::get_singleton();
			const DisplayModeMgr& display_mode_mgr = DisplayModeMgr::get_singleton();
			const DisplayMode& display_mode = display_mode_mgr.get_mode_by_index(selected_resolution_index_);
			configuration.screen_width = display_mode.width;
			configuration.screen_height = display_mode.height;
		}

		dialog_result = DialogResult::ok;
	}
	is_escape_down_ = false;
	has_display_modes_ = false;
	is_close_button_clicked_ = false;
	is_ok_button_clicked_ = false;
	is_cancel_button_clicked_ = false;
	if (dialog_result != DialogResult::none)
	{
		show(false);
		set_dialog_result(dialog_result);
	}
}

// ======================================

AdvancedSettingsWindow::AdvancedSettingsWindow()
	:
	Window{
		WindowCreateParam{
			.width = narrow_cast<int>(window_width * ScaleMgr::get_singleton().get_scale()),
			.height = narrow_cast<int>(window_height * ScaleMgr::get_singleton().get_scale()),
			.title = "LTJS " LTJS_GAME_ID_STRING_UC " advanced settings"}}
{
	initialize_check_box_contents();
}

void AdvancedSettingsWindow::initialize_check_box_contents()
{
	Configuration& configuration = Configuration::get_singleton();
	check_box_contexts_ = {
		// Disable sound.
		CheckBoxContext{
			.position = ImVec2{26.0F, 69.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_disablesound,
			.resource_string_default = "IDS_OD_DISABLESOUND",
			.hint_resource_string_id = ResourceStringId::ids_help_disablesound,
			.hint_resource_string_default = "IDS_HELP_DISABLESOUND",
			.setting_value_ptr = &configuration.is_disable_sound_effects},
		// Disable music.
		CheckBoxContext{
			.position = ImVec2{26.0F, 94.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_disablemusic,
			.resource_string_default = "IDS_OD_DISABLEMUSIC",
			.hint_resource_string_id = ResourceStringId::ids_help_disablemusic,
			.hint_resource_string_default = "IDS_HELP_DISABLEMUSIC",
			.setting_value_ptr = &configuration.is_disable_music},
		// Disable movies.
		CheckBoxContext{
			.position = ImVec2{26.0F, 119.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_disablemovies,
			.resource_string_default = "IDS_OD_DISABLEMOVIES",
			.hint_resource_string_id = ResourceStringId::ids_help_disablemovies,
			.hint_resource_string_default = "IDS_HELP_DISABLEMOVIES",
			.setting_value_ptr = &configuration.is_disable_fmvs},
		// Disable hardware sound.
		CheckBoxContext{
			.position = ImVec2{26.0F, 144.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_disablehardwaresound,
			.resource_string_default = "IDS_OD_DISABLEHARDWARESOUND",
			.hint_resource_string_id = ResourceStringId::ids_help_disablehardwaresound,
			.hint_resource_string_default = "IDS_HELP_DISABLEHARDWARESOUND",
			.setting_value_ptr = &configuration.is_disable_hardware_sound},
		// Disable animated load screens.
		CheckBoxContext{
			.position = ImVec2{26.0F, 169.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_disableanimatedloadscreens,
			.resource_string_default = "IDS_OD_DISABLEANIMATEDLOADSCREENS",
			.hint_resource_string_id = ResourceStringId::ids_help_disableanimatedloadscreen,
			.hint_resource_string_default = "IDS_HELP_DISABLEANIMATEDLOADSCREEN",
			.setting_value_ptr = &configuration.is_disable_animated_loading_screen},
		// Disable triple buffering.
		CheckBoxContext{
			.position = ImVec2{228.0F, 69.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_disabletriplebuffering,
			.resource_string_default = "IDS_OD_DISABLETRIPLEBUFFERING",
			.hint_resource_string_id = ResourceStringId::ids_help_disabletriplebuffering,
			.hint_resource_string_default = "IDS_HELP_DISABLETRIPLEBUFFERING",
			.setting_value_ptr = &configuration.is_disable_triple_buffering},
		// Disable joysticks.
		CheckBoxContext{
			.position = ImVec2{228.0F, 94.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_disablejoysticks,
			.resource_string_default = "IDS_OD_DISABLEJOYSTICKS",
			.hint_resource_string_id = ResourceStringId::ids_help_disablejoysticks,
			.hint_resource_string_default = "IDS_HELP_DISABLEJOYSTICKS",
			.setting_value_ptr = &configuration.is_disable_controllers},
		// Disable hardware cursor.
		CheckBoxContext{
			.position = ImVec2{228.0F, 119.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_disablehardwarecursor,
			.resource_string_default = "IDS_OD_DISABLEHARDWARECURSOR",
			.hint_resource_string_id = ResourceStringId::ids_help_disablehardwarecursor,
			.hint_resource_string_default = "IDS_HELP_DISABLEHARDWARECURSOR",
			.setting_value_ptr = &configuration.is_disable_hardware_cursor},
		// Disable sound filters.
		CheckBoxContext{
			.position = ImVec2{228.0F, 144.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_disablesoundfilters,
			.resource_string_default = "IDS_OD_DISABLESOUNDFILTERS",
			.hint_resource_string_id = ResourceStringId::ids_help_disablesoundfilters,
			.hint_resource_string_default = "IDS_HELP_DISABLESOUNDFILTERS",
			.setting_value_ptr = &configuration.is_disable_sound_filters},
		// Restore defaults.
		CheckBoxContext{
			.position = ImVec2{26.0F, 221.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_restoredefaults,
			.resource_string_default = "IDS_OD_RESTOREDEFAULTS",
			.hint_resource_string_id = ResourceStringId::ids_help_restoredefaults,
			.hint_resource_string_default = "IDS_HELP_RESTOREDEFAULTS",
			.setting_value_ptr = &configuration.is_restore_defaults},
		// Always pass command line.
		CheckBoxContext{
			.position = ImVec2{26.0F, 297.0F},
			.checked_value = true,
			.is_pressed = false,
			.resource_string_id = ResourceStringId::ids_od_alwaysspecify,
			.resource_string_default = "IDS_OD_ALWAYSSPECIFY",
			.hint_resource_string_id = ResourceStringId::ids_help_alwaysspecify,
			.hint_resource_string_default = "IDS_HELP_ALWAYSSPECIFY",
			.setting_value_ptr = &configuration.is_pass_custom_arguments}
	};
}

void AdvancedSettingsWindow::do_draw()
{
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	const ResourceStrings& resource_strings = window_mgr.get_resource_strings();
	Configuration& configuration = Configuration::get_singleton();
	const ScaleMgr& scale_mgr = ScaleMgr::get_singleton();
	FontMgr& font_mgr = FontMgr::get_singleton();
	const float scale = scale_mgr.get_scale();
	const float regular_font_size = font_mgr.get_regular_size() * scale;
	TextureMgr& texture_mgr = get_texture_manager();
	is_escape_down_ = ImGui::IsKeyPressed(ImGuiKey_Escape);
	hint_ = resource_strings.get(ResourceStringId::ids_help_default, "IDS_HELP_DEFAULT");
	// Begin advanced settings window.
	constexpr ImGuiWindowFlags main_flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_None;
	ImGui::Begin("advanced settings", nullptr, main_flags);
	ImGui::SetWindowPos(ImVec2{}, ImGuiCond_Always);
	ImGui::SetWindowSize(
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale,
		ImGuiCond_Always);
	// Background.
	ImGui::SetCursorPos(ImVec2{});
	const ImTextureID displaybackground_tex_id = texture_mgr.get(ImageId::optionsbackground);
	ImGui::Image(
		displaybackground_tex_id,
		ImVec2{narrow_cast<float>(window_width), narrow_cast<float>(window_height)} * scale);
	// Close button.
	const ImVec2 close_pos = ImVec2{434.0F, 6.0F} * scale;
	const ImVec2 close_size = ImVec2{17.0F, 14.0F} * scale;
	imgui_add_title_close_button(
		/* position       */ close_pos,
		/* size           */ close_size,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_close_button_clicked_);
	// Check boxes.
	const int check_box_count = narrow_cast<int>(check_box_contexts_.size());
	for (int i_check_box = 0; i_check_box < check_box_count; ++i_check_box)
	{
		draw_check_box(i_check_box);
	}
	// Command line.
	const ImVec2 cmd_pos = ImVec2{25.0F, 265.0F} * scale;
	const ImVec2 cmd_size = ImVec2{405.0F, 23.0F} * scale;
	const ImVec4 cmd_rect{cmd_pos.x, cmd_pos.y, cmd_size.x, cmd_size.y};
	ImGui::SetCursorPos(cmd_pos);
	ImFont* const cmd_font = get_regular_imgui_font();
	const ImVec2 cmd_input_pos = center_size_inside_rect(cmd_rect, ImVec2{0.0F, regular_font_size});
	const ImVec2 cmd_input_rel_pos{0.0F, cmd_input_pos.y - cmd_rect.y};
	ImGui::BeginChild("##command_line_child", cmd_size);
	ImGui::SetCursorPos(cmd_input_rel_pos);
	ImGui::PushFont(cmd_font, regular_font_size);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
	ImGui::PushItemWidth(-1);
	ImGui::InputText("##command_line", configuration.custom_arguments.get_ptr());
	ImGui::PopItemWidth();
	ImGui::PopStyleColor();
	ImGui::PopFont();
	ImGui::EndChild();
	// Hint.
	const ImVec2 hint_pos = ImVec2{14.0F, 341.0F} * scale;
	const ImVec2 hint_size = ImVec2{418.0F, 74.0F} * scale;
	const ImVec4 hint_rect{close_pos.x, close_pos.y, close_size.x, close_size.y};
	ImGui::SetCursorPos(hint_pos);
	ImGui::BeginChild("##hint_text", hint_size);
	ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
	ImGui::TextWrapped("%s", hint_.c_str());
	ImGui::PopStyleColor();
	ImGui::PopFont();
	ImGui::EndChild();
	// "Ok" button.
	imgui_add_ok_button(
		/* position       */ ImVec2{123.0F, 435.0F} * scale,
		/* size           */ ImVec2{100.0F, 30.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_ok_button_clicked_);
	// "Cancel" button.
	imgui_add_cancel_button(
		/* position       */ ImVec2{235.0F, 435.0F} * scale,
		/* size           */ ImVec2{100.0F, 30.0F} * scale,
		/* texture_mgr    */ texture_mgr,
		/* out_is_clicked */ is_cancel_button_clicked_);
	// End advanced settings window.
	ImGui::End();
}

void AdvancedSettingsWindow::do_imgui_handle_events()
{
	Configuration& configuration = Configuration::get_singleton();
	DialogResult dialog_result = DialogResult::none;
	if (is_close_button_clicked_ || is_cancel_button_clicked_ || is_escape_down_)
	{
		update_check_box_configuration(false);
		configuration.custom_arguments.reject();
		dialog_result = DialogResult::cancel;
	}
	else if (is_ok_button_clicked_)
	{
		update_check_box_configuration(true);
		configuration.custom_arguments.accept();
		dialog_result = DialogResult::ok;
	}
	is_escape_down_ = false;
	is_close_button_clicked_ = false;
	is_ok_button_clicked_ = false;
	is_cancel_button_clicked_ = false;
	if (dialog_result != DialogResult::none)
	{
		show(false);
		set_dialog_result(dialog_result);
	}
}

void AdvancedSettingsWindow::update_check_box_configuration(bool is_accept)
{
	const auto method = (is_accept ? &SettingValue<bool>::accept : &SettingValue<bool>::reject);
	for (CheckBoxContext& check_box_context : check_box_contexts_)
	{
		(check_box_context.setting_value_ptr->*method)();
	}
}

void AdvancedSettingsWindow::draw_check_box(int index)
{
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	FontMgr& font_mgr = FontMgr::get_singleton();
	ScaleMgr& scale_mgr = ScaleMgr::get_singleton();
	const float scale = scale_mgr.get_scale();
	const float regular_font_size = font_mgr.get_regular_size() * scale;
	const ResourceStrings& resource_strings = window_mgr.get_resource_strings();
	TextureMgr& texture_mgr = get_texture_manager();
	const bool im_is_mouse_button_down = ImGui::IsMouseDown(0);
	const bool im_is_mouse_button_up = ImGui::IsMouseReleased(0);
	const ImVec2 im_mouse_pos = ImGui::GetMousePos();
	const float check_box_space_px = 4 * scale;
	const ImU32 check_box_text_normal_color = IM_COL32(0xC0, 0xA0, 0x1C, 0xFF);
	const ImU32 check_box_text_highlighted_color = IM_COL32(0xFF, 0xFF, 0x0B, 0xFF);
	const ImU32 check_box_text_disabled_color = IM_COL32(0x80, 0x80, 0x80, 0xFF);
	CheckBoxContext& check_box_context = check_box_contexts_[index];
	const ImVec2 check_box_size = ImVec2{20.0F, 11.0F} * scale;
	const ImVec2 check_box_pos = check_box_context.position * scale;
	const ImVec4 check_box_rect{check_box_pos.x, check_box_pos.y, check_box_size.x, check_box_size.y};
	const std::string& text = resource_strings.get(
		check_box_context.resource_string_id,
		check_box_context.resource_string_default);
	ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
	const ImVec2& text_size = ImGui::CalcTextSize(text.c_str(), text.c_str() + text.size());
	ImGui::PopFont();
	const ImVec2 text_pos{
		check_box_pos.x + check_box_size.x + check_box_space_px,
		check_box_pos.y - (text_size.y - check_box_size.y) * 0.5F};
	const ImVec2 size{
		check_box_size.x + check_box_space_px + text_size.x,
		check_box_size.y};
	const ImVec4 rect{check_box_pos.x, check_box_pos.y, size.x, size.y};
	bool is_mouse_button_down = false;
	const bool is_hightlighted = is_point_inside_rect(im_mouse_pos, rect);
	if (is_hightlighted && (im_is_mouse_button_down || im_is_mouse_button_up))
	{
		if (im_is_mouse_button_down)
		{
			if (!check_box_context.is_pressed)
			{
				check_box_context.is_pressed = true;
				is_mouse_button_down = true;
			}
		}
		if (im_is_mouse_button_up)
		{
			check_box_context.is_pressed = false;
		}
	}
	ImageId disable_sound_image_id;
	ImU32 disable_sound_check_box_color;
	SettingValue<bool>& setting_value = *check_box_context.setting_value_ptr;
	if (is_mouse_button_down)
	{
		setting_value = !setting_value;
	}
	if (setting_value == check_box_context.checked_value)
	{
		disable_sound_image_id = ImageId::checkboxc;
		disable_sound_check_box_color = check_box_text_disabled_color;
	}
	else if (is_hightlighted)
	{
		disable_sound_image_id = ImageId::checkboxf;
		disable_sound_check_box_color = check_box_text_highlighted_color;
		hint_ = resource_strings.get(
			check_box_context.hint_resource_string_id,
			check_box_context.hint_resource_string_default);
	}
	else
	{
		disable_sound_image_id = ImageId::checkboxn;
		disable_sound_check_box_color = check_box_text_normal_color;
	}
	const ImTextureID disable_sound_tex_id = texture_mgr.get(disable_sound_image_id);
	ImGui::SetCursorPos(check_box_pos);
	ImGui::Image(disable_sound_tex_id, check_box_size);
	ImGui::SetCursorPos(text_pos);
	ImGui::PushFont(get_regular_imgui_font(), regular_font_size);
	ImGui::PushStyleColor(ImGuiCol_Text, disable_sound_check_box_color);
	ImGui::Text("%s", text.c_str());
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

// ======================================

WindowMgr& WindowMgr::get_singleton()
{
	static WindowMgr window_mgr{};
	return window_mgr;
}

MainWindow* WindowMgr::get_main_window()
{
	return main_window_.get();
}

DisplaySettingsWindow* WindowMgr::get_display_settings_window()
{
	return display_settings_window_.get();
}

AdvancedSettingsWindow* WindowMgr::get_advanced_settings_window()
{
	return advanced_settings_window_.get();
}

DetailSettingsWindow* WindowMgr::get_detail_settings_window()
{
	return detail_settings_window_.get();
}

MessageBoxWindow* WindowMgr::get_message_box_window()
{
	return message_box_window_.get();
}

const ResourceStrings& WindowMgr::get_resource_strings() const
{
	return resource_strings_;
}

ResourceStrings& WindowMgr::get_resource_strings()
{
	return resource_strings_;
}

const SearchPaths& WindowMgr::get_search_paths() const
{
	return search_paths_;
}

void WindowMgr::setup_search_paths(const std::string& language_id_string)
{
	search_paths_.clear();
	search_paths_.reserve(2);
	const std::string& base_path = Configuration::get_resources_base_path();
	search_paths_.emplace_back(normalize_file_path(base_path));
	search_paths_.emplace_back(combine_and_normalize_file_paths(base_path, language_id_string));
}

void WindowMgr::hide_all()
{
	main_window_->hide();
	display_settings_window_->hide();
	advanced_settings_window_->hide();
	detail_settings_window_->hide();
	message_box_window_->hide();
}

void WindowMgr::handle_event(const SDL_Event& sdl_event)
{
	for (Window* window : windows_)
	{
		window->handle_event(sdl_event);
	}
}

void WindowMgr::imgui_handle_events()
{
	for (Window* window : windows_)
	{
		window->imgui_handle_events();
	}
}

void WindowMgr::draw()
{
	for (Window* window : windows_)
	{
		window->draw();
	}
}

WindowMgr::WindowMgr()
{
	message_box_window_ = std::make_unique<MessageBoxWindow>();
	detail_settings_window_ = std::make_unique<DetailSettingsWindow>();
	display_settings_window_ = std::make_unique<DisplaySettingsWindow>();
	advanced_settings_window_ = std::make_unique<AdvancedSettingsWindow>();
	main_window_ = std::make_unique<MainWindow>();
	//
	message_box_window_->set_parent(main_window_.get());
	detail_settings_window_->set_parent(main_window_.get());
	display_settings_window_->set_parent(main_window_.get());
	advanced_settings_window_->set_parent(main_window_.get());
	//
	windows_ = {
		main_window_.get(),
		detail_settings_window_.get(),
		display_settings_window_.get(),
		advanced_settings_window_.get(),
		message_box_window_.get()};
}

// ======================================

std::string Launcher::launcher_commands_file_name = "launchcmds.txt";
std::string Launcher::resource_strings_file_name = "strings.txt";
std::string Launcher::icon_path = "ltjs/" LTJS_GAME_ID_STRING "/launcher/icon_48x48.bmp";

Launcher& Launcher::get_singleton()
{
	static Launcher launcher{};
	return launcher;
}

void Launcher::initialize()
{
	initialize_sdl();
	Configuration& configuration = Configuration::get_singleton();
	{
		const std::string& config_directory = configuration.get_config_path();
		const bool is_config_directory_created = SDL_CreateDirectory(config_directory.c_str());
		assert(is_config_directory_created);
		const std::string& log_file_name = configuration.get_log_file_name();
		const std::string file_path = combine_and_normalize_file_paths(config_directory, log_file_name);
		logger_ = make_logger(LTJS_GAME_ID_STRING, file_path.c_str());
	}
	{
		logger_->info("[Window icon]");
		const std::string& normalized_icon_path = normalize_file_path(icon_path);
		icon_sdl_surface_uptr_.reset(SDL_LoadBMP(normalized_icon_path.c_str()));
		if (icon_sdl_surface_uptr_ == nullptr)
		{
			logger_->warn("Failed to load icon \"" + normalized_icon_path + "\".");
		}
	}
	SupportedLanguages& supported_languages = SupportedLanguages::get_singleton();
	logger_->info("[Supported languages]");
	supported_languages.load();
	if (supported_languages.get().empty())
	{
		fail("No supported languages.");
	}
	configuration.reload();
	if (!supported_languages.has_id(configuration.language))
	{
		fail("Unsupported language. (id={})", configuration.language.get_ref());
	}
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	window_mgr.setup_search_paths(configuration.language);
	ResourceStrings& resource_strings = window_mgr.get_resource_strings();
	{
		const bool resource_strings_result = resource_strings.initialize(
			window_mgr.get_search_paths(),
			resource_strings_file_name);

		if (!resource_strings_result)
		{
			fail(resource_strings.get_error_message());
		}
	}
	// WindowManager
	[[maybe_unused]] ScaleMgr& scale_mgr = ScaleMgr::get_singleton();
	// FontManager
	logger_->info("[Font manager]");
	[[maybe_unused]] FontMgr& font_mgr = FontMgr::get_singleton();
	{
		logger_->info("[Message box window]");
		MessageBoxWindow* const message_box_window = window_mgr.get_message_box_window();
		const std::string& title = resource_strings.get(ResourceStringId::ids_appname, "IDS_APPNAME");
		message_box_window->set_icon(icon_sdl_surface_uptr_.get());
		message_box_window->set_message_box_title(title);
	}
	{
		logger_->info("[Display settings window]");
		window_mgr.get_display_settings_window()->set_icon(icon_sdl_surface_uptr_.get());
	}
	{
		logger_->info("[Advanced settings window]");
		window_mgr.get_advanced_settings_window()->set_icon(icon_sdl_surface_uptr_.get());
	}
	{
		logger_->info("[Detail settings window]");
		window_mgr.get_detail_settings_window()->set_icon(icon_sdl_surface_uptr_.get());
	}
	{
		logger_->info("[Main window]");
		window_mgr.get_main_window()->set_icon(icon_sdl_surface_uptr_.get());
	}
}

Launcher::Launcher()
{
	initialize();
}

Logger* Launcher::get_logger()
{
	return logger_.get();
}

void Launcher::run()
{
	logger_->info("[Main loop]");
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	window_mgr.get_main_window()->show(true);
	constexpr Uint64 max_delay_ns = 100'000'000;
	constexpr Uint64 delay_bias_ns = 5'000'000;
	constexpr Uint64 target_fps = 60;
	constexpr Uint64 target_delay_ns = 1'000'000'000 / target_fps;
	while (!is_quit_requested_)
	{
		const Uint64 begin_time = SDL_GetTicksNS();
		handle_events();
		draw();
		const Uint64 end_time = SDL_GetTicksNS();
		const Uint64 process_time_ns = end_time - begin_time;
		const Uint64 delay_ns = std::clamp(target_delay_ns - process_time_ns - delay_bias_ns, 0ULL, max_delay_ns);
		if (delay_ns > 5'000'000)
		{
			SDL_DelayNS(delay_ns);
		}
	}
	Configuration& configuration = Configuration::get_singleton();
	configuration.save();
}

void Launcher::initialize_sdl()
{
	if (const bool sdl_result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
		!sdl_result)
	{
		fail("Failed to initialize SDL. (error_message={})", SDL_GetError());
	}
}

void Launcher::handle_events()
{
	WindowMgr& window_mgr = WindowMgr::get_singleton();
	SDL_Event sdl_event;
	while (SDL_PollEvent(&sdl_event))
	{
		if (sdl_event.type == SDL_EVENT_QUIT)
		{
			is_quit_requested_ = true;
		}
		window_mgr.handle_event(sdl_event);
	}
	window_mgr.imgui_handle_events();
}

void Launcher::draw()
{
	WindowMgr::get_singleton().draw();
}

// ======================================

ImVec2 operator*(const ImVec2& v, float scale)
{
	return {v.x * scale, v.y * scale};
}

ImVec2& operator*=(ImVec2& v, float scale)
{
	v.x *= scale;
	v.y *= scale;
	return v;
}

} // namespace

} // namespace ltjs::launcher

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	try
	{
		ltjs::launcher::Launcher& launcher = ltjs::launcher::Launcher::get_singleton();
		launcher.run();
		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		const std::string title = std::format("LTJS {} Launcher", LTJS_GAME_ID_STRING_UC);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), exception.what(), nullptr);
		return EXIT_FAILURE;
	}
}
