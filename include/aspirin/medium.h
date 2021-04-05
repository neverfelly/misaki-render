#pragma once

#include "fwd.h"
#include "object.h"
#include "ray.h"

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Medium : public Object {

public:
    APR_IMPORT_CORE_TYPES(Float)
    using Ray                = Ray<Float, Spectrum>;
    using PhaseFunction      = PhaseFunction<Float, Spectrum>;
    using Sampler            = Sampler<Float, Spectrum>;
    using Scene              = Scene<Float, Spectrum>;
    using Texture            = Texture<Float, Spectrum>;
    using MediumInteraction  = MediumInteraction<Float, Spectrum>;
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;

    /// Sample a free-flight distance in the medium.
    virtual std::pair<MediumInteraction, Float> sample_interaction(const Ray &ray, Float sample,
                                                 uint32_t channel) const = 0;

    /// Compute the transmittance and PDF
    virtual Spectrum eval_transmittance(const Ray &ray) const = 0;

    const PhaseFunction *phase_function() const {
        return m_phase_function.get();
    }

    bool is_homogeneous() const { return m_is_homogeneous; }
    std::string id() const override { return m_id; }
    std::string to_string() const override = 0;

    APR_DECLARE_CLASS()
protected:
    Medium();
    Medium(const Properties &props);
    virtual ~Medium();

protected:
    ref<PhaseFunction> m_phase_function;
    bool m_is_homogeneous;

    std::string m_id;
};
APR_EXTERN_CLASS(Medium)

} // namespace aspirin