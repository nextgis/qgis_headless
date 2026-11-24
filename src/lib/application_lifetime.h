#ifndef QGIS_HEADLESS_APPLICATION_LIFETIME_H
#define QGIS_HEADLESS_APPLICATION_LIFETIME_H

#include <memory>
#include <utility>

namespace HeadlessRender
{
  // Native resources retain the application until their own destructors finish.
  QGIS_HEADLESS_EXPORT std::shared_ptr<void> retainApplication();
  QGIS_HEADLESS_EXPORT bool applicationCleanupSafe();

  template<typename T>
  std::shared_ptr<T> ownQgisResource( T *resource, std::shared_ptr<void> application )
  {
    return std::shared_ptr<T>( resource, [application = std::move( application )]( T *value ) mutable {
      // Embedded Python can release wrappers after C++ thread-local teardown.
      if ( applicationCleanupSafe() )
        delete value;
      application.reset();
    } );
  }

  template<typename T, typename... Args> std::shared_ptr<T> makeQgisResource( Args &&...args )
  {
    auto application = retainApplication();
    return ownQgisResource( new T( std::forward<Args>( args )... ), std::move( application ) );
  }
} //namespace HeadlessRender

#endif
