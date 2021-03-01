#pragma once

#include "component.h"
#include "interaction.h"

namespace aspirin {

enum class TransportMode : uint32_t {
    Radiance,   // Camera to light
    Importance, // Light to camera
    TransportMode = 2
};

enum class BSDFFlags : uint32_t {
    None                = 0x00000,
    Null                = 0x00001,
    DiffuseReflection   = 0x00002,
    DiffuseTransmission = 0x00004,
    GlossyReflection    = 0x00008,
    GlossyTransmission  = 0x0010,
    DeltaReflection     = 0x0020,
    DeltaTransmission   = 0x00040,
    Reflection   = DiffuseReflection | GlossyReflection | DeltaReflection,
    Diffuse      = DiffuseReflection | DiffuseTransmission,
    Transmission = DiffuseTransmission | GlossyTransmission | DeltaTransmission,
    Glossy       = GlossyReflection | GlossyTransmission,
    Delta        = Null | DeltaReflection | DeltaTransmission,
    Smooth       = Diffuse | Glossy,
    All          = Diffuse | Glossy | Delta
};

constexpr uint32_t operator|(BSDFFlags f1, BSDFFlags f2) {
    return (uint32_t) f1 | (uint32_t) f2;
}
constexpr uint32_t operator|(uint32_t f1, BSDFFlags f2) {
    return f1 | (uint32_t) f2;
}
constexpr uint32_t operator&(BSDFFlags f1, BSDFFlags f2) {
    return (uint32_t) f1 & (uint32_t) f2;
}
constexpr uint32_t operator&(uint32_t f1, BSDFFlags f2) {
    return f1 & (uint32_t) f2;
}
constexpr uint32_t operator~(BSDFFlags f1) { return ~(uint32_t) f1; }
constexpr uint32_t operator+(BSDFFlags e) { return (uint32_t) e; }
template <typename UInt32> constexpr auto has_flag(UInt32 flags, BSDFFlags f) {
    return (flags & (uint32_t) f) != 0u;
}

struct APR_EXPORT BSDFContext {
    TransportMode mode;
    uint32_t type_mask = +BSDFFlags::All;
    uint32_t component = (uint32_t) -1;

    BSDFContext(TransportMode mode = TransportMode::Radiance) : mode(mode) {}
    BSDFContext(TransportMode mode, uint32_t type_mask, uint32_t component)
        : mode(mode), type_mask(type_mask), component(component) {}

    // Checks whether a given BSDF component type are enable in this context
    bool is_enabled(BSDFFlags type_, uint32_t component_ = 0) const {
        uint32_t type = (uint32_t) type_;
        return (type_mask == +BSDFFlags::All || (type_mask & type) == type) &&
               (component == (uint32_t) -1 || component == component_);
    }
};

struct BSDFSample {
    Vector3 wo;
    Float pdf;
    Float eta;
    uint32_t sampled_type;
    uint32_t sampled_component;

    BSDFSample()
        : wo(Vector3(0)), pdf(0.f), eta(1.f), sampled_type(0),
          sampled_component(-1) {}
    BSDFSample(const Vector3 &wo)
        : wo(wo), pdf(0.f), eta(1.f), sampled_type(0), sampled_component(-1) {}
};

template <typename Spectrum> class APR_EXPORT BSDF : public Component {
public:
    using SceneInteraction = SceneInteraction<Spectrum>;

    BSDF(const Properties &props);

    // Sample BSDF * cos(theta) and returns sampled bsdf information with BSDF *
    // cos(theta) divided by pdf
    virtual std::pair<BSDFSample, Color3>
    sample(const BSDFContext &ctx, const SceneInteraction &si,
           Float sample1, // For selecting different bsdf lobe
           const Vector2 &sample) const;

    // Returns evaluated BSDF * cos(theta)
    virtual Color3 eval(const BSDFContext &ctx, const SceneInteraction &si,
                        const Vector3 &wo) const;

    // Returns pdf of BSDF * cos(theta)
    virtual Float pdf(const BSDFContext &ctx, const SceneInteraction &si,
                      const Vector3 &wo) const;

    uint32_t flags() const { return m_flags; }

    uint32_t flags(size_t i) const {
        assert(i < m_components.size());
        return m_components[i];
    }

    size_t component_count() const { return m_components.size(); }

protected:
    uint32_t m_flags;
    std::vector<uint32_t> m_components;
    std::string m_id;
};

} // namespace aspirin