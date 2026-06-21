#ifndef CORAL_BUFFER_HPP
#define CORAL_BUFFER_HPP

#include <Coral/Buffer.h>

#include <cstddef>
#include <memory>

namespace Coral
{

/*!
 *
 */
class Buffer
{
public:

    using CreateConfig = CoBufferCreateConfig;

    enum CreateError
    {
        INTERNAL_ERROR = CO_ERROR_INTERNAL,
        OUT_OF_MEMORY  = CO_ERROR_OUT_OF_MEMORY,
        INVALID_SIZE   = CO_ERROR_INVALID_SIZE,
    };

    virtual ~Buffer() = default;

    /*!
     * \brief Get the size of the buffer in bytes
     */
    virtual size_t size() const = 0;

    /*! 
     * \brief Get the type of the buffer
     */ 
    virtual CoBufferType type() const = 0;

    /*! 
     * \brief Map the buffer memory to CPU-accessible memory. 
     * 
     * This function only returns a valid pointer if the buffer was created with the \p cpuVisible option enabled. The
     * memory CPU <-> GPU memory synchronization functions \ref map and \ref unmap should not be called whilst the
     * buffer is in use by a command buffer.
     * 
     * \return Returns a pointer to the start of the mapped memory
     */
    virtual std::byte* map() = 0;

    /*!
     * \brief Unmap the buffer memory, mapped previously by \ref map.
     * 
     * Updates to the buffer's CPU data are only synchronized with the buffer's GPU data once \ref unmap is called.
     * 
     * \return True of the buffer was unmapped successfully, false otherwise.
     */
    virtual bool unmap() = 0;

}; // class Buffer

} // namespace Coral

/*!
 * 
 */
struct CoBuffer_T
{
    std::shared_ptr<Coral::Buffer> impl;
};

#endif // !CORAL_BUFFER_HPP
