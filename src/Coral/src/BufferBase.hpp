#ifndef CORAL_BUFFERBASE_HPP
#define CORAL_BUFFERBASE_HPP

#include <Coral/Buffer.hpp>

#include "ContextBase.hpp"


namespace Coral
{

class BufferBase : public Coral::Buffer
{
public:

    BufferBase(Coral::ContextBase& context)
        : mContext(context)
    { }
    
    ContextBase& context() { return mContext; }

private:

    ContextBase& mContext;

};

} // namespace Coral

#endif // !CORAL_BUFFERBASE_HPP
