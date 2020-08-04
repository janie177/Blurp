#include "Settings.h"
#include "Texture.h"

namespace blurp
{
    void MaterialSettings::EnableAttribute(MaterialAttribute a_Attribute)
    {
        //Ensure that no bitmasking was performed.
        assert(static_cast<int>(a_Attribute) && !(static_cast<int>(a_Attribute) & (static_cast<int>(a_Attribute) - 1)) && "Only a single attribute can be enabled at a time! No masking allowed.");

        switch (a_Attribute)
        {
        case MaterialAttribute::DIFFUSE_TEXTURE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::DIFFUSE_CONSTANT_VALUE);
        }
        break;
        case MaterialAttribute::EMISSIVE_TEXTURE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::EMISSIVE_CONSTANT_VALUE);
        }
        break;
        case MaterialAttribute::METALLIC_TEXTURE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::METALLIC_CONSTANT_VALUE);
        }
        break;
        case MaterialAttribute::ROUGHNESS_TEXTURE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::ROUGHNESS_CONSTANT_VALUE);
        }
        break;
        case MaterialAttribute::ALPHA_TEXTURE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::ALPHA_CONSTANT_VALUE);
        }
        break;
        case MaterialAttribute::DIFFUSE_CONSTANT_VALUE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::DIFFUSE_TEXTURE);
        }
        break;
        case MaterialAttribute::EMISSIVE_CONSTANT_VALUE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::EMISSIVE_TEXTURE);
        }
        break;
        case MaterialAttribute::METALLIC_CONSTANT_VALUE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::METALLIC_TEXTURE);
        }
        break;
        case MaterialAttribute::ROUGHNESS_CONSTANT_VALUE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::ROUGHNESS_TEXTURE);
        }
        break;
        case MaterialAttribute::ALPHA_CONSTANT_VALUE:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
            DisableAttribute(MaterialAttribute::ALPHA_TEXTURE);
        }
        break;

        //No special action needed.
        default:
        {
            m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) | static_cast<std::uint16_t>(a_Attribute));
        }
        break;
        }
    }

    void MaterialSettings::DisableAttribute(MaterialAttribute a_Attribute)
    {
        m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) & ~static_cast<std::uint16_t>(a_Attribute));
    }

    void MaterialSettings::SetDiffuseTexture(const std::shared_ptr<Texture>& a_Texture)
    {
        //Ensure that the right pixel format and data type are used.
        assert(a_Texture == nullptr || IsSigned(a_Texture->GetDataType()) && a_Texture->GetPixelFormat() == PixelFormat::RGB);
        m_DiffuseTexture = a_Texture;
    }

    void MaterialSettings::SetEmissiveTexture(const std::shared_ptr<Texture>& a_Texture)
    {
        //Ensure that the right pixel format and data type are used.
        assert(a_Texture == nullptr || IsSigned(a_Texture->GetDataType()) && a_Texture->GetPixelFormat() == PixelFormat::RGB);
        m_EmissiveTexture = a_Texture;
    }

    void MaterialSettings::SetNormalTexture(const std::shared_ptr<Texture>& a_Texture)
    {
        //Ensure that the right pixel format and data type are used.
        assert(a_Texture == nullptr || IsSigned(a_Texture->GetDataType()) && a_Texture->GetPixelFormat() == PixelFormat::RGB);
        m_NormalTexture = a_Texture;
    }

    void MaterialSettings::SetHeightTexture(const std::shared_ptr<Texture>& a_Texture)
    {
        //Ensure that the right pixel format and data type are used.
        assert(a_Texture == nullptr || IsSigned(a_Texture->GetDataType()) && a_Texture->GetPixelFormat() == PixelFormat::RG);
        m_HeightTexture = a_Texture;
    }

    void MaterialSettings::SetOMRATexture(const std::shared_ptr<Texture>& a_Texture)
    {
        //Ensure that the right pixel format and data type are used.
        assert(a_Texture == nullptr || IsSigned(a_Texture->GetDataType()) && a_Texture->GetPixelFormat() == PixelFormat::RGBA);
        m_OcclusionMetallicRoughnessAlphaTexture = a_Texture;
    }

    void MaterialSettings::SetDiffuseConstant(const glm::vec3& a_Value)
    {
        //Make sure that the value is normalized.
        assert(a_Value.x >= 0.f && a_Value.x <= 1.f);
        assert(a_Value.y >= 0.f && a_Value.y <= 1.f);
        assert(a_Value.z >= 0.f && a_Value.z <= 1.f);
        m_DiffuseValue = a_Value;
    }

    void MaterialSettings::SetEmissiveConstant(const glm::vec3& a_Value)
    {
        assert(a_Value.x >= 0.f && a_Value.x <= 1.f);
        assert(a_Value.y >= 0.f && a_Value.y <= 1.f);
        assert(a_Value.z >= 0.f && a_Value.z <= 1.f);
        m_EmissiveValue = a_Value;
    }

    void MaterialSettings::SetMetallicConstant(float a_Value)
    {
        assert(a_Value >= 0.f && a_Value <= 1.f);
        m_MetallicValue = a_Value;
    }

    void MaterialSettings::SetRoughnessConstant(float a_Value)
    {
        assert(a_Value >= 0.f && a_Value <= 1.f);
        m_RoughnessValue = a_Value;
    }

    void MaterialSettings::SetAlphaConstant(float a_Value)
    {
        assert(a_Value >= 0.f && a_Value <= 1.f);
        m_AlphaValue = a_Value;
    }

    std::shared_ptr<Texture> MaterialSettings::GetDiffuseTexture() const
    {
        return m_DiffuseTexture;
    }

    std::shared_ptr<Texture> MaterialSettings::GetEmissiveTexture() const
    {
        return m_EmissiveTexture;
    }

    std::shared_ptr<Texture> MaterialSettings::GetNormalTexture() const
    {
        return m_NormalTexture;
    }

    std::shared_ptr<Texture> MaterialSettings::GetHeightTexture() const
    {
        return m_HeightTexture;
    }

    std::shared_ptr<Texture> MaterialSettings::GetOMRATexture() const
    {
        return m_OcclusionMetallicRoughnessAlphaTexture;
    }

    glm::vec3 MaterialSettings::GetDiffuseValue() const
    {
        return m_DiffuseValue;
    }

    glm::vec3 MaterialSettings::GetEmissiveValue() const
    {
        return m_EmissiveValue;
    }

    float MaterialSettings::GetMetallicValue() const
    {
        return m_MetallicValue;
    }

    float MaterialSettings::GetRoughnessValue() const
    {
        return m_RoughnessValue;
    }

    float MaterialSettings::GetAlphaValue() const
    {
        return m_AlphaValue;
    }

    bool MaterialSettings::IsAttributeEnabled(MaterialAttribute a_Attribute) const
    {
        const auto cast = static_cast<std::uint16_t>(a_Attribute);
        return (static_cast<std::uint16_t>(m_Mask) & cast) == cast;
    }
}
