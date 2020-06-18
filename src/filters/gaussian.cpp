#include <misaki/render/properties.h>
#include <misaki/render/rfilter.h>

namespace misaki::render {

class GaussianFilter final : public ReconstructionFilter {
 public:
  GaussianFilter(const Properties &props) : ReconstructionFilter(props) {
    m_stddev = props.get_float("stddev", 0.5f);
    m_radius = 4 * m_stddev;
    m_alpha = -1.f / (2.f * m_stddev * m_stddev);
    m_bias = std::exp(m_alpha * m_radius * m_radius);
  }

  Float eval(Float x) const {
    return std::max(0.f, std::exp(m_alpha * x * x) - m_bias);
  }

 private:
  Float m_stddev, m_alpha, m_bias;
};

}  // namespace misaki::render