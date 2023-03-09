#ifndef INTERACTIVESURFACEREVOLUTION_H
#define INTERACTIVESURFACEREVOLUTION_H

#include "interactivesurface.h"

class Geom_Curve;
class InteractiveSurfaceRevolutionPrivate;

class InteractiveSurfaceRevolution : public InteractiveSurface
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurfaceRevolution, InteractiveSurface)

public:
    InteractiveSurfaceRevolution();
    ~InteractiveSurfaceRevolution();

    gp_Dir getRevolutionDirection() const;
    Standard_Real getUmax() const;
    Standard_Real getVmax() const;

    void setRevolutionDirection(const gp_Dir &direction);
    void setUmax(Standard_Real U);
    void setVmax(Standard_Real V);

protected:
    void updateSurface(const Handle(Geom_Curve) &curve);
    virtual Handle(Geom_Curve) getCurve() const = 0;

private:
    InteractiveSurfaceRevolutionPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfaceRevolution, InteractiveSurface)

#endif // INTERACTIVESURFACEREVOLUTION_H
