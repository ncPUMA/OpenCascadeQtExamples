#ifndef XCAFSTEPLOADER_H
#define XCAFSTEPLOADER_H

#include <AIS_ListOfInteractive.hxx>

class XCafStepLoaderPrivate;

class XCafStepLoader
{
public:
    XCafStepLoader();
    ~XCafStepLoader();

    bool loadStep(const char *fName);

    AIS_ListOfInteractive result() const;
    TCollection_ExtendedString name(const Handle(AIS_InteractiveObject) &object) const;

private:
    XCafStepLoaderPrivate *d;

    XCafStepLoader(const XCafStepLoader &) = delete;
    XCafStepLoader &operator=(const XCafStepLoader &) = delete;
};

#endif // XCAFSTEPLOADER_H
