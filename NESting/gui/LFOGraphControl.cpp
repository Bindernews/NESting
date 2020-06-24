
#include "LFOGraphControl.h"
#include "math_utils.h"

#define VERTICAL_MOUSE_PADDING (2.f)

LFOGraphControl::LFOGraphControl(const IRECT& bounds, int maxSteps, float defaultValue, const char *label,
		const IVStyle& style)
	: IControl(bounds), IVectorBase(style, false, false)
{
	AttachIControl(this, label);
	mNumSteps = 0;
	mIsContinuous = false;
	mDefaultValue = defaultValue;
	mValues.resize(maxSteps, defaultValue);
	mStops.resize(0);
	mLabelWidth = -1.f;
	mLoopPoint = 1.0f;
	
	iStyle.barColor = COLOR_RED;
	iStyle.padding = 8.f;
	iStyle.labelWidth = 30.f;
	iStyle.labelPadding = 6.f;

	mStyle.labelText.mAlign = EAlign::Far;
	mStyle.labelText.mVAlign = EVAlign::Middle;
}

LFOGraphControl::~LFOGraphControl()
{}

void LFOGraphControl::Draw(IGraphics& g)
{
	DrawBackground(g, mRECT);
	DrawWidget(g);
	DrawLabel(g);
	DrawValue(g, mMouseIsOver);
}

void LFOGraphControl::DrawWidget(IGraphics& g)
{
	// Check if we need to recalc our max label width
	if (mLabelWidth == -1.f) {
		IRECT lblBounds;
		float maxWidth = 0.f;
		for (int i = 0; i < mStops.size(); i += 1) {
			auto stop = mStops[i];
			g.MeasureText(mStyle.labelText, stop.label, lblBounds);
			maxWidth = std::max(maxWidth, lblBounds.W());
		}
		mLabelWidth = maxWidth;
	}
	
	// Start by drawing the scale to the left
	IRECT textBounds = mRECT.GetPadded(-iStyle.padding);
	for (int i = 0; i < mStops.size(); i += 1) {
		auto stop = mStops[i];
		if (stop.label) {
			float y = textBounds.B - int(stop.value * textBounds.H());
			g.DrawText(mStyle.labelText, stop.label, textBounds.L + iStyle.labelWidth, y);
		}
	}

	// Now draw the bars
	mBarBounds = textBounds;
	mBarBounds.L += iStyle.labelWidth + iStyle.labelPadding;
	DrawBars(g, mBarBounds);

	// Finally draw the loop point indicator
	IRECT loopBox = mBarBounds.SubRectHorizontal(mNumSteps, mLoopPoint);
	loopBox.T = mRECT.T;
	loopBox.B = mRECT.T - 1.f;
	loopBox.R = mBarBounds.R;
	g.FillRect(iStyle.barColor, loopBox);

}

int LFOGraphControl::ProcessGraphClick(float x, float y, const IMouseMod& mod)
{
	// Make sure the mouse is within the bar graph area, with a little allowance on the top and bottom.
	float allowed = iStyle.padding / 2.f;
	IRECT mouseBounds = mBarBounds.GetAltered(0., -allowed, 0., allowed);
	if (!mouseBounds.ContainsEdge(x, y)) {
		return -1;
	}
	// We only respond to left-clicks.
	if (mod.L) {
		// Determine bar to set value for
		int barIndex = int(lerp(0.f, float(mNumSteps), unlerp(mBarBounds.L, mBarBounds.R, x)));
		// Determine the new bar value
		float barValue = 1.f - clampf(0.f, 1.f, unlerp(mBarBounds.T, mBarBounds.B, y));
		SetBarValue(barIndex, barValue);
		// Return the clicked index
		return barIndex;
	}
	// If all else fails, we didn't do anything, so return -1.
	return -1;
}

void LFOGraphControl::DrawBars(IGraphics& g, const IRECT& bounds)
{
	if (mNumSteps == 0) {
		return;
	}
	IRECT box0 = bounds.SubRectHorizontal(mNumSteps, 0);

	// Take our bar color and lighten it slightly for the ending bar color
	float h, s, l, a;
	iStyle.barColor.GetHSLA(h, l, s, a);
	l -= 0.2f;
	IColor barColor1 = IColor::FromHSLA(h, l, s, a);

	g.PathTransformSave();
	auto gradient = IPattern::CreateLinearGradient(box0, EDirection::Horizontal, { IColorStop(iStyle.barColor, 0.f), IColorStop(barColor1, 1.f) });
	for (int i = 0; i < mNumSteps; i += 1) {
		// For drawing purposes we at least make a tiny bar
		float val = std::max(mValues[i], 0.01f);
		IRECT bound = box0.FracRectVertical(val, false);
		g.PathRect(bound);
		g.PathFill(gradient);
		g.PathTransformTranslate(box0.W(), 0);
	}
	g.PathTransformRestore();
}

void LFOGraphControl::SetBarValue(int index, float value, bool updateDSP)
{
	// Convert it to be one of the stop values if necessary.
	if (!mIsContinuous) {
		// Find the two closest stop points.
		float stopL = 0;
		float stopH = 1;
		for (size_t i = 0; i < mStops.size(); i++) {
			if (mStops[i].value >= value) {
				stopH = mStops[i].value;
				if (i > 0) {
					stopL = mStops[i - 1].value;
				}
				break;
			}
		}
		// Now that we have our max and min, decide which to round towards
		float halfPoint = (stopL + stopH) / 2.f;
		// Round down if it's exactly in the middle
		if (value <= halfPoint) {
			value = stopL;
		}
		else {
			value = stopH;
		}
	}
	// Now that we've determined our new value, only change it if it's different.
	if (mValues[index] != value) {
		mValues[index] = value;
		// For performance, we only do this if our value REALLY changed.
		SetDirty(false);
		// Once a bar value has changed, we need to notify people.
		auto actionFn = GetActionFunction();
		if (actionFn && updateDSP) {
			actionFn(this);
		}
	}
}

void LFOGraphControl::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod)
{
	ProcessGraphClick(x, y, mod);
}

void LFOGraphControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
	ProcessGraphClick(x, y, mod);
}

void LFOGraphControl::OnMouseDblClick(float x, float y, const IMouseMod& mod)
{
	int barIndex = ProcessGraphClick(x, y, mod);
	if (barIndex != -1) {
		SetBarValue(barIndex, mDefaultValue, true);
	}
}

void LFOGraphControl::SetSteps(int steps)
{
	if (steps < 1 || steps > mValues.size()) { return; }
	if (mNumSteps == steps) { return; }
	mNumSteps = steps;
	SetDirty(false);
}

void LFOGraphControl::SetStops(const bn::slice<LabelPoint> l)
{
	mStops.assign(l.data(), l.data() + l.len());
}

bn::slice<float> LFOGraphControl::GetBarValues() const
{
	return bn::slice(mValues.data(), mValues.size());
}
