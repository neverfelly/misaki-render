#pragma once

#include "fresnel.h"
#include "fwd.h"
#include "properties.h"

namespace aspirin {

namespace microfacet {

Float gtr1(Float cos_theta_h, Float alpha) noexcept {
    const Float a2 = alpha * alpha;
    const Float t  = 1 + (a2 - 1) * math::sqr(cos_theta_h);
    return (a2 - 1) / (math::Pi<Float> * std::log(a2) * t);
}

Float gtr2(Float cos_theta_h, Float alpha) noexcept {
    const Float a2 = alpha * alpha;
    const Float t  = 1 + (a2 - 1) * math::sqr(cos_theta_h);
    return a2 / (math::Pi<Float> * t * t);
}

Float gtr2_aniso(Float sin_theta_h, Float cos_theta_h, Float sin_phi_h,
                 Float cos_phi_h, Float ax, Float ay) noexcept {
    const Float t  = math::sqr(cos_phi_h / ax) + math::sqr(sin_phi_h / ay);
    const Float t2 = math::sqr(sin_theta_h) * t + math::sqr(cos_theta_h);
    return 1.f / (math::Pi<Float> * ax * ay * math::sqr(t2));
}

Float smith_g1_ggx(Float tan_theta, Float alpha) noexcept {
    if (!tan_theta)
        return 1.f;
    const Float t = tan_theta * alpha;
    return 2.f / (1.f + std::sqrt(1 + t * t));
}

Float smith_g1_ggx_aniso(Float tan_theta, Float sin_phi, Float cos_phi,
                         Float ax, Float ay) noexcept {
    const Float t       = math::sqr(ax * cos_phi) + math::sqr(ay * sin_phi);
    const Float sqr_val = 1 + t * math::sqr(tan_theta);
    const Float lambda  = -Float(0.5) + Float(0.5) * std::sqrt(sqr_val);
    return 1 / (1 + lambda);
}

Vector3 sample_gtr1(Float alpha, const Vector2 &sample) noexcept {
    const Float phi       = 2 * math::Pi<Float> * sample.x();
    const Float cos_theta = std::sqrt(
        (std::pow(alpha, 2 - 2 * sample.y()) - 1) / (math::sqr(alpha) - 1));
    const Float sin_theta = math::safe_sqrt(1.f - cos_theta * cos_theta);
    return math::normalize(Vector3(sin_theta * std::cos(phi),
                                   sin_theta * std::sin(phi), cos_theta));
}

Vector3 sample_gtr2_aniso(Float ax, Float ay, const Vector2 &sample) noexcept {
    Float sin_phi_h = ay * std::sin(2 * math::Pi<Float> * sample.x());
    Float cos_phi_h = ax * std::cos(2 * math::Pi<Float> * sample.x());
    const Float nor =
        1 / std::sqrt(math::sqr(sin_phi_h) + math::sqr(cos_phi_h));
    sin_phi_h *= nor;
    cos_phi_h *= nor;

    const Float A = math::sqr(cos_phi_h / ax) + math::sqr(sin_phi_h / ay);
    const Float cos_theta_h =
        std::sqrt(A * (1 - sample.y()) / ((1 - A) * sample.y() + A));
    const Float sin_theta_h =
        std::sqrt(std::max<Float>(0, 1 - math::sqr(cos_theta_h)));

    return math::normalize(
        Vector3(sin_theta_h * cos_phi_h, sin_theta_h * sin_phi_h, cos_theta_h));
}

} // namespace microfacet

class MicrofacetDistribution {
public:
    enum class Type : uint32_t { Beckmann = 0, GGX = 1 };
    MicrofacetDistribution(const Properties &props, Type type = Type::Beckmann,
                           Float alpha_u = 0.1, Float alpha_v = 0.1,
                           bool sample_visible = false)
        : m_type(type), m_alpha_u(alpha_u), m_alpha_v(alpha_v),
          m_sample_visible(sample_visible) {
        if (props.has_property("distribution")) {
            auto distr = props.string("distribution");
            if (distr == "beckmann")
                m_type = Type::Beckmann;
            else if (distr == "ggx")
                m_type = Type::GGX;
            else
                Throw("Specified an invalid microfacet distribution {}", distr);
        }

        if (props.has_property("alpha")) {
            m_alpha_u = m_alpha_v = props.get_float("alpha");
            if (props.has_property("alpha_u") || props.has_property("alpha_v"))
                Throw("Microfacet model: please specify"
                      "either 'alpha' or 'alpha_u'/'alpha_v'.");
        } else if (props.has_property("alpha_u") ||
                   props.has_property("alpha_v")) {
            if (!props.has_property("alpha_u") ||
                !props.has_property("alpha_v"))
                Throw("Microfacet model: both 'alpha_u' and 'alpha_v' must be "
                      "specified.");
            if (props.has_property("alpha"))
                Throw("Microfacet model: please specify"
                      "either 'alpha' or 'alpha_u'/'alpha_v'.");
            m_alpha_u = props.get_float("alpha_u");
            m_alpha_v = props.get_float("alpha_v");
        }

        if (alpha_u == 0.f || alpha_v == 0.f)
            Log(Warn, "Cannot create a microfacet distribution with "
                      "alpha_u/alpha_v=0 (clamped to 10^-4). "
                      "Please use the corresponding smooth reflectance model "
                      "to get zero roughness.");

        m_sample_visible = props.get_bool("sample_visible", sample_visible);

        configure();
    }

    MicrofacetDistribution(Type type, Float alpha_u, Float alpha_v,
                           bool sample_visible = false)
        : m_type(type), m_alpha_u(alpha_u), m_alpha_v(alpha_v),
          m_sample_visible(sample_visible) {
        configure();
    }

    MicrofacetDistribution(Type type, Float alpha, bool sample_visible = false)
        : m_type(type), m_alpha_u(alpha), m_alpha_v(alpha),
          m_sample_visible(sample_visible) {
        configure();
    }

    Float eval(const Vector3 &m) const {
        Float alpha_uv = m_alpha_u * m_alpha_v, cos_theta = Frame::cos_theta(m),
              cos_theta_2 = cos_theta * cos_theta,
              temporal    = (m.x() / m_alpha_u) * (m.x() / m_alpha_u) +
                         (m.y() / m_alpha_v) * (m.y() / m_alpha_v),
              result;
        switch (m_type) {
            case Type::Beckmann: {
                result =
                    std::exp(-temporal / cos_theta_2) /
                    (math::Pi<Float> * alpha_uv * cos_theta_2 * cos_theta_2);
                break;
            }
            case Type::GGX: {
                result = 1.f / (math::Pi<Float> * alpha_uv *
                                (temporal + m.z() * m.z()) *
                                (temporal + m.z() * m.z()));
                break;
            }
            default:
                break;
        }
        return result * cos_theta > 1e-20f ? result : 0.f;
    }

    Float pdf(const Vector3 &wi, const Vector3 &m) const {
        Float result = eval(m);
        if (m_sample_visible)
            result *=
                smith_g1(wi, m) * std::abs(wi.dot(m)) / Frame::cos_theta(wi);
        else
            result *= Frame::cos_theta(m);
        return result;
    }

    std::pair<Vector3, Float> sample(const Vector3 &wi,
                                     const Vector2 &sample) const {
        if (!m_sample_visible) {
            Float sin_phi, cos_phi, cos_theta, cos_theta_2, alpha_2, pdf;
            if (is_isotropic()) {
                Float temp = (2.f * math::Pi<Float>) *sample.y();
                sin_phi    = std::sin(temp);
                cos_phi    = std::cos(temp);
                alpha_2    = m_alpha_u * m_alpha_u;
            } else {
                Float ratio = m_alpha_v / m_alpha_u,
                      tmp   = ratio * tan((2.f * math::Pi<Float>) *sample.y());
                cos_phi     = 1.f / std::sqrt(tmp * tmp + 1.f);
                cos_phi =
                    cos_phi * std::copysign(1.f, abs(sample.y() - .5f) - .25f);

                sin_phi = cos_phi * tmp;
                alpha_2 = 1.f / (math::sqr(cos_phi / m_alpha_u) +
                                 math::sqr(sin_phi / m_alpha_v));
            }
            switch (m_type) {
                case Type::Beckmann: {
                    // Beckmann distribution function for Gaussian random
                    // surfaces
                    cos_theta =
                        1.f /
                        std::sqrt(1.f - alpha_2 * std::log(1.f - sample.x()));
                    cos_theta_2 = math::sqr(cos_theta);

                    // Compute probability density of the sampled position
                    Float cos_theta_3 =
                        std::max(cos_theta_2 * cos_theta, 1e-20f);
                    pdf = (1.f - sample.x()) / (math::Pi<Float> * m_alpha_u *
                                                m_alpha_v * cos_theta_3);
                }
                case Type::GGX: {
                    // GGX / Trowbridge-Reitz distribution function
                    Float tan_theta_m_2 =
                        alpha_2 * sample.x() / (1.f - sample.x());
                    cos_theta   = 1.f / std::sqrt(1.f + tan_theta_m_2);
                    cos_theta_2 = math::sqr(cos_theta);

                    // Compute probability density of the sampled position
                    Float temp = 1.f + tan_theta_m_2 / alpha_2,
                          cos_theta_3 =
                              std::max(cos_theta_2 * cos_theta, 1e-20f);
                    pdf = 1.f / (math::Pi<Float> * m_alpha_u * m_alpha_v *
                                 cos_theta_3 * math::sqr(temp));
                }
                default:
                    break;
            }
            Float sin_theta = std::sqrt(1.f - cos_theta_2);
            return { { cos_phi * sin_theta, sin_phi * sin_theta, cos_theta },
                     pdf };
        } else {
            Log(Warn, "sample visible not supported");
        }
    }

    Float G(const Vector3 &wi, const Vector3 &wo, const Vector3 &m) const {
        return smith_g1(wi, m) * smith_g1(wo, m);
    }

    Float smith_g1(const Vector3 &v, const Vector3 &m) const {
        Float xy_alpha_2 =
                  math::sqr(m_alpha_u * v.x()) + math::sqr(m_alpha_v * v.y()),
              tan_theta_alpha_2 = xy_alpha_2 / math::sqr(v.z()), result;
        if (xy_alpha_2 == 0.f)
            return 1.f;
        if (v.dot(m) * Frame::cos_theta(v) <= 0.f)
            return 0.f;
        switch (m_type) {
            case Type::Beckmann: {
                Float a     = 1.f / std::sqrt(tan_theta_alpha_2),
                      a_sqr = math::sqr(a);
                result      = a >= 1.6f ? 1.f
                                        : (3.535f * a + 2.181f * a_sqr) /
                                         (1.f + 2.276f * a + 2.577f * a_sqr);
                break;
            }
            case Type::GGX: {
                result = 2.f / (1.f + std::sqrt(1.f + tan_theta_alpha_2));
                break;
            }
            default:
                break;
        }
        return result;
    }

    Type type() const { return m_type; }
    // isotropic case
    Float alpha() const { return m_alpha_u; }
    Float alpha_u() const { return m_alpha_u; }
    Float alpha_v() const { return m_alpha_v; }
    bool sample_visible() const { return m_sample_visible; };
    bool is_isotropic() const { return m_alpha_u == m_alpha_v; }
    void scale_alpha(Float value) {
        m_alpha_u *= value;
        m_alpha_v *= value;
    }

protected:
    void configure() {
        m_alpha_u = std::max(m_alpha_u, 1e-4f);
        m_alpha_v = std::max(m_alpha_v, 1e-4f);
    }

protected:
    Type m_type;
    Float m_alpha_u, m_alpha_v;
    bool m_sample_visible;
};

} // namespace aspirin