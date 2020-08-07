#include "Settings.h"
#include "Texture.h"

namespace blurp
{
    bool IsSigned(DataType a_Type)
    {
        //NOTE: This only works if the order of DataType remains the same with all unsigned numbers last.
        return a_Type < DataType::UINT;
    }

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

    bool MaterialSettings::IsAttributeEnabled(MaterialAttribute a_Attribute) const
    {
        const auto cast = static_cast<std::uint16_t>(a_Attribute);
        return (static_cast<std::uint16_t>(m_Mask) & cast) == cast;
    }

    std::uint16_t MaterialSettings::GetMask() const
    {
        return static_cast<std::uint16_t>(m_Mask);
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

    void MaterialSettings::SetOHTexture(const std::shared_ptr<Texture>& a_Texture)
    {
        //Ensure that the right pixel format and data type are used.
        assert(a_Texture == nullptr || IsSigned(a_Texture->GetDataType()) && a_Texture->GetPixelFormat() == PixelFormat::RGB);
        m_OcclusionHeightTexture = a_Texture;
    }

    void MaterialSettings::SetMRATexture(const std::shared_ptr<Texture>& a_Texture)
    {
        //Ensure that the right pixel format and data type are used.
        assert(a_Texture == nullptr || IsSigned(a_Texture->GetDataType()) && a_Texture->GetPixelFormat() == PixelFormat::RGB);
        m_MetallicRoughnessAlphaTexture = a_Texture;
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

    std::shared_ptr<Texture> MaterialSettings::GetOHTexture() const
    {
        return m_OcclusionHeightTexture;
    }

    std::shared_ptr<Texture> MaterialSettings::GetMRATexture() const
    {
        return m_MetallicRoughnessAlphaTexture;
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

    void MaterialBatchSettings::EnableAttribute(MaterialAttribute a_Attribute)
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

    void MaterialBatchSettings::DisableAttribute(MaterialAttribute a_Attribute)
    {
        m_Mask = static_cast<MaterialAttribute>(static_cast<std::uint16_t>(m_Mask) & ~static_cast<std::uint16_t>(a_Attribute));
    }

    bool MaterialBatchSettings::IsAttributeEnabled(MaterialAttribute a_Attribute) const
    {
        const auto cast = static_cast<std::uint16_t>(a_Attribute);
        return (static_cast<std::uint16_t>(m_Mask) & cast) == cast;
    }

    std::uint16_t MaterialBatchSettings::GetMask() const
    {
        return static_cast<std::uint16_t>(m_Mask);
    }
}
