#ifndef VOXEN_INPUT_H
#define VOXEN_INPUT_H

extern float move_speed;
extern float mouse_sensitivity;
extern bool keys[];
extern int mouse_x, mouse_y;
extern int debugView;
extern int debugValue;
extern float cam_fov;

void Input_Init(void);
void Input_MouselookApply();
int Input_KeyDown(uint32_t scancode);
int Input_KeyUp(uint32_t scancode);
int Input_MouseMove(float xrel, float yrel);
void ProcessInput(void);

#endif // VOXEN_INPUT_H
