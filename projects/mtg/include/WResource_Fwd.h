#ifndef WRESOURCE_FWD_H
#define WRESOURCE_FWD_H

#if !defined(WP8) && !(defined(SDL_CONFIG) && defined(__MINGW32__))
#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<JQuad> JQuadPtr;
#else
#include <memory>
typedef std::shared_ptr<JQuad> JQuadPtr;
#endif

#endif 
