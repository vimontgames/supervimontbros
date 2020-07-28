#pragma once

static const uint maxButtonCount = 8;
static const float joyDeadZone = 16.0f;

using ControllerID = u8;

struct JoystickConfig
{
	uint button[maxButtonCount];
};

struct KeyboardConfig
{
	sf::Keyboard::Key top, left, down, right;
	sf::Keyboard::Key button[maxButtonCount];
};

enum class JoystickType
{
	Unknown = 0,
	Playstation,
	XBox,

	Count
};

struct ControllerState
{
	ControllerState() : x(0.0f), y(0.0f)
	{
		for (uint b = 0; b < maxButtonCount; ++b)
		{
			buttonPressed[b] = false;
			buttonJustPressed[b] = false;
			buttonJustReleased[b] = false;
		}
	}

	float x, y;
	bool buttonPressed[maxButtonCount];
	bool buttonJustPressed[maxButtonCount];
	bool buttonJustReleased[maxButtonCount];
};

class Controller
{
private:
	enum class ControllerType
	{
		Joystick = 0,
		Keyboard,

		Count
	};

public:
	Controller(uint _globalIndex, const JoystickConfig & _config, ControllerID _ID);
	Controller(uint _globalIndex, const KeyboardConfig & _config, ControllerID _ID);

	float getXAxis() const;
	float getYAxis() const;
	sf::Vector2f getXYAxis() const;

	bool isButtonPressed(uint _index) const;
	bool isButtonJustPressed(uint _index) const;
	bool isButtonJustReleased(uint _index) const;

	static uint init();
	static uint deinit();
	static void update();
	static JoystickType getJoystickType(const sf::Joystick::Identification & _info);
	static const JoystickConfig & getJoystickConfig(JoystickType _joystickType);
	static uint getJoystickConfigCount();
	static const KeyboardConfig & getKeyboardConfig(uint _keyboardID);
	static uint getKeyboardConfigCount();
	static uint getControllerCount();
	static Controller & getController(uint _index);
	static uint getButtonCount();

private:
	ControllerID m_controllerID;
	ControllerState m_state;
	ControllerType m_type;
	union
	{
		JoystickConfig joystick;
		KeyboardConfig keyboard;
	} m_config;
	u8 m_index;

	static std::vector<Controller> s_controllers;
};

#ifdef ENABLE_INL
#include "Controller.inl"
#endif