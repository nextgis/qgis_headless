/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2020 NextGIS, info@nextgis.ru
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <QFile>
#include <QImage>
#include <iostream>
#include <sstream>

#include <lib.h>

template<typename T> T stringToNum( const char *str )
{
  std::stringstream sstream;
  T val;

  sstream << str;
  sstream >> val;

  return val;
}

int main( int argc, char **argv )
{
  if ( argc < 11 )
  {
    std::cout << "Usage: ./qgis filepath_to_geodata filepath_to_style output_path minx miny maxx "
                 "maxy width height epsg \n";
    return EXIT_FAILURE;
  }

  int epsg = stringToNum<int>( argv[10] );

  HeadlessRender::init( argc, argv );

  HeadlessRender::setLoggingLevel( HeadlessRender::LogLevel::Info );

  std::shared_ptr<HeadlessRender::MapRequest> request;
  std::shared_ptr<HeadlessRender::Layer> layer;

  HeadlessRender::CRS crs = HeadlessRender::CRS::fromEPSG( epsg );
  HeadlessRender::Style style = HeadlessRender::Style::fromFile( argv[2] );

  layer.reset( new HeadlessRender::Layer( HeadlessRender::Layer::fromOgr( argv[1] ) ) );
  request.reset( new HeadlessRender::MapRequest );

  request->setDpi( 96 );
  request->setCrs( crs );
  request->addLayer( *layer, style, "layername" );

  HeadlessRender::Extent extent = std::make_tuple(
    stringToNum<double>( argv[4] ), // minx
    stringToNum<double>( argv[5] ), // miny
    stringToNum<double>( argv[6] ), // maxx
    stringToNum<double>( argv[7] )  // maxy
  );

  HeadlessRender::Size size = std::make_tuple(
    stringToNum<int>( argv[8] ), // width
    stringToNum<int>( argv[9] )  // height
  );

  auto imageData = request->renderImage( extent, size );

  QImage
    image( imageData->data(), imageData->sizeWidthHeight().first, imageData->sizeWidthHeight().second, QImage::Format_RGBA8888 );
  image.save( argv[3] + QString( ".tiff" ) );

  request.reset();
  layer.reset();

  HeadlessRender::deinit();
}
