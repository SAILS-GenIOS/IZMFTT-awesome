#pragma once

// On Linux with Mesa, GL_GLEXT_PROTOTYPES gives direct access to GL 3.3+ functions.
// For cross-platform support, replace this with glad in the future.
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
