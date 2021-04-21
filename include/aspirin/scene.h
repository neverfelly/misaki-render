#pragma once

#include <optional>

#include "emitter.h"
#include "fwd.h"
#include "object.h"
#include "ray.h"
#include "sensor.h"

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Scene : public Object {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Ray                     = Ray<Float, Spectrum>;
    using Interaction             = Interaction<Float, Spectrum>;
    using PreliminaryIntersection = PreliminaryIntersection<Float, Spectrum>;
    using SurfaceInteraction      = SurfaceInteraction<Float, Spectrum>;
    using DirectionSample         = DirectionSample<Float, Spectrum>;
    using Sensor                  = Sensor<Float, Spectrum>;
    using Integrator              = Integrator<Float, Spectrum>;
    using Emitter                 = Emitter<Float, Spectrum>;
    using Shape                   = Shape<Float, Spectrum>;

    Scene(const Properties &props);

    bool ray_test(const Ray &ray) const;
    SurfaceInteraction ray_intersect(const Ray &ray) const;
    void accel_init(const Properties &props);
    void accel_release();

    std::pair<DirectionSample, Spectrum>
    sample_emitter_direction(const Interaction &ref, const Vector2 &sample,
                             bool test_visibility = true) const;
    Float pdf_emitter_direction(const Interaction &ref,
                                const DirectionSample &ds) const;

    const Sensor *sensor() const { return m_sensor; }
    Sensor *sensor() { return m_sensor; }

    const Integrator *integrator() const { return m_integrator; }
    Integrator *integrator() { return m_integrator; }

    const Emitter *environment() const { return m_environment; }

    const std::vector<ref<Emitter>> &emitters() const { return m_emitters; }
    std::vector<ref<Emitter>> &emitters() { return m_emitters; }

    const std::vector<ref<Shape>> &shapes() const { return m_shapes; }
    std::vector<ref<Shape>> &shapes() { return m_shapes; }

    const BoundingBox3 &bbox() const { return m_bbox; }

    APR_DECLARE_CLASS()
protected:
    ~Scene();

protected:
    void *m_accel = nullptr;
    ref<Integrator> m_integrator;
    ref<Sensor> m_sensor;
    std::vector<ref<Shape>> m_shapes;
    std::vector<ref<Emitter>> m_emitters;
    ref<Emitter> m_environment;
    BoundingBox3 m_bbox;
};

// See interaction.h
template <typename Float, typename Spectrum>
typename SurfaceInteraction<Float, Spectrum>::EmitterPtr
SurfaceInteraction<Float, Spectrum>::emitter(const Scene *scene) const {
    if (is_valid())
        return shape->emitter();
    else
        return scene->environment();
}

extern APR_EXPORT void library_nop();

APR_EXTERN_CLASS(Scene)

} // namespace aspirin