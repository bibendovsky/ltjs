//------------------------------------------------------------------
//
//	FILE	  : Input.cpp
//
//	PURPOSE	  : 
//
//	CREATED	  : May 17 1997
//
//	COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

#include "bdefs.h"

#if LTJS_SDL_BACKEND
#include <string.h>

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <list>
#include <memory>
#include <limits>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

//#include "dinput.h"
#include "concommand.h"
#include "console.h"
#include "ltpvalue.h"
#include "input.h"

#include "ltjs_dinput.h"
#include "ltjs_exception.h"
#include "ltjs_sdl_joystick_guid.h"
#include "ltjs_sdl_subsystem.h"
#include "ltjs_sdl_uresources.h"
#include "ltjs_system_event_handler.h"
#include "ltjs_system_event_queue.h"


namespace
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

enum class SdlInputAxisMode
{
	none,

	absolute,
	relative,
}; // SdlInputAxisMode

enum class SdlInputDeviceObjectType
{
	none,

	x_axis,
	y_axis,
	z_axis,

	rx_axis,
	ry_axis,
	rz_axis,

	key,
	pov,
	push_button,
}; // SdlInputDeviceObjectType

enum class SdlInputDeviceType
{
	none,

	mouse,
	keyboard,
	gamepad,
}; // SdlInputDeviceType

struct SdlInputDeviceObject
{
	using Name = std::string;


	int id{};
	SdlInputDeviceObjectType type{};
	SdlInputAxisMode axis_mode{};
	Name name{};
}; // SdlInputDeviceObject

struct SdlInputDeviceObjectDataFormat
{
	int offset{};
	int object_id{};
}; // SdlInputDeviceObjectDataFormat

struct SdlInputDeviceObjectData
{
	int offset{};
	int value{};
}; // SdlInputDeviceObjectData

struct SdlInputDeviceDataFormat
{
	int state_size{};
	int object_count{};
	const SdlInputDeviceObjectDataFormat* object_data_formats{};
}; // SdlInputDeviceDataFormat

struct SdlInputDeviceInfo
{
	using Name = std::string;


	int id{};
	SdlInputDeviceType type{};
	Name name{};
}; // SdlInputDeviceInfo

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void sdl_input_write_axis_data(
	void* buffer,
	int offset,
	int value)
{
	assert(buffer);
	assert(offset >= 0);

	auto& axis = *reinterpret_cast<int*>(static_cast<char*>(buffer) + offset);
	axis = value;
}

void sdl_input_write_button_data(
	void* buffer,
	int offset,
	bool value)
{
	assert(buffer);
	assert(offset >= 0);

	auto& button = *(static_cast<unsigned char*>(buffer) + offset);
	button = (value ? 0x80 : 0x00);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInputDevice
{
public:
	SdlInputDevice() noexcept = default;

	virtual ~SdlInputDevice() = default;


	virtual const SdlInputDeviceInfo& get_device_info() const noexcept = 0;


	virtual int get_object_count() const noexcept = 0;

	virtual const SdlInputDeviceObject& get_object(
		int index) const = 0;

	virtual const SdlInputDeviceObject* find_object_by_id(
		int object_id) const noexcept = 0;


	virtual void set_data_format(
		const SdlInputDeviceDataFormat& data_format) = 0;


	virtual void get_state(
		void* state_buffer,
		int state_size) = 0;

	virtual int get_data(
		SdlInputDeviceObjectData* object_datas,
		int max_count) = 0;

	virtual void flush_data() = 0;


	virtual void handle_system_event(
		const ltjs::SystemEvent& system_event) = 0;
}; // SdlInputDevice

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInputMouseDeviceException :
	public ltjs::Exception
{
public:
	explicit SdlInputMouseDeviceException(
		const char* message)
		:
		Exception{"LTJS_SDL_INPUT_MOUSE_DEVICE", message}
	{
	}
}; // SdlInputMouseDeviceException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInputMouseDevice final :
	public SdlInputDevice
{
public:
	SdlInputMouseDevice(
		const SdlInputDeviceInfo& device_info);


	// ======================================================================
	// SdlInputDevice

	const SdlInputDeviceInfo& get_device_info() const noexcept override;


	int get_object_count() const noexcept override;

	const SdlInputDeviceObject& get_object(
		int index) const override;

	const SdlInputDeviceObject* find_object_by_id(
		int object_id) const noexcept override;


	void set_data_format(
		const SdlInputDeviceDataFormat& data_formats) override;


	void get_state(
		void* state_buffer,
		int state_size) override;

	int get_data(
		SdlInputDeviceObjectData* object_datas,
		int max_count) override;

	void flush_data() override;


	void handle_system_event(
		const ltjs::SystemEvent& system_event) override;

	// SdlInputDevice
	// ======================================================================


private:
	static constexpr auto max_objects = 8;
	static constexpr auto max_event_queue_size = 4'096;


	static constexpr auto x_axis_object_id = 0;
	static constexpr auto y_axis_object_id = 1;
	static constexpr auto z_axis_object_id = 2;

	static constexpr auto button_0_object_id = 3;
	static constexpr auto button_1_object_id = 4;
	static constexpr auto button_2_object_id = 5;
	static constexpr auto button_3_object_id = 6;
	static constexpr auto button_4_object_id = 7;


	using Name = std::string;
	using Objects = std::array<SdlInputDeviceObject, max_objects>;
	using ObjectDataFormats = std::array<SdlInputDeviceObjectDataFormat, max_objects>;


	SdlInputDeviceInfo info_{};
	Objects objects_{};
	SdlInputDeviceDataFormat data_format_{};
	ObjectDataFormats object_data_formats_{};
	ltjs::SystemEventQueue event_queue_{};

	bool is_mouse_motion_pending_{};
	SdlInputDeviceObjectData mouse_motion_pending_data_{};

	bool has_focus_{};


	void initialize_objects();

	void initialize_event_queue();

	void clear_events();

	void handle_window_event(
		const ltjs::SystemEvent& system_event);

	const SdlInputDeviceObjectDataFormat* find_object_data_format_by_id(
		int object_id) const;
}; // SdlInputMouseDevice

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SdlInputMouseDevice::SdlInputMouseDevice(
	const SdlInputDeviceInfo& device_info)
{
	if (device_info.type != SdlInputDeviceType::mouse)
	{
		throw SdlInputMouseDeviceException{"Unsupported device type."};
	}

	info_ = device_info;
	initialize_objects();
	initialize_event_queue();
}

const SdlInputDeviceInfo& SdlInputMouseDevice::get_device_info() const noexcept
{
	return info_;
}

int SdlInputMouseDevice::get_object_count() const noexcept
{
	return max_objects;
}

const SdlInputDeviceObject& SdlInputMouseDevice::get_object(
	int index) const
{
	if (index < 0 || index >= get_object_count())
	{
		throw SdlInputMouseDeviceException{"Object index out of range."};
	}

	return objects_[index];
}

const SdlInputDeviceObject* SdlInputMouseDevice::find_object_by_id(
	int object_id) const noexcept
{
	const auto object_end_it = objects_.cend();

	const auto object_it = std::find_if(
		objects_.cbegin(),
		object_end_it,
		[object_id](
			const SdlInputDeviceObject& object)
		{
			return object.id == object_id;
		}
	);

	if (object_it == object_end_it)
	{
		return nullptr;
	}

	return &(*object_it);
}

void SdlInputMouseDevice::set_data_format(
	const SdlInputDeviceDataFormat& data_format)
{
	if (data_format.state_size < 0)
	{
		throw SdlInputMouseDeviceException{"State size out of range."};
	}

	if (data_format.object_count > 0 && !data_format.object_data_formats)
	{
		throw SdlInputMouseDeviceException{"Null object data formats."};
	}

	if (data_format.object_count < 0 || data_format.object_count > get_object_count())
	{
		throw SdlInputMouseDeviceException{"Object count out of range."};
	}

	data_format_ = data_format;

	std::uninitialized_copy_n(
		data_format_.object_data_formats,
		data_format_.object_count,
		object_data_formats_.begin()
	);

	clear_events();
}

void SdlInputMouseDevice::get_state(
	void* state_buffer,
	int state_size)
{
	if (!state_buffer)
	{
		throw SdlInputMouseDeviceException{"Null state buffer."};
	}

	if (state_size != data_format_.state_size)
	{
		throw SdlInputMouseDeviceException{"State size out of range."};
	}

	flush_data();

	std::uninitialized_fill_n(
		static_cast<char*>(state_buffer),
		data_format_.state_size,
		char{}
	);

	auto sdl_x = 0;
	auto sdl_y = 0;
	const auto sdl_buttons = ::SDL_GetMouseState(&sdl_x, &sdl_y);

	for (auto i = 0; i < data_format_.object_count; ++i)
	{
		const auto& object_data_format = object_data_formats_[i];
		const auto& object = objects_[object_data_format.object_id];

		switch (object.id)
		{
			// X-axis
			case x_axis_object_id:
				sdl_input_write_axis_data(
					state_buffer,
					object_data_format.offset,
					0
				);

				break;

			// Y-axis
			case y_axis_object_id:
				sdl_input_write_axis_data(
					state_buffer,
					object_data_format.offset,
					0
				);

				break;

			// Wheel
			case z_axis_object_id:
				sdl_input_write_axis_data(
					state_buffer,
					object_data_format.offset,
					0
				);

				break;

			// Button 0
			case button_0_object_id:
				sdl_input_write_button_data(
					state_buffer,
					object_data_format.offset,
					(sdl_buttons & SDL_BUTTON_LMASK) != 0
				);

				break;

			// Button 1
			case button_1_object_id:
				sdl_input_write_button_data(
					state_buffer,
					object_data_format.offset,
					(sdl_buttons & SDL_BUTTON_RMASK) != 0
				);

				break;

			// Button 2
			case button_2_object_id:
				sdl_input_write_button_data(
					state_buffer,
					object_data_format.offset,
					(sdl_buttons & SDL_BUTTON_MMASK) != 0
				);

				break;

			// Button 3
			case button_3_object_id:
				sdl_input_write_button_data(
					state_buffer,
					object_data_format.offset,
					(sdl_buttons & SDL_BUTTON_X1MASK) != 0
				);

				break;

			// Button 4
			case button_4_object_id:
				sdl_input_write_button_data(
					state_buffer,
					object_data_format.offset,
					(sdl_buttons & SDL_BUTTON_X2MASK) != 0
				);

				break;

			default:
				break;
		}
	}
}

int SdlInputMouseDevice::get_data(
	SdlInputDeviceObjectData* object_datas,
	int max_count)
{
	if (max_count > 0 && !object_datas)
	{
		throw SdlInputMouseDeviceException{"Null object datas."};
	}

	if (max_count < 0)
	{
		throw SdlInputMouseDeviceException{"Max count out of range."};
	}

	auto count = 0;

	while (count < max_count &&
		(is_mouse_motion_pending_ || !event_queue_.is_empty()))
	{
		auto& object_data = object_datas[count];

		if (is_mouse_motion_pending_)
		{
			is_mouse_motion_pending_ = false;

			count += 1;
			object_data = mouse_motion_pending_data_;
			continue;
		}

		const auto system_event = event_queue_.pop();

		switch (system_event.type)
		{
			case ::SDL_MOUSEMOTION:
				if (has_focus_)
				{
					SdlInputDeviceObjectData* const object_data_ptrs[] =
					{
						&object_data,
						&mouse_motion_pending_data_,
					};

					const int motion_values[] =
					{
						system_event.motion.xrel,
						system_event.motion.yrel,
					};

					constexpr int axis_object_ids[] =
					{
						x_axis_object_id,
						y_axis_object_id,
					};

					auto object_data_index = 0;

					for (auto i = 0; i < 2; ++i)
					{
						const auto motion_value = motion_values[i];

						if (motion_value != 0)
						{
							auto& dst_object_data = *(object_data_ptrs[object_data_index]);
							const auto axis_object_id = axis_object_ids[i];
							const auto object_data_format = find_object_data_format_by_id(axis_object_id);

							if (object_data_format)
							{
								dst_object_data.offset = object_data_format->offset;
								dst_object_data.value = motion_value;

								object_data_index += 1;
							}
						}
					}

					is_mouse_motion_pending_ = (object_data_index > 1);

					count += (object_data_index > 0);
				}

				break;

			case ::SDL_MOUSEBUTTONDOWN:
			case ::SDL_MOUSEBUTTONUP:
				if (has_focus_)
				{
					auto object_id = -1;

					switch (system_event.button.button)
					{
						case SDL_BUTTON_LEFT:
							object_id = button_0_object_id;
							break;

						case SDL_BUTTON_MIDDLE:
							object_id = button_2_object_id;
							break;

						case SDL_BUTTON_RIGHT:
							object_id = button_1_object_id;
							break;

						case SDL_BUTTON_X1:
							object_id = button_3_object_id;
							break;

						case SDL_BUTTON_X2:
							object_id = button_4_object_id;
							break;

						default:
							break;
					}

					if (object_id >= 0)
					{
						const auto object_data_format = find_object_data_format_by_id(object_id);

						if (object_data_format)
						{
							object_data.offset = object_data_format->offset;
							object_data.value = (system_event.button.state == SDL_PRESSED ? 0x80 : 0x00);

							count += 1;
						}
					}
				}

				break;

			case ::SDL_MOUSEWHEEL:
				if (has_focus_ && system_event.wheel.y != 0)
				{
					auto value = system_event.wheel.y;

#if SDL_VERSION_ATLEAST(2, 0, 4)
					if (system_event.wheel.direction == ::SDL_MOUSEWHEEL_FLIPPED)
					{
						value = -value;
					}
#endif // SDL_VERSION_ATLEAST(2, 0, 4)

					const auto object_data_format = find_object_data_format_by_id(z_axis_object_id);

					if (object_data_format)
					{
						object_data.offset = object_data_format->offset;
						object_data.value = value;

						count += 1;
					}
				}

				break;

			default:
				handle_window_event(system_event);
				break;
		}
	}

	return count;
}

void SdlInputMouseDevice::flush_data()
{
	while (!event_queue_.is_empty())
	{
		const auto system_event = event_queue_.pop();
		handle_window_event(system_event);
	}
}

void SdlInputMouseDevice::handle_system_event(
	const ltjs::SystemEvent& system_event)
{
	switch (system_event.type)
	{
		case ::SDL_WINDOWEVENT:
		case ::SDL_MOUSEMOTION:
		case ::SDL_MOUSEBUTTONDOWN:
		case ::SDL_MOUSEBUTTONUP:
		case ::SDL_MOUSEWHEEL:
			event_queue_.push(system_event);
			break;

		default:
			break;
	}
}

void SdlInputMouseDevice::initialize_objects()
{
	auto object_index = 0;

	// Axes.
	//
	{
		auto& object = objects_[object_index++];
		object.id = x_axis_object_id;
		object.type = SdlInputDeviceObjectType::x_axis;
		object.axis_mode = SdlInputAxisMode::relative;
		object.name = "X-axis";
	}

	{
		auto& object = objects_[object_index++];
		object.id = y_axis_object_id;
		object.type = SdlInputDeviceObjectType::y_axis;
		object.axis_mode = SdlInputAxisMode::relative;
		object.name = "Y-axis";
	}

	{
		auto& object = objects_[object_index++];
		object.id = z_axis_object_id;
		object.type = SdlInputDeviceObjectType::z_axis;
		object.axis_mode = SdlInputAxisMode::relative;
		object.name = "Wheel";
	}

	// Buttons.
	//
	{
		auto& object = objects_[object_index++];
		object.id = button_0_object_id;
		object.type = SdlInputDeviceObjectType::push_button;
		object.axis_mode = SdlInputAxisMode::none;
		object.name = "Button 0";
	}

	{
		auto& object = objects_[object_index++];
		object.id = button_1_object_id;
		object.type = SdlInputDeviceObjectType::push_button;
		object.axis_mode = SdlInputAxisMode::none;
		object.name = "Button 1";
	}

	{
		auto& object = objects_[object_index++];
		object.id = button_2_object_id;
		object.type = SdlInputDeviceObjectType::push_button;
		object.axis_mode = SdlInputAxisMode::none;
		object.name = "Button 2";
	}

	{
		auto& object = objects_[object_index++];
		object.id = button_3_object_id;
		object.type = SdlInputDeviceObjectType::push_button;
		object.axis_mode = SdlInputAxisMode::none;
		object.name = "Button 3";
	}

	{
		auto& object = objects_[object_index++];
		object.id = button_4_object_id;
		object.type = SdlInputDeviceObjectType::push_button;
		object.axis_mode = SdlInputAxisMode::none;
		object.name = "Button 4";
	}
}

void SdlInputMouseDevice::initialize_event_queue()
{
	event_queue_.set_max_size(max_event_queue_size);
}

void SdlInputMouseDevice::clear_events()
{
	flush_data();

	event_queue_.clear();
	is_mouse_motion_pending_ = false;
}

void SdlInputMouseDevice::handle_window_event(
	const ltjs::SystemEvent& system_event)
{
	if (system_event.type != ::SDL_WINDOWEVENT)
	{
		return;
	}

	switch (system_event.window.event)
	{
		case ::SDL_WINDOWEVENT_ENTER:
			has_focus_ = true;
			break;

		case ::SDL_WINDOWEVENT_LEAVE:
			has_focus_ = false;
			break;

		default:
			break;
	}
}

const SdlInputDeviceObjectDataFormat* SdlInputMouseDevice::find_object_data_format_by_id(
	int object_id) const
{
	for (auto i = 0; i < data_format_.object_count; ++i)
	{
		const auto& object_data_format = object_data_formats_[i];

		if (object_data_format.object_id == object_id)
		{
			return &object_data_format;
		}
	}

	return nullptr;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInputKeyboardDeviceException :
	public ltjs::Exception
{
public:
	explicit SdlInputKeyboardDeviceException(
		const char* message)
		:
		Exception{"LTJS_SDL_INPUT_KEYBOARD_DEVICE", message}
	{
	}
}; // SdlInputKeyboardDeviceException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInputKeyboardDevice final :
	public SdlInputDevice
{
public:
	SdlInputKeyboardDevice(
		const SdlInputDeviceInfo& device_info);


	// ======================================================================
	// SdlInputDevice

	const SdlInputDeviceInfo& get_device_info() const noexcept override;


	int get_object_count() const noexcept override;

	const SdlInputDeviceObject& get_object(
		int index) const override;

	const SdlInputDeviceObject* find_object_by_id(
		int object_id) const noexcept override;


	void set_data_format(
		const SdlInputDeviceDataFormat& data_formats) override;


	void get_state(
		void* state_buffer,
		int state_size) override;

	int get_data(
		SdlInputDeviceObjectData* object_datas,
		int max_count) override;

	void flush_data() override;


	void handle_system_event(
		const ltjs::SystemEvent& system_event) override;

	// SdlInputDevice
	// ======================================================================


private:
	static constexpr auto max_event_queue_size = 4'096;


	struct DiObject
	{
		int key{};
		::SDL_Keycode sdl_keycode{};
		const char* name{};
	}; // DiObject

	static constexpr DiObject di_objects[] =
	{
		DiObject{DIK_ESCAPE, ::SDLK_ESCAPE, "Esc"},
		DiObject{DIK_1, ::SDLK_1, "1"},
		DiObject{DIK_2, ::SDLK_2, "2"},
		DiObject{DIK_3, ::SDLK_3, "3"},
		DiObject{DIK_4, ::SDLK_4, "4"},
		DiObject{DIK_5, ::SDLK_5, "5"},
		DiObject{DIK_6, ::SDLK_6, "6"},
		DiObject{DIK_7, ::SDLK_7, "7"},
		DiObject{DIK_8, ::SDLK_8, "8"},
		DiObject{DIK_9, ::SDLK_9, "9"},
		DiObject{DIK_0, ::SDLK_0, "0"},
		DiObject{DIK_MINUS, ::SDLK_MINUS, "-"},
		DiObject{DIK_EQUALS, ::SDLK_EQUALS, "="},
		DiObject{DIK_BACK, ::SDLK_BACKSPACE, "Backspace"},
		DiObject{DIK_TAB, ::SDLK_TAB, "Tab"},
		DiObject{DIK_Q, ::SDLK_q, "Q"},
		DiObject{DIK_W, ::SDLK_w, "W"},
		DiObject{DIK_E, ::SDLK_e, "E"},
		DiObject{DIK_R, ::SDLK_r, "R"},
		DiObject{DIK_T, ::SDLK_t, "T"},
		DiObject{DIK_Y, ::SDLK_y, "Y"},
		DiObject{DIK_U, ::SDLK_u, "U"},
		DiObject{DIK_I, ::SDLK_i, "I"},
		DiObject{DIK_O, ::SDLK_o, "O"},
		DiObject{DIK_P, ::SDLK_p, "P"},
		DiObject{DIK_LBRACKET, ::SDLK_LEFTBRACKET, "["},
		DiObject{DIK_RBRACKET, ::SDLK_RIGHTBRACKET, "]"},
		DiObject{DIK_RETURN, ::SDLK_RETURN, "Enter"},
		DiObject{DIK_LCONTROL, ::SDLK_LCTRL, "Ctrl"},
		DiObject{DIK_A, ::SDLK_a, "A"},
		DiObject{DIK_S, ::SDLK_s, "S"},
		DiObject{DIK_D, ::SDLK_d, "D"},
		DiObject{DIK_F, ::SDLK_f, "F"},
		DiObject{DIK_G, ::SDLK_g, "G"},
		DiObject{DIK_H, ::SDLK_h, "H"},
		DiObject{DIK_J, ::SDLK_j, "J"},
		DiObject{DIK_K, ::SDLK_k, "K"},
		DiObject{DIK_L, ::SDLK_l, "L"},
		DiObject{DIK_SEMICOLON, ::SDLK_SEMICOLON, ";"},
		DiObject{DIK_APOSTROPHE, ::SDLK_QUOTE, "'"},
		DiObject{DIK_GRAVE, ::SDLK_BACKQUOTE, "`"},
		DiObject{DIK_LSHIFT, ::SDLK_LSHIFT, "Shift"},
		DiObject{DIK_BACKSLASH, ::SDLK_BACKSLASH, "\\"},
		DiObject{DIK_Z, ::SDLK_z, "Z"},
		DiObject{DIK_X, ::SDLK_x, "X"},
		DiObject{DIK_C, ::SDLK_c, "C"},
		DiObject{DIK_V, ::SDLK_v, "V"},
		DiObject{DIK_B, ::SDLK_b, "B"},
		DiObject{DIK_N, ::SDLK_n, "N"},
		DiObject{DIK_M, ::SDLK_m, "M"},
		DiObject{DIK_COMMA, ::SDLK_COMMA, ","},
		DiObject{DIK_PERIOD, ::SDLK_PERIOD, "."},
		DiObject{DIK_SLASH, ::SDLK_SLASH, "/"},
		DiObject{DIK_RSHIFT, ::SDLK_RSHIFT, "Right Shift"},
		DiObject{DIK_MULTIPLY, ::SDLK_KP_MULTIPLY, "*"},
		DiObject{DIK_LMENU, ::SDLK_LALT, "Alt"},
		DiObject{DIK_SPACE, ::SDLK_SPACE, "Space"},
		DiObject{DIK_CAPITAL, ::SDLK_CAPSLOCK, "Caps Lock"},
		DiObject{DIK_F1, ::SDLK_F1, "F1"},
		DiObject{DIK_F2, ::SDLK_F2, "F2"},
		DiObject{DIK_F3, ::SDLK_F3, "F3"},
		DiObject{DIK_F4, ::SDLK_F4, "F4"},
		DiObject{DIK_F5, ::SDLK_F5, "F5"},
		DiObject{DIK_F6, ::SDLK_F6, "F6"},
		DiObject{DIK_F7, ::SDLK_F7, "F7"},
		DiObject{DIK_F8, ::SDLK_F8, "F8"},
		DiObject{DIK_F9, ::SDLK_F9, "F9"},
		DiObject{DIK_F10, ::SDLK_F10, "F10"},
		DiObject{DIK_NUMLOCK, ::SDLK_NUMLOCKCLEAR, "Num Lock"},
		DiObject{DIK_SCROLL, ::SDLK_SCROLLLOCK, "Scroll Lock"},
		DiObject{DIK_NUMPAD7, ::SDLK_KP_7, "Num 7"},
		DiObject{DIK_NUMPAD8, ::SDLK_KP_8, "Num 8"},
		DiObject{DIK_NUMPAD9, ::SDLK_KP_9, "Num 9"},
		DiObject{DIK_SUBTRACT, ::SDLK_KP_MINUS, "-"},
		DiObject{DIK_NUMPAD4, ::SDLK_KP_4, "Num 4"},
		DiObject{DIK_NUMPAD5, ::SDLK_KP_5, "Num 5"},
		DiObject{DIK_NUMPAD6, ::SDLK_KP_6, "Num 6"},
		DiObject{DIK_ADD, ::SDLK_KP_PLUS, "+"},
		DiObject{DIK_NUMPAD1, ::SDLK_KP_1, "Num 1"},
		DiObject{DIK_NUMPAD2, ::SDLK_KP_2, "Num 2"},
		DiObject{DIK_NUMPAD3, ::SDLK_KP_3, "Num 3"},
		DiObject{DIK_NUMPAD0, ::SDLK_KP_0, "Num 0"},
		DiObject{DIK_DECIMAL, ::SDLK_KP_DECIMAL, "Num Del"},
		//DiObject{DIK_OEM_102, ::SDLK_, ""},
		DiObject{DIK_F11, ::SDLK_F11, "F11"},
		DiObject{DIK_F12, ::SDLK_F12, "F12"},
		DiObject{DIK_F13, ::SDLK_F13, "F13"},
		DiObject{DIK_F14, ::SDLK_F14, "F14"},
		DiObject{DIK_F15, ::SDLK_F15, "F15"},
		//DiObject{DIK_KANA, ::SDLK_, ""},
		//DiObject{DIK_ABNT_C1, ::SDLK_, ""},
		//DiObject{DIK_CONVERT, ::SDLK_, ""},
		//DiObject{DIK_NOCONVERT, ::SDLK_, ""},
		//DiObject{DIK_YEN, ::SDLK_, ""},
		//DiObject{DIK_ABNT_C2, ::SDLK_, ""},
		//DiObject{DIK_NUMPADEQUALS, ::SDLK_, ""},
		//DiObject{DIK_PREVTRACK, ::SDLK_, ""},
		//DiObject{DIK_AT, ::SDLK_, ""},
		//DiObject{DIK_COLON, ::SDLK_, ""},
		//DiObject{DIK_UNDERLINE, ::SDLK_, ""},
		//DiObject{DIK_KANJI, ::SDLK_, ""},
		//DiObject{DIK_STOP, ::SDLK_, ""},
		//DiObject{DIK_AX, ::SDLK_, ""},
		//DiObject{DIK_UNLABELED, ::SDLK_, ""},
		//DiObject{DIK_NEXTTRACK, ::SDLK_, ""},
		DiObject{DIK_NUMPADENTER, ::SDLK_KP_ENTER, "Num Enter"},
		DiObject{DIK_RCONTROL, ::SDLK_RCTRL, "Right Ctrl"},
		//DiObject{DIK_MUTE, ::SDLK_, ""},
		//DiObject{DIK_CALCULATOR, ::SDLK_, ""},
		//DiObject{DIK_PLAYPAUSE, ::SDLK_, ""},
		//DiObject{DIK_MEDIASTOP, ::SDLK_, ""},
		//DiObject{DIK_VOLUMEDOWN, ::SDLK_, ""},
		//DiObject{DIK_VOLUMEUP, ::SDLK_, ""},
		//DiObject{DIK_WEBHOME, ::SDLK_, ""},
		//DiObject{DIK_NUMPADCOMMA, ::SDLK_, ""},
		DiObject{DIK_DIVIDE, ::SDLK_KP_DIVIDE, "Num /"},
		DiObject{DIK_SYSRQ, ::SDLK_PRINTSCREEN, "Prnt Scrn"},
		DiObject{DIK_RMENU, ::SDLK_RALT, "Right Alt"},
		DiObject{DIK_PAUSE, ::SDLK_PAUSE, "Pause"},
		DiObject{DIK_HOME, ::SDLK_HOME, "Home"},
		DiObject{DIK_UP, ::SDLK_UP, "Up"},
		DiObject{DIK_PRIOR, ::SDLK_PAGEUP, "Page Up"},
		DiObject{DIK_LEFT, ::SDLK_LEFT, "Left"},
		DiObject{DIK_RIGHT, ::SDLK_RIGHT, "Right"},
		DiObject{DIK_END, ::SDLK_END, "End"},
		DiObject{DIK_DOWN, ::SDLK_DOWN, "Down"},
		DiObject{DIK_NEXT, ::SDLK_PAGEDOWN, "Page Down"},
		DiObject{DIK_INSERT, ::SDLK_INSERT, "Insert"},
		DiObject{DIK_DELETE, ::SDLK_DELETE, "Delete"},
		//DiObject{DIK_LWIN, ::SDLK_, ""},
		//DiObject{DIK_RWIN, ::SDLK_, ""},
		//DiObject{DIK_APPS, ::SDLK_, ""},
		//DiObject{DIK_POWER, ::SDLK_, ""},
		//DiObject{DIK_SLEEP, ::SDLK_, ""},
		//DiObject{DIK_WAKE, ::SDLK_, ""},
		//DiObject{DIK_WEBSEARCH, ::SDLK_, ""},
		//DiObject{DIK_WEBFAVORITES, ::SDLK_, ""},
		//DiObject{DIK_WEBREFRESH, ::SDLK_, ""},
		//DiObject{DIK_WEBSTOP, ::SDLK_, ""},
		//DiObject{DIK_WEBFORWARD, ::SDLK_, ""},
		//DiObject{DIK_WEBBACK, ::SDLK_, ""},
		//DiObject{DIK_MYCOMPUTER, ::SDLK_, ""},
		//DiObject{DIK_MAIL, ::SDLK_, ""},
		//DiObject{DIK_MEDIASELECT, ::SDLK_, ""},
	}; // di_objects


	static constexpr auto max_objects = 256;

	static constexpr auto max_di_objects = std::extent<decltype(di_objects)>::value;
	static_assert(max_di_objects > 0 && max_di_objects <= max_objects, "Unsupported object count.");


	using Name = std::string;
	using Objects = std::array<SdlInputDeviceObject, max_objects>;
	using SdlKeycodes = std::array<::SDL_Keycode, max_objects>;
	using ObjectDataFormats = std::array<SdlInputDeviceObjectDataFormat, max_objects>;
	using SdlKeycodeToDiKeyMap = std::unordered_map<::SDL_Keycode, int>;


	SdlInputDeviceInfo info_{};
	Objects objects_{};
	SdlKeycodes sdl_keycodes_{};
	SdlKeycodeToDiKeyMap sdl_keycode_to_di_key_map_{};
	SdlInputDeviceDataFormat data_format_{};
	ObjectDataFormats object_data_formats_{};
	ltjs::SystemEventQueue event_queue_{};

	bool has_focus_{};


	void initialize_sdl_keycode_to_di_key_map();

	void initialize_objects();

	void initialize_sdl_keycodes();

	void initialize_event_queue();

	void clear_events();

	void handle_window_event(
		const ltjs::SystemEvent& system_event);

	const SdlInputDeviceObjectDataFormat* find_object_data_format_by_id(
		int object_id) const;

	int sdl_map_keycode_to_di_key(
		SDL_Keycode sdl_keycode) const noexcept;
}; // SdlInputKeyboardDevice

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

constexpr SdlInputKeyboardDevice::DiObject SdlInputKeyboardDevice::di_objects[];

// ==========================================================================

SdlInputKeyboardDevice::SdlInputKeyboardDevice(
	const SdlInputDeviceInfo& device_info)
{
	if (device_info.type != SdlInputDeviceType::keyboard)
	{
		throw SdlInputMouseDeviceException{"Unsupported device type."};
	}

	info_ = device_info;

	initialize_objects();
	initialize_sdl_keycodes();
	initialize_sdl_keycode_to_di_key_map();
	initialize_event_queue();
}

const SdlInputDeviceInfo& SdlInputKeyboardDevice::get_device_info() const noexcept
{
	return info_;
}

int SdlInputKeyboardDevice::get_object_count() const noexcept
{
	return max_di_objects;
}

const SdlInputDeviceObject& SdlInputKeyboardDevice::get_object(
	int index) const
{
	if (index < 0 && index >= max_di_objects)
	{
		throw SdlInputKeyboardDeviceException{"Object index out of range."};
	}

	const auto object_index = di_objects[index].key;
	return objects_[object_index];
}

const SdlInputDeviceObject* SdlInputKeyboardDevice::find_object_by_id(
	int object_id) const noexcept
{
	if (object_id <= 0 || object_id >= max_objects)
	{
		return nullptr;
	}

	return &objects_[object_id];
}

void SdlInputKeyboardDevice::set_data_format(
	const SdlInputDeviceDataFormat& data_format)
{
	if (data_format.object_count > 0 && !data_format.object_data_formats)
	{
		throw SdlInputKeyboardDeviceException{"Null object data formats."};
	}

	if (data_format.object_count < 0 || data_format.object_count > max_di_objects)
	{
		throw SdlInputKeyboardDeviceException{"Object count out of range."};
	}

	data_format_ = data_format;

	std::uninitialized_copy_n(
		data_format_.object_data_formats,
		data_format_.object_count,
		object_data_formats_.begin()
	);

	clear_events();
}

void SdlInputKeyboardDevice::get_state(
	void* state_buffer,
	int state_size)
{
	if (!state_buffer)
	{
		throw SdlInputKeyboardDeviceException{"Null state buffer."};
	}

	if (state_size != data_format_.state_size)
	{
		throw SdlInputKeyboardDeviceException{"State size out of range."};
	}

	flush_data();

	std::uninitialized_fill_n(
		static_cast<char*>(state_buffer),
		data_format_.state_size,
		char{}
	);

	auto sdl_key_count = 0;
	const auto sdl_scancodes = ::SDL_GetKeyboardState(&sdl_key_count);

	for (auto i = 0; i < data_format_.object_count; ++i)
	{
		const auto& object_data_format = object_data_formats_[i];
		const auto object_id = object_data_format.object_id;
		const auto sdl_keycode = sdl_keycodes_[object_id];

		auto is_pressed = false;

		if (sdl_keycode != ::SDLK_UNKNOWN)
		{
			const auto sdl_scancode = ::SDL_GetScancodeFromKey(sdl_keycode);

			if (sdl_scancode != ::SDL_SCANCODE_UNKNOWN)
			{
				is_pressed = (sdl_scancodes[sdl_scancode] != 0);
			}
		}

		sdl_input_write_button_data(
			state_buffer,
			object_data_format.offset,
			is_pressed
		);
	}
}

int SdlInputKeyboardDevice::get_data(
	SdlInputDeviceObjectData* object_datas,
	int max_count)
{
	if (max_count > 0 && !object_datas)
	{
		throw SdlInputKeyboardDeviceException{"Null object datas."};
	}

	if (max_count < 0)
	{
		throw SdlInputKeyboardDeviceException{"Max count out of range."};
	}

	auto count = 0;

	while (count < max_count && !event_queue_.is_empty())
	{
		auto& object_data = object_datas[count];

		const auto system_event = event_queue_.pop();

		switch (system_event.type)
		{
			case ::SDL_KEYDOWN:
			case ::SDL_KEYUP:
				{
					const auto sdk_key_code = system_event.key.keysym.sym;
					const auto di_key = sdl_map_keycode_to_di_key(sdk_key_code);

					if (di_key != 0)
					{
						const auto object_data_format = find_object_data_format_by_id(di_key);

						if (object_data_format)
						{
							object_data.offset = object_data_format->offset;
							object_data.value = (system_event.key.state == SDL_PRESSED ? 0x80 : 0x00);

							count += 1;
						}
					}
				}

				break;

			default:
				handle_window_event(system_event);
				break;
		}
	}

	return count;
}

void SdlInputKeyboardDevice::flush_data()
{
	while (!event_queue_.is_empty())
	{
		const auto system_event = event_queue_.pop();
		handle_window_event(system_event);
	}
}

void SdlInputKeyboardDevice::handle_system_event(
	const ltjs::SystemEvent& system_event)
{
	switch (system_event.type)
	{
		case ::SDL_WINDOWEVENT:
		case ::SDL_KEYDOWN:
		case ::SDL_KEYUP:
			event_queue_.push(system_event);
			break;

		default:
			break;
	}
}

void SdlInputKeyboardDevice::initialize_sdl_keycode_to_di_key_map()
{
	sdl_keycode_to_di_key_map_.reserve(max_di_objects);

	for (const auto& di_object : di_objects)
	{
		sdl_keycode_to_di_key_map_[di_object.sdl_keycode] = di_object.key;
	}
}

void SdlInputKeyboardDevice::initialize_objects()
{
	for (const auto& di_object : di_objects)
	{
		auto& object = objects_[di_object.key];
		object.id = di_object.key;
		object.type = SdlInputDeviceObjectType::key;
		object.axis_mode = SdlInputAxisMode::none;
		object.name = di_object.name;
	}
}

void SdlInputKeyboardDevice::initialize_sdl_keycodes()
{
	for (const auto& di_object : di_objects)
	{
		auto& sdl_keycode = sdl_keycodes_[di_object.key];
		sdl_keycode = di_object.sdl_keycode;
	}
}

void SdlInputKeyboardDevice::initialize_event_queue()
{
	event_queue_.set_max_size(max_event_queue_size);
}

void SdlInputKeyboardDevice::clear_events()
{
	flush_data();

	event_queue_.clear();
}

void SdlInputKeyboardDevice::handle_window_event(
	const ltjs::SystemEvent& system_event)
{
	if (system_event.type != ::SDL_WINDOWEVENT)
	{
		return;
	}

	switch (system_event.window.event)
	{
		case ::SDL_WINDOWEVENT_FOCUS_GAINED:
			has_focus_ = true;
			break;

		case ::SDL_WINDOWEVENT_FOCUS_LOST:
			has_focus_ = false;
			break;

		default:
			break;
	}
}

const SdlInputDeviceObjectDataFormat* SdlInputKeyboardDevice::find_object_data_format_by_id(
	int object_id) const
{
	for (auto i = 0; i < data_format_.object_count; ++i)
	{
		const auto& object_data_format = object_data_formats_[i];

		if (object_data_format.object_id == object_id)
		{
			return &object_data_format;
		}
	}

	return nullptr;
}

int SdlInputKeyboardDevice::sdl_map_keycode_to_di_key(
	::SDL_Keycode sdl_keycode) const noexcept
{
	const auto sdl_keycode_to_di_key_map_item_it = sdl_keycode_to_di_key_map_.find(sdl_keycode);

	if (sdl_keycode_to_di_key_map_item_it == sdl_keycode_to_di_key_map_.cend())
	{
		return 0;
	}

	return sdl_keycode_to_di_key_map_item_it->second;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInputGamepadDeviceException :
	public ltjs::Exception
{
public:
	explicit SdlInputGamepadDeviceException(
		const char* message)
		:
		Exception{"LTJS_SDL_INPUT_GAMEPAD_DEVICE", message}
	{
	}
}; // SdlInputGamepadDeviceException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInputGamepadDevice final :
	public SdlInputDevice
{
public:
	explicit SdlInputGamepadDevice(
		const SdlInputDeviceInfo& device_info);


	// ======================================================================
	// SdlInputDevice

	const SdlInputDeviceInfo& get_device_info() const noexcept override;


	int get_object_count() const noexcept override;

	const SdlInputDeviceObject& get_object(
		int index) const override;

	const SdlInputDeviceObject* find_object_by_id(
		int object_id) const noexcept override;


	void set_data_format(
		const SdlInputDeviceDataFormat& data_formats) override;


	void get_state(
		void* state_buffer,
		int state_size) override;

	int get_data(
		SdlInputDeviceObjectData* object_datas,
		int max_count) override;

	void flush_data() override;


	void handle_system_event(
		const ltjs::SystemEvent& system_event) override;

	// SdlInputDevice
	// ======================================================================


private:
	static constexpr auto dead_zone = 8'192;
	static constexpr auto dead_zone_radius = dead_zone / 2;

	static constexpr auto max_objects = 128;

	static constexpr auto max_event_queue_size = 4'096;


	static constexpr auto x_axis_object_id = 0;
	static constexpr auto y_axis_object_id = 1;
	static constexpr auto z_axis_object_id = 2;

	static constexpr auto rx_axis_object_id = 3;
	static constexpr auto ry_axis_object_id = 4;

	static constexpr auto pov_object_id = 6;

	static constexpr auto button_base_object_id = 10;


	struct ButtonDescriptor
	{
		int object_id{};
		::SDL_GameControllerButton sdl_button{};
		const char* name{};
	}; // ButtonDescriptor


	static constexpr ButtonDescriptor button_defs[] =
	{
		ButtonDescriptor{button_base_object_id + 0, ::SDL_CONTROLLER_BUTTON_A, "A"},
		ButtonDescriptor{button_base_object_id + 1, ::SDL_CONTROLLER_BUTTON_B, "B"},
		ButtonDescriptor{button_base_object_id + 2, ::SDL_CONTROLLER_BUTTON_X, "X"},
		ButtonDescriptor{button_base_object_id + 3, ::SDL_CONTROLLER_BUTTON_Y, "Y"},
		ButtonDescriptor{button_base_object_id + 4, ::SDL_CONTROLLER_BUTTON_BACK, "Back"},
		ButtonDescriptor{button_base_object_id + 5, ::SDL_CONTROLLER_BUTTON_GUIDE, "Guide"},
		ButtonDescriptor{button_base_object_id + 6, ::SDL_CONTROLLER_BUTTON_START, "Start"},
		ButtonDescriptor{button_base_object_id + 7, ::SDL_CONTROLLER_BUTTON_LEFTSTICK, "LStick"},
		ButtonDescriptor{button_base_object_id + 8, ::SDL_CONTROLLER_BUTTON_RIGHTSTICK, "RStick"},
		ButtonDescriptor{button_base_object_id + 9, ::SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "LShoulder"},
		ButtonDescriptor{button_base_object_id + 10, ::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, "RShoulder"},
		ButtonDescriptor{button_base_object_id + 11, ::SDL_CONTROLLER_BUTTON_DPAD_UP, "D-Pad Up"},
		ButtonDescriptor{button_base_object_id + 12, ::SDL_CONTROLLER_BUTTON_DPAD_DOWN, "D-Pad Down"},
		ButtonDescriptor{button_base_object_id + 13, ::SDL_CONTROLLER_BUTTON_DPAD_LEFT, "D-Pad Left"},
		ButtonDescriptor{button_base_object_id + 14, ::SDL_CONTROLLER_BUTTON_DPAD_RIGHT, "D-Pad Right"},
		ButtonDescriptor{button_base_object_id + 15, ::SDL_CONTROLLER_BUTTON_MISC1, "Misc 1"},
		ButtonDescriptor{button_base_object_id + 16, ::SDL_CONTROLLER_BUTTON_PADDLE1, "Paddle 1"},
		ButtonDescriptor{button_base_object_id + 17, ::SDL_CONTROLLER_BUTTON_PADDLE2, "Paddle 2"},
		ButtonDescriptor{button_base_object_id + 18, ::SDL_CONTROLLER_BUTTON_PADDLE3, "Paddle 3"},
		ButtonDescriptor{button_base_object_id + 19, ::SDL_CONTROLLER_BUTTON_PADDLE4, "Paddle 4"},
		ButtonDescriptor{button_base_object_id + 20, ::SDL_CONTROLLER_BUTTON_TOUCHPAD, "Touchpad"},
	}; // button_defs


	using Name = std::string;
	using Objects = std::array<SdlInputDeviceObject, max_objects>;
	using ObjectDataFormats = std::array<SdlInputDeviceObjectDataFormat, max_objects>;


	ltjs::SdlGameControllerUResource game_controller_{};
	SdlInputDeviceInfo info_{};
	::SDL_JoystickID joystick_id_{};
	int object_count_{};
	Objects objects_{};
	SdlInputDeviceDataFormat data_format_{};
	ObjectDataFormats object_data_formats_{};
	ltjs::SystemEventQueue event_queue_{};

	bool has_focus_{};

	bool is_pov_pending_{};
	SdlInputDeviceObjectData pov_pending_data_{};

	int left_trigger_{};
	int right_trigger_{};

	bool is_dpad_up_pressed_{};
	bool is_dpad_down_pressed_{};
	bool is_dpad_left_pressed_{};
	bool is_dpad_right_pressed_{};


	void open_game_controller();

	void close_game_controller();

	void initialize_objects();

	void initialize_event_queue();

	void clear_events();

	void handle_window_event(
		const ltjs::SystemEvent& system_event);

	void handle_add_remove_controller_event(
		const ltjs::SystemEvent& system_event);

	const SdlInputDeviceObjectDataFormat* find_object_data_format_by_id(
		int object_id) const;

	void reset_objects_state();

	int make_direct_input_pov() const noexcept;

	int normalize_axis_value(
		int value) const;
}; // SdlInputGamepadDevice

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

constexpr SdlInputGamepadDevice::ButtonDescriptor SdlInputGamepadDevice::button_defs[];

// ==========================================================================

SdlInputGamepadDevice::SdlInputGamepadDevice(
	const SdlInputDeviceInfo& device_info)
{
	if (device_info.type != SdlInputDeviceType::gamepad)
	{
		throw SdlInputGamepadDeviceException{"Unsupported device type."};
	}

	info_ = device_info;
	open_game_controller();
	initialize_objects();
	initialize_event_queue();
}

const SdlInputDeviceInfo& SdlInputGamepadDevice::get_device_info() const noexcept
{
	return info_;
}

int SdlInputGamepadDevice::get_object_count() const noexcept
{
	return object_count_;
}

const SdlInputDeviceObject& SdlInputGamepadDevice::get_object(
	int index) const
{
	if (index < 0 || index >= get_object_count())
	{
		throw SdlInputGamepadDeviceException{"Object index out of range."};
	}

	return objects_[index];
}

const SdlInputDeviceObject* SdlInputGamepadDevice::find_object_by_id(
	int object_id) const noexcept
{
	const auto object_begin_it = objects_.cbegin();
	const auto object_end_it = object_begin_it + object_count_;

	const auto object_it = std::find_if(
		object_begin_it,
		object_end_it,
		[object_id](
			const SdlInputDeviceObject& object)
		{
			return object.id == object_id;
		}
	);

	if (object_it == object_end_it)
	{
		return nullptr;
	}

	return &(*object_it);
}

void SdlInputGamepadDevice::set_data_format(
	const SdlInputDeviceDataFormat& data_format)
{
	if (data_format.state_size < 0)
	{
		throw SdlInputGamepadDeviceException{"State size out of range."};
	}

	if (data_format.object_count > 0 && !data_format.object_data_formats)
	{
		throw SdlInputGamepadDeviceException{"Null object data formats."};
	}

	if (data_format.object_count < 0 || data_format.object_count > get_object_count())
	{
		throw SdlInputGamepadDeviceException{"Object count out of range."};
	}

	data_format_ = data_format;

	std::uninitialized_copy_n(
		data_format_.object_data_formats,
		data_format_.object_count,
		object_data_formats_.begin()
	);

	clear_events();
}

void SdlInputGamepadDevice::get_state(
	void* state_buffer,
	int state_size)
{
	if (!state_buffer)
	{
		throw SdlInputGamepadDeviceException{"Null state buffer."};
	}

	if (state_size != data_format_.state_size)
	{
		throw SdlInputGamepadDeviceException{"State size out of range."};
	}

	flush_data();

	std::uninitialized_fill_n(
		static_cast<char*>(state_buffer),
		data_format_.state_size,
		char{}
	);

	const auto can_read_state = (
		has_focus_ &&
		game_controller_ &&
		::SDL_GameControllerGetAttached(game_controller_.get()) != ::SDL_FALSE
	);

	for (auto i = 0; i < data_format_.object_count; ++i)
	{
		const auto& object_data_format = object_data_formats_[i];
		const auto offset = object_data_format.offset;
		const auto object = find_object_by_id(object_data_format.object_id);

		if (!object)
		{
			continue;
		}

		if (false)
		{
		}
		else if (can_read_state && object->id == x_axis_object_id)
		{
			const auto value = normalize_axis_value(::SDL_GameControllerGetAxis(
				game_controller_.get(),
				::SDL_CONTROLLER_AXIS_LEFTX
			));

			sdl_input_write_axis_data(state_buffer, offset, value);
		}
		else if (can_read_state && object->id == y_axis_object_id)
		{
			const auto value = normalize_axis_value(::SDL_GameControllerGetAxis(
				game_controller_.get(),
				::SDL_CONTROLLER_AXIS_LEFTY
			));

			sdl_input_write_axis_data(state_buffer, offset, value);
		}
		else if (can_read_state && object->id == z_axis_object_id)
		{
			left_trigger_ = ::SDL_GameControllerGetAxis(
				game_controller_.get(),
				::SDL_CONTROLLER_AXIS_TRIGGERLEFT
			);

			right_trigger_ = ::SDL_GameControllerGetAxis(
				game_controller_.get(),
				::SDL_CONTROLLER_AXIS_TRIGGERRIGHT
			);

			const auto value = normalize_axis_value(left_trigger_ - right_trigger_);

			sdl_input_write_axis_data(state_buffer, offset, value);
		}
		else if (can_read_state && object->id == rx_axis_object_id)
		{
			const auto value = normalize_axis_value(::SDL_GameControllerGetAxis(
				game_controller_.get(),
				::SDL_CONTROLLER_AXIS_RIGHTX
			));

			sdl_input_write_axis_data(state_buffer, offset, value);
		}
		else if (can_read_state && object->id == ry_axis_object_id)
		{
			const auto value = normalize_axis_value(::SDL_GameControllerGetAxis(
				game_controller_.get(),
				::SDL_CONTROLLER_AXIS_RIGHTY
			));

			sdl_input_write_axis_data(state_buffer, offset, value);
		}
		else if (object->id == pov_object_id)
		{
			if (can_read_state)
			{
				const auto dpad_up = ::SDL_GameControllerGetButton(
					game_controller_.get(),
					::SDL_CONTROLLER_BUTTON_DPAD_UP
				);

				const auto dpad_down = ::SDL_GameControllerGetButton(
					game_controller_.get(),
					::SDL_CONTROLLER_BUTTON_DPAD_DOWN
				);

				const auto dpad_left = ::SDL_GameControllerGetButton(
					game_controller_.get(),
					::SDL_CONTROLLER_BUTTON_DPAD_LEFT
				);

				const auto dpad_right = ::SDL_GameControllerGetButton(
					game_controller_.get(),
					::SDL_CONTROLLER_BUTTON_DPAD_RIGHT
				);

				is_dpad_up_pressed_ = (dpad_up != 0);
				is_dpad_down_pressed_ = (dpad_down != 0);
				is_dpad_left_pressed_ = (dpad_left != 0);
				is_dpad_right_pressed_ = (dpad_right != 0);

				const auto value = make_direct_input_pov();

				sdl_input_write_axis_data(state_buffer, offset, value);
			}
			else
			{
				sdl_input_write_axis_data(state_buffer, offset, -1);
			}
		}
		else if (has_focus_ && object->id >= button_base_object_id)
		{
			const auto button_index = object->id - button_base_object_id;
			const auto sdl_button = button_defs[button_index].sdl_button;

			const auto sdl_value = ::SDL_GameControllerGetButton(
				game_controller_.get(),
				sdl_button
			);

			sdl_input_write_button_data(state_buffer, offset, sdl_value != 0);
		}
	}
}

int SdlInputGamepadDevice::get_data(
	SdlInputDeviceObjectData* object_datas,
	int max_count)
{
	if (max_count > 0 && !object_datas)
	{
		throw SdlInputGamepadDeviceException{"Null object datas."};
	}

	if (max_count < 0)
	{
		throw SdlInputGamepadDeviceException{"Max count out of range."};
	}

	const auto can_read_state = (
		has_focus_ &&
		game_controller_ &&
		::SDL_GameControllerGetAttached(game_controller_.get()) != ::SDL_FALSE
	);

	auto count = 0;

	while (count < max_count && (is_pov_pending_ || !event_queue_.is_empty()))
	{
		auto& object_data = object_datas[count];

		if (can_read_state && is_pov_pending_)
		{
			is_pov_pending_ = false;

			object_data = pov_pending_data_;
			count += 1;

			continue;
		}

		const auto system_event = event_queue_.pop();

		if (false)
		{
		}
		else if (
			can_read_state &&
			system_event.type == ::SDL_CONTROLLERAXISMOTION &&
			system_event.caxis.which == joystick_id_)
		{
			auto object_id = -1;

			switch (system_event.caxis.axis)
			{
				case ::SDL_CONTROLLER_AXIS_LEFTX:
					object_id = x_axis_object_id;
					break;

				case ::SDL_CONTROLLER_AXIS_LEFTY:
					object_id = y_axis_object_id;
					break;

				case ::SDL_CONTROLLER_AXIS_RIGHTX:
					object_id = rx_axis_object_id;
					break;

				case ::SDL_CONTROLLER_AXIS_RIGHTY:
					object_id = ry_axis_object_id;
					break;

				case ::SDL_CONTROLLER_AXIS_TRIGGERLEFT:
					object_id = z_axis_object_id;
					left_trigger_ = system_event.caxis.value;
					break;

				case ::SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
					object_id = z_axis_object_id;
					right_trigger_ = system_event.caxis.value;
					break;

				default:
					break;
			}

			const auto object_data_format = find_object_data_format_by_id(object_id);

			if (object_data_format)
			{
				const auto value = normalize_axis_value(
					object_id == z_axis_object_id ?
					left_trigger_ - right_trigger_ :
					system_event.caxis.value
				);

				object_data.offset = object_data_format->offset;
				object_data.value = value;

				count += 1;
			}
		}
		else if (
			can_read_state &&
			(system_event.type == ::SDL_CONTROLLERBUTTONDOWN ||
				system_event.type == ::SDL_CONTROLLERBUTTONUP) &&
			system_event.cbutton.which == joystick_id_)
		{
			const auto is_pressed = (system_event.cbutton.state == SDL_PRESSED);
			const auto object_id = button_base_object_id + system_event.cbutton.button;

			auto is_dpad = false;

			switch (system_event.cbutton.button)
			{
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					is_dpad = true;
					is_dpad_up_pressed_ = is_pressed;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					is_dpad = true;
					is_dpad_down_pressed_ = is_pressed;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					is_dpad = true;
					is_dpad_left_pressed_ = is_pressed;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					is_dpad = true;
					is_dpad_right_pressed_ = is_pressed;
					break;
			}

			const auto object_data_format = find_object_data_format_by_id(object_id);

			if (object_data_format)
			{
				object_data.offset = object_data_format->offset;
				object_data.value = (is_pressed ? 0x80 : 0x00);

				count += 1;
			}

			if (is_dpad)
			{
				const auto di_pov = make_direct_input_pov();

				if (di_pov >= -1)
				{
					const auto pov_object_data_format = find_object_data_format_by_id(pov_object_id);

					if (pov_object_data_format)
					{
						is_pov_pending_ = true;
						pov_pending_data_.offset = pov_object_data_format->offset;
						pov_pending_data_.value = di_pov;
					}
				}
			}
		}
		else
		{
			handle_window_event(system_event);
			handle_add_remove_controller_event(system_event);
		}
	}

	return count;
}

void SdlInputGamepadDevice::flush_data()
{
	is_pov_pending_ = false;

	while (!event_queue_.is_empty())
	{
		const auto system_event = event_queue_.pop();
		handle_window_event(system_event);
	}
}

void SdlInputGamepadDevice::handle_system_event(
	const ltjs::SystemEvent& system_event)
{
	switch (system_event.type)
	{
		case ::SDL_WINDOWEVENT:

		case ::SDL_CONTROLLERAXISMOTION:
		case ::SDL_CONTROLLERBUTTONDOWN:
		case ::SDL_CONTROLLERBUTTONUP:

		case ::SDL_CONTROLLERDEVICEADDED:
		case ::SDL_CONTROLLERDEVICEREMOVED:
			event_queue_.push(system_event);
			break;

		default:
			break;
	}
}

void SdlInputGamepadDevice::open_game_controller()
{
	const auto joystick_count = ::SDL_NumJoysticks();

	for (auto joystick_index = 0; joystick_index < joystick_count; ++joystick_index)
	{
		if (::SDL_IsGameController(joystick_index) == ::SDL_FALSE)
		{
			continue;
		}

		const auto game_controller_name = ::SDL_GameControllerNameForIndex(joystick_index);

		if (!game_controller_name)
		{
			continue;
		}

		if (game_controller_name != info_.name)
		{
			continue;
		}

		auto game_controller = ltjs::SdlGameControllerUResource{::SDL_GameControllerOpen(joystick_index)};

		if (!game_controller)
		{
			return;
		}

		const auto joystick_id = ::SDL_JoystickGetDeviceInstanceID(joystick_index);

		if (joystick_id < 0)
		{
			return;
		}

		joystick_id_ = joystick_id;
		game_controller_ = std::move(game_controller);

		return;
	}
}

void SdlInputGamepadDevice::close_game_controller()
{
	joystick_id_ = -1;
	game_controller_ = nullptr;
}

void SdlInputGamepadDevice::initialize_objects()
{
	if (!game_controller_)
	{
		return;
	}

	auto object_index = 0;

	const auto has_axis = [this](
		const ::SDL_GameControllerAxis axis)
	{
		return ::SDL_GameControllerHasAxis(game_controller_.get(), axis) != ::SDL_FALSE;
	};

	if (has_axis(::SDL_CONTROLLER_AXIS_LEFTX))
	{
		auto& object = objects_[object_index++];
		object.id = x_axis_object_id;
		object.type = SdlInputDeviceObjectType::x_axis;
		object.axis_mode = SdlInputAxisMode::absolute;
		object.name = "X Axis";
	}

	if (has_axis(::SDL_CONTROLLER_AXIS_LEFTY))
	{
		auto& object = objects_[object_index++];
		object.id = y_axis_object_id;
		object.type = SdlInputDeviceObjectType::y_axis;
		object.axis_mode = SdlInputAxisMode::absolute;
		object.name = "Y Axis";
	}

	if (has_axis(::SDL_CONTROLLER_AXIS_TRIGGERLEFT) &&
		has_axis(::SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
	{
		auto& object = objects_[object_index++];
		object.id = z_axis_object_id;
		object.type = SdlInputDeviceObjectType::z_axis;
		object.axis_mode = SdlInputAxisMode::absolute;
		object.name = "Z Axis";
	}

	if (has_axis(::SDL_CONTROLLER_AXIS_RIGHTX))
	{
		auto& object = objects_[object_index++];
		object.id = rx_axis_object_id;
		object.type = SdlInputDeviceObjectType::rx_axis;
		object.axis_mode = SdlInputAxisMode::absolute;
		object.name = "X Rotation";
	}

	if (has_axis(::SDL_CONTROLLER_AXIS_RIGHTY))
	{
		auto& object = objects_[object_index++];
		object.id = ry_axis_object_id;
		object.type = SdlInputDeviceObjectType::ry_axis;
		object.axis_mode = SdlInputAxisMode::absolute;
		object.name = "Y Rotation";
	}


	const auto has_button = [this](
		const ::SDL_GameControllerButton button)
	{
		return ::SDL_GameControllerHasButton(game_controller_.get(), button) != ::SDL_FALSE;
	};

	const auto has_hat =
		has_button(::SDL_CONTROLLER_BUTTON_DPAD_UP) &&
		has_button(::SDL_CONTROLLER_BUTTON_DPAD_DOWN) &&
		has_button(::SDL_CONTROLLER_BUTTON_DPAD_LEFT) &&
		has_button(::SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
	;

	if (has_hat)
	{
		auto& object = objects_[object_index++];
		object.id = pov_object_id;
		object.type = SdlInputDeviceObjectType::pov;
		object.axis_mode = SdlInputAxisMode::none;
		object.name = "Hat Switch";
	}

	for (const auto& button_descriptor : button_defs)
	{
		if (!has_button(button_descriptor.sdl_button))
		{
			continue;
		}

		auto& object = objects_[object_index++];
		object.id = button_descriptor.object_id;
		object.type = SdlInputDeviceObjectType::push_button;
		object.axis_mode = SdlInputAxisMode::none;
		object.name = button_descriptor.name;
	}

	object_count_ = object_index;
}

void SdlInputGamepadDevice::initialize_event_queue()
{
	event_queue_.set_max_size(max_event_queue_size);
}

void SdlInputGamepadDevice::clear_events()
{
	flush_data();

	event_queue_.clear();
}

void SdlInputGamepadDevice::handle_window_event(
	const ltjs::SystemEvent& system_event)
{
	if (system_event.type != ::SDL_WINDOWEVENT)
	{
		return;
	}

	switch (system_event.window.event)
	{
		case ::SDL_WINDOWEVENT_FOCUS_GAINED:
			has_focus_ = true;
			reset_objects_state();
			break;

		case ::SDL_WINDOWEVENT_FOCUS_LOST:
			has_focus_ = false;
			reset_objects_state();
			break;

		default:
			break;
	}
}

void SdlInputGamepadDevice::handle_add_remove_controller_event(
	const ltjs::SystemEvent& system_event)
{
	switch (system_event.type)
	{
		case ::SDL_CONTROLLERDEVICEADDED:
			if (!game_controller_)
			{
				open_game_controller();
			}

			break;

		case ::SDL_CONTROLLERDEVICEREMOVED:
			if (system_event.cdevice.which == joystick_id_)
			{
				close_game_controller();
			}

			break;
	}
}

const SdlInputDeviceObjectDataFormat* SdlInputGamepadDevice::find_object_data_format_by_id(
	int object_id) const
{
	for (auto i = 0; i < data_format_.object_count; ++i)
	{
		const auto& object_data_format = object_data_formats_[i];

		if (object_data_format.object_id == object_id)
		{
			return &object_data_format;
		}
	}

	return nullptr;
}

void SdlInputGamepadDevice::reset_objects_state()
{
	is_pov_pending_ = false;

	left_trigger_ = 0;
	right_trigger_ = 0;

	is_dpad_up_pressed_ = false;
	is_dpad_down_pressed_ = false;
	is_dpad_left_pressed_ = false;
	is_dpad_right_pressed_ = false;
}

int SdlInputGamepadDevice::make_direct_input_pov() const noexcept
{
	constexpr int di_hundred_degrees[] =
	{
		// u[ ], d[ ], l[ ], r[ ]
		-1,

		// u[ ], d[ ], l[ ], r[x]
		90'00,

		// u[ ], d[ ], l[x], r[ ]
		270'00,

		// u[ ], d[ ], l[x], r[x]
		-2,

		// u[ ], d[x], l[ ], r[ ]
		180'00,

		// u[ ], d[x], l[ ], r[x]
		135'00,

		// u[ ], d[x], l[x], r[ ]
		225'00,

		// u[ ], d[x], l[x], r[x]
		-2,

		// u[x], d[ ], l[ ], r[ ]
		0'00,

		// u[x], d[ ], l[ ], r[x]
		45'00,

		// u[x], d[ ], l[x], r[ ]
		315'00,

		// u[x], d[ ], l[x], r[x]
		-2,

		// u[x], d[x], l[ ], r[ ]
		-2,

		// u[x], d[x], l[ ], r[x]
		-2,

		// u[x], d[x], l[x], r[ ]
		-2,

		// u[x], d[x], l[x], r[x]
		-2,
	};

	const auto index =
		(is_dpad_up_pressed_ << 3) |
		(is_dpad_down_pressed_ << 2) |
		(is_dpad_left_pressed_ << 1) |
		(is_dpad_right_pressed_ << 0);

	return di_hundred_degrees[index];
}

int SdlInputGamepadDevice::normalize_axis_value(
	int value) const
{
	if (std::abs(value) > dead_zone_radius)
	{
		return value;
	}
	else
	{
		return 0;
	}
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInput
{
public:
	SdlInput() noexcept = default;

	virtual ~SdlInput() = default;


	virtual int get_available_device_count() const noexcept = 0;

	virtual const SdlInputDeviceInfo& get_available_device_info(
		int index) const = 0;

	virtual const SdlInputDeviceInfo* find_available_device_info(
		SdlInputDeviceType device_type) const noexcept = 0;


	virtual SdlInputDevice* add_device(
		int device_id) = 0;

	virtual void remove_device(
		SdlInputDevice* device) = 0;

	virtual int get_device_count() const noexcept = 0;

	virtual const SdlInputDevice& get_device(
		int index) const = 0;


	static constexpr auto unrestricted_min_axis_value = std::numeric_limits<std::int32_t>::min();
	static constexpr auto unrestricted_max_axis_value = std::numeric_limits<std::int32_t>::max();

	static constexpr auto min_axis_value = -32'768;
	static constexpr auto max_axis_value = +32'767;
}; // SdlInput


struct SdlInputCreateParam
{
	ltjs::SystemEventHandlerMgr* system_event_handler_mgr{};
	bool supports_joystick{};
}; // SdlInputCreateParam

using SdlInputUPtr = std::unique_ptr<SdlInput>;

SdlInputUPtr make_sdl_input(
	const SdlInputCreateParam& param);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInputImplException :
	public ltjs::Exception
{
public:
	explicit SdlInputImplException(
		const char* message)
		:
		Exception{"LTJS_SDL_INPUT", message}
	{
	}
}; // SdlInputImplException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlInputImpl final :
	public SdlInput
{
public:
	explicit SdlInputImpl(
		const SdlInputCreateParam& param);

	~SdlInputImpl() override;


	// ======================================================================
	// SdlInput

	int get_available_device_count() const noexcept override;

	const SdlInputDeviceInfo& get_available_device_info(
		int index) const override;

	const SdlInputDeviceInfo* find_available_device_info(
		SdlInputDeviceType device_type) const noexcept override;


	SdlInputDevice* add_device(
		int device_id) override;

	void remove_device(
		SdlInputDevice* device) override;

	int get_device_count() const noexcept override;

	const SdlInputDevice& get_device(
		int index) const override;

	// SdlInput
	// ======================================================================


private:
	class SdlInputSystemEventHandler final :
		public ltjs::SystemEventHandler
	{
	public:
		void initialize(
			SdlInputImpl* sdl_input) noexcept;


		// ==================================================================
		// ltjs::SystemEventHandler

		bool operator()(
			const ltjs::SystemEvent& system_event) override;

		// ltjs::SystemEventHandler
		// ==================================================================


	private:
		SdlInputImpl* sdl_input_{};
	}; // SdlInputSystemEventHandler


	using GameControllerName = std::string;

	struct AvailableDeviceInfo
	{
		SdlInputDeviceInfo device_info{};
	}; // AvailableDeviceInfo

	using AvailableDeviceInfos = std::vector<AvailableDeviceInfo>;

	using Device = std::unique_ptr<SdlInputDevice>;
	using Devices = std::list<Device>;


	ltjs::SystemEventHandlerMgr* system_event_handler_mgr_{};
	SdlInputSystemEventHandler system_event_handler_{};
	int device_unique_id_{};
	ltjs::SdlSubsystem sdl_joystick_subsystem_{};
	ltjs::SdlSubsystem sdl_game_controller_subsystem_{};
	AvailableDeviceInfos available_device_infos_{};
	Devices devices_{};


	int generate_unique_id() noexcept;

	AvailableDeviceInfo make_keyboard_device_info() noexcept;

	AvailableDeviceInfo make_mouse_device_info() noexcept;

	bool has_available_game_controller(
		const GameControllerName& game_controller_name) const noexcept;

	GameControllerName get_game_controller_name_from_joystick_index(
		int joystick_index) const noexcept;

	void update_game_controllers();

	void initialize_available_devices();

	void handle_system_event(
		const ltjs::SystemEvent& system_event);
}; // SdlInputImpl

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void SdlInputImpl::SdlInputSystemEventHandler::initialize(
	SdlInputImpl* sdl_input) noexcept
{
	assert(sdl_input);
	sdl_input_ = sdl_input;
}

bool SdlInputImpl::SdlInputSystemEventHandler::operator()(
	const ltjs::SystemEvent& system_event)
{
	if (sdl_input_)
	{
		sdl_input_->handle_system_event(system_event);
	}

	return false;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SdlInputImpl::SdlInputImpl(
	const SdlInputCreateParam& param)
{
	if (!param.system_event_handler_mgr)
	{
		throw SdlInputImplException{"Null system event handler manager."};
	}

	if (param.supports_joystick)
	{
		sdl_joystick_subsystem_ = ltjs::SdlSubsystem{SDL_INIT_JOYSTICK};
		sdl_game_controller_subsystem_ = ltjs::SdlSubsystem{SDL_INIT_GAMECONTROLLER};
	}

	initialize_available_devices();

	system_event_handler_mgr_ = param.system_event_handler_mgr;

	system_event_handler_.initialize(this);

	system_event_handler_mgr_->add(
		&system_event_handler_,
		ltjs::SystemEventHandlerPriority::high
	);
}

SdlInputImpl::~SdlInputImpl()
{
	if (system_event_handler_mgr_)
	{
		system_event_handler_mgr_->remove(&system_event_handler_);
	}
}

int SdlInputImpl::get_available_device_count() const noexcept
{
	return static_cast<int>(available_device_infos_.size());
}

const SdlInputDeviceInfo& SdlInputImpl::get_available_device_info(
	int index) const
{
	if (index < 0 || index >= get_available_device_count())
	{
		throw SdlInputImplException{"Device index out of range."};
	}

	return available_device_infos_[index].device_info;
}

const SdlInputDeviceInfo* SdlInputImpl::find_available_device_info(
	SdlInputDeviceType device_type) const noexcept
{
	const auto available_device_info_end_it = available_device_infos_.cend();

	const auto available_device_info_it = std::find_if(
		available_device_infos_.cbegin(),
		available_device_info_end_it,
		[device_type](
			const AvailableDeviceInfo& available_device_info)
		{
			return available_device_info.device_info.type == device_type;
		}
	);

	if (available_device_info_it == available_device_info_end_it)
	{
		return nullptr;
	}

	return &(available_device_info_it->device_info);
}

SdlInputDevice* SdlInputImpl::add_device(
	int device_id)
{
	const auto device_end_it = devices_.cend();

	const auto device_it = std::find_if(
		devices_.cbegin(),
		device_end_it,
		[device_id](
			const Device& device)
		{
			const auto& device_info = device->get_device_info();
			return device_info.id == device_id;
		}
	);

	if (device_it != device_end_it)
	{
		return device_it->get();
	}

	const auto available_device_info_end_it = available_device_infos_.cend();

	const auto available_device_info_it = std::find_if(
		available_device_infos_.cbegin(),
		available_device_info_end_it,
		[device_id](
			const AvailableDeviceInfo& available_device_info)
		{
			return available_device_info.device_info.id == device_id;
		}
	);

	if (available_device_info_it == available_device_info_end_it)
	{
		throw SdlInputImplException{"Device not available."};
	}

	switch (available_device_info_it->device_info.type)
	{
		case SdlInputDeviceType::gamepad:
			devices_.emplace_back(new SdlInputGamepadDevice{available_device_info_it->device_info});
			break;

		case SdlInputDeviceType::keyboard:
			devices_.emplace_back(new SdlInputKeyboardDevice{available_device_info_it->device_info});
			break;

		case SdlInputDeviceType::mouse:
			devices_.emplace_back(new SdlInputMouseDevice{available_device_info_it->device_info});
			break;

		default:
			throw SdlInputImplException{"Unsupported device type."};
	}

	return devices_.back().get();
}

void SdlInputImpl::remove_device(
	SdlInputDevice* device)
{
	devices_.remove_if(
		[device_to_remove = device](
			const Device& device)
		{
			return device.get() == device_to_remove;
		}
	);
}

int SdlInputImpl::get_device_count() const noexcept
{
	return static_cast<int>(devices_.size());
}

const SdlInputDevice& SdlInputImpl::get_device(
	int index) const
{
	if (index < 0 && index >= get_device_count())
	{
		throw SdlInputImplException{"Index out of range."};
	}

	auto device_it = devices_.begin();

	for (auto i = 0; i < index; ++i)
	{
		++device_it;
	}

	return *(device_it->get());
}

int SdlInputImpl::generate_unique_id() noexcept
{
	return ++device_unique_id_;
}

SdlInputImpl::AvailableDeviceInfo SdlInputImpl::make_keyboard_device_info() noexcept
{
	auto available_device_info = AvailableDeviceInfo{};

	auto& device_info = available_device_info.device_info;
	device_info.id = generate_unique_id();
	device_info.type = SdlInputDeviceType::keyboard;
	device_info.name = "Keyboard";

	return available_device_info;
}

SdlInputImpl::AvailableDeviceInfo SdlInputImpl::make_mouse_device_info() noexcept
{
	auto available_device_info = AvailableDeviceInfo{};

	auto& device_info = available_device_info.device_info;
	device_info.id = generate_unique_id();
	device_info.type = SdlInputDeviceType::mouse;
	device_info.name = "Mouse";

	return available_device_info;
}

bool SdlInputImpl::has_available_game_controller(
	const GameControllerName& game_controller_name) const noexcept
{
	return std::any_of(
		available_device_infos_.cbegin(),
		available_device_infos_.cend(),
		[&game_controller_name](
			const AvailableDeviceInfo& available_device_info)
		{
			return available_device_info.device_info.name == game_controller_name;
		}
	);
}

SdlInputImpl::GameControllerName SdlInputImpl::get_game_controller_name_from_joystick_index(
	int joystick_index) const noexcept
{
	if (::SDL_IsGameController(joystick_index) == ::SDL_FALSE)
	{
		return GameControllerName{};
	}

	const auto name = ::SDL_GameControllerNameForIndex(joystick_index);

	if (!name)
	{
		return GameControllerName{};
	}

	return name;
}

void SdlInputImpl::update_game_controllers()
{
	const auto joystick_count = ::SDL_NumJoysticks();

	for (auto joystick_index = 0; joystick_index < joystick_count; ++joystick_index)
	{
		const auto game_controller_name = get_game_controller_name_from_joystick_index(joystick_index);

		if (game_controller_name.empty())
		{
			continue;
		}

		if (has_available_game_controller(game_controller_name))
		{
			continue;
		}

		auto available_device_info = AvailableDeviceInfo{};

		auto& device_info = available_device_info.device_info;
		device_info.id = generate_unique_id();
		device_info.type = SdlInputDeviceType::gamepad;
		device_info.name = game_controller_name;

		available_device_infos_.emplace_back(available_device_info);
	}
}

void SdlInputImpl::initialize_available_devices()
{
	available_device_infos_.emplace_back(make_keyboard_device_info());
	available_device_infos_.emplace_back(make_mouse_device_info());

	if (sdl_joystick_subsystem_)
	{
		const auto joystick_event_state = ::SDL_JoystickEventState(SDL_ENABLE);
		assert(joystick_event_state == SDL_ENABLE);

		const auto game_controller_event_state = ::SDL_GameControllerEventState(SDL_ENABLE);
		assert(game_controller_event_state == SDL_ENABLE);

		update_game_controllers();
	}
}

void SdlInputImpl::handle_system_event(
	const ltjs::SystemEvent& system_event)
{
	for (const auto& device : devices_)
	{
		switch (system_event.type)
		{
			case ::SDL_CONTROLLERDEVICEADDED:
			case ::SDL_CONTROLLERDEVICEREMOVED:
				update_game_controllers();
				break;

			default:
				break;
		}

		device->handle_system_event(system_event);
	}
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SdlInputUPtr make_sdl_input(
	const SdlInputCreateParam& param)
{
	return std::make_unique<SdlInputImpl>(param);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


}


/*

	LithTech supports:

		3 axis -- x,y (mouse/joystick), and z
		toggle buttons - trigger actions
		push buttons - trigger actions

		Now, an actioncode of -10000 or less uses a console variable with the trigger name!
*/

extern int32 g_CV_InputRate;
extern int32 g_CV_JoystickDisable;
extern int32 g_CV_InputDebug;


// Forward declarations
LTRESULT input_GetManager(InputMgr** pMgr);


namespace
{


enum InputType
{
	Input_PushButton = 0,
	Input_ToggleButton = 1,
	Input_AbsAxis = 2,
	Input_RelAxis = 3,
	Input_Pov = 4,
};


struct TrackObjectInfo
{
	SdlInputDeviceObjectType guidType;
	int dwType;
	char tszName[INPUTNAME_LEN];
};


constexpr auto MAX_OBJECT_BINDINGS = 150;
constexpr auto INPUT_BUFFER_SIZE = 64;

// --------------------------------------------------------------------- //
// Action definitions
// --------------------------------------------------------------------- //

// moved to basedefs_de.h
//#define MAX_ACTIONNAME_LEN	30

class ActionDef
{
public:
	ActionDef()
	{
		m_pPrev = this;
		m_pNext = this;
	}

	int m_ActionCode;
	char m_ActionName[MAX_ACTIONNAME_LEN];

	ActionDef* m_pPrev;
	ActionDef* m_pNext;
};

ActionDef g_ActionDefHead;


ConsoleState* g_pInputConsoleState = nullptr;


// --------------------------------------------------------------------- //
// Device-to-action mappings
// --------------------------------------------------------------------- //

class CTriggerActionConsoleVar;

// list of all the console variables that are used in the TriggerAction
CTriggerActionConsoleVar* g_lstTriggerActionConsoleVariables = nullptr;

class TriggerAction
{
public:

	TriggerAction()
	{
		m_pConsoleString = nullptr;
		m_pAction = nullptr;
		m_pNext = nullptr;
		m_pConsoleVar = nullptr;
	}

	inline ~TriggerAction();

	// Both zero if it's not using range.
	float m_RangeLow;
	float m_RangeHigh;

	// If this is non-nullptr, then the action is a console string instead of an ActionDef.
	char* m_pConsoleString;

	ActionDef* m_pAction;
	TriggerAction* m_pNext;

	// Contains a pointer to a trigger console var structure that needs to be updated
	// this currently only works for axis types
	// Set to nullptr if there is no console variable to update.
	CTriggerActionConsoleVar* m_pConsoleVar;
};

class DeviceDef;

class TriggerObject :
	public CGLLNode
{
public:
	TriggerObject()
	{
		m_State = 0.0F;
		m_PrevState = 0.0F;
		m_BaseState = 0.0F;
		m_bJustWentDown = true;
		m_dwUpdateTime = 0;
		m_dwPrevUpdateTime = 0;
		m_dwBaseUpdateTime = 0;
		m_Scale = 1.0F;
	}

	~TriggerObject()
	{
		TriggerAction* pCur;
		TriggerAction* pNext;

		pCur = m_pActionHead;

		while (pCur)
		{
			pNext = pCur->m_pNext;
			delete pCur;
			pCur = pNext;
		}
	}

	// Info on the trigger (key, mouse axis, or mouse button...)
	char m_TriggerName[INPUTNAME_LEN];

	// Alternate name for it (like if m_TriggerName is ##21, this could be 'Space bar').
	char m_RealName[INPUTNAME_LEN];

	int m_diType;
	SdlInputDeviceObjectType m_diGuid;

	InputType m_InputType; // What DirectEngine sees it as.
	uint32 m_StateIndex; // index in the object state data array

	float m_State; // The current state of the trigger
	float m_PrevState; // Previous state of the trigger
	float m_BaseState;

	uint32 m_dwUpdateTime; // The time of the most recent update.
	uint32 m_dwPrevUpdateTime; // The time of the previous update
	uint32 m_dwBaseUpdateTime;

	bool m_bJustWentDown;

	// If it's a relative axis trigger, this scales the value.
	float m_Scale;

	// scale an axis value to a range if these are not both 0.0
	float m_fRangeScaleMin;
	float m_fRangeScaleMax;
	float m_fRangeScaleMultiplier;
	float m_fRangeScaleOffset;
	float m_fRangeScaleMultiplierLo;
	float m_fRangeScaleOffsetLo;
	float m_fRangeScaleMultiplierHi;
	float m_fRangeScaleOffsetHi;
	float m_fRangeScalePreCenterOffset;
	float m_fRangeScalePreCenter;

	// Which device it comes from.
	DeviceDef* m_pDevice;

	// The actions it will trigger.
	TriggerAction* m_pActionHead;

	// Data range.
	float m_DataMin;
	float m_DataMax;
};


// Trigger objects that have been created.
auto g_TriggerHead = TriggerObject{};

// class that holds records for storing trigger actions in a console variable
class CTriggerActionConsoleVar
{
public:
	CTriggerActionConsoleVar()
	{
		m_pTriggerAction = nullptr;
		m_pCommandVar = nullptr;
	}

	~CTriggerActionConsoleVar()
	{
		m_pTriggerAction = nullptr;
		m_pCommandVar = nullptr;

		// remove from global list
		if (g_lstTriggerActionConsoleVariables == this)
		{
			g_lstTriggerActionConsoleVariables = this->m_pNext;
		}
		else
		{
			CTriggerActionConsoleVar* pConVar = g_lstTriggerActionConsoleVariables;

			while (pConVar != nullptr)
			{
				if (pConVar->m_pNext == this)
				{
					pConVar->m_pNext = this->m_pNext;
					pConVar = nullptr;
				}
				else
				{
					pConVar = pConVar->m_pNext;
				}
			}
		}
	}

	void InitCommandVar()
	{
		m_pCommandVar = cc_FindConsoleVar(g_pInputConsoleState, GetCommandName());
	}

	char* GetCommandName()
	{
		if (m_pTriggerAction == nullptr)
		{
			return nullptr;
		}

		return m_pTriggerAction->m_pAction->m_ActionName;
	}

	void SetFloat(float nVal)
	{
		char sConValue[100];

		if (m_pTriggerAction != nullptr)
		{
			LTSNPrintF(sConValue, sizeof(sConValue), "%f", nVal);
			cc_SetConsoleVariable(g_pInputConsoleState, GetCommandName(), sConValue);
		}
	}

	float GetFloat()
	{
		if (m_pCommandVar == nullptr)
		{
			return 0.0;
		}
		else
		{
			return m_pCommandVar->floatVal;
		}
	}

	// trigger action that contains this console var
	TriggerAction* m_pTriggerAction;

	// pointer to data structure that holds information about the console variable
	LTCommandVar* m_pCommandVar;

	// pointer to next item in the list
	CTriggerActionConsoleVar* m_pNext;
};


class DeviceDef
{
public:
	DeviceDef()
	{
		m_pDevice = nullptr;
		m_pSpecialName = nullptr;
		m_bTracking = false;
		m_bTrackingOnly = false;
		m_dwLastTime = GetTickCount();
		m_pTrackObjects = nullptr;
		m_nTrackObjects = 0;
	}

	~DeviceDef()
	{
		GPOS pos;

		for (pos = m_Triggers; pos; )
		{
			delete m_Triggers.GetNext(pos);
		}

		m_Triggers.RemoveAll();
	}

	bool IsEnabled()
	{
		return m_pDevice != nullptr;
	}

	bool IsJoystick()
	{
		// TODO
		return false;
	}

	bool IsGamepad()
	{
		return m_DeviceType == SdlInputDeviceType::gamepad;
	}

	// nullptr if this device hasn't been enabled.
	SdlInputDevice* m_pDevice;

	// If it's referenced by one of the special names (##mouse or ##keyboard),
	// then it's stored here and should be saved in the bindings as such.
	const char* m_pSpecialName;

	// How many bytes used to read a state (GetDeviceState()).
	int m_StateReadSize;

	char m_InstanceName[INPUTNAME_LEN];

	int m_InstanceGuid;

	SdlInputDeviceType m_DeviceType;

	CGLinkedList<TriggerObject*> m_Triggers;

	// Trigger object pointers (indexed by device data offset / 4)
	TriggerObject* m_TriggerTable[MAX_OBJECT_BINDINGS];

	// Are we tracking this device?
	bool m_bTracking;

	// Is this device enabled solely for tracking purposes?
	bool m_bTrackingOnly;

	// Info on all objects for this device
	TrackObjectInfo* m_pTrackObjects;

	int m_nTrackObjects;

	uint32 m_dwLastTime;

	DeviceDef* m_pNext;
};

// DirectInput stuff.

auto g_pDirectInput = SdlInputUPtr{};

// Devices we've enumerated.
DeviceDef* g_pDeviceHead = nullptr;


TriggerAction::~TriggerAction()
{
	if (m_pConsoleString)
	{
		delete m_pConsoleString;
	}

	// remove the console var
	if (m_pConsoleVar != nullptr)
	{
		delete m_pConsoleVar;
		m_pConsoleVar = nullptr;
	}
}

// --------------------------------------------------------------------- //
// Helpers.
// --------------------------------------------------------------------- //

ActionDef* input_FindAction(const char* pActionName)
{
	ActionDef* pCur;

	pCur = g_ActionDefHead.m_pNext;

	while (pCur != &g_ActionDefHead)
	{
		if (CHelpers::UpperStrcmp(pActionName, pCur->m_ActionName))
		{
			return pCur;
		}

		pCur = pCur->m_pNext;
	}

	return nullptr;
}

DeviceDef* input_FindDeviceByType(
	SdlInputDeviceType nType)
{
	for (auto pDev = g_pDeviceHead; pDev != nullptr; pDev = pDev->m_pNext)
	{
		if (pDev->m_DeviceType == nType)
		{
			return pDev;
		}
	}

	return nullptr;
}

DeviceDef* FindDeviceByTypeWithSkip(
	SdlInputDeviceType nType,
	int nSkipNum = 0)
{
	auto pDev = g_pDeviceHead;

	while (pDev)
	{
		if (pDev->m_DeviceType == nType)
		{
			if (nSkipNum <= 0)
			{
				return pDev;
			}
			else
			{
				nSkipNum--;
			}
		}

		pDev = pDev->m_pNext;
	}

	return nullptr;
}

DeviceDef* input_FindDeviceByName(
	const char* pName)
{
	DeviceDef* pDev;

	// Look for it with the normal name.
	pDev = g_pDeviceHead;

	while (pDev)
	{
		if (CHelpers::UpperStrcmp(pName, pDev->m_InstanceName))
		{
			return pDev;
		}

		pDev = pDev->m_pNext;
	}

	// Look for it by its special name.
	if (stricmp(pName, "##mouse") == 0)
	{
		pDev = input_FindDeviceByType(SdlInputDeviceType::mouse);

		if (pDev)
		{
			pDev->m_pSpecialName = "##mouse";
		}

		return pDev;
	}
	else if (stricmp(pName, "##keyboard") == 0)
	{
		pDev = input_FindDeviceByType(SdlInputDeviceType::keyboard);

		if (pDev)
		{
			pDev->m_pSpecialName = "##keyboard";
		}

		return pDev;
	}
	// BBi
	// TODO
#if 0
	// KEF - 1/4/00 - Evil hack to handle Win98 SE not defining "Joystick 1"
	else if (strnicmp(pName, "Joystick 1", 8) == 0)
	{
		pDev = input_FindDeviceByType(DI8DEVTYPE_JOYSTICK);
		if (pDev)
			pDev->m_pSpecialName = "Joystick 1";

		return pDev;
	}
#endif

	return nullptr;
}


TriggerObject* input_FindTrigger(DeviceDef* pDef, const char* pName)
{
	TriggerObject* pCur;
	GPOS pos;

	// Look for it the normal way..
	for (pos = pDef->m_Triggers; pos; )
	{
		pCur = pDef->m_Triggers.GetNext(pos);

		if (stricmp(pCur->m_TriggerName, pName) == 0 ||
			stricmp(pCur->m_RealName, pName) == 0)
		{
			return pCur;
		}
	}

	return nullptr;
}

TriggerObject* input_AddTrigger(DeviceDef* pDevice, const char* pTriggerName)
{
	TriggerObject* pTrigger;
	int inputType;

	ASSERT(pDevice->IsEnabled());

	auto object_type = SdlInputDeviceObjectType::none;
	auto object_instance = -1;
	auto object_id = -1;

	// Setup for special types..
	if (pTriggerName[0] != 0 && pTriggerName[1] != 0)
	{
		if (pTriggerName[0] == '#' && pTriggerName[1] == '#')
		{
			if (stricmp(&pTriggerName[2], "x-axis") == 0)
			{
				object_type = SdlInputDeviceObjectType::x_axis;
			}
			else if (stricmp(&pTriggerName[2], "y-axis") == 0)
			{
				object_type = SdlInputDeviceObjectType::y_axis;
			}
			else if (stricmp(&pTriggerName[2], "z-axis") == 0)
			{
				object_type = SdlInputDeviceObjectType::z_axis;
			}
			else if (isalnum(pTriggerName[2]))
			{
				object_instance = ::atoi(&pTriggerName[2]);
			}
		}
	}

	// Enumerate objects on this device.
	const auto object_count = pDevice->m_pDevice->get_object_count();

	for (auto i = 0; i < object_count; ++i)
	{
		const auto& object = pDevice->m_pDevice->get_object(i);

		if (object_type != SdlInputDeviceObjectType::none)
		{
			if (object.type == object_type)
			{
				object_id = object.id;
				break;
			}
		}
		else if (object_instance >= 0)
		{
			if (object.id == object_instance)
			{
				object_id = object.id;
				break;
			}
		}
		else
		{
			if (object.name == pTriggerName)
			{
				object_id = object.id;
				break;
			}
		}
	}

	if (object_id < 0)
	{
		return nullptr;
	}

	// Get the object found.  We don't trust the name found during enumeration.  It was
	// causing bugs on WinXP.  This seems more reliable.
	const auto object = pDevice->m_pDevice->find_object_by_id(object_id);

	if (!object)
	{
		return nullptr;
	}

	// Found an object with that name.  Make sure DirectEngine supports its type of input.
	inputType = -1;

	if (object->type == SdlInputDeviceObjectType::key ||
		object->type == SdlInputDeviceObjectType::push_button)
	{
		inputType = Input_PushButton;
	}
	else if (object->axis_mode == SdlInputAxisMode::relative)
	{
		inputType = Input_RelAxis;
	}
	else if (object->axis_mode == SdlInputAxisMode::absolute)
	{
		inputType = Input_AbsAxis;
	}
	else if (object->type == SdlInputDeviceObjectType::pov)
	{
		inputType = Input_Pov;
	}

	if (inputType == -1)
	{
		return nullptr;
	}

	// Setup a TriggerObject for it.
	LT_MEM_TRACK_ALLOC(pTrigger = new TriggerObject, LT_MEM_TYPE_INPUT);
	memset(pTrigger, 0, sizeof(TriggerObject));

	pDevice->m_Triggers.AddHead(pTrigger);

	LTStrCpy(pTrigger->m_RealName, pTriggerName, INPUTNAME_LEN);
	LTStrCpy(pTrigger->m_TriggerName, object->name.c_str(), INPUTNAME_LEN);

	pTrigger->m_InputType = (InputType)inputType;

	pTrigger->m_diGuid = object->type;
	pTrigger->m_diType = object->id;

	pTrigger->m_pDevice = pDevice;
	pTrigger->m_Scale = 1.0f;
	pTrigger->m_fRangeScaleMin = 0.0f;
	pTrigger->m_fRangeScaleMax = 0.0f;
	pTrigger->m_fRangeScaleMultiplier = 1.0f;
	pTrigger->m_fRangeScaleOffset = 0.0f;
	pTrigger->m_fRangeScaleMultiplierHi = 1.0f;
	pTrigger->m_fRangeScaleOffsetHi = 0.0f;
	pTrigger->m_fRangeScaleMultiplierLo = 1.0f;
	pTrigger->m_fRangeScaleOffsetLo = 0.0f;
	pTrigger->m_fRangeScalePreCenterOffset = 0.0f;
	pTrigger->m_fRangeScalePreCenter = 0.0f;

	// Get the trigger max range.
	pTrigger->m_DataMin = static_cast<float>(SdlInput::unrestricted_min_axis_value);
	pTrigger->m_DataMax = static_cast<float>(SdlInput::unrestricted_max_axis_value);

	if (pDevice->m_pDevice && inputType == Input_AbsAxis)
	{
		pTrigger->m_DataMin = static_cast<float>(SdlInput::min_axis_value);
		pTrigger->m_DataMax = static_cast<float>(SdlInput::max_axis_value);
	}

	return pTrigger;
}



// --------------------------------------------------------------------- //
// The main function that sets up how we'll communicate with DirectInput
// about the device objects.
// --------------------------------------------------------------------- //

bool input_SetupDeviceFormats(DeviceDef* pDevice)
{
	SdlInputDeviceObjectDataFormat objectFormats[MAX_OBJECT_BINDINGS];
	TriggerObject* pTrigger;
	int iTrigger;
	int curOffset;
	int dwStates[MAX_OBJECT_BINDINGS];
	bool bDuplicate;
	TriggerObject* ptr;
	GPOS pos, pos2;


	ASSERT(pDevice->IsEnabled());

	// Reset the TriggerTable
	memset(pDevice->m_TriggerTable, 0, sizeof(TriggerObject*) * MAX_OBJECT_BINDINGS);

	// Setup the object formats.
	iTrigger = 0;
	curOffset = 0;

	for (pos = pDevice->m_Triggers; pos; )
	{
		pTrigger = pDevice->m_Triggers.GetNext(pos);

		if (iTrigger >= MAX_OBJECT_BINDINGS)
		{
			break;
		}

		// make sure we haven't already created on object data instance for this object
		bDuplicate = false;

		for (pos2 = pDevice->m_Triggers; pos2; )
		{
			ptr = pDevice->m_Triggers.GetNext(pos2);

			if (ptr == pTrigger)
			{
				break;
			}

			if (stricmp(ptr->m_TriggerName, pTrigger->m_TriggerName) == 0 ||
				stricmp(ptr->m_RealName, pTrigger->m_RealName) == 0)
			{
				pTrigger->m_StateIndex = ptr->m_StateIndex;
				bDuplicate = true;
				break;
			}
		}

		if (bDuplicate)
		{
			continue;
		}

		// create the object data instance
		objectFormats[iTrigger].offset = curOffset;
		objectFormats[iTrigger].object_id = pTrigger->m_diType;
		pTrigger->m_StateIndex = iTrigger;

		// put the pointer to this trigger in the TriggerTable
		pDevice->m_TriggerTable[curOffset >> 2] = pTrigger;

		++iTrigger;
		curOffset += 4;
	}


	// Set format.
	auto format = SdlInputDeviceDataFormat{};
	format.state_size = curOffset;
	format.object_count = iTrigger;
	format.object_data_formats = objectFormats;

	pDevice->m_pDevice->set_data_format(format);

	pDevice->m_StateReadSize = curOffset;

	// Init some data in case we can't read the current state.
	for (pos = pDevice->m_Triggers; pos; )
	{
		pTrigger = pDevice->m_Triggers.GetNext(pos);

		if ((pTrigger->m_InputType == Input_RelAxis) || (pTrigger->m_InputType == Input_AbsAxis))
		{
			pTrigger->m_State =
				((pTrigger->m_DataMax - pTrigger->m_DataMin) / 2.0F) +
				pTrigger->m_fRangeScalePreCenterOffset;

			pTrigger->m_PrevState = pTrigger->m_State;
			pTrigger->m_BaseState = pTrigger->m_PrevState;
		}
		else
		{
			pTrigger->m_State = 0.0F;
			pTrigger->m_PrevState = 0.0F;
			pTrigger->m_BaseState = 0.0F;
		}
	}

	// Init the states of all the triggers for this device
	pDevice->m_pDevice->get_state(dwStates, pDevice->m_StateReadSize);

	for (pos = pDevice->m_Triggers; pos; )
	{
		pTrigger = pDevice->m_Triggers.GetNext(pos);

		if (pTrigger->m_InputType == Input_RelAxis ||
			pTrigger->m_InputType == Input_AbsAxis)
		{
			pTrigger->m_State = static_cast<float>(dwStates[pTrigger->m_StateIndex]);
			pTrigger->m_PrevState = pTrigger->m_State;
			pTrigger->m_BaseState = pTrigger->m_State;
		}
		else
		{
			pTrigger->m_State = static_cast<float>(dwStates[pTrigger->m_StateIndex]);
			pTrigger->m_PrevState = pTrigger->m_State;
			pTrigger->m_BaseState = pTrigger->m_State;
		}
	}

	return true;
}

const char* GetDeviceObjectTypeString(
	const SdlInputDeviceObject& pObj)
{
	if (pObj.type == SdlInputDeviceObjectType::x_axis)
	{
		return "X axis";
	}
	else if (pObj.type == SdlInputDeviceObjectType::y_axis)
	{
		return "Y axis";
	}
	else if (pObj.type == SdlInputDeviceObjectType::z_axis)
	{
		return "Z axis";
	}
	else if (pObj.type == SdlInputDeviceObjectType::rx_axis)
	{
		return "X axis rotation";
	}
	else if (pObj.type == SdlInputDeviceObjectType::ry_axis)
	{
		return "Y axis rotation";
	}
	else if (pObj.type == SdlInputDeviceObjectType::rz_axis)
	{
		return "Z axis rotation";
	}
	else if (pObj.type == SdlInputDeviceObjectType::push_button)
	{
		return "button";
	}
	else if (pObj.type == SdlInputDeviceObjectType::key)
	{
		return "key";
	}
	else if (pObj.type == SdlInputDeviceObjectType::pov)
	{
		return "POV hat";
	}
	else
	{
		return "unknown";
	}
}

// --------------------------------------------------------------------- //
// Term..
// --------------------------------------------------------------------- //

void input_Term(InputMgr* pMgr)
{
	DeviceDef* pDef;
	DeviceDef* pNext;
	ActionDef* pCurAction;
	ActionDef* pNextAction;

	g_pInputConsoleState = nullptr;

	// Clear the device defs.
	pDef = g_pDeviceHead;

	while (pDef)
	{
		pNext = pDef->m_pNext;

		if (pDef->m_pDevice)
		{
			g_pDirectInput->remove_device(pDef->m_pDevice);
		}

		delete pDef;

		pDef = pNext;
	}

	g_pDeviceHead = nullptr;
	g_pDirectInput = nullptr;

	// Free all action defs.
	pCurAction = g_ActionDefHead.m_pNext;

	while (pCurAction != &g_ActionDefHead)
	{
		pNextAction = pCurAction->m_pNext;
		delete pCurAction;
		pCurAction = pNextAction;
	}

	g_ActionDefHead.m_pPrev = &g_ActionDefHead;
	g_ActionDefHead.m_pNext = &g_ActionDefHead;
}


// --------------------------------------------------------------------- //
// Init input.
// --------------------------------------------------------------------- //

bool input_Init(InputMgr* pMgr, ConsoleState* pState)
{
	g_pInputConsoleState = pState;

	const auto system_event_handler_mgr = static_cast<ltjs::SystemEventHandlerMgr*>(
		dsi_get_system_event_handler_mgr()
	);

	if (!system_event_handler_mgr)
	{
		assert(!"Null system event handler manager.");
		return false;
	}

	auto sdl_input_create_param = SdlInputCreateParam{};
	sdl_input_create_param.system_event_handler_mgr = system_event_handler_mgr;
	sdl_input_create_param.supports_joystick = (::g_CV_JoystickDisable == 0);

	g_pDirectInput = make_sdl_input(sdl_input_create_param);

	const auto device_count = g_pDirectInput->get_available_device_count();

	for (auto i = 0; i < device_count; ++i)
	{
		const auto& device_info = g_pDirectInput->get_available_device_info(i);

		DeviceDef* pDef;

		LT_MEM_TRACK_ALLOC(pDef = new DeviceDef, LT_MEM_TYPE_INPUT);
		(*pDef) = DeviceDef{};
		LTStrCpy(pDef->m_InstanceName, device_info.name.c_str(), INPUTNAME_LEN);
		pDef->m_InstanceGuid = device_info.id;
		pDef->m_DeviceType = device_info.type;

		pDef->m_pNext = g_pDeviceHead;
		g_pDeviceHead = pDef;
	}

	return true;
}


bool input_IsInitted(InputMgr* pMgr)
{
	return g_pDirectInput != nullptr;
}


void input_ListDevices(InputMgr* pMgr)
{
	if (!g_pDirectInput)
	{
		dsi_ConsolePrint("Input not initialized");
	}

	dsi_ConsolePrint("------------ Input devices ------------");

	const auto device_count = g_pDirectInput->get_device_count();

	for (auto i = 0; i < device_count; ++i)
	{
		const auto& device = g_pDirectInput->get_device(i);
		const auto& device_info = device.get_device_info();

		dsi_ConsolePrint("- Device %s", device_info.name.c_str());

		const auto object_count = device.get_object_count();

		for (auto j = 0; j < object_count; ++j)
		{
			const auto& object = device.get_object(j);

			dsi_ConsolePrint("-    Object %s (type: %s)", object.name.c_str(), GetDeviceObjectTypeString(object));
		}
	}
}


// --------------------------------------------------------------------- //
// Plays a force feedback effect if loaded
// --------------------------------------------------------------------- //

long input_PlayJoystickEffect(InputMgr* pMgr, const char* strEffectName, float x, float y)
{
	return 0;
}

// --------------------------------------------------------------------- //
// Enables input from a particular device.
// --------------------------------------------------------------------- //

bool input_EnableDevice(InputMgr* pMgr, const char* pDeviceName)
{
	DeviceDef* pDevice;

	bool bKeyboard;
	bool bMouse;

	int theGuid;


	bKeyboard = false;
	bMouse = false;

	if (!input_IsInitted(pMgr))
	{
		return false;
	}

	// Find the device.
	pDevice = input_FindDeviceByName(pDeviceName);

	if (!pDevice)
	{
		return false;
	}

	// Create the device if it hasn't been created yet.
	if (!pDevice->m_pDevice)
	{
		theGuid = pDevice->m_InstanceGuid;

		pDevice->m_pDevice = g_pDirectInput->add_device(theGuid);

		if (!pDevice->m_pDevice)
		{
			return false;
		}

		auto data_format = SdlInputDeviceDataFormat{};
		pDevice->m_pDevice->set_data_format(data_format);
	}

	return true;
}


// --------------------------------------------------------------------- //
// This is the main routine that reads input from all devices and sets
// any actions that are on.
// --------------------------------------------------------------------- //
void input_ReadInput(InputMgr* pMgr, BYTE* pActionsOn, float axisOffsets[3])
{
	DeviceDef* pDevice;
	TriggerObject* pTrigger;
	TriggerAction* pAction;
	SdlInputDeviceObjectData data[INPUT_BUFFER_SIZE];
	int i;
	int nEvents;
	int actionCode;
	float addAmt;
	uint32 dwTickCount;
	GPOS pos;
	CTriggerActionConsoleVar* pTrigConVar;

	// clear the axis array
	axisOffsets[0] = 0.0F;
	axisOffsets[1] = 0.0F;
	axisOffsets[2] = 0.0F;

	// clear all of the axis console variables 
	pTrigConVar = g_lstTriggerActionConsoleVariables;

	while (pTrigConVar != nullptr)
	{
		pTrigConVar->SetFloat(0.0F);
		pTrigConVar->InitCommandVar();
		pTrigConVar = pTrigConVar->m_pNext;
	}

	// Get the time stamp for this frame...
	dwTickCount = GetTickCount();

	// Query each device.
	pDevice = g_pDeviceHead;

	while (pDevice)
	{
		while (pDevice->IsEnabled())
		{
			memset(data, 0, sizeof(data));

			nEvents = pDevice->m_pDevice->get_data(data, INPUT_BUFFER_SIZE);

			// Go through each event and update the associated triggers
			// (unless it's a key up event - handle those at the end in
			//  case there was a down-up combination in the same frame)
			for (i = 0; i < nEvents; i++)
			{
				if ((data[i].offset >> 2) >= static_cast<int>(pDevice->m_Triggers.GetSize()))
				{
					continue;
				}

				// get the trigger associated with this event
				pTrigger = pDevice->m_TriggerTable[data[i].offset >> 2];

				if (g_CV_InputDebug)
				{
					dsi_ConsolePrint(
						"%s (%s) generated %d",
						pTrigger->m_TriggerName,
						pTrigger->m_RealName,
						data[i].value
					);
				}

				// See if the trigger is a button...
				if (pTrigger->m_InputType == Input_PushButton)
				{
					// If the button was up, then worry about it later, otherwise save the data...
					if (data[i].value != 0)
					{
						// Did it just go down?
						if (!pTrigger->m_State)
						{
							// Maybe trigger a console command..
							pTrigger->m_bJustWentDown = true;
						}

						pTrigger->m_State = static_cast<float>(data[i].value);
					}

					continue;
				}

				// See if the trigger is a hat...
				if (pTrigger->m_InputType == Input_Pov)
				{
					pTrigger->m_State = static_cast<float>(data[i].value);
					continue;
				}

				// Process all the non-buttons...
				pTrigger->m_dwBaseUpdateTime = pTrigger->m_dwPrevUpdateTime;
				pTrigger->m_dwPrevUpdateTime = pTrigger->m_dwUpdateTime;
				pTrigger->m_dwUpdateTime = dwTickCount;

				// set the trigger's state
				pTrigger->m_BaseState = pTrigger->m_PrevState;
				pTrigger->m_PrevState = pTrigger->m_State;
				pTrigger->m_State = static_cast<float>(data[i].value);
			}


			// Go thru each trigger and check the state.
			for (pos = pDevice->m_Triggers; pos; )
			{
				pTrigger = pDevice->m_Triggers.GetNext(pos);

				// Interpolate the non-pushbuttons...
				if (pTrigger->m_InputType == Input_PushButton)
				{
					// Scale the state...
					addAmt = pTrigger->m_State * pTrigger->m_Scale;
				}
				else
				{
					float fCurTriggerState = pTrigger->m_State;

					if (pTrigger->m_InputType != Input_Pov)
					{
						fCurTriggerState = (pTrigger->m_BaseState + pTrigger->m_PrevState + pTrigger->m_State) / 3.0F;
						float fInterpolant = static_cast<float>(g_CV_InputRate) / 100.0F;
						fCurTriggerState = LTLERP(pTrigger->m_State, fCurTriggerState, fInterpolant);

						// Zero out relative axes based on the input rate
						if (pTrigger->m_InputType == Input_RelAxis)
						{
							// Simulate a zero sample based on the input rate
							if ((dwTickCount - pTrigger->m_dwUpdateTime) > (static_cast<uint32>(g_CV_InputRate) / 10))
							{
								pTrigger->m_BaseState = pTrigger->m_PrevState;
								pTrigger->m_PrevState = pTrigger->m_State;
								pTrigger->m_State = 0.0F;

								pTrigger->m_dwBaseUpdateTime = pTrigger->m_dwPrevUpdateTime;
								pTrigger->m_dwPrevUpdateTime = pTrigger->m_dwUpdateTime;
								pTrigger->m_dwUpdateTime = dwTickCount;
							}
						}
					}

					// Scale the state...
					addAmt = LTCLAMP(fCurTriggerState, pTrigger->m_DataMin, pTrigger->m_DataMax);

					if (pTrigger->m_fRangeScalePreCenterOffset != 0.0F)
					{
						if (addAmt <= pTrigger->m_fRangeScalePreCenter)
						{
							addAmt = pTrigger->m_fRangeScaleOffsetLo + (pTrigger->m_fRangeScaleMultiplierLo * addAmt);
						}
						else
						{
							addAmt = pTrigger->m_fRangeScaleOffsetHi + (pTrigger->m_fRangeScaleMultiplierHi * addAmt);
						}
					}
					else
					{
						addAmt = pTrigger->m_fRangeScaleOffset + (pTrigger->m_fRangeScaleMultiplier * addAmt);
					}

					// NOTE : the RangeScale stuff will not work well if there are mutiple addAmt's
					addAmt = addAmt * pTrigger->m_Scale;
				}


				// Hit all its actions.
				pAction = pTrigger->m_pActionHead;

				while (pAction)
				{
					if (pAction->m_pConsoleString && pTrigger->m_bJustWentDown && g_pInputConsoleState)
					{
						cc_HandleCommand(g_pInputConsoleState, pAction->m_pConsoleString);
					}
					else if (pAction->m_pAction)
					{
						// Map the trigger state to the action!
						actionCode = pAction->m_pAction->m_ActionCode;

						if (pTrigger->m_InputType == Input_RelAxis && actionCode < 0)
						{
							if (actionCode == -1)
							{
								axisOffsets[0] += addAmt;
							}
							else if (actionCode == -2)
							{
								axisOffsets[1] += addAmt;
							}
							else if (actionCode == -3)
							{
								axisOffsets[2] += addAmt;
							}
							else if (pAction->m_pConsoleVar)
							{
								pAction->m_pConsoleVar->SetFloat(pAction->m_pConsoleVar->GetFloat() + addAmt);

								if (g_CV_InputDebug)
								{
									dsi_ConsolePrint(
										"rel Trigger %s set console var %s to %22.12f",
										pTrigger->m_TriggerName,
										pAction->m_pConsoleVar->GetCommandName(),
										static_cast<double>(pAction->m_pConsoleVar->GetFloat())
									);
								}
							}
						}
						else if (pTrigger->m_InputType == Input_AbsAxis)
						{
							if (actionCode < 0)
							{
								if (actionCode == -1)
								{
									axisOffsets[0] += addAmt;
								}
								else if (actionCode == -2)
								{
									axisOffsets[1] += addAmt;
								}
								else if (actionCode == -3)
								{
									axisOffsets[2] += addAmt;
								}
								else if (pAction->m_pConsoleVar != nullptr)
								{
									pAction->m_pConsoleVar->SetFloat(pAction->m_pConsoleVar->GetFloat() + addAmt);

									if (g_CV_InputDebug)
									{
										dsi_ConsolePrint(
											"abs Trigger %s set console var %s to %22.12f",
											pTrigger->m_TriggerName,
											pAction->m_pConsoleVar->GetCommandName(),
											static_cast<double>(pAction->m_pConsoleVar->GetFloat())
										);
									}
								}
							}
							else if (addAmt >= pAction->m_RangeLow && addAmt <= pAction->m_RangeHigh)
							{
								ASSERT(actionCode < 256);
								pActionsOn[actionCode] |= 1;
							}
						}
						else if (pTrigger->m_InputType == Input_Pov && pAction->m_pConsoleVar)
						{
							if (pAction->m_pConsoleVar != nullptr)
							{
								pAction->m_pConsoleVar->SetFloat(pAction->m_pConsoleVar->GetFloat() + addAmt);
							}
						}
						else if (pTrigger->m_InputType == Input_Pov && actionCode < 0 && addAmt != 65536.0f)
						{
							if (addAmt >= 4500 && addAmt <= 13500)
							{
								axisOffsets[0] += 30;
							}
							else if (addAmt >= 22500 && addAmt <= 31500)
							{
								axisOffsets[0] -= 30;
							}

							if ((addAmt >= 31500 && addAmt <= 36000) || (addAmt <= 4500 && addAmt >= 0))
							{
								axisOffsets[1] -= 30;
							}

							if (addAmt >= 13500 && addAmt <= 22500)
							{
								axisOffsets[1] += 30;
							}
						}
						else if (pAction->m_RangeLow != 0.0F || pAction->m_RangeHigh != 0.0F)
						{
							if (addAmt >= pAction->m_RangeLow && addAmt <= pAction->m_RangeHigh)
							{
								ASSERT(actionCode < 256);
								pActionsOn[actionCode] |= 1;
							}
						}
						else if (actionCode >= 0)
						{
							ASSERT(actionCode < 256);
							pActionsOn[actionCode] |= pTrigger->m_State != 0.0f;
						}
					}

					pAction = pAction->m_pNext;
				}

				// Clear this..
				pTrigger->m_bJustWentDown = false;
			}

			// Go through any key up events and update the associated triggers
			// Also clear any relative axis data...
			for (i = 0; i < nEvents; i++)
			{
				if ((data[i].offset >> 2) >= static_cast<int>(pDevice->m_Triggers.GetSize()))
				{
					continue;
				}

				// get the trigger associated with this event
				pTrigger = pDevice->m_TriggerTable[data[i].offset >> 2];

				// see if the trigger is a button and if it's state has changed to 'up'
				if (pTrigger->m_InputType == Input_PushButton && data[i].value == 0)
				{
					pTrigger->m_State = static_cast<float>(data[i].value);
				}
			}

			if (nEvents == 0)
			{
				break;
			}
		}

		// Update the last time...
		pDevice->m_dwLastTime = dwTickCount;

		pDevice = pDevice->m_pNext;
	}
}


// --------------------------------------------------------------------- //
// Flush the DirectInput buffers
// --------------------------------------------------------------------- //

bool input_FlushInputBuffers(InputMgr* pMgr)
{
	auto pDef = g_pDeviceHead;

	while (pDef)
	{
		if (pDef->IsEnabled())
		{
			pDef->m_pDevice->flush_data();
		}

		pDef = pDef->m_pNext;
	}

	return true;
}

LTRESULT input_ClearInput()
{
	auto pDevice = g_pDeviceHead;

	while (pDevice)
	{
		if (pDevice->IsEnabled())
		{
			pDevice->m_pDevice->flush_data();
		}

		pDevice = pDevice->m_pNext;
	}

	return LT_OK;
}


// --------------------------------------------------------------------- //
// You want to add action defs first so there will be something to bind to!
// --------------------------------------------------------------------- //

void input_AddAction(InputMgr* pMgr, const char* pActionName, int code)
{
	ActionDef* pDef;

	if (code >= 256)
	{
		return;
	}

	// If there's already an action with that name, change its code.
	pDef = input_FindAction(pActionName);

	if (pDef)
	{
		pDef->m_ActionCode = code;
		return;
	}

	// Ok, add a new one.
	LT_MEM_TRACK_ALLOC(pDef = new ActionDef, LT_MEM_TYPE_INPUT);
	pDef->m_pPrev = g_ActionDefHead.m_pPrev;
	pDef->m_pNext = &g_ActionDefHead;
	pDef->m_pPrev->m_pNext = pDef->m_pNext->m_pPrev = pDef;
	LTStrCpy(pDef->m_ActionName, pActionName, MAX_ACTIONNAME_LEN);
	pDef->m_ActionCode = code;
}


// --------------------------------------------------------------------- //
// Clear bindings for an input trigger.
// Note:  It would probably be better to have this routine remove the trigger completely,
//        but it's not too big a deal right now.
// --------------------------------------------------------------------- //

bool input_ClearBindings(InputMgr* pMgr, const char* pDeviceName, const char* pTriggerName)
{
	DeviceDef* pDef = input_FindDeviceByName(pDeviceName);

	if (!pDef)
	{
		return false;
	}

	if (!pDef->IsEnabled())
	{
		return false;
	}

	TriggerObject* pTrigger = input_FindTrigger(pDef, pTriggerName);

	if (!pTrigger)
	{
		return false;
	}

	// Delete the trigger and remove it from the list.
	pDef->m_Triggers.RemoveAt(pTrigger);
	delete pTrigger;

	if (g_DebugLevel > 10)
	{
		dsi_ConsolePrint("Cleared bindings for %s (on %s)", pTriggerName, pDeviceName);
	}

	// Re-setup the device data formats / possibly acquire the device.
	return input_SetupDeviceFormats(pDef);
}


// --------------------------------------------------------------------- //
// Add a binding for a device.
// --------------------------------------------------------------------- //

bool input_AddBinding(InputMgr* pMgr,
	const char* pDeviceName, const char* pTriggerName, const char* pActionName,
	float rangeLow, float rangeHigh)
{
	TriggerObject* pTrigger;
	TriggerAction* pAction;
	CTriggerActionConsoleVar* pConVar = nullptr;
	ActionDef* pActionDef;
	DeviceDef* pDevice;
	float temp;


	if (rangeLow > rangeHigh)
	{
		temp = rangeLow;
		rangeLow = rangeHigh;
		rangeHigh = temp;
	}

	if (stricmp(pDeviceName, "Joystick 1") == 0)
	{
	}

	pDevice = input_FindDeviceByName(pDeviceName);

	if (!pDevice)
	{
		return false;
	}

	if (!pDevice->IsEnabled())
	{
		if (!input_EnableDevice(pMgr, pDeviceName))
		{
			return false;
		}
	}

	pTrigger = input_FindTrigger(pDevice, pTriggerName);

	if (!pTrigger)
	{
		pTrigger = input_AddTrigger(pDevice, pTriggerName);

		if (!pTrigger)
		{
			return false;
		}
	}

	if (strlen(pActionName) > 1 && pActionName[0] == '*')
	{
		pActionDef = nullptr;
	}
	else
	{
		// find the action
		pActionDef = input_FindAction(pActionName);

		if (!pActionDef)
		{
			return false;
		}

		// If we are using a console variable to send input data to the game set it up
		if (pActionDef->m_ActionCode <= -10000)
		{
			// make a new console var
			LT_MEM_TRACK_ALLOC(pConVar = new CTriggerActionConsoleVar, LT_MEM_TYPE_INPUT);

			// add to list of console vars for trigger actions
			pConVar->m_pNext = g_lstTriggerActionConsoleVariables;
			g_lstTriggerActionConsoleVariables = pConVar;
		}
	}

	// Add the action.
	LT_MEM_TRACK_ALLOC(pAction = new TriggerAction, LT_MEM_TYPE_INPUT);
	pAction->m_pNext = pTrigger->m_pActionHead;
	pAction->m_RangeLow = rangeLow;
	pAction->m_RangeHigh = rangeHigh;
	pAction->m_pConsoleVar = pConVar;

	if (pConVar != nullptr)
	{
		pConVar->m_pTriggerAction = pAction;
	}

	// If pActionDef is nullptr then it's a console string.
	pAction->m_pAction = pActionDef;

	if (!pActionDef)
	{
		LT_MEM_TRACK_ALLOC(pAction->m_pConsoleString = new char[strlen(pActionName)], LT_MEM_TYPE_INPUT);

		if (!pAction->m_pConsoleString)
		{
			delete pAction;
			return false;
		}

		strcpy(pAction->m_pConsoleString, &pActionName[1]);
	}

	pTrigger->m_pActionHead = pAction;

	// Re-setup the device data formats / possibly acquire the device.
	if (!input_SetupDeviceFormats(pDevice))
	{
		// Get rid of the trigger we just tried to add.
		if (pTrigger->m_pActionHead == pAction)
		{
			pTrigger->m_pActionHead = pAction->m_pNext;
			delete pAction;
		}

		input_SetupDeviceFormats(pDevice);

		return false;
	}

	if (g_DebugLevel > 10)
	{
		dsi_ConsolePrint(
			"Bound %s (on %s) to action %s",
			pTriggerName,
			pDeviceName,
			pActionName
		);
	}

	return true;
}

// --------------------------------------------------------------------- //
// Sets the trigger's scale.
// --------------------------------------------------------------------- //

bool input_ScaleTrigger(InputMgr* pMgr, const char* pDeviceName, const char* pTriggerName, float scale, float fRangeScaleMin, float fRangeScaleMax, float fRangeScalePreCenterOffset)
{
	DeviceDef* pDevice;
	TriggerObject* pTrigger;

	scale = LTCLAMP(scale, -500.0f, 500.0f);

	pDevice = input_FindDeviceByName(pDeviceName);

	if (!pDevice)
	{
		return false;
	}

	pTrigger = input_FindTrigger(pDevice, pTriggerName);

	if (!pTrigger)
	{
		return false;
	}

	pTrigger->m_Scale = scale;

	if (fRangeScaleMin != 0.0F || fRangeScaleMax != 0.0F)
	{
		pTrigger->m_fRangeScaleMin = fRangeScaleMin;
		pTrigger->m_fRangeScaleMax = fRangeScaleMax;

		float fDataMinMinusMax = pTrigger->m_DataMin - pTrigger->m_DataMax;

		if (fDataMinMinusMax != 0.0F)
		{
			pTrigger->m_fRangeScaleMultiplier = (fRangeScaleMin - fRangeScaleMax) / fDataMinMinusMax;
		}
		else
		{
			pTrigger->m_fRangeScaleMultiplier = 1.0F;
		}

		pTrigger->m_fRangeScaleOffset = fRangeScaleMin - (pTrigger->m_fRangeScaleMultiplier * pTrigger->m_DataMin);

		pTrigger->m_fRangeScalePreCenterOffset = fRangeScalePreCenterOffset;

		if (fRangeScalePreCenterOffset != 0.0F)
		{
			float fDataCenter =
				((pTrigger->m_DataMax - pTrigger->m_DataMin) / 2.0F) +
				pTrigger->m_DataMin + fRangeScalePreCenterOffset
			;

			float fRangeScaleCenter = ((fRangeScaleMax - fRangeScaleMin) / 2.0F) + fRangeScaleMin;

			float fDataMinMinusMaxHi = (fDataCenter - pTrigger->m_DataMax);

			if (fDataMinMinusMaxHi != 0.0F)
			{
				pTrigger->m_fRangeScaleMultiplierHi = (fRangeScaleCenter - fRangeScaleMax) / fDataMinMinusMaxHi;
			}
			else
			{
				pTrigger->m_fRangeScaleMultiplierHi = 1.0F;
			}

			pTrigger->m_fRangeScaleOffsetHi = fRangeScaleCenter - (pTrigger->m_fRangeScaleMultiplierHi * fDataCenter);

			float fDataMinMinusMaxLo = (pTrigger->m_DataMin - fDataCenter);

			if (fDataMinMinusMaxLo != 0.0F)
			{
				pTrigger->m_fRangeScaleMultiplierLo = (fRangeScaleMin - fRangeScaleCenter) / fDataMinMinusMaxLo;
			}
			else
			{
				pTrigger->m_fRangeScaleMultiplierLo = 1.0F;
			}

			pTrigger->m_fRangeScaleOffsetLo = fRangeScaleMin - (pTrigger->m_fRangeScaleMultiplierLo * pTrigger->m_DataMin);
			pTrigger->m_fRangeScalePreCenter = fDataCenter;
		}
	}

	return true;
}


// --------------------------------------------------------------------- //
// Device Binding Retrieval.
// --------------------------------------------------------------------- //

void input_FreeDeviceBindings(DeviceBinding* pBindings)
{
	DeviceBinding* pBinding = pBindings;

	while (pBinding)
	{
		GameAction* pAction = pBinding->pActionHead;

		while (pAction)
		{
			GameAction* pNext = pAction->pNext;
			delete pAction;
			pAction = pNext;
		}

		DeviceBinding* pNext = pBinding->pNext;
		delete pBinding;
		pBinding = pNext;
	}
}

DeviceBinding* input_GetDeviceBindings(uint32 nDevice)
{
	DeviceDef* pDevice;
	DeviceBinding* pBindingsHead = nullptr;
	TriggerObject* pTrigger;
	GPOS triggerPos;


	pDevice = nullptr;
	pTrigger = nullptr;

	// get the device they are looking for
	if (nDevice == DEVICETYPE_KEYBOARD)
	{
		pDevice = input_FindDeviceByType(SdlInputDeviceType::keyboard);
	}
	else if (nDevice & DEVICETYPE_MOUSE)
	{
		pDevice = input_FindDeviceByType(SdlInputDeviceType::mouse);
	}
	else if (nDevice & DEVICETYPE_GAMEPAD)
	{
		pDevice = input_FindDeviceByType(SdlInputDeviceType::gamepad);
	}

	if (!pDevice)
	{
		return nullptr;
	}

	// go through each trigger, building a list of DeviceBindings to return
	for (triggerPos = pDevice->m_Triggers; triggerPos; )
	{
		pTrigger = pDevice->m_Triggers.GetNext(triggerPos);

		DeviceBinding* pBinding;

		LT_MEM_TRACK_ALLOC(pBinding = new DeviceBinding, LT_MEM_TYPE_INPUT);

		if (!pBinding)
		{
			input_FreeDeviceBindings(pBindingsHead);
			return nullptr;
		}

		memset(pBinding, 0, sizeof(DeviceBinding));

		SAFE_STRCPY(pBinding->strDeviceName, pDevice->m_InstanceName);
		SAFE_STRCPY(pBinding->strTriggerName, pTrigger->m_TriggerName);
		SAFE_STRCPY(pBinding->strRealName, pTrigger->m_RealName);
		pBinding->m_nObjectId = pTrigger->m_diType;

		// store all the scale information for the trigger
		pBinding->nScale = pTrigger->m_Scale;
		pBinding->nRangeScaleMin = pTrigger->m_fRangeScaleMin;
		pBinding->nRangeScaleMax = pTrigger->m_fRangeScaleMax;
		pBinding->nRangeScalePreCenterOffset = pTrigger->m_fRangeScalePreCenterOffset;

		// go through the actions, adding them to the trigger
		GameAction* pActionHead = nullptr;
		TriggerAction* pTriggerAction = pTrigger->m_pActionHead;

		while (pTriggerAction)
		{
			if (pTriggerAction->m_pAction)
			{
				GameAction* pNewAction;
				LT_MEM_TRACK_ALLOC(pNewAction = new GameAction, LT_MEM_TYPE_INPUT);

				if (!pNewAction)
				{
					input_FreeDeviceBindings(pBindingsHead);
					return nullptr;
				}

				memset(pNewAction, 0, sizeof(GameAction));

				pNewAction->nActionCode = pTriggerAction->m_pAction->m_ActionCode;
				SAFE_STRCPY(pNewAction->strActionName, pTriggerAction->m_pAction->m_ActionName);
				pNewAction->nRangeLow = pTriggerAction->m_RangeLow;
				pNewAction->nRangeHigh = pTriggerAction->m_RangeHigh;
				pNewAction->pNext = pActionHead;
				pActionHead = pNewAction;
			}

			pTriggerAction = pTriggerAction->m_pNext;
		}

		if (pTriggerAction)
		{
			input_FreeDeviceBindings(pBindingsHead);
			return nullptr;
		}

		pBinding->pActionHead = pActionHead;

		pBinding->pNext = pBindingsHead;
		pBindingsHead = pBinding;
	}

	return pBindingsHead;
}


// --------------------------------------------------------------------- //
// Device Tracking.
// --------------------------------------------------------------------- //

bool input_StartDeviceTrack(InputMgr* pMgr, uint32 nDevices, uint32 nBufferSize)
{
	if (nBufferSize > MAX_INPUT_BUFFER_SIZE)
	{
		return false;
	}

	// this will track the first device found of the requested type

	int i = 0;

	for (i = 0; i < 5; i++)
	{
		DeviceDef* pDevice = nullptr;
		int nObjects = 0;
		TrackObjectInfo* pInfo = nullptr;
		SdlInputDeviceObjectDataFormat* pObjectDataFormats = nullptr;
		SdlInputDeviceDataFormat deviceDataFormat;

		// check for each possible type of device
		if (i == 0)
		{
			if (nDevices & DEVICETYPE_KEYBOARD)
			{
				pDevice = input_FindDeviceByType(SdlInputDeviceType::keyboard);
			}
			else
			{
				continue;
			}
		}
		else if (i == 1)
		{
			if (nDevices & DEVICETYPE_MOUSE)
			{
				pDevice = input_FindDeviceByType(SdlInputDeviceType::mouse);
			}
			else
			{
				continue;
			}
		}
		else if (i == 2)
		{
			// Joystick.
			continue;
		}
		else if (i == 3)
		{
			if (nDevices & DEVICETYPE_GAMEPAD)
			{
				pDevice = input_FindDeviceByType(SdlInputDeviceType::gamepad);
			}
			else
			{
				continue;
			}
		}
		else if (i == 4)
		{
			// Unknown device.
			continue;
		}

		if (!pDevice)
		{
			continue;
		}

		// we now have a device - make sure it's been created
		if (!pDevice->IsEnabled())
		{
			input_EnableDevice(pMgr, pDevice->m_InstanceName);

			if (!pDevice->IsEnabled())
			{
				break;
			}

			pDevice->m_bTrackingOnly = true;
		}

		nObjects = pDevice->m_pDevice->get_object_count();

		// now enumerate through all objects on the device
		LT_MEM_TRACK_ALLOC(pDevice->m_pTrackObjects = new TrackObjectInfo[nObjects], LT_MEM_TYPE_INPUT);
		if (!pDevice->m_pTrackObjects) break;

		pInfo = pDevice->m_pTrackObjects;

		for (auto i_object = 0; i_object < nObjects; ++i_object)
		{
			const auto& object = pDevice->m_pDevice->get_object(i_object);

			auto ppInfo = &pInfo;
			(*ppInfo)->guidType = object.type;
			(*ppInfo)->dwType = object.id;
			LTStrCpy((*ppInfo)->tszName, object.name.c_str(), INPUTNAME_LEN);

			(*ppInfo) += 1;
		}

		// set up the object formats for each object
		LT_MEM_TRACK_ALLOC(pObjectDataFormats = new SdlInputDeviceObjectDataFormat[nObjects], LT_MEM_TYPE_INPUT);

		if (!pObjectDataFormats)
		{
			delete[] pDevice->m_pTrackObjects;
			pDevice->m_pTrackObjects = nullptr;
			break;
		}

		for (auto i2 = 0; i2 < nObjects; ++i2)
		{
			pObjectDataFormats[i2].offset = i2 << 2;
			pObjectDataFormats[i2].object_id = pDevice->m_pTrackObjects[i2].dwType;
		}

		// set the format for the device
		deviceDataFormat.state_size = nObjects << 2;
		deviceDataFormat.object_count = nObjects;
		deviceDataFormat.object_data_formats = pObjectDataFormats;

		pDevice->m_pDevice->set_data_format(deviceDataFormat);

		delete[] pObjectDataFormats;

		// set the tracking flag
		pDevice->m_bTracking = true;
		pDevice->m_nTrackObjects = nObjects;
	}

	if (i < 4)
	{
		return false;
	}

	return true;
}

bool input_TrackDevice(DeviceInput* pInputArray, uint32* pnInOut)
{
	SdlInputDeviceObjectData data[MAX_INPUT_BUFFER_SIZE];
	int nEvents;
	int nArraySize;

	nArraySize = *pnInOut;
	*pnInOut = 0;

	auto pDevice = g_pDeviceHead;

	while (pDevice)
	{
		if (pDevice->m_bTracking && pDevice->m_pTrackObjects)
		{
			std::fill(std::begin(data), std::end(data), SdlInputDeviceObjectData{});

			nEvents = pDevice->m_pDevice->get_data(data, MAX_INPUT_BUFFER_SIZE);

			// go through events and add them to input array
			for (auto i = 0; i < nEvents; ++i)
			{
				if (*pnInOut == static_cast<uint32>(nArraySize))
				{
					return true;
				}

				switch (pDevice->m_DeviceType)
				{
					case SdlInputDeviceType::keyboard:
						pInputArray[*pnInOut].m_DeviceType = DEVICETYPE_KEYBOARD;
						break;

					case SdlInputDeviceType::mouse:
						pInputArray[*pnInOut].m_DeviceType = DEVICETYPE_MOUSE;
						break;

					case SdlInputDeviceType::gamepad:
						pInputArray[*pnInOut].m_DeviceType = DEVICETYPE_GAMEPAD;
						break;

					default:
						pInputArray[*pnInOut].m_DeviceType = DEVICETYPE_UNKNOWN;
						break;
				}

				LTStrCpy(
					pInputArray[*pnInOut].m_DeviceName,
					pDevice->m_InstanceName,
					sizeof(pInputArray[*pnInOut].m_DeviceName)
				);

				auto nOffset = data[i].offset >> 2;

				if (nOffset < pDevice->m_nTrackObjects)
				{
					if (pDevice->m_pTrackObjects[nOffset].guidType == SdlInputDeviceObjectType::x_axis)
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_XAXIS;
					}
					else if (pDevice->m_pTrackObjects[nOffset].guidType == SdlInputDeviceObjectType::y_axis)
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_YAXIS;
					}
					else if (pDevice->m_pTrackObjects[nOffset].guidType == SdlInputDeviceObjectType::z_axis)
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_ZAXIS;
					}
					else if (pDevice->m_pTrackObjects[nOffset].guidType == SdlInputDeviceObjectType::rx_axis)
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_RXAXIS;
					}
					else if (pDevice->m_pTrackObjects[nOffset].guidType == SdlInputDeviceObjectType::ry_axis)
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_RYAXIS;
					}
					else if (pDevice->m_pTrackObjects[nOffset].guidType == SdlInputDeviceObjectType::rz_axis)
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_RZAXIS;
					}
					else if (pDevice->m_pTrackObjects[nOffset].guidType == SdlInputDeviceObjectType::push_button)
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_BUTTON;
					}
					else if (pDevice->m_pTrackObjects[nOffset].guidType == SdlInputDeviceObjectType::key)
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_KEY;
					}
					else if (pDevice->m_pTrackObjects[nOffset].guidType == SdlInputDeviceObjectType::pov)
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_POV;
					}
					else
					{
						pInputArray[*pnInOut].m_ControlType = CONTROLTYPE_UNKNOWN;
					}

					LTStrCpy(
						pInputArray[*pnInOut].m_ControlName,
						pDevice->m_pTrackObjects[nOffset].tszName,
						sizeof(pInputArray[*pnInOut].m_ControlName)
					);

					const auto objInstance = static_cast<uint16>(pDevice->m_pTrackObjects[nOffset].dwType);

					pInputArray[*pnInOut].m_ControlCode = objInstance;
					pInputArray[*pnInOut].m_nObjectId = pDevice->m_pTrackObjects[nOffset].dwType;
					pInputArray[*pnInOut].m_InputValue = static_cast<uint32>(data[i].value);

					(*pnInOut)++;
				}
			}
		}

		pDevice = pDevice->m_pNext;
	}

	return true;
}

bool input_EndDeviceTrack()
{
	auto pDef = g_pDeviceHead;

	while (pDef)
	{
		if (pDef->IsEnabled())
		{
			pDef->m_bTracking = false;

			// disable any device enabled specifically for device tracking
			if (pDef->m_bTrackingOnly)
			{
				if (pDef->m_pDevice)
				{
					g_pDirectInput->remove_device(pDef->m_pDevice);
					pDef->m_pDevice = nullptr;
				}

				// reset the tracking only flag
				pDef->m_bTrackingOnly = false;

				// remove any tracking object structures
				if (pDef->m_pTrackObjects)
				{
					delete[] pDef->m_pTrackObjects;
					pDef->m_pTrackObjects = nullptr;
				}
			}
			else
			{
				// re-setup the device
				input_SetupDeviceFormats(pDef);
			}
		}

		pDef = pDef->m_pNext;
	}

	return true;
}

int input_get_device_object_calculate_total_object_count(
	::uint32 device_flags)
{
	if (device_flags == 0)
	{
		return 0;
	}

	auto total_object_count = 0;

	for (auto device_def = ::g_pDeviceHead; device_def; device_def = device_def->m_pNext)
	{
		auto api_device_type = 0;

		switch (device_def->m_DeviceType)
		{
			case SdlInputDeviceType::keyboard:
				api_device_type = ::DEVICETYPE_KEYBOARD;
				break;

			case SdlInputDeviceType::mouse:
				api_device_type = ::DEVICETYPE_MOUSE;
				break;

			case SdlInputDeviceType::gamepad:
				api_device_type = ::DEVICETYPE_GAMEPAD;
				break;

			default:
				break;
		}

		if ((device_flags & api_device_type) == 0)
		{
			continue;
		}

		if (!device_def->IsEnabled())
		{
			::InputMgr* pMgr;
			input_GetManager(&pMgr);
			input_EnableDevice(pMgr, device_def->m_InstanceName);
		}

		if (!device_def->IsEnabled())
		{
			continue;
		}

		const auto object_count = device_def->m_pDevice->get_object_count();
		total_object_count += object_count;
	}

	return total_object_count;
}

::DeviceObject* input_GetDeviceObjects(
	::uint32 nDeviceFlags)
{
	const auto total_object_count = input_get_device_object_calculate_total_object_count(nDeviceFlags);

	if (total_object_count <= 0)
	{
		return nullptr;
	}

	auto api_device_object_index = 0;
	auto api_device_objects = std::make_unique<::DeviceObject[]>(total_object_count);
	auto next_api_device_object = api_device_objects.get() + 1;

	for (auto device_def = ::g_pDeviceHead; device_def; device_def = device_def->m_pNext)
	{
		if (!device_def->IsEnabled())
		{
			continue;
		}

		auto api_device_type = 0;

		switch (device_def->m_DeviceType)
		{
			case SdlInputDeviceType::keyboard:
				api_device_type = ::DEVICETYPE_KEYBOARD;
				break;

			case SdlInputDeviceType::mouse:
				api_device_type = ::DEVICETYPE_MOUSE;
				break;

			case SdlInputDeviceType::gamepad:
				api_device_type = ::DEVICETYPE_GAMEPAD;
				break;

			default:
				break;
		}

		if ((nDeviceFlags & api_device_type) == 0)
		{
			continue;
		}

		const auto device = device_def->m_pDevice;
		const auto object_count = device->get_object_count();

		for (auto i_object = 0; i_object < object_count; ++i_object)
		{
			const auto& object = device->get_object(i_object);
			auto& api_object = api_device_objects[api_device_object_index];

			// Device type.
			//
			api_object.m_DeviceType = api_device_type;

			// Device name.
			//
			::strncpy_s(
				api_object.m_DeviceName,
				std::extent<decltype(::DeviceObject::m_DeviceName)>::value,
				device_def->m_InstanceName,
				std::extent<decltype(DeviceDef::m_InstanceName)>::value
			);

			// Object type.
			//
			auto api_object_type = ::CONTROLTYPE_UNKNOWN;

			if (object.type == SdlInputDeviceObjectType::x_axis)
			{
				api_object_type = ::CONTROLTYPE_XAXIS;
			}
			else if (object.type == SdlInputDeviceObjectType::y_axis)
			{
				api_object_type = ::CONTROLTYPE_YAXIS;
			}
			else if (object.type == SdlInputDeviceObjectType::z_axis)
			{
				api_object_type = ::CONTROLTYPE_ZAXIS;
			}
			else if (object.type == SdlInputDeviceObjectType::rx_axis)
			{
				api_object_type = ::CONTROLTYPE_RXAXIS;
			}
			else if (object.type == SdlInputDeviceObjectType::ry_axis)
			{
				api_object_type = ::CONTROLTYPE_RYAXIS;
			}
			else if (object.type == SdlInputDeviceObjectType::rz_axis)
			{
				api_object_type = ::CONTROLTYPE_RZAXIS;
			}
			else if (object.type == SdlInputDeviceObjectType::push_button)
			{
				api_object_type = ::CONTROLTYPE_BUTTON;
			}
			else if (object.type == SdlInputDeviceObjectType::key)
			{
				api_object_type = ::CONTROLTYPE_KEY;
			}
			else if (object.type == SdlInputDeviceObjectType::pov)
			{
				api_object_type = ::CONTROLTYPE_POV;
			}

			api_object.m_ObjectType = api_object_type;

			// Object name.
			//
			::strncpy_s(
				api_object.m_ObjectName,
				std::extent<decltype(::DeviceObject::m_ObjectName)>::value,
				object.name.c_str(),
				object.name.size()
			);

			// Object id.
			//
			api_object.m_nObjectId = object.id;

			// Object range.
			//
			if (object.type == SdlInputDeviceObjectType::x_axis ||
				object.type == SdlInputDeviceObjectType::y_axis ||
				object.type == SdlInputDeviceObjectType::z_axis ||
				object.type == SdlInputDeviceObjectType::rx_axis ||
				object.type == SdlInputDeviceObjectType::ry_axis ||
				object.type == SdlInputDeviceObjectType::rz_axis ||
				object.type == SdlInputDeviceObjectType::pov)
			{
				api_object.m_RangeLow = static_cast<float>(SdlInput::min_axis_value);
				api_object.m_RangeHigh = static_cast<float>(SdlInput::max_axis_value);
			}
			else
			{
				api_object.m_RangeLow = 0.0F;
				api_object.m_RangeHigh = 0.0F;
			}

			// Next element.
			//

			api_object.m_pNext = next_api_device_object;

			next_api_device_object += 1;
			api_device_object_index += 1;
		}
	}

	api_device_objects[api_device_object_index - 1].m_pNext = nullptr;

	return api_device_objects.release();
}

void input_FreeDeviceObjects(
	DeviceObject* pObjectList)
{
	delete[] pObjectList;
}

// --------------------------------------------------------------------- //
// Device Helper Functions.
// --------------------------------------------------------------------- //

bool input_GetDeviceName(uint32 nDeviceType, char* pStrBuffer, uint32 nBufferSize)
{
	if (!pStrBuffer)
	{
		return false;
	}

	DeviceDef* pDevice = nullptr;

	// get the device they are looking for
	if (nDeviceType == DEVICETYPE_KEYBOARD)
	{
		pDevice = input_FindDeviceByType(SdlInputDeviceType::keyboard);
	}
	else if (nDeviceType & DEVICETYPE_MOUSE)
	{
		pDevice = input_FindDeviceByType(SdlInputDeviceType::mouse);
	}
	else if (nDeviceType & DEVICETYPE_GAMEPAD)
	{
		pDevice = input_FindDeviceByType(SdlInputDeviceType::gamepad);
	}

	if (!pDevice)
	{
		return false;
	}

	LTStrCpy(pStrBuffer, pDevice->m_InstanceName, nBufferSize);

	return true;
}

bool input_IsDeviceEnabled(const char* pDeviceName)
{
	DeviceDef* pDev = input_FindDeviceByName(pDeviceName);

	if (!pDev)
	{
		return false;
	}

	return pDev->IsEnabled();
}

bool input_GetDeviceObjectName(char const* pszDeviceName, uint32 nDeviceObjectId,
	char* pszDeviceObjectName, uint32 nDeviceObjectNameLen)
{
	auto pDev = input_FindDeviceByName(pszDeviceName);

	if (!pDev)
	{
		return false;
	}

	// Get the name using the objectid.
	auto object = pDev->m_pDevice->find_object_by_id(static_cast<int>(nDeviceObjectId));

	if (!object)
	{
		return false;
	}

	strncpy(pszDeviceObjectName, object->name.c_str(), nDeviceObjectNameLen);

	return true;
}

// --------------------------------------------------------------------- //
// Print out the available controls for a device
// --------------------------------------------------------------------- //

bool input_ShowDeviceObjects(const char* sDeviceName)
{
	return false;
}


// --------------------------------------------------------------------- //
// Print out the available input devices to the console
// --------------------------------------------------------------------- //

bool input_ShowInputDevices()
{
	return false;
}


} // namespace


// Input managers.
InputMgr g_MainInputMgr =
{
	input_Init,
	input_Term,
	input_IsInitted,
	input_ListDevices,
	input_PlayJoystickEffect,
	input_ReadInput,
	input_FlushInputBuffers,
	input_ClearInput,
	input_AddAction,
	input_EnableDevice,
	input_ClearBindings,
	input_AddBinding,
	input_ScaleTrigger,
	input_GetDeviceBindings,
	input_FreeDeviceBindings,
	input_StartDeviceTrack,
	input_TrackDevice,
	input_EndDeviceTrack,
	input_GetDeviceObjects,
	input_FreeDeviceObjects,
	input_GetDeviceName,
	input_GetDeviceObjectName,
	input_IsDeviceEnabled,
	input_ShowDeviceObjects,
	input_ShowInputDevices
};

LTRESULT input_GetManager(InputMgr** pMgr)
{
	*pMgr = &g_MainInputMgr;

	return LT_OK;
}

// --------------------------------------------------------------------- //
// Saves the state of all the input bindings.
// --------------------------------------------------------------------- //

void input_SaveBindings(FILE* fp)
{
	DeviceDef* pDevice;
	TriggerObject* pTrigger;
	TriggerAction* pAction;
	ActionDef* pCurActionDef;
	char str[1024];
	char str2[1024];
	GPOS triggerPos;


	// Save all action defs.
	pCurActionDef = g_ActionDefHead.m_pNext;

	while (pCurActionDef != &g_ActionDefHead)
	{
		fprintf(fp, "AddAction %s %d\n", pCurActionDef->m_ActionName, pCurActionDef->m_ActionCode);
		pCurActionDef = pCurActionDef->m_pNext;
	}

	fprintf(fp, "\n");

	// Save all the active devices.
	pDevice = g_pDeviceHead;

	while (pDevice)
	{
		if (pDevice->IsEnabled() && pDevice->m_Triggers.GetSize() > 0)
		{
			const auto pDeviceName = pDevice->m_pSpecialName ? pDevice->m_pSpecialName : pDevice->m_InstanceName;
			fprintf(fp, "enabledevice \"%s\"\n", pDeviceName);

			// Save all the triggers.
			for (triggerPos = pDevice->m_Triggers; triggerPos; )
			{
				pTrigger = pDevice->m_Triggers.GetNext(triggerPos);

				LTSNPrintF(str, sizeof(str), "rangebind \"%s\" \"%s\" ", pDeviceName, pTrigger->m_RealName);

				pAction = pTrigger->m_pActionHead;

				while (pAction)
				{
					if (pAction->m_pAction)
					{
						LTSNPrintF(
							str2,
							sizeof(str2),
							"%f %f \"%s\" ",
							pAction->m_RangeLow,
							pAction->m_RangeHigh,
							pAction->m_pAction->m_ActionName
						);
					}
					else if (pAction->m_pConsoleString)
					{
						LTSNPrintF(
							str2,
							sizeof(str2),
							"%f %f \"*%s\" ",
							pAction->m_RangeLow,
							pAction->m_RangeHigh,
							pAction->m_pConsoleString
						);
					}
					else
					{
						str2[0] = 0;
					}

					LTStrCat(str, str2, sizeof(str));
					pAction = pAction->m_pNext;
				}

				LTStrCat(str, "\n", sizeof(str));
				fprintf(fp, str);

				if (pTrigger->m_fRangeScaleMin != 0.0F ||
					pTrigger->m_fRangeScaleMax != 0.0F ||
					pTrigger->m_fRangeScalePreCenterOffset != 0.0F)
				{
					fprintf(
						fp,
						"rangescale \"%s\" \"%s\" %f %f %f %f\n",
						pDeviceName,
						pTrigger->m_RealName,
						pTrigger->m_Scale,
						pTrigger->m_fRangeScaleMin,
						pTrigger->m_fRangeScaleMax,
						pTrigger->m_fRangeScalePreCenterOffset
					);
				}
				else if (pTrigger->m_Scale != 1.0F)
				{
					fprintf(fp, "scale \"%s\" \"%s\" %f\n", pDeviceName, pTrigger->m_RealName, pTrigger->m_Scale);
				}
			}
		}

		pDevice = pDevice->m_pNext;
	}
}


#endif // LTJS_SDL_BACKEND
