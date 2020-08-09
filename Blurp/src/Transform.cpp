#include "Transform.h"

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace blurp
{
    Transform::Transform()
        : m_Rotation(glm::quat_identity<glm::quat::value_type, glm::highp>())
        , m_Translation(0)
        , m_Scale(1)
        , m_Transformation(1)
        , m_Flag(false)
    {

    }

    glm::mat4 Transform::GetTransformation() const
    {
        if (m_Flag)
        {
            Rebuild();
        }
        return m_Transformation;
    }

    void Transform::Rotate(const glm::vec3& a_Axis, float a_Angle)
    {
        Rotate(glm::angleAxis(a_Angle, a_Axis));
    }

    void Transform::Rotate(const glm::quat& a_Quat)
    {
        m_Flag = true;
        m_Rotation = a_Quat * m_Rotation;
    }

    void Transform::Translate(const glm::vec3& a_Translation)
    {
        m_Flag = true;
        m_Translation += a_Translation;
    }

    void Transform::Scale(const glm::vec3& a_Scale)
    {
        m_Flag = true;
        m_Scale *= a_Scale;
    }

    void Transform::Scale(float a_Scale)
    {
        m_Flag = true;
        m_Scale *= a_Scale;
    }

    void Transform::LookAt(const glm::vec3& a_Position, const glm::vec3& a_Target, const glm::vec3& a_Up)
    {
        //Make sure up is normalized.
        assert(glm::length(a_Up) - 1.f <= std::numeric_limits<float>::epsilon() * 5.f && "a_Up must be normalized!");

        //Make sure that position and target are not the same.
        assert(glm::length2(a_Up - a_Position) - 1.f != 0.f && "Position and target cannot be equal!");

        //Construct a new rotation matrix from the up and target direction.
        auto forward = (a_Target - a_Position);
        if (glm::length2(forward) != 0.f)
        {
            forward = glm::normalize(forward);
        }

        //Calculate the right vector. Already normalized since forward and up are too.
        const auto right = glm::cross(a_Up, forward);

        //Make a matrix that contains the right up and forward directions.
        glm::mat4 matrix = glm::mat4();
        matrix[0] = glm::vec4(right, 0.f);
        matrix[1] = glm::vec4(a_Up, 0.f);
        matrix[2] = glm::vec4(forward, 0.f);
        matrix[3] = glm::vec4(a_Position, 1.f); //Also insert the position as a translation.

        m_Transformation = matrix;
        m_Flag = false;

        //Not used
        glm::vec3 skew;
        glm::vec4 perspective;

        //Read back the values from the matrix, and store them locally.
        glm::decompose(m_Transformation, m_Scale, m_Rotation, m_Translation, skew, perspective);
    }

    void Transform::RotateAround(const glm::vec3& a_Position, const glm::vec3& a_Axis, float a_Angle)
    {
        RotateAround(a_Position, glm::angleAxis(a_Angle, a_Axis));
    }

    void Transform::RotateAround(const glm::vec3& a_Position, const glm::quat& a_Rotation)
    {
        //First calculate a matrix that can rotate around
        const auto identity = glm::identity<glm::mat4>();
        const auto translate = glm::translate(identity, a_Position);
        const auto inverseTranslate = glm::translate(identity, -a_Position);

        //Make rotation matrix that rotates around the point.
        const auto quatToMat = glm::mat4_cast(a_Rotation);
        const auto rotate = translate * quatToMat * inverseTranslate;

        //Now combine the old matrix with this new matrix.
        if (m_Flag)
        {
            Rebuild();
        }

        //Rotate the entire existing thing around the point.
        m_Transformation = rotate * m_Transformation;

        //Not used
        glm::vec3 skew;
        glm::vec4 perspective;

        //Read back the values from the matrix, and store them locally.
        glm::decompose(m_Transformation, m_Scale, m_Rotation, m_Translation, skew, perspective);
    }

    void Transform::SetScale(const glm::vec3& a_Scale)
    {
        m_Flag = true;
        m_Scale = a_Scale;
    }

    void Transform::SetScale(float a_Scale)
    {
        m_Flag = true;
        m_Scale = glm::vec3(a_Scale);
    }

    void Transform::SetRotation(const glm::vec3& a_Axis, float a_Angle)
    {
        SetRotation(glm::angleAxis(a_Angle, a_Axis));
    }

    void Transform::SetRotation(const glm::quat& a_Quaternion)
    {
        m_Flag = true;
        m_Rotation = a_Quaternion;
    }

    void Transform::SetTranslation(const glm::vec3& a_Translation)
    {
        m_Flag = true;
        m_Translation = a_Translation;
    }

    glm::quat Transform::GetRotation() const
    {
        return m_Rotation;
    }

    glm::vec3 Transform::GetTranslation() const
    {
        return m_Translation;
    }

    glm::vec3 Transform::GetScale() const
    {
        return m_Scale;
    }

    glm::vec3 Transform::GetForward() const
    {
        //If flag is set, then calculate forward from new data.
        if(m_Flag)
        {
            return glm::mat3_cast(m_Rotation) * GetWorldForward();
        }

        //Flag not set, so return cached matrix data.
        const auto column = glm::column(m_Transformation, 2);
        return glm::normalize(glm::vec3(column.x, column.y, column.z));
    }

    glm::vec3 Transform::GetUp() const
    {
        //If flag is set, then calculate forward from new data.
        if (m_Flag)
        {
            return glm::mat3_cast(m_Rotation) * GetWorldUp();
        }

        //Flag not set, so return cached matrix data. Normalize because scale may have affected it.
        const auto column = glm::column(m_Transformation, 1);
        return glm::normalize(glm::vec3(column.x, column.y, column.z));
    }

    glm::vec3 Transform::GetRight() const
    {
        //If flag is set, then calculate forward from new data.
        if (m_Flag)
        {
            return glm::mat3_cast(m_Rotation) * GetWorldRight();
        }

        //Flag not set, so return cached matrix data.
        const auto column = glm::column(m_Transformation, 0);
        return glm::normalize(glm::vec3(column.x, column.y, column.z));
    }

    glm::vec3 Transform::GetBack() const
    {
        return GetForward() * -1.f;
    }

    glm::vec3 Transform::GetDown() const
    {
        return GetUp() * -1.f;
    }

    glm::vec3 Transform::GetLeft() const
    {
        return GetRight() * -1.f;
    }

    void Transform::Rebuild() const
    {
        //Build.
        const glm::mat4 identity = glm::mat4(1);
        const glm::mat4 translation = glm::translate(identity, m_Translation);
        const glm::mat4 rotation = glm::mat4_cast(m_Rotation);
        const glm::mat4 scale = glm::scale(identity, m_Scale);

        m_Transformation = (translation * rotation * scale);

        //Unset the flag.
        m_Flag = false;
    }
   

}