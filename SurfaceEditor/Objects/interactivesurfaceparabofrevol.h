#ifndef INTERACTIVESURFACEPARABOFREVOL_H
#define INTERACTIVESURFACEPARABOFREVOL_H

#include "interactivesurface.h"

class InteractiveSurfaceParabOfRevolPrivate;

class InteractiveSurfaceParabOfRevol : public InteractiveSurface
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurfaceParabOfRevol, InteractiveSurface)

public:
    InteractiveSurfaceParabOfRevol();
    ~InteractiveSurfaceParabOfRevol();

    Standard_Real getFocal() const;
    gp_Dir getRevolutionDirection() const;
    Standard_Real getUmax() const;
    Standard_Real getVmax() const;

    void setFocal(Standard_Real F);
    void setRevolutionDirection(const gp_Dir &direction);
    void setUmax(Standard_Real U);
    void setVmax(Standard_Real V);

private:
    InteractiveSurfaceParabOfRevolPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfaceParabOfRevol, InteractiveSurface)

#endif // INTERACTIVESURFACEPARABOFREVOL_H
