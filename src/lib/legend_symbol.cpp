#include "legend_symbol.h"

using namespace HeadlessRender;

LegendSymbol::LegendSymbol( const ImagePtr icon, const QString &title )
    : mIcon( icon )
    , mTitle( title )
{

}

ImagePtr LegendSymbol::icon() const
{
    return mIcon;
}

const QString & LegendSymbol::title() const
{
    return mTitle;
}
