#ifndef INTERACTIVESURFACEPLANE_H
#define INTERACTIVESURFACEPLANE_H

#include "interactivesurface.h"

class InteractiveSurfacePlanePrivate;

class InteractiveSurfacePlane : public InteractiveSurface
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurfacePlane, InteractiveSurface)
public:
    InteractiveSurfacePlane();
    ~InteractiveSurfacePlane();

    Standard_Real getUmax() const;
    Standard_Real getVmax() const;
    void setUmax(Standard_Real U);
    void setVmax(Standard_Real V);

private:
    InteractiveSurfacePlanePrivate *d;

private:
    InteractiveSurfacePlane(const InteractiveSurfacePlane &) = delete;
    InteractiveSurfacePlane& operator=(InteractiveSurfacePlane &) = delete;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfacePlane, InteractiveSurface)

#endif // INTERACTIVESURFACEPLANE_H
