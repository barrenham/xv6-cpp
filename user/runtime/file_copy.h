#pragma once

namespace fileutil
{
bool destination(const char *source, const char *target, char *output,
                 unsigned capacity);
int copy(const char *source, const char *target);
} // namespace fileutil
