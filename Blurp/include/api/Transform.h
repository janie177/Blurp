#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace blurp
{
    class Transform
    {
    public:
        Transform();

        /*
         * Returns true when this Transform has its dirty flag set to true.
         */
        bool NeedsRebuilding() const;

        /**
         * Return the transformation.
         * If the dirty flag is set, this will rebuild the matrix.
         */
        glm::mat4 GetTransformation() const;

        /**
         * Rotate the transformation around the given axis for the given angle in radians.
         */
        void Rotate(const glm::vec3& a_Axis, float a_Angle);

        /**
         * Rotate the transformation with the given quaternion.
         */
        void Rotate(const glm::quat& a_Quat);

        /**
         * Translate the transformation by the given translation.
         */
        void Translate(const glm::vec3& a_Translation);

        /**
         * Scale the transformation with the given scalars on each axis.
         */
        void Scale(const glm::vec3& a_Scale);

        /**
         * Scale the transformation by the given scalar on each axis.
         */
        void Scale(float a_Scale);

        /**
         * Create a matrix that will place an object
         *
         * The way this works is the following:
         * If you pass a_Position as your camera position, and a_Target as where the camera is looking,
         * then transforming an object with this resulting matrix will put the object right in front of the camera.
         */
        void LookAt(const glm::vec3& a_Position, const glm::vec3& a_Target, const glm::vec3& a_Up);

        /**
         * Rotate around the given axis, with the given angle in radians, offset from the given position.
         */
        void RotateAround(const glm::vec3& a_Position, const glm::vec3& a_Axis, float a_Angle);

        /**
         * Rotate around the given position with the given quaternion.
         */
        void RotateAround(const glm::vec3& a_Position, const glm::quat& a_Rotation);

        /**
         * Set the scale of the transformation.
         */
        void SetScale(const glm::vec3& a_Scale);

        /**
         * Set the scale of the transformation.
         */
        void SetScale(float a_Scale);

        /**
         * Set the rotation of the transformation to be the given angle around the given axis in radians.
         */
        void SetRotation(const glm::vec3& a_Axis, float a_Angle);

        /**
         * Set the rotation of the transform to be equal to the given quaternion.
         */
        void SetRotation(const glm::quat& a_Quaternion);

        /**
         * Set the translation of the transform.
         */
        void SetTranslation(const glm::vec3& a_Translation);

        /**
         * Get the current rotation of this transform.
         */
        glm::quat GetRotation() const;

        /**
         * Get the translation of this transform.
         */
        glm::vec3 GetTranslation() const;

        /**
         * Get the scale of this transform.
         */
        glm::vec3 GetScale() const;

        /**
         * Get the forward direction of this matrix.
         */
        glm::vec3 GetForward() const;

        /**
         * Get the up direction of this matrix.
         */
        glm::vec3 GetUp() const;

        /**
         * Get the right direction of this matrix.
         */
        glm::vec3 GetRight() const;

        /**
         * Get the back vector of this matrix.
         */
        glm::vec3 GetBack() const;

        /**
         * Get the down vector of this matrix.
         */
        glm::vec3 GetDown() const;

        /**
         * Get the left vector of this matrix.
         */
        glm::vec3 GetLeft() const;

        /**
         * Get the global forward direction.
         */
        static constexpr glm::vec3 GetWorldForward() { return glm::vec3(0, 0, 1); }

        /**
         * Get the global up direction.
         */
        static constexpr glm::vec3 GetWorldUp() { return glm::vec3(0, 1, 0); }

        /**
         * Get the global right direction.
         */
        static constexpr glm::vec3 GetWorldRight() { return glm::vec3(1, 0, 0); }

        /**
         * Get the global back vector.
         */
        static constexpr glm::vec3 GetWorldBack() { return -GetWorldForward(); }

        /**
         * Get the global down vector.
         */
        static constexpr glm::vec3 GetWorldDown() { return -GetWorldDown(); }

        /**
         * Get the global left vector.
         */
        static constexpr glm::vec3 GetWorldLeft() { return -GetWorldLeft(); }

    private:
        //Internally updates flag and transformation.
        void Rebuild() const;

    private:
        glm::quat m_Rotation;
        glm::vec3 m_Translation;
        glm::vec3 m_Scale;

        mutable glm::mat4 m_Transformation;
        mutable bool m_Flag;
    };
}