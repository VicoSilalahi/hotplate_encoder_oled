#pragma once

void Process_Init();
void Process_Update();
// Now accepts two params: Source (Manual vs Preset) and Mode (Auto vs Attended)
void Process_Start(bool useManualSource, bool autoAdvance);
void Process_NextStep();