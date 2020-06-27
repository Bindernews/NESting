#pragma once

#include <IControl.h>
#include <heapbuf.h>
#include <slice.hpp>
#include <vector>
#include <functional>

using namespace iplug;
using namespace iplug::igraphics;

class LFOGraphControl;

class LoopPointControl : public IVectorBase, public IControl
{
public:

	enum class EDrag {
		kNone,
		kStart,
		kEnd,
	};

	LoopPointControl(const IRECT& bounds, LFOGraphControl& parent, const char* label = "", const IVStyle& style = DEFAULT_STYLE);
	~LoopPointControl();

	void Draw(IGraphics& g) override;
	void DrawWidget(IGraphics& g) override;
	void OnMouseDown(float x, float y, const IMouseMod& mod) override;
	void OnMouseUp(float x, float y, const IMouseMod& mod) override;
	void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override;
	void OnMouseOver(float x, float y, const IMouseMod& mod) override;
	void OnMouseOut();
	void OnMouseDblClick(float x, float y, const IMouseMod& mod) override;
	int GetValIdxForPos(float x, float y) const override;
	
protected:
	LFOGraphControl& mParent;

private:
	void drawFlag(IGraphics& g, IColor color, float x, float y, float w, float h);

	IRECT mBStartFlag;
	IRECT mBEndFlag;

	/// What is the user currently dragging
	EDrag mDragging;
	/// Cached value of the number of bars
	int mNumBars;
	/// Draw area for the loop point indicators
	IRECT mDrawArea;
};
