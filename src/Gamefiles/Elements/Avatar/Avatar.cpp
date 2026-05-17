#include "Avatar.h"

#include <fstream>
#include <sstream>

// =============================================================
// Avatar Interpolation Drawing Definitions:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

KeyAnimFrame::KeyAnimFrame (const Avatar& _avatar) {
    for (int i = 0; i < _avatar.default_frame.joints.size(); i++) {
        // Default draw order should for a key animation frame
        // match the anchor-frame joint order for consistency.
        joints.push_back(AnimJointAdjustmentFrame({0.0f, 0.0f}, 0.0f, i));
        joints.back().unique_id = i;

    }
}


void Avatar::DrawAvatar(Vec2 position, float scale, const Animation& animation, int tick) {

    


    

}



bool Avatar::SaveAvrFile (const std::string& filename, const std::string& path) {

    std::string fullpath = path + filename + ".avr";

    std::ofstream file(fullpath);


    if (!file.is_open())
        return false;


    // Items to serialize

    // --> Avatar's Name (not the same as the filename)     || std::string
    // --> Anchor Frame                                     || Avatar::AvatarTextureRig
    // --> Default Texturing                                || Avatar::KeyFrame

    // --------------------------
    // :: AvatarTextureRig
    // --------------------------
    // ----> std::vector<AvatarJoints> [Joint Count, Joint Data]

    // :: AvatarJoint -->
    // ----> Joint Name        || std::string
    // ----> Texture Path      || std::string
    // ----> Offset            || Vec2
    // ----> Scale             || Float
    // ----> Rotation          || Float
    // ----> Crop Min          || Vec2
    // ----> Crop Max          || Vec2
    

    // ---------------------------
    // :: KeyFrame 
    // ---------------------------
    // ----> std::vector<JointFramePosition>

    // :: JointFramePosition -->
    // ----> Unique ID      || int
    // ----> Origin         || Vec2
    // ----> Direction      || Vec2

    // ==============================================================================================
    // ==> Serialize Version
    // --------------------------->


    const std::string version = "VERSION_1";

    file << "~AVATAR_FILE" << '\n';
    file << version << '\n';

    // ==============================================================================================
    // ==> Serialize Avatar
    // --------------------------->
    
    const std::string avatar_name = name;

    file << "~AVATAR_NAME" << '\n';
    file << name << '\n';

    file << "~DEFAULT_TEXTURING" << '\n';

    const int number_of_joints = default_texturing.joints.size();
    file << number_of_joints << '\n';

    for (int i = 0; i < number_of_joints; i++) {

        const std::string jname = default_texturing.joints[i].name;
        const std::string jpath = default_texturing.joints[i].texturePath;
        const float joffset_x = default_texturing.joints[i].offset.x;
        const float joffset_y = default_texturing.joints[i].offset.y;
        const float jscale_x = default_texturing.joints[i].scale.x;
        const float jscale_y = default_texturing.joints[i].scale.y;
        const float jrotation = default_texturing.joints[i].rotation;
        const float jcmin_x = default_texturing.joints[i].crop_min.x;
        const float jcmin_y = default_texturing.joints[i].crop_min.y;
        const float jcmax_x = default_texturing.joints[i].crop_max.x;
        const float jcmax_y = default_texturing.joints[i].crop_max.y;

        file << jname << '\n';
        
        file << jpath << '\n';

        file << joffset_x << " " << joffset_y << " " << jscale_x << " " << jscale_y << " " << jrotation << " " << jcmin_x << " " << jcmin_y << " " << jcmax_x  << " " << jcmax_y << '\n';


    }

    

    file << "~ANCHOR_FRAME" << '\n';

    
    for (int i = 0; i < number_of_joints; i++) {

        const int juniqueid = default_frame.joints[i].unique_id;
        const float jox = default_frame.joints[i].origin.x;
        const float joy = default_frame.joints[i].origin.y;
        const float jdx = default_frame.joints[i].direction.x;
        const float jdy = default_frame.joints[i].direction.y;

        file << juniqueid << " " << jox << " " << joy << " " << jdx << " " << jdy << '\n';

    }

    file << "~END_OF_FILE" << '\n';



    return true;

}

bool Avatar::LoadAvrFile (const std::string& filename, const std::string& path) {

    std::string fullpath = path + filename + ".avr";
    

    std::ifstream file(fullpath);

    if (!file.is_open())
        return false;

    std::string token;

    // =========================================================================================
    // ==> FILE HEADER
    // =========================================================================================

    file >> token;

    if (token != "~AVATAR_FILE")
        return false;

    std::string version;
    file >> version;

    const bool version_1 = (version == "VERSION_1");

    if (!version_1)
        return false;

    // =========================================================================================
    // ==> AVATAR NAME
    // =========================================================================================

    file >> token;

    if (token != "~AVATAR_NAME")
        return false;


    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(file, token, '\n');

    name = token;

    // =========================================================================================
    // ==> DEFAULT TEXTURING
    // =========================================================================================

    file >> token;

    if (token != "~DEFAULT_TEXTURING")
        return false;


    int number_of_joints = 0;

    file >> number_of_joints;

    default_texturing.joints.clear();

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < number_of_joints; i++) {
        AvatarJoint joint;

        std::getline(file, joint.name, '\n');

        std::getline(file, joint.texturePath, '\n');
        file >> joint.offset.x
                >> joint.offset.y
                >> joint.scale.x
                >> joint.scale.y
                >> joint.rotation
                >> joint.crop_min.x
                >> joint.crop_min.y
                >> joint.crop_max.x
                >> joint.crop_max.y;
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        default_texturing.joints.push_back(joint);
    }

    // =========================================================================================
    // ==> ANCHOR FRAME
    // =========================================================================================

    file >> token;

    if (token != "~ANCHOR_FRAME")
        return false;

    default_frame.joints.clear();

    for (int i = 0; i < number_of_joints; i++) {
        JointFramePosition frame_joint;

        file >> frame_joint.unique_id
             >> frame_joint.origin.x
             >> frame_joint.origin.y
             >> frame_joint.direction.x
             >> frame_joint.direction.y;

        default_frame.joints.push_back(frame_joint);
    }

    // =========================================================================================
    // ==> END OF FILE
    // =========================================================================================

    file >> token;

    if (token != "~END_OF_FILE")
        return false;

    return true;



}