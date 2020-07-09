#include "Mouse.h"
#include <cmath>

namespace blurp
{
	Mouse::Mouse(std::uint32_t a_StartX, std::uint32_t a_StartY) :m_Position(a_StartX, a_StartY)
	{

	}

    Mouse::Mouse(const glm::vec2& a_Position): m_Position(a_Position) {

    }

    Mouse::Mouse() : m_Position(0, 0)
    {

    }

    glm::vec2 Mouse::GetPosition() const
	{
		return m_Position;
	}

	void Mouse::SetPosition(const glm::vec2& a_Position)
	{
		this->m_Position = a_Position;
	}

	void Mouse::AddPosition(const glm::vec2& a_Added)
	{
		this->m_Position = m_Position + a_Added;
	}

}