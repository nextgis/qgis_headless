#include "random_device.h"

using namespace HeadlessRender;

RandomDevice::RandomDevice( RandomDevice::SeedType seed ) noexcept : mSeed( seed )
{}

RandomDevice::SeedType RandomDevice::seed() const noexcept
{
  return mSeed;
}
