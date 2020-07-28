#include "Precomp.h"
#include "Controller/Controller.h"

#ifndef ENABLE_INL
#include "Controller.inl"
#endif

using namespace sf;

std::vector<Controller> Controller::s_controllers;

//--------------------------------------------------------------------------
static KeyboardConfig g_keyboardConfig[] =
{
	{
		Keyboard::Key::Z,			// Up
		Keyboard::Key::Q,			// Down
		Keyboard::Key::S,			// Left
		Keyboard::Key::D,			// Right

		Keyboard::Key::LControl,	// Shit
		Keyboard::Key::LShift,		// Run
		Keyboard::Key::Space,		// Kick
		Keyboard::Key::E			// Punch
	},
	{
		Keyboard::Key::Up,			// Up
		Keyboard::Key::Left,		// Down
		Keyboard::Key::Down,		// Left
		Keyboard::Key::Right,		// Right

		Keyboard::Key::RControl,	// Shit
		Keyboard::Key::RShift,		// Run
		Keyboard::Key::Numpad0,		// Kick
		Keyboard::Key::Numpad1		// Punch
	}
};

//--------------------------------------------------------------------------
uint Controller::getKeyboardConfigCount()
{
	return COUNT_OF(g_keyboardConfig);
}

//--------------------------------------------------------------------------
const KeyboardConfig & Controller::getKeyboardConfig(uint _keyboardID)
{
	assert(_keyboardID < getKeyboardConfigCount());
	return g_keyboardConfig[_keyboardID];
}

//--------------------------------------------------------------------------
static JoystickConfig g_joystickConfig[] =
{
	// PS4
	// 0 3
	// 1 2
	{ 0, 1, 2, 3, 4, 5, 6, 7 },

	// XBox
	// 2 3
	// 0 1
	{ 2, 0, 1, 3, 4, 5, 6, 7 }
};

//--------------------------------------------------------------------------
uint Controller::getJoystickConfigCount()
{
	return COUNT_OF(g_joystickConfig);
}

//--------------------------------------------------------------------------
const JoystickConfig & Controller::getJoystickConfig(JoystickType _joystickType)
{
	assert((uint)_joystickType < (uint)JoystickType::Count);

	switch (_joystickType)
	{
		case JoystickType::Playstation:
			return g_joystickConfig[0];

		default:
		case JoystickType::XBox:
			return g_joystickConfig[1];
	}
}

//--------------------------------------------------------------------------
JoystickType Controller::getJoystickType(const sf::Joystick::Identification & _info)
{
	switch (_info.vendorId)
	{
		case 0x054C:
			return JoystickType::Playstation;

		case 0x045E:
			return JoystickType::XBox;

		default:
			return JoystickType::Unknown;
	}
}

//--------------------------------------------------------------------------
uint Controller::init()
{
	static uint maxControllers = 4;

	s_controllers.clear();

	Joystick::update();

	uint index = 0;
	uint joystickCount = 0;
	for (uint j = 0; j < Joystick::Count; ++j)
	{
		if (Joystick::isConnected(j))
		{
			const Joystick::Identification info = Joystick::getIdentification(j);

			//debugPrint("Joystick %u : %s 0x%08x 0%08x\n", j, info.name.toAnsiString().c_str(), info.productId, info.vendorId);

			auto joystickType = getJoystickType(info);
			auto & config = getJoystickConfig(joystickType);
			s_controllers.push_back(Controller(index, config, j));

			++joystickCount;
			++index;
		}
	}

	uint keyboardCount = 0;
	for (uint k = 0; k < getKeyboardConfigCount(); ++k)
	{
		if (index < maxControllers)
		{
			auto & config = getKeyboardConfig(keyboardCount);
			s_controllers.push_back(Controller(index, config, keyboardCount));

			++keyboardCount;
			++index;
		}
	}

	debugPrint("%u controllers detected (Joystick: %u, Keyboard: %u)\n", index, joystickCount, keyboardCount);

	return index;
}

//--------------------------------------------------------------------------
void Controller::update()
{
	for (Controller & controller : s_controllers)
	{
		ControllerState & state = controller.m_state;

		bool wasPressed[maxButtonCount];
		for (uint b = 0; b < maxButtonCount; ++b)
		{
			wasPressed[b] = state.buttonPressed[b];
		}

		switch (controller.m_type)
		{
			default:
				assert(false);
				break;

			case ControllerType::Joystick:
			{
				const auto & config = controller.m_config.joystick;
				const auto id = controller.m_index;

				const uint joyButtonCount = controller.getButtonCount();

				state.x = Joystick::getAxisPosition(id, Joystick::Axis::X);
				state.y = Joystick::getAxisPosition(id, Joystick::Axis::Y);

				//debugPrint("%f %f\n", Joystick::getAxisPosition(id, Joystick::PovX), Joystick::getAxisPosition(id, Joystick::PovY));

				for (uint b = 0; b < maxButtonCount; ++b)
				{
					const bool wasPressed = state.buttonPressed[b];
					const bool pressed = Joystick::isButtonPressed(id, config.button[b]);

					if (pressed)
					{
						state.buttonPressed[b] = true;
						//debugPrint("Joystick %u button %u pressed\n", id, b);
					}
					else
					{
						state.buttonPressed[b] = false;
					}
				}
			}
			break;

			case ControllerType::Keyboard:
			{
				const auto & config = controller.m_config.keyboard;
				if (Keyboard::isKeyPressed(config.left))
				{
					state.x = -100.0f;
				}
				else if (Keyboard::isKeyPressed(config.right))
				{
					state.x = +100.0f;
				}
				else
				{
					state.x = 0.0f;
				}

				if (Keyboard::isKeyPressed(config.top))
				{
					state.y = -100.0f;
				}
				else if (Keyboard::isKeyPressed(config.down))
				{
					state.y = +100.0f;
				}
				else
				{
					state.y = 0.0f;
				}

				for (uint b = 0; b < maxButtonCount; ++b)
				{
					if (Keyboard::isKeyPressed(config.button[b]))
					{
						state.buttonPressed[b] = true;
					}
					else
					{
						state.buttonPressed[b] = false;
					}
				}
			}
			break;
		}

		const float nDir = length(Vector2f(state.x/100.0f, state.y/100.0f));
		if (nDir != 0.0f)
		{
			state.x *= nDir;
			state.y *= nDir;
		}

		for (uint b = 0; b < maxButtonCount; ++b)
		{
			if (state.buttonPressed[b])
			{
				if (wasPressed[b] == false)
				{
					state.buttonJustPressed[b] = true;
					//debugPrint("Button %u just pressed\n", b);
				}
				else
				{
					state.buttonJustPressed[b] = false;
				}
			}
			else
			{
				if (wasPressed[b])
				{
					state.buttonJustReleased[b] = true;
					//debugPrint("Button %u just released\n", b);
				}
				else
				{
					state.buttonJustReleased[b] = false;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
uint Controller::deinit()
{
	s_controllers.clear();
	return 0;
}

//--------------------------------------------------------------------------
Controller::Controller(uint _index, const JoystickConfig & _config, ControllerID _ID) :
	m_controllerID(_index),
	m_type(ControllerType::Joystick),
	m_index(_ID)
{
	m_config.joystick = _config;
}

//--------------------------------------------------------------------------
Controller::Controller(uint _index, const KeyboardConfig & _config, ControllerID _ID) :
	m_controllerID(_index),
	m_type(ControllerType::Keyboard),
	m_index(_ID)
{
	m_config.keyboard = _config;
}

//--------------------------------------------------------------------------
uint Controller::getControllerCount()
{
	return (uint)s_controllers.size();
}

//--------------------------------------------------------------------------
Controller & Controller::getController(uint _index)
{
	return s_controllers[_index];
}