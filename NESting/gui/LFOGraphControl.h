#pragma once

#include <IControl.h>
#include <heapbuf.h>
#include "ColorSpec.h"
#include <slice.hpp>
#include <vector>
#include <functional>

using namespace iplug;
using namespace iplug::igraphics;

using DisplayMessageCallback = std::function<void(float,WDL_String&)>;

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
    
    
    /// Determine the value of the graph at the given mouse location.
    /// @param x Mouse-X
    /// @param y Mouse-Y
    /// @param out_value value of the mouse at that location
    /// @return The bar index or -1 if the mouse isn't over any bar
    int GetValueAt(float x, float y, float &out_value) const;
    
    void SetDisplayFn(DisplayMessageCallback cb);
    
    /// Set the number of unique values for each bar of the graph.
    /// The step size will be 1/steps.
    void SetNumSteps(int steps);
    
    int GetNumSteps() const;

	/// Set the number of steps displayed in the LFO Graph.
	/// This must be at least one, and at most `maxValues` as specified in the constructor.
	void SetNumBars(int steps);
	
	/// Returns the number of currently displayed steps.
	inline int GetNumBars() const { return mNumSteps; }

  inline void SetLoopStart(int value) { mLoopStart = value; }
  inline int GetLoopStart() const { return mLoopStart; }
	inline void SetLoopEnd(int value) { mLoopEnd = value; }
	inline int GetLoopEnd() const { return mLoopEnd; }

	void SetBarValue(int index, float value, bool updateDSP=true);
	bn::slice<float> GetBarValues() const;

	Style iStyle;

protected:
  int mNumBars;
	int mNumSteps;
	int mLoopStart;
  int mLoopEnd;
	/// The stored values for the bar graph.
	std::vector<float> mValues;
	/// Default value for new steps
	float mDefaultValue;
    /// Callback function used to display values.
    DisplayMessageCallback mDisplayFn;

private:
	/// Buffer for mDisplayFn
    WDL_String mDisplayStr;
	/// Current bounds of the actual bar graph
	IRECT mBarBounds;
};
