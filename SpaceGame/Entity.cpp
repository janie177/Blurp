#include "Entity.h"

Entity::Entity(const int a_MeshId) : m_MeshId(a_MeshId), m_Age(0), m_Direction({ 1.f, 0.f, 0.f }), m_Velocity(0.f), m_Acceleration(0.f), m_MarkedForDelete(false)
{

}

Entity::Entity() : m_MeshId(-1), m_Age(0), m_Direction({ 1.f, 0.f, 0.f }), m_Velocity(0.f), m_Acceleration(0.f), m_MarkedForDelete(false)
{

}

blurp::Transform& Entity::GetTransform()
{
	return m_Transform;
}

float Entity::GetAcceleration() const
{
	return m_Acceleration;
}

glm::vec3 Entity::GetVelocity() const
{
	return m_Velocity;
}

glm::vec3 Entity::GetDirection() const
{
	return m_Direction;
}

int Entity::GetAge() const
{
	return m_Age;
}

int Entity::GetMeshId() const
{
	return m_MeshId;
}

void SpaceShip::Update(float a_DeltaTime, Game& a_Game)
{
	//TODO be careful here, Disney owns star wars now so this might get me sued.
}

void Asteroid::SetRotation(const glm::vec3& a_X, const glm::vec3& a_Y, const glm::vec3& a_Z, const glm::vec3& a_Speeds)
{
	m_Rotate = true;
	m_RotationX = a_X;
	m_RotationY = a_Y;
	m_RotationZ = a_Z;
	m_RotationSpeeds = a_Speeds;
}

void Asteroid::Update(float a_DeltaTime, Game& a_Game)
{
	//TODO Do asteroid things. Probably rotate around some random axis. Maybe extinguish the dinosaurs idk.

	if(m_Rotate)
	{
		m_Transform.Rotate(m_RotationX, m_RotationSpeeds.x * a_DeltaTime);
		m_Transform.Rotate(m_RotationY, m_RotationSpeeds.y * a_DeltaTime);
		m_Transform.Rotate(m_RotationZ, m_RotationSpeeds.z * a_DeltaTime);
	}
}

void KillBot::Update(float a_DeltaTime, Game& a_Game)
{
	//TODO Make war not love.
}

void Planet::SetRotationSpeed(float a_Speed)
{
	m_RotationSpeed = a_Speed;
}

void Planet::SetOrbit(float a_Speed, const glm::vec3& a_Point, const glm::vec3& a_OrbitAxis)
{
	m_Orbits = true;
	m_OrbitPoint = a_Point;
	m_OrbitSpeed = a_Speed;
	m_OrbitAxis = a_OrbitAxis;
}


void Planet::Update(float a_DeltaTime, Game& a_Game)
{
	//TODO rotate around axis, maybe at a nice angle? Other cool planet things like magnetic field and ozone layer yeeeee.
	m_Transform.Rotate({ 0.f, 1.f, 0.f }, m_RotationSpeed * a_DeltaTime);

	if(m_Orbits)
	{
		m_Transform.RotateAround(m_OrbitPoint, m_OrbitAxis, m_RotationSpeed * a_DeltaTime);
	}
}


void Laser::Update(float a_DeltaTime, Game& a_Game)
{
	//TODO shoot and kill things.
}

void Light::SetLight(std::shared_ptr<blurp::PointLight>& a_Light)
{
	m_Light = a_Light;
}

std::shared_ptr<blurp::Light> Light::GetLight() const
{
	return m_Light;
}

void Light::Update(float a_DeltaTime, Game& a_Game)
{
	//TODO lit
}
