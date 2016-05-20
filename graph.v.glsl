attribute float coord1d;
varying vec4 f_color;
uniform float offset_x;
uniform float scale_x;
uniform sampler2D mytexture;

void main(void) {
	float x = (coord1d / scale_x) - offset_x;
	float y = (texture2D(mytexture, vec2(x / 10.24 / 2.0 + 0.5, 0)).r - 0.5) * 2.0;

	gl_Position = vec4(coord1d, y, 0.0, 1.0);
	f_color = vec4(x / 2.0 + 0.5, y / 2.0 + 0.5, 1.0, 1.0);
	gl_PointSize = 5.0;
}
