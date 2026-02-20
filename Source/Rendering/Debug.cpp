#include "Debug.h"

#include "GLHelper.h"

void PushDebugMarker(std::string_view Name)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(Name.size()), Name.data());
}

void PopDebugMarker()
{
	glPopDebugGroup();
}
