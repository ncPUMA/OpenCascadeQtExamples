#ifndef INTERACTIVESURFACE_H
#define INTERACTIVESURFACE_H

#include "interactiveobject.h"

namespace ExamplesBase {

class InteractiveSurfacePrivate;

class InteractiveSurface : public InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurface, InteractiveObject)

public:
    enum SurfaceTypes
    {
        SurfaceTypeSurface,
        SurfaceTypeGlass,
        SurfaceTypeMirror,
    };

public:
    InteractiveSurface();
    ~InteractiveSurface();

    SurfaceTypes surfaceType() const;
    void setSurfaceType(SurfaceTypes surfaceType);

private:
    InteractiveSurfacePrivate *d;
    friend class InteractiveSurfacePrivate;
};

DEFINE_STANDARD_HANDLE(InteractiveSurface, InteractiveObject)

}

#endif // INTERACTIVESURFACE_H
