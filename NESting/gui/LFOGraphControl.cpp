
#include "LFOGraphControl.h"
#include "math_utils.h"

#define VERTICAL_MOUSE_PADDING (2.f)

LFOGraphControl::LFOGraphControl(const IRECT& bounds, int maxSteps, float defaultValue, const char *label,
		const IVStyle& style)
	: IControl(bounds), IVectorBase(style, false, false)
{
	AttachIControl(this, label);
	mNumSteps = 1;
  mNumBars = 0;
	mDefaultValue = defaultValue;
	mValues.resize(maxSteps, defaultValue);
  mLoopStart = 1.0f;
  mLoopEnd = 1.0f;
  mDisplayFn = [](float value, WDL_String& msg) { msg.SetFormatted(20, "%.2f", value); };
	
	iStyle.barColor = COLOR_RED;
	iStyle.padding = 8.f;
	iStyle.labelWidth = 30.f;
	iStyle.labelPadding = 6.f;

	mStyle.labelText.mAlign = EAlign::Center;
	mStyle.labelText.mVAlign = EVAlign::Top;
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
	// Now draw the bars
  mBarBounds = mRECT.GetPadded(-iStyle.padding);
  mBarBounds.B -= iStyle.padding;
	DrawBars(g, mBarBounds);

	// Finally draw the loop point indicator
	IRECT loopBox = mBarBounds.SubRectHorizontal(mNumBars, mLoopStart);
	loopBox.T = mRECT.T;
	loopBox.B = mRECT.T - 1.f;
	loopBox.R = mBarBounds.R;
	g.FillRect(iStyle.barColor, loopBox);

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
    SetDirty();
  }
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

void LFOGraphControl::DrawBars(IGraphics& g, const IRECT& bounds)
{
	if (mNumSteps == 0) {
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

void LFOGraphControl::SetNumBars(int steps)
{
	if (steps < 1 || steps > mValues.size()) { return; }
	if (mNumBars == steps) { return; }
	mNumBars = steps;
	SetDirty(false);
}

bn::slice<float> LFOGraphControl::GetBarValues() const
{
	return bn::slice(mValues.data(), mValues.size());
}

void LFOGraphControl::SetDisplayFn(DisplayMessageCallback cb)
{
    mDisplayFn = cb;
}
