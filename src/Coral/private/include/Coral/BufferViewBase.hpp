#ifndef CORAL_BUFFERVIEWBASE_HPP
#define CORAL_BUFFERVIEWBASE_HPP

#include <Coral/BufferView.hpp>

#include "ContextBase.hpp"


namespace Coral
{

class BufferViewBase : public Coral::BufferView
{
public:

    BufferViewBase(Coral::ContextBase& context)
        : mContext(context)
    { }
    
    ContextBase& context() { return mContext; }

private:

    ContextBase& mContext;

};

} // namespace Coral

#endif // !CORAL_BUFFERVIEWBASE_HPP
