#pragma once
#include <Transform.h>
#include <memory>
#include <Light.h>

class Game;

enum class EntityType
{
    SPACE_SHIP,
	ASTEROID,
	KILL_BOT,
	PLANET,
	LASER,
	LIGHT
};

class Entity
{
public:
	Entity(int a_MeshId);
	Entity();

	virtual ~Entity() = default;

	blurp::Transform& GetTransform();

	float GetAcceleration() const;

	glm::vec3 GetVelocity() const;

	glm::vec3 GetDirection() const;

	int GetAge() const;

	int GetMeshId() const;

	/*
	 * Update this entity and increase age.
	 */
	void OnUpdate(float a_DeltaTime, Game& a_Game)
	{
		++m_Age;
		Update(a_DeltaTime, a_Game);
	}

	/*
	 * If true, entity is removed after next game cycle.
	 */
	bool MarkedForDelete()
	{
		return m_MarkedForDelete;
	}

protected:

	/*
     * Update for a game tick.
     */
	virtual void Update(float a_DeltaTime, Game& a_Game) = 0;

	//Transformation.
	blurp::Transform m_Transform;

	//Id into the mesh array of the game.
	//If not used, set to -1.
	int m_MeshId;

	//Age of the entity 
	int m_Age;

	//Direction the entity is facing.
	glm::vec3 m_Direction;

	//Current velocity on each axis.
	glm::vec3 m_Velocity;

	//How fast is the entity accelerating.
	float m_Acceleration;

	//Removed next game cycle if true.
	bool m_MarkedForDelete;
};

class SpaceShip : public Entity
{
public:
	SpaceShip(int a_MeshId) : Entity(a_MeshId) {}
	SpaceShip() : Entity() {}
protected:
	void Update(float a_DeltaTime, Game& a_Game) override final;
};

class Asteroid : public Entity
{
public:
	Asteroid(int a_MeshId) : Entity(a_MeshId) {}
	Asteroid() : Entity() {}
protected:
	void Update(float a_DeltaTime, Game& a_Game) override final;
};

class KillBot : public Entity
{
public:
	KillBot(int a_MeshId) : Entity(a_MeshId) {}
	KillBot() : Entity() {}
protected:
	void Update(float a_DeltaTime, Game& a_Game) override final;
};

class Planet : public Entity
{
public:
	Planet(int a_MeshId) : Entity(a_MeshId) {}
	Planet() : Entity() {}
protected:
	void Update(float a_DeltaTime, Game& a_Game) override final;
};

class Laser : public Entity
{
public:
	Laser(int a_MeshId) : Entity(a_MeshId) {}
	Laser() : Entity() {}
protected:
	void Update(float a_DeltaTime, Game& a_Game) override final;
};

class Light : public Entity
{
public:
	Light() : Entity() {}

	void SetLight(std::shared_ptr<blurp::PointLight>& a_Light);

	std::shared_ptr<blurp::Light> GetLight() const;

protected:
	void Update(float a_DeltaTime, Game& a_Game) override final;

private:
	std::shared_ptr<blurp::PointLight> m_Light;
};