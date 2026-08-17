#include "random_color_generator.h"

using namespace HeadlessRender;

RandomColorGenerator::RandomColorGenerator( SeedType seed, size_t colorsCount )
  : mSeed( seed ), mColorsCount( colorsCount )
{}

RandomColorGenerator::Iterator RandomColorGenerator::begin()
{
  return Iterator( mSeed, 0 );
}

RandomColorGenerator::Iterator RandomColorGenerator::end()
{
  return Iterator( 0, mColorsCount );
}

RandomColorGenerator::Iterator::Iterator( SeedType seed, size_t index )
  : mGenerator( seed ), mIndex( index )
{
  generateColor();
}

const QColor &RandomColorGenerator::Iterator::operator*() const noexcept
{
  return mCurrentColor;
}

void RandomColorGenerator::Iterator::operator++()
{
  generateColor();
  mIndex++;
}

RandomColorGenerator::Iterator::difference_type RandomColorGenerator::Iterator::operator-(
  const Iterator &other
) const noexcept
{
  return mIndex - other.mIndex;
}

bool RandomColorGenerator::Iterator::operator!=( const Iterator &other ) const noexcept
{
  return mIndex != other.mIndex;
}

void RandomColorGenerator::Iterator::generateColor()
{
  mCurrentColor = QColor::fromRgb( mGenerator() );
}