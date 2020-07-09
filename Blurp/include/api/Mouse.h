#pragma once
#include <cinttypes>
#include <glm/glm.hpp>

namespace blurp
{
	/*
	 * Mouse class which keeps track of where the cursor is.
	 * The cursor position can be set and moved.
	 */
	class Mouse
	{
	public:
		/*
		 * Init the mouse with the given screen bounds.
		 */
		Mouse(std::uint32_t a_StartX, std::uint32_t a_StartY);

		Mouse(const glm::vec2& a_Position);

		/*
		 * Default 0,0 position.
		 */
		Mouse();

		/*
		 * Get the current position of the mouse.
		 */
		glm::vec2 GetPosition() const;

		/*
		 * Set the position of the mouse.
		 */
		void SetPosition(const glm::vec2& a_Position);

		/*
		 * Add to the position of the mouse.
		 */
		void AddPosition(const glm::vec2& a_Added);

	private:
		glm::vec2 m_Position;
	};
}