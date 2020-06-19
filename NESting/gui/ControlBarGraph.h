#pragma once

#include <IControl.h>
#include <heapbuf.h>
#include "ColorSpec.h"

using namespace iplug;
using namespace iplug::igraphics;

struct LabelPoint
{
	LabelPoint(float value, const char* label = nullptr) : value(value), label(label) {}

	/// A label for the point value (optional)
	const char* label;
	/// The value of the point (range 0.0 - 1.0) (required)
	float value;
};

class ControlBarGraph : public IVectorBase, public IControl
{
public:

	struct Style {
		IColor barColor;
		IRECT labelPadding;
	};

	ControlBarGraph(const IRECT& bounds, const char *label = "", const IVStyle& style = DEFAULT_STYLE);
	~ControlBarGraph();

	void Draw(IGraphics& g) override;
	void DrawWidget(IGraphics& g) override;
	void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override;

	void DrawBars(IGraphics& g, const IRECT& bounds);

	/// Set the stop points.
	///
	/// The point values must be sorted lowest to highest.
	/// 
	/// @param l initializer list of points
	void SetStops(const std::initializer_list<LabelPoint> l);

	void SetSteps(int steps);
	inline int GetSteps() const { return mNumSteps; }

	Style iStyle;

protected:
	int mNumSteps;
	int mLoopPoint;
	/// Are the values continuous, or are they in descrete steps?
	bool mIsContinuous;
	/// The different step values available in this bar graph.
	/// Note that these MUST be normalized to the range [0, 1].
	/// Denormalize them when using them.
	WDL_TypedBuf<LabelPoint> mStops;
	/// The stored values for the bar graph.
	WDL_TypedBuf<float> mValues;
	/// Default value for new steps
	float mDefaultValue;

private:
	/// Maximum width of the labels. If this is -1, it must be re-calculated.
	float mLabelWidth;
	/// Current bounds of the actual bar graph
	IRECT mBarBounds;
};
