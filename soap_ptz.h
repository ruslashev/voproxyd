#pragma once

#include "address_manager.h"
#include "soap_global.h"
#include "soap_utils.h"
#include "worker.h"

void soap_ptz_continuous_move(float pan_x, float pan_y, float zoom);
void soap_ptz_goto_home();
void soap_ptz_stop_pantilt();
void soap_ptz_stop_zoom();
void soap_ptz_get_position(float *pan, float *tilt, float *zoom);

