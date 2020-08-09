#pragma once
#include <Window.h>

namespace blurp {
    class BlurpEngine;
}

class Scene
{
public:
    virtual ~Scene() = default;
    Scene(blurp::BlurpEngine& a_Engine, std::shared_ptr<blurp::Window> a_Window) : m_Engine(a_Engine), m_Window(std::move(a_Window)) {}

	/*
	 * Initialize the scene.
	 */
	virtual void Init() = 0;

	/*
	 * Update the scene.
	 */
	virtual void Update() = 0;

protected:
	blurp::BlurpEngine& m_Engine;
	std::shared_ptr<blurp::Window> m_Window;
};
