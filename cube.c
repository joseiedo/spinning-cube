#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
void usleep(__int64 usec) {
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative
                              // value indicates relative time

  SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
}
#endif

// To give the 3D depth
int CAM_DISTANCE = 100;
float CUBE_SCALE = 20;
float INCREMENT_SPEED = 0.3;

float rollAngle;  //  X-axis rotation
float pitchAngle; // Y-axis rotation
float yawAngle;   // Z-Axis rotation
float pointX3D, pointY3D, pointZ3D;
int pointX2D, pointY2D;
int pointIndex;
float closeness;

int screenWidth = 80, screenHeight = 22;
float cubeWidth = 20;
float horizontalOffSet;
// Buffer to deal with the depth, to know if something if in front or behind
float zBuffer[80 * 22];
// Buffer to store the current frame
char output[80 * 22];

/**
 *  See: https://en.wikipedia.org/wiki/Rotation_matrix#Basic_3D_rotations
 */
float calculateRotatedXPoint(int x, int y, int z) {
  return y * sin(rollAngle) * sin(pitchAngle) * cos(yawAngle) -
         z * cos(rollAngle) * sin(pitchAngle) * cos(yawAngle) +
         y * cos(rollAngle) * sin(yawAngle) +
         z * sin(rollAngle) * sin(yawAngle) +
         x * cos(pitchAngle) * cos(yawAngle);
}
float calculateRotatedYPoint(int x, int y, int z) {
  return y * cos(rollAngle) * cos(yawAngle) +
         z * sin(rollAngle) * cos(yawAngle) -
         y * sin(rollAngle) * sin(pitchAngle) * sin(yawAngle) +
         z * cos(rollAngle) * sin(pitchAngle) * sin(yawAngle) -
         x * cos(pitchAngle) * sin(yawAngle);
}
float calculateRotatedZPoint(int x, int y, int z) {
  return z * cos(rollAngle) * cos(pitchAngle) -
         y * sin(rollAngle) * cos(pitchAngle) + x * sin(pitchAngle);
}

/**
 * Receives the x, y, z points in the 3D space, converts it to 2d
 * and add to the buffer.
 */
void computeAndDrawPixel(float x, float y, float z, char ch) {
  pointX3D = calculateRotatedXPoint(x, y, z);
  pointY3D = calculateRotatedYPoint(x, y, z);
  pointZ3D = calculateRotatedZPoint(x, y, z) + CAM_DISTANCE;

  closeness = 1 / pointZ3D;

  // Projecting to 2D space
  pointX2D = (int)(screenWidth / 2 + horizontalOffSet +
                   pointX3D * closeness * CUBE_SCALE * 2);
  pointY2D = (int)(screenHeight / 2 + pointY3D * closeness * CUBE_SCALE);

  // Adding the pixel to the buffer
  // Needed to multiplicate by the width because
  // i'm using a 1-D array in the buffer
  pointIndex = pointX2D + pointY2D * screenWidth;
  if (pointIndex >= 0 && pointIndex < screenWidth * screenHeight) {
    if (closeness > zBuffer[pointIndex]) {
      zBuffer[pointIndex] = closeness;
      output[pointIndex] = ch;
    }
  }
}

/**
 * See: https://en.wikipedia.org/wiki/3D_projection#Diagram
 */

int main() {
  // Clear terminal and escape
  printf("\x1b[2J");
  while (1) {
    memset(output, ' ', screenWidth * screenHeight);
    memset(zBuffer, 0, screenWidth * screenHeight * 4);
    cubeWidth = 20;
    horizontalOffSet = -cubeWidth;

    for (float x = -cubeWidth; x < cubeWidth; x += INCREMENT_SPEED) {
      for (float y = -cubeWidth; y < cubeWidth; y += INCREMENT_SPEED) {
        computeAndDrawPixel(x, y, -cubeWidth, '@');
        computeAndDrawPixel(cubeWidth, y, x, '$');
        computeAndDrawPixel(-cubeWidth, y, -x, '~');
        computeAndDrawPixel(-x, y, cubeWidth, '#');
        computeAndDrawPixel(x, -cubeWidth, -y, ';');
        computeAndDrawPixel(x, cubeWidth, y, '+');
      }
    }
    printf("\x1b[H");
    for (int i = 0; i < screenWidth * screenHeight; i++) {
      putchar(i % screenWidth ? output[i] : 10);
    }

    rollAngle += 0.05;
    pitchAngle += 0.05;
    yawAngle += 0.01;
    usleep(3000);
  }
  return 0;
}
