#pragma once
#include <nanovg/nanovg.h>


// To disable a scrollbar set max_scroll to 0 or a negative number
// Scrollbars will otherwise always show, even if there is nowhere
// to scroll
// draw Disables the graphics of the scrollbar, but not function
// enabled Disables the function and graphics
struct GUIScrollbar
{
	bool draw;
	bool enabled;

	int scroll;
	int max_scroll;
	// Positive pos changes the scrollbar from the left or top to the right or bottom
	bool positive_pos; 
	int width;
	NVGcolor color = nvgRGBA(100, 100, 100, 255);
	NVGcolor scroller_color = nvgRGBA(170, 170, 170, 255);
	NVGcolor scroller_sel_color = nvgRGBA(255, 255, 255, 255);

};

