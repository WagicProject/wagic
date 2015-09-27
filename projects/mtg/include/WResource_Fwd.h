#ifndef WRESOURCE_FWD_H
#define WRESOURCE_FWD_H

#if (__cplusplus > 199711L)
#include <memory>
typedef std::shared_ptr<JQuad> JQuadPtr;
#elif defined(QT_CONFIG)
#include <QSharedPointer>
class JQuadPtr : public QSharedPointer<JQuad>
{
public:
    JQuadPtr() : QSharedPointer(){};
    JQuadPtr(JQuad*ptr) : QSharedPointer(ptr){};
    JQuad* get() const {return data();};
};
#else
#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<JQuad> JQuadPtr;
#endif

#endif 
