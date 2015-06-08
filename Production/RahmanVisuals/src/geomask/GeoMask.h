#ifndef _GEO_MASK_
#define _GEO_MASK_
#include <memory>

using namespace std;

class GeoMask;
typedef unique_ptr<GeoMask> GeoMaskPtr;

class GeoMask
{
public:
	static GeoMaskPtr create() { return GeoMaskPtr(new GeoMask()); }
	~GeoMask();

	void Update();
	void Draw();

protected:
	GeoMask();

private:
};
#endif