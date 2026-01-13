
// Show controls for the demo markers (checkbox, line number, etc.)
void DemoMarker_ShowShortInfo();

// Internal marker callback handling: set line info number, may call the callback
void DemoMarker_HandleCallback(const char* file, int line, const char* section);

#define IMGUI_DEMO_MARKER(section)  do { DemoMarker_HandleCallback("imgui_demo.cpp", __LINE__, section); } while (0)
#define IMGUI_DEMO_MARKER_SHOW_SHORT_INFO() DemoMarker_ShowShortInfo()
