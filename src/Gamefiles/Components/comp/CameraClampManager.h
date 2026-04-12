// CameraClampManager.h

#ifndef CAMERACLAMPMANAGER_H
#define CAMERACLAMPMANAGER_H

namespace comp {

struct CameraClampManager {


    bool isThereActiveClamping;

    float camera_x_min;
    float camera_x_max;

    float camera_y_min;
    float camera_y_max;

    float camera_smoothing_override;
    float camera_zoom_override;



};


}


#endif



