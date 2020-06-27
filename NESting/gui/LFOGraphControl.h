#pragma once

#include <IControl.h>
#include <heapbuf.h>
#include <slice.hpp>
#include <vector>
#include <functional>

using namespace iplug;
using namespace iplug::igraphics;

using DisplayMessageCallback = std::function<void(float,WDL_String&)>;
class LoopPointControl;

class LFOGraphControl : public IVectorBase, public IControl
{
public:

	struct Style {
		Style(IColor barColor = COLOR_RED, float padding = 8.0f, float loopSpace = 8.0f, float loopFlagWidth = 8.0f)
			: barColor(barColor), padding(padding), labelWidth(30.0f), labelPadding(6.0f), loopSpace(loopSpace),
			loopFlagWidth(loopFlagWidth) {}

		IColor barColor;
		float padding;
		float labelWidth;
		float labelPadding;
		float loopSpace;
		float loopFlagWidth;
	};

	LFOGraphControl(const IRECT& bounds, int minBars, int maxBars = 64, float defaultValue = 0.5f,
		const char *label = "", const IVStyle& style = DEFAULT_STYLE);
	~LFOGraphControl();

	void Draw(IGraphics& g) override;
	void DrawWidget(IGraphics& g) override;
	void OnAttached() override;
	// N.B This can't actually override the super-class method b/c the superclass method isn't virtual.
	void SetParamIdx(int paramIdx, int valIdx);
	void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override;
	void OnMouseDown(float x, float y, const IMouseMod& mod) override;
	void OnMouseOver(float x, float y, const IMouseMod& mod) override;
	void OnMouseDblClick(float x, float y, const IMouseMod& mod) override;

	int ProcessGraphClick(float x, float y, const IMouseMod& mod);
	/** Get the bar index for the given X value, or -1 if no bar would be selected. */
	int GetBarForX(float x) const;

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
	
	/// Returns the number of currently displayed bars.
	int GetNumBars() const;

	int GetLoopStart() const;
	int GetLoopEnd() const;

	double NormLoopPoint(double v) const;

	void SetBarValue(int index, float value, bool updateDSP=true);
	bn::slice<float> GetBarValues() const;

	inline const IRECT& GetBarBounds() const { return mBarBounds; }

	Style iStyle;

protected:
	int mMinBars;
	int mNumSteps;
	/// The stored values for the bar graph.
	std::vector<float> mValues;
	/// Default value for new steps
	float mDefaultValue;
  /// Callback function used to display values.
  DisplayMessageCallback mDisplayFn;
	/** The loop point control we're using to manage loop points. */
	LoopPointControl* mLoopPointControl;

private:
	float topPadding() const;

	/// Cached value of the number of bars we're displaying. This is updated from the attached parameter.
	int mNumBars;
	/// Buffer for mDisplayFn
  WDL_String mDisplayStr;
	/// Current bounds of the actual bar graph
	IRECT mBarBounds;
};
