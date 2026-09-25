// One quad covering the window; the fragment shader draws the whole scene
attribute vec2 coord2d;

void main(void) {
	gl_Position = vec4(coord2d, 0.0, 1.0);
}
