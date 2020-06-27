
#include "LFOGraphControl.h"
#include "math_utils.h"
#include "LoopPointControl.h"

#define VERTICAL_MOUSE_PADDING (2.f)

LFOGraphControl::LFOGraphControl(const IRECT& bounds, int minBars, int maxBars, float defaultValue, const char *label,
		const IVStyle& style)
	: IControl(bounds), IVectorBase(style, false, false)
{
	AttachIControl(this, label);
	mNumSteps = 1;
  mNumBars = 0;
	mDefaultValue = defaultValue;
	mMinBars = minBars;
	mValues.resize(maxBars, defaultValue);
  mDisplayFn = [](float value, WDL_String& msg) { msg.SetFormatted(20, "%.2f", value); };

	mStyle.labelText.mAlign = EAlign::Center;
	mStyle.labelText.mVAlign = EVAlign::Top;

	mLoopPointControl = nullptr;

	SetNVals(3);
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
	// Update our cached value
	mNumBars = GetNumBars();
	int idxLoopStart = GetLoopStart();
	int idxLoopEnd = GetLoopEnd();

	// Now draw the bars
  mBarBounds = mRECT.GetPadded(-iStyle.padding, -topPadding(), -iStyle.padding, -iStyle.padding);
  mBarBounds.B -= iStyle.padding;
	DrawBars(g, mBarBounds);

	// Draw loop lines
	IRECT bLoopStart = mBarBounds.SubRectHorizontal(mNumBars, idxLoopStart);
	IRECT bLoopEnd = mBarBounds.SubRectHorizontal(mNumBars, idxLoopEnd);
	g.DrawVerticalLine(iStyle.barColor, bLoopStart.L, mBarBounds.B, mRECT.T);
	g.DrawVerticalLine(iStyle.barColor, bLoopEnd.R, mBarBounds.B, mRECT.T);

  // Draw the value at the current mouse position
  if (mMouseIsOver) {
    float mX, mY;
    GetUI()->GetMouseLocation(mX, mY);
    float bVal;
    int bar = GetValueAt(mX, mY, bVal);
    if (bar != -1) {
      IRECT place = mBarBounds.SubRectHorizontal(mNumBars, bar);
      mDisplayFn(bVal, mDisplayStr);
      g.DrawText(mStyle.labelText, mDisplayStr.Get(), place.MW(), place.B + 2.f);
    }
    // If the mouse is over us, we have to update every frame
    //SetDirty();
  }
}

void LFOGraphControl::OnAttached()
{
	// Once we're attached, create a sub-control and set parameters accordingly.
	mLoopPointControl = new LoopPointControl(mRECT.GetFromTop(topPadding()), *this);
	GetUI()->AttachControl(mLoopPointControl, -1, GetGroup());
	for (int i = 0; i < NVals(); i++) {
		mLoopPointControl->SetParamIdx(GetParamIdx(i), i);
	}
}

void LFOGraphControl::SetParamIdx(int paramIdx, int valIdx)
{
	if (mLoopPointControl) {
		mLoopPointControl->SetParamIdx(paramIdx, valIdx);
	}
	IControl::SetParamIdx(paramIdx, valIdx);
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
    float barValue;
    int barIndex = this->GetValueAt(x, y, barValue);
		SetBarValue(barIndex, barValue);
		// Return the clicked index
		return barIndex;
	}
	// If all else fails, we didn't do anything, so return -1.
	return -1;
}

int LFOGraphControl::GetBarForX(float x) const
{
	if (!mBarBounds.ContainsEdge(x, mBarBounds.T + 0.1)) {
		return -1;
	}
	return int(lerp(0.f, float(mNumBars), unlerp(mBarBounds.L, mBarBounds.R, x)));
}

void LFOGraphControl::DrawBars(IGraphics& g, const IRECT& bounds)
{
	if (mNumSteps == 0 || mNumBars == 0) {
		return;
	}
	IRECT box0 = bounds.SubRectHorizontal(mNumBars, 0);

	// Take our bar color and lighten it slightly for the ending bar color
	float h, s, l, a;
	iStyle.barColor.GetHSLA(h, l, s, a);
	l -= 0.2f;
	IColor barColor1 = IColor::FromHSLA(h, l, s, a);

	g.PathTransformSave();
	auto gradient = IPattern::CreateLinearGradient(box0, EDirection::Horizontal, { IColorStop(iStyle.barColor, 0.f), IColorStop(barColor1, 1.f) });
	for (int i = 0; i < mNumBars; i += 1) {
		// For drawing purposes we at least make a tiny bar
		float val = std::max(mValues[i], 0.01f);
		IRECT bound = box0.FracRectVertical(val, false);
		g.PathRect(bound);
		g.PathFill(gradient);
		g.PathTransformTranslate(box0.W(), 0);
	}
	g.PathTransformRestore();
}

int LFOGraphControl::GetValueAt(float x, float y, float& out_value) const
{
    // Make sure the mouse is within the bar graph area, with a little allowance on the top and bottom.
    float allowed = iStyle.padding / 2.f;
    IRECT mouseBounds = mBarBounds.GetAltered(0., -allowed, 0., allowed);
    if (!mouseBounds.ContainsEdge(x, y)) {
        return -1;
    }

    // Determine bar to set value for
    int barIndex = int(lerp(0.f, float(mNumBars), unlerp(mBarBounds.L, mBarBounds.R, x)));
    // Determine the new bar value
    float barValue = 1.f - clampf(0.f, 1.f, unlerp(mBarBounds.T, mBarBounds.B, y));
    if (mNumSteps == 0) {
        out_value = barValue;
    }
    else {
        out_value = make_stepped(barValue, float(mNumSteps));
    }
    return barIndex;
}

void LFOGraphControl::SetBarValue(int index, float value, bool updateDSP)
{
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

void LFOGraphControl::OnMouseOver(float x, float y, const IMouseMod& mod)
{
	IControl::OnMouseOver(x, y, mod);
	ProcessGraphClick(x, y, mod);
	SetDirty(false);
}

void LFOGraphControl::OnMouseDblClick(float x, float y, const IMouseMod& mod)
{
  float nvalue;
	int barIndex = GetValueAt(x, y, nvalue);
	if (barIndex != -1) {
		SetBarValue(barIndex, mDefaultValue, true);
	}
}

void LFOGraphControl::SetNumSteps(int steps)
{
    mNumSteps = steps;
    SetDirty(false);
}

bn::slice<float> LFOGraphControl::GetBarValues() const
{
	return bn::slice(mValues.data(), mValues.size());
}

float LFOGraphControl::topPadding() const
{
	return iStyle.loopSpace + 2.0f;
}

void LFOGraphControl::SetDisplayFn(DisplayMessageCallback cb)
{
    mDisplayFn = cb;
}

int LFOGraphControl::GetNumBars() const
{
	return int(lerp(double(mMinBars), double(mValues.size()), GetValue(0)));
}

int LFOGraphControl::GetLoopStart() const
{
	return int(lerp(0.0, double(mValues.size() - 1), GetValue(1)));
}

int LFOGraphControl::GetLoopEnd() const
{
	return int(lerp(0.0, double(mValues.size() - 1), GetValue(2)));
}

double LFOGraphControl::NormLoopPoint(double v) const
{
	return unlerp(0.0, double(mValues.size() - 1), v);
}
