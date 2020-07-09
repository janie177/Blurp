#include "InputQueue.h"

#include <cassert>

namespace blurp
{
	InputData::InputData() : keyStates(), mouseStates()
	{
		//Init keys to not pressed.
		for (auto& keyState : keyStates)
		{
			keyState = ButtonState::NOT_PRESSED;
		}

		for (auto& buttonState : mouseStates)
		{
			buttonState = ButtonState::NOT_PRESSED;
		}
	}

	InputData InputData::takeData()
	{
		//Create new data object.
		InputData data;

		//Swap contents of the queues.
		data.mouseEvents.swap(mouseEvents);
		data.keyboardEvents.swap(keyboardEvents);

		//Copy the key events and reset any that were marked as PRESSED_RELEASED because they are no longer pressed.
		for (auto i = 0; i < 512; i++)
		{
			const ButtonState state = keyStates[i];
			data.keyStates[i] = state;

			//If the key was briefly pressed and then released, set it as unpressed again.
			if (state == ButtonState::PRESSED_RELEASED)
			{
				keyStates[i] = ButtonState::NOT_PRESSED;
			}

			//First pressed keys now become held down.
			if (state == ButtonState::FIRST_PRESSED)
			{
				keyStates[i] = ButtonState::HELD_DOWN;
			}
		}

		//Copy the mouse states over.
		for (auto i = 0; i < 3; i++)
		{
			const ButtonState state = mouseStates[i];
			data.mouseStates[i] = state;

			if (state == ButtonState::PRESSED_RELEASED)
			{
				mouseStates[i] = ButtonState::NOT_PRESSED;
			}
		}

		//Move the data object.
		return data;
	}

	bool InputData::getNextEvent(KeyboardEvent& keyboardEvent)
	{
		const bool hasData = !keyboardEvents.empty();
		if (hasData)
		{
			keyboardEvent = keyboardEvents.front();
			keyboardEvents.pop();
		}
		return hasData;
	}

	bool InputData::getNextEvent(MouseEvent& mouseEvent)
	{
		const bool hasData = !mouseEvents.empty();
		if (hasData)
		{
			mouseEvent = mouseEvents.front();
			mouseEvents.pop();
		}

		return hasData;
	}

	void InputData::addMouseEvent(const MouseEvent& event)
	{
		//Mouse buttons can be held down between queries.
		//This keeps track of that state and only changes it once it has been released.
		if (event.action == MouseAction::CLICK)
		{
			if (mouseStates[static_cast<uint16_t>(event.button)] != ButtonState::HELD_DOWN)
			{
				mouseStates[static_cast<uint16_t>(event.button)] = ButtonState::FIRST_PRESSED;
			}
		}
		else if (event.action == MouseAction::RELEASE)
		{
			mouseStates[static_cast<uint16_t>(event.button)] = ButtonState::PRESSED_RELEASED;
		}

		mouseEvents.push(event);
	}

	void InputData::addKeyboardEvent(const KeyboardEvent& event)
	{
		//Invalid or unknown keys are skipped.
		if (event.keyCode < 0 || event.keyCode > 511)
		{
			return;
		}

		//Keys can be held down for several ticks.
		//This keeps track of state until a button was released.
		//Only then will the key state change.
		if (event.action == KeyboardAction::KEY_PRESSED)
		{
			if (keyStates[event.keyCode] != ButtonState::HELD_DOWN)
			{
				keyStates[event.keyCode] = ButtonState::FIRST_PRESSED;
			}
		}
		else if (event.action == KeyboardAction::KEY_RELEASED)
		{
			keyStates[event.keyCode] = ButtonState::PRESSED_RELEASED;
		}

		keyboardEvents.push(event);
	}

	ButtonState InputData::getMouseButtonState(MouseButton button) const
	{
		return mouseStates[static_cast<std::uint8_t>(button)];
	}

	void InputData::setKeyState(std::uint16_t keyCode, ButtonState state)
	{
		assert(keyCode < 512);
		keyStates[keyCode] = state;
	}

	void InputData::setMouseButtonState(MouseButton button, ButtonState state)
	{
		assert(static_cast<unsigned char>(button) < 3);
		mouseStates[static_cast<unsigned char>(button)] = state;
	}

	ButtonState InputData::getKeyState(std::uint16_t keyCode) const
	{
		assert(keyCode < 512);
		return keyStates[keyCode];
	}

	InputData InputQueue::getQueuedEvents()
	{
		//Make sure no input can be added for now.
		mutex.lock();

		//Take the data 
		InputData copy = data.takeData();

		//Allow new input.
		mutex.unlock();

		//Move the new data object that now contains the data.
		return copy;
	}

	void InputQueue::addMouseEvent(const MouseEvent& event)
	{
		mutex.lock();
		data.addMouseEvent(event);
		mutex.unlock();
	}

	void InputQueue::addKeyboardEvent(const KeyboardEvent& event)
	{
		mutex.lock();
		data.addKeyboardEvent(event);
		mutex.unlock();
	}

	void InputQueue::setKeyState(std::uint16_t keyCode, ButtonState state)
	{
		mutex.lock();
		data.setKeyState(keyCode, state);
		mutex.unlock();
	}

	ButtonState InputQueue::getKeyState(std::uint16_t keyCode) const
	{
		return data.getKeyState(keyCode);
	}

	void InputQueue::setMouseState(MouseButton button, ButtonState state)
	{
		mutex.lock();
		data.setMouseButtonState(button, state);
		mutex.unlock();
	}

	ButtonState InputQueue::getMouseState(MouseButton button) const
	{
		return data.getMouseButtonState(button);
	}
}