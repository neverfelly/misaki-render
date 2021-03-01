#pragma once

#include <optional>

#include "component.h"
#include "fwd.h"
#include "ray.h"

namespace aspirin {

template <typename Spectrum> class APR_EXPORT Scene : public Component {
public:
    using SceneInteraction = SceneInteraction<Spectrum>;
    using DirectSample     = DirectSample<Spectrum>;
    using Camera           = Camera<Spectrum>;
    using Integrator       = Integrator<Spectrum>;
    using Light            = Light<Spectrum>;
    using Shape            = Shape<Spectrum>;

    Scene(const Properties &props);
    ~Scene();

    bool ray_test(const Ray &ray) const;
    SceneInteraction ray_intersect(const Ray &ray) const;
    void accel_init(const Properties &props);
    void accel_release();

    std::pair<DirectSample, Color3>
    sample_direct_light(const PointGeometry &geom, const Vector2 &sample,
                        bool test_visibility = true) const;
    Float pdf_direct_light(const PointGeometry &geom_ref,
                           const DirectSample &ds, const Light *light) const;

    const Camera *camera() const { return m_camera.get(); }
    Camera *camera() { return m_camera.get(); }

    const Integrator *integrator() const { return m_integrator.get(); }
    Integrator *integrator() { return m_integrator.get(); }

    const Light *environment() const { return m_environment.get(); }
    const std::vector<std::shared_ptr<Light>> &lights() const {
        return m_lights;
    }
    std::vector<std::shared_ptr<Light>> &lights() { return m_lights; }

    const std::vector<std::shared_ptr<Shape>> &shapes() const {
        return m_shapes;
    }
    std::vector<std::shared_ptr<Shape>> &shapes() { return m_shapes; }

    const BoundingBox3 &bbox() const { return m_bbox; }

protected:
    void *m_accel = nullptr;
    std::shared_ptr<Integrator> m_integrator;
    std::shared_ptr<Camera> m_camera;
    std::vector<std::shared_ptr<Shape>> m_shapes;
    std::vector<std::shared_ptr<Light>> m_lights;
    std::shared_ptr<Light> m_environment;
    BoundingBox3 m_bbox;
};

} // namespace aspirin