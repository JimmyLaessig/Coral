#ifndef CORAL_IMAGEBASE_HPP
#define CORAL_IMAGEBASE_HPP

#include <Coral/Image.hpp>

#include "ContextBase.hpp"

namespace Coral
{

    class ImageBase : public Coral::Image
    {
    public:

        ImageBase(Coral::ContextBase& context)
            : mContext(context)
        {
        }

        ContextBase& context() { return mContext; }

    private:

        ContextBase& mContext;

    };

} // namespace Coral

#endif // !CORAL_IMAGEBASE_HPP
