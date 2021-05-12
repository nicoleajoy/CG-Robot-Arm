#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void computeMatricesFromInputs();
glm::vec3 UpdateBaseTranslate();
glm::vec3 getCameraPosition();
float UpdateTopRotate();
float UpdateArm1Rotate();
float UpdateArm2Rotate();
float UpdatePenLongitudeRotate();
float UpdatePenLatitudeRotate();
float UpdatePenAxisRotate();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif