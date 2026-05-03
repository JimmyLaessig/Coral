#ifndef CORAL_CONTEXT_H
#define CORAL_CONTEXT_H

#include <Coral/System.hpp>
#include <Coral/Core.h>

#include <cstdint>


typedef enum
{
    CO_GRAPHICS_API_VULKAN = 0
} CoGraphicsAPI;


typedef struct
{
    // 
    CoGraphicsAPI graphicsAPI;
    const char* pApplicationName;
} CoContextCreateConfig;


struct CoContext_T;

typedef CoContext_T* CoContext;

CORAL_API CoResult coCreateContext(const CoContextCreateConfig* config, CoContext* context);

CORAL_API CoGraphicsAPI coContextGetGraphicsAPI(const CoContext context);

CORAL_API void coDestroyContext(CoContext context);

#endif // !CORAL_CONTEXT_H
