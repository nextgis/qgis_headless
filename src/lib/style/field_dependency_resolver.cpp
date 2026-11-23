#include "field_dependency_resolver.h"

#include <qgsexpression.h>
#include <qgsfeaturerequest.h>
#include <qgsvectorlayer.h>

using namespace HeadlessRender;

FieldDependencyResolver::FieldDependencyResolver( const QgsVectorLayer &layer )
  : mLayer( layer ), mFields( layer.fields() )
{}

UsedAttributes FieldDependencyResolver::resolvePhysicalFields(
  const QgsVectorLayer &layer, const std::set<std::string> &attributes
)
{
  FieldDependencyResolver resolver( layer );
  for ( const auto &attribute : attributes )
  {
    if ( !resolver.visitField( QString::fromStdString( attribute ) ) )
      return { false, {} };
  }
  return { true, std::move( resolver.mPhysicalFields ) };
}

bool FieldDependencyResolver::visitField( const QString &name )
{
  if ( name == QgsFeatureRequest::ALL_ATTRIBUTES )
    return false;

  // QGIS handles name and alias precedence.
  const int index = mFields.lookupField( name );
  if ( index < 0 || mVisiting.contains( index ) )
    return false;
  if ( mResolved.contains( index ) )
    return true;

  if ( mFields.fieldOrigin( index ) != Qgis::FieldOrigin::Expression )
    mPhysicalFields.insert( mFields.at( index ).name().toStdString() );
  else if ( !visitExpressionField( index ) )
    return false;

  mResolved.insert( index );
  return true;
}

bool FieldDependencyResolver::visitExpressionField( int index )
{
  const QgsExpression expression( mLayer.expressionField( index ) );
  if ( expression.hasParserError() )
    return false;

  mVisiting.insert( index );
  for ( const auto &dependency : expression.referencedColumns() )
  {
    if ( !visitField( dependency ) )
      return false;
  }
  mVisiting.remove( index );
  return true;
}
