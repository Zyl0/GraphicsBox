#pragma once

#include <string_view>
#include "Math/Vector.h"

void PushDebugMarker(std::string_view Name);

void PopDebugMarker();

struct DebugScopeMarker
{
	DebugScopeMarker(std::string_view Name)
	{
		PushDebugMarker(Name);
	}
	~DebugScopeMarker()
	{
		PopDebugMarker();
	}
};