#ifndef MEU_INTERIOR_H
#define MEU_INTERIOR_H
#include <stdbool.h>
#include <stddef.h>
#define MEU_EYE_HEIGHT 1.7f
#define MEU_PLAYER_RADIUS 0.28f
#define MEU_PLAYER_HEIGHT 1.8f
#define MEU_ROBOT_HEIGHT 240.0f
enum { MEU_SOLID = 1, MEU_FLOOR = 2, MEU_HIDDEN = 4 };
typedef enum { MEU_BRIDGE, MEU_QUARTERS, MEU_ENGINEERING, MEU_LIFT } MeuRoom;
typedef struct { float x,y,z,w,h,d,r,g,b; unsigned flags; bool cylinder; } MeuBox;
const MeuBox *meu_interior_boxes(size_t *count);
float meu_deck_height(int deck);
const char *meu_room_name(int room);
bool meu_interior_floor(float x, float z, float feet, float lift_y, float *height);
bool meu_interior_blocked(float x, float feet, float z, float lift_y, float door);
bool meu_interior_clear_ray(float x, float y, float z, float tx, float ty, float tz);
#endif
