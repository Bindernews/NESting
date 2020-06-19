#pragma once

#include <IControl.h>

using namespace iplug;
using namespace iplug::igraphics;

/**
 * A group of colors used by a control.
 * 
 * @param Size the size of the array of colors.
 */
template<int Size>
class ColorSpec
{
public:
	ColorSpec()
	{
		for (int i = 0; i < Size; i++) {
			colors[i] = COLOR_WHITE;
		}
	}

	inline void set(int index, IColor color)
	{
		if (index < Size) {
			colors[index] = color;
		}
	}

	inline IColor get(int index) const
	{
		if (index < Size) {
			return colors[index];
		}
		else {
			return COLOR_TRANSPARENT;
		}
	}

	inline size_t size() const
	{
		return Size;
	}

private:
	IColor colors[Size];
};
