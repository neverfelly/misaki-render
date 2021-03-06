#pragma once

#include "fwd.h"
#include "object.h"

namespace aspirin {

class APR_EXPORT Texture : public Object {
public:
    virtual Float eval_1(const SurfaceInteraction &si) const;
    virtual Color3 eval_3(const SurfaceInteraction &si) const;
    virtual Float mean() const;

    APR_DECLARE_CLASS()
protected:
    Texture(const Properties &props);
    virtual ~Texture();

protected:
    std::string m_id;
};

} // namespace aspirin