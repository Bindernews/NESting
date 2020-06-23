#pragma once

#include <IControl.h>
#include <heapbuf.h>
#include "ColorSpec.h"
#include <slice.hpp>
#include <vector>

using namespace iplug;
using namespace iplug::igraphics;

struct LabelPoint
{
	LabelPoint() : value(0.f), label(nullptr) {}
	LabelPoint(float value, const char* label = nullptr) : value(value), label(label) {}

	/// A label for the point value (optional)
	const char* label;
	/// The value of the point (range 0.0 - 1.0) (required)
	float value;
};

class LFOGraphControl : public IVectorBase, public IControl
{
public:

	struct Style {
		IColor barColor;
		float padding;
		float labelWidth;
		float labelPadding;
	};

	LFOGraphControl(const IRECT& bounds, int maxValues = 64, float defaultValue = 0.5f, 
		const char *label = "", const IVStyle& style = DEFAULT_STYLE);
	~LFOGraphControl();

	void Draw(IGraphics& g) override;
	void DrawWidget(IGraphics& g) override;
	void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override;
	void OnMouseDown(float x, float y, const IMouseMod& mod) override;
	void OnMouseDblClick(float x, float y, const IMouseMod& mod) override;

	int ProcessGraphClick(float x, float y, const IMouseMod& mod);
	void DrawBars(IGraphics& g, const IRECT& bounds);

	/// Set the stop points.
	///
	/// The point values must be sorted lowest to highest.
	/// 
	/// @param l initializer list of points
	void SetStops(const bn::slice<LabelPoint> stops);

	/// Set the number of steps displayed in the LFO Graph.
	/// This must be at least one, and at most `maxValues` as specified in the constructor.
	void SetSteps(int steps);
	
	/// Returns the number of currently displayed steps.
	inline int GetSteps() const { return mNumSteps; }

	inline void SetLoopPoint(int value) { mLoopPoint = value; }
	inline int GetLoopPoint() const { return mLoopPoint; }

	void SetBarValue(int index, float value, bool updateDSP=true);
	bn::slice<float> GetBarValues() const;

	Style iStyle;

protected:
	int mNumSteps;
	int mLoopPoint;
	/// Are the values continuous, or are they in descrete steps?
	bool mIsContinuous;
	/// The different step values available in this bar graph.
	/// Note that these MUST be normalized to the range [0, 1].
	/// Denormalize them when using them.
	std::vector<LabelPoint> mStops;
	/// The stored values for the bar graph.
	std::vector<float> mValues;
	/// Default value for new steps
	float mDefaultValue;

private:
	/// Maximum width of the labels. If this is -1, it must be re-calculated.
	float mLabelWidth;
	/// Current bounds of the actual bar graph
	IRECT mBarBounds;
};
