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
	Asteroid(int a_MeshId) : Entity(a_MeshId), m_Rotate(false), m_RotationX(0.f), m_RotationY(0.f), m_RotationZ(0.f), m_RotationSpeeds(0.f) {}
	Asteroid() : Entity(), m_Rotate(false), m_RotationX(0.f), m_RotationY(0.f), m_RotationZ(0.f), m_RotationSpeeds(0.f) {}

	void SetRotation(const glm::vec3& a_X, const glm::vec3& a_Y, const glm::vec3& a_Z, const glm::vec3& a_Speeds);

protected:
	void Update(float a_DeltaTime, Game& a_Game) override final;

	bool m_Rotate;
	glm::vec3 m_RotationX;
	glm::vec3 m_RotationY;
	glm::vec3 m_RotationZ;
	glm::vec3 m_RotationSpeeds;
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
	Planet(int a_MeshId) : Entity(a_MeshId), m_RotationSpeed(0), m_Orbits(false), m_OrbitSpeed(0.f), m_OrbitPoint(0.f), m_OrbitAxis({0.f, 1.f, 0.f}) {}
	Planet() : Entity(), m_RotationSpeed(0), m_Orbits(false), m_OrbitSpeed(0.f), m_OrbitPoint(0.f), m_OrbitAxis({0.f, 1.f, 0.f}) {}

	void SetRotationSpeed(float a_Speed);
	void SetOrbit(float a_Speed, const glm::vec3& a_Point, const glm::vec3& a_OrbitAxis);

protected:
	void Update(float a_DeltaTime, Game& a_Game) override final;

	float m_RotationSpeed;
	bool m_Orbits;
	float m_OrbitSpeed;
	glm::vec3 m_OrbitPoint;
	glm::vec3 m_OrbitAxis;
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