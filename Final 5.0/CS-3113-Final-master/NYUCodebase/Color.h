#ifndef COLOR_H
#define COLOR_H

struct Color {
	Color();
	Color(float red, float green, float blue, float alpha);
	float r;
	float g;
	float b;
	float a;
};

#endif