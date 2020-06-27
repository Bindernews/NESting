#include "LoopPointControl.h"
#include "LFOGraphControl.h"

LoopPointControl::LoopPointControl(const IRECT& bounds, LFOGraphControl& parent, const char* label, const IVStyle& style)
  : IControl(bounds), IVectorBase(style), mParent(parent)
{
  AttachIControl(this, label);
  SetNVals(3);

  mDragging = EDrag::kNone;
}

LoopPointControl::~LoopPointControl()
{
}

void LoopPointControl::Draw(IGraphics& g)
{
  DrawBackground(g, mRECT);
  DrawWidget(g);
  DrawLabel(g);
  DrawValue(g, mMouseIsOver);
}

void LoopPointControl::DrawWidget(IGraphics& g)
{
  auto iStyle = &mParent.iStyle;
  mDrawArea = mRECT.GetHPadded(-iStyle->padding);
  mNumBars = mParent.GetNumBars();

  int loopStart = mParent.GetLoopStart();
  int loopEnd = mParent.GetLoopEnd();
  if (loopStart >= loopEnd) {
    loopStart = loopEnd;
  }

  // Draw start flag
  mBStartFlag = mDrawArea.SubRectHorizontal(mNumBars, loopStart);
  mBStartFlag = mBStartFlag.GetFromTLHC(iStyle->loopFlagWidth, iStyle->loopSpace);
  drawFlag(g, iStyle->barColor, mBStartFlag.L, mBStartFlag.T, mBStartFlag.W(), mBStartFlag.H());

  // Draw end flag
  mBEndFlag = mDrawArea.SubRectHorizontal(mNumBars, loopEnd);
  mBEndFlag = mBEndFlag.GetFromTRHC(iStyle->loopFlagWidth, iStyle->loopSpace);
  drawFlag(g, iStyle->barColor, mBEndFlag.R, mBEndFlag.T, -mBEndFlag.W(), mBEndFlag.H());
}

void LoopPointControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  IControl::OnMouseDown(x, y, mod);
  if (mod.L) {
    switch (GetValIdxForPos(x, y)) {
    case 1:
      mDragging = EDrag::kStart;
      break;
    case 2:
      mDragging = EDrag::kEnd;
      break;
    default:
      mDragging = EDrag::kNone;
      break;
    }
  }
}

void LoopPointControl::OnMouseUp(float x, float y, const IMouseMod& mod)
{
  IControl::OnMouseUp(x, y, mod);
  mDragging = EDrag::kNone;
}

void LoopPointControl::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod)
{
  IControl::OnMouseOver(x, y, mod);

  if (mDragging == EDrag::kStart) {
    int barIdx = mParent.GetBarForX(x);
    if (barIdx != -1 && barIdx <= mParent.GetLoopEnd() && barIdx < mNumBars) {
      SetValue(mParent.NormLoopPoint(double(barIdx)), 1);
      SetDirty(true, 1);
    }
  }
  else if (mDragging == EDrag::kEnd) {
    int barIdx = mParent.GetBarForX(x);
    if (barIdx != -1 && barIdx >= mParent.GetLoopStart() && barIdx < mNumBars) {
      SetValue(mParent.NormLoopPoint(double(barIdx)), 2);
      SetDirty(true, 2);
    }
  }
  else {
    SetDirty(false);
  }
}

void LoopPointControl::OnMouseOver(float x, float y, const IMouseMod& mod)
{
  SetDirty(false);
}

void LoopPointControl::OnMouseOut()
{
  IControl::OnMouseOut();
  mDragging = EDrag::kNone;
}

void LoopPointControl::OnMouseDblClick(float x, float y, const IMouseMod& mod)
{
  if (mod.L) {
    int valIdx = GetValIdxForPos(x, y);
    if (valIdx != -1) {
      SetValueToDefault(1);
      SetValueToDefault(2);
    }
  }
}

int LoopPointControl::GetValIdxForPos(float x, float y) const
{
  // If the user double-clicks to reset, always reset the END first.
  if (mBEndFlag.ContainsEdge(x, y)) {
    return 2;
  }
  else if (mBStartFlag.ContainsEdge(x, y)) {
    return 1;
  }
  return -1;
}

void LoopPointControl::drawFlag(IGraphics& g, IColor color, float x, float y, float w, float h)
{
  g.PathTransformReset();
  g.PathTransformTranslate(x, y);
  g.PathTransformScale(w, h);
  g.PathMoveTo(0.0f, 0.0f);
  g.PathLineTo(0.9f, 0.0f);
  g.PathLineTo(0.9f, 0.7f);
  g.PathLineTo(0.0f, 1.0f);
  g.PathLineTo(0.0f, 0.0f);
  g.PathFill(IPattern(color));
  g.PathClose();
  g.PathClear();
}
