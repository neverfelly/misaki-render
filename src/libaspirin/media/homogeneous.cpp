#include <aspirin/interaction.h>
#include <aspirin/medium.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>
#include <aspirin/volume.h>
#include <iostream>
namespace aspirin {

class HomogeneousMedium final : public Medium {
public:
    explicit HomogeneousMedium(const Properties &props) : Medium(props) {
        m_is_homogeneous = true;
        m_sigma_a        = props.color("sigma_a", Spectrum::Constant(1.f));
        m_sigma_s        = props.color("sigma_s", Spectrum::Constant(1.f));
        m_sigma_t        = m_sigma_s + m_sigma_a;
        m_scale          = props.float_("scale", 1.0f);
    }

    std::pair<MediumInteraction, Float>
    sample_interaction(const Ray &ray, Float sample,
                       uint32_t channel) const override {
        Float sampled_distance = -std::log(1 - sample) / m_sigma_t[channel];

        MediumInteraction mi;
        Float pdf;
        if (sampled_distance < ray.maxt - ray.mint) {
            mi.t       = sampled_distance + ray.mint;
            mi.p       = ray(mi.t);
            mi.sigma_a = m_sigma_a;
            mi.sigma_s = m_sigma_s;
            if (mi.p == ray.o) {
                mi.t = math::Infinity<Float>;
                pdf  = (m_sigma_t * (-sampled_distance)).exp().mean();
            } else
                pdf = ((m_sigma_t * (-sampled_distance)).exp() * m_sigma_t)
                          .mean();
        } else {
            mi.t             = math::Infinity<Float>;
            sampled_distance = ray.maxt - ray.mint;
            pdf              = (m_sigma_t * (-sampled_distance)).exp().mean();
        }
        mi.transmittance = (m_sigma_t * (-sampled_distance)).exp();
        if (mi.transmittance.maxCoeff() < 1e-20)
            mi.transmittance = Spectrum::Zero();
        mi.medium = this;
        return { mi, pdf };
    }

    Spectrum eval_transmittance(const Ray &ray) const override {
        Float neg_length = ray.mint - ray.maxt;
        return (m_sigma_t * neg_length).exp();
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "HomogeneousMedium[" << std::endl
            << "  sigma_a = " << string::indent(m_sigma_a) << std::endl
            << "  sigma_t = " << string::indent(m_sigma_s) << std::endl
            << "  scale   = " << m_scale << std::endl
            << "]";
        return oss.str();
    }

    APR_DECLARE_CLASS()
private:
    Spectrum m_sigma_s, m_sigma_a, m_sigma_t;
    Float m_scale;
};

APR_IMPLEMENT_CLASS(HomogeneousMedium, Medium)
APR_INTERNAL_PLUGIN(HomogeneousMedium, "homogeneous")

} // namespace aspirin