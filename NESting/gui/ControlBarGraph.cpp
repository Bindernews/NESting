
#include "ControlBarGraph.h"
#include "math_utils.h"

ControlBarGraph::ControlBarGraph(const IRECT& bounds, int maxSteps, float defaultValue, const char *label,
		const IVStyle& style)
	: IControl(bounds), IVectorBase(style, false, false)
{
	AttachIControl(this, label);
	mNumSteps = 0;
	mIsContinuous = false;
	mDefaultValue = defaultValue;
	mStops.Resize(0, true);
	mLabelWidth = -1.f;
	
	iStyle.barColor = COLOR_RED;
	iStyle.padding = 8.f;
	iStyle.labelWidth = 30.f;
	iStyle.labelPadding = 6.f;

	mStyle.labelText.mAlign = EAlign::Far;
	mStyle.labelText.mVAlign = EVAlign::Middle;

	// Initialize values
	float* values_ = mValues.Resize(maxSteps);
	for (int i = 0; i < maxSteps; i++) {
		values_[i] = mDefaultValue;
	}
}

ControlBarGraph::~ControlBarGraph()
{}

void ControlBarGraph::Draw(IGraphics& g)
{
	DrawBackground(g, mRECT);
	DrawWidget(g);
	DrawLabel(g);
	DrawValue(g, mMouseIsOver);
}

void ControlBarGraph::DrawWidget(IGraphics& g)
{
	// Check if we need to recalc our max label width
	if (mLabelWidth == -1.f) {
		IRECT lblBounds;
		float maxWidth = 0.f;
		for (int i = 0; i < mStops.GetSize(); i += 1) {
			auto stop = mStops.GetFast()[i];
			g.MeasureText(mStyle.labelText, stop.label, lblBounds);
			maxWidth = std::max(maxWidth, lblBounds.W());
		}
		mLabelWidth = maxWidth;
	}
	
	// Start by drawing the scale to the left
	IRECT textBounds = mRECT.GetPadded(-iStyle.padding);
	for (int i = 0; i < mStops.GetSize(); i += 1) {
		auto stop = mStops.GetFast()[i];
		if (stop.label) {
			float y = textBounds.B - int(stop.value * textBounds.H());
			g.DrawText(mStyle.labelText, stop.label, textBounds.L + iStyle.labelWidth, y);
		}
	}

	// Now draw the bars
	mBarBounds = textBounds;
	mBarBounds.L += iStyle.labelWidth + iStyle.labelPadding;
	DrawBars(g, mBarBounds);
}

void ControlBarGraph::DrawBars(IGraphics& g, const IRECT& bounds)
{
	if (mNumSteps == 0) {
		return;
	}
	IRECT box0 = bounds.GetGridCell(0, 1, mNumSteps);

	// Take our bar color and lighten it slightly for the ending bar color
	float h, s, l, a;
	iStyle.barColor.GetHSLA(h, l, s, a);
	l -= 0.2f;
	IColor barColor1 = IColor::FromHSLA(h, l, s, a);

	g.PathTransformSave();
	auto gradient = IPattern::CreateLinearGradient(box0, EDirection::Horizontal, { IColorStop(iStyle.barColor, 0.f), IColorStop(barColor1, 1.f) });
	for (int i = 0; i < mNumSteps; i += 1) {
		float val = mValues.GetFast()[i];
		IRECT bound = box0.FracRectVertical(val, false);
		g.PathRect(bound);
		g.PathFill(gradient);
		g.PathTransformTranslate(box0.W(), 0);
	}
	g.PathTransformRestore();
}

void ControlBarGraph::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod)
{
	// Don't do anything if the mouse is outside the bar graph area.
	if (!mBarBounds.Contains(IRECT::MakeXYWH(x, y, 0.1f, 0.1f))) {
		return;
	}
	if (mod.L) {
		// Determine bar to set value for
		int barIndex = int(lerp(0.f, float(mNumSteps), unlerp(mBarBounds.L, mBarBounds.R, x)));
		// Determine the new bar value
		float barValue = 1.f - unlerp(mBarBounds.T, mBarBounds.B, y);
		// Convert it to be one of the stop values if necessary.
		if (!mIsContinuous) {
			// Find the two closest stop points.
			float stopL = 0;
			float stopH = 1;
			auto stops = mStops.GetFast();
			for (int i = 0; i < mStops.GetSize(); i += 1) {
				if (stops[i].value >= barValue) {
					stopH = stops[i].value;
					if (i > 0) {
						stopL = stops[i - 1].value;
					}
					break;
				}
			}
			// Now that we have our max and min, decide which to round towards
			float halfPoint = (stopL + stopH) / 2.f;
			// Round down if it's exactly in the middle
			if (barValue <= halfPoint) {
				barValue = stopL;
			}
			else {
				barValue = stopH;
			}
		}
		// Now that we've determined our new value, only change it if it's different.
		auto values_ = mValues.GetFast();
		if (values_[barIndex] != barValue) {
			values_[barIndex] = barValue;
			// For performance, we only do this if our value REALLY changed.
			SetDirty(false);
		}
	}
}

void ControlBarGraph::SetSteps(int steps)
{
	if (steps < 0) { return; }
	auto values_ = mValues.Resize(steps);
	if (steps > mNumSteps) {
		for (int i = mNumSteps; i < steps; i += 1) {
			values_[i] = mDefaultValue;
		}
	}
	// Nothing special if we're down-sizing.
	mNumSteps = steps;
}

void ControlBarGraph::SetStops(const bn::slice<LabelPoint> l)
{
	auto ptr = mStops.Resize(l.len(), true);
	for (int i = 0; i < l.len(); i += 1) {
		ptr[i] = l[i];
	}
}
