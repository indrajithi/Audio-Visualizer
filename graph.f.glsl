uniform sampler2D mytexture;   // one texel per band: luminance = bar level, alpha = peak level
uniform vec2 resolution;       // window size in pixels
uniform float offset_x;
uniform float scale_x;
uniform float bands;
uniform float progress;        // 0..1 through the song
uniform float bass;            // 0..1 low-frequency energy, drives the background pulse
uniform float smooth_mode;     // 1 = filled curve, 0 = separate bars
uniform float show_peaks;

// Layout in pixels, shared with the text overlay and mouse seeking in draw.cpp
const float TRACK_Y = 28.0;
const float TRACK_MARGIN = 70.0;
const float TOP_MARGIN = 44.0;

// Cyan -> violet -> magenta -> amber as a bar rises, so loud bands read warm
vec3 palette(float t) {
	vec3 c0 = vec3(0.10, 0.85, 0.95);
	vec3 c1 = vec3(0.45, 0.35, 1.00);
	vec3 c2 = vec3(0.95, 0.25, 0.75);
	vec3 c3 = vec3(1.00, 0.72, 0.30);
	t = clamp(t, 0.0, 1.0);
	if (t < 0.33)
		return mix(c0, c1, t / 0.33);
	if (t < 0.66)
		return mix(c1, c2, (t - 0.33) / 0.33);
	return mix(c2, c3, (t - 0.66) / 0.34);
}

// Signed distance from p to a box centered at c with half-size h and corner radius r
float roundedBox(vec2 p, vec2 c, vec2 h, float r) {
	vec2 q = abs(p - c) - h + r;
	return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

// Distance to the bar (or curve) surface; negative inside
float barDistance(vec2 q, float centerX, float halfW, float baseY, float h) {
	if (smooth_mode > 0.5)
		return max(q.y - (baseY + h), baseY - q.y);
	return roundedBox(q, vec2(centerX, baseY + h * 0.5), vec2(halfW, h * 0.5), min(halfW, 4.0));
}

void main(void) {
	vec2 p = gl_FragCoord.xy;
	vec2 uv = p / resolution;

	// Background: deep vertical gradient plus a glow that swells with the bass
	vec3 col = mix(vec3(0.02, 0.02, 0.05), vec3(0.07, 0.04, 0.13), uv.y);
	float glow = exp(-3.0 * length((uv - vec2(0.5, 0.32)) * vec2(1.0, 1.8)));
	col += vec3(0.28, 0.08, 0.38) * glow * (0.2 + bass);

	float baseY = resolution.y * 0.28;
	float maxH = resolution.y - TOP_MARGIN - baseY;

	// Horizontal position in spectrum space, with pan and zoom applied
	float sx = ((uv.x * 2.0 - 1.0) / scale_x - offset_x) * 0.5 + 0.5;
	float bandPos = sx * bands;
	float cell = resolution.x / bands * scale_x;          // on-screen width of one band
	float centerX = p.x - (fract(bandPos) - 0.5) * cell;  // pixel x of this band's center
	float halfW = cell * 0.34;

	// Bars sample their band's center; the smooth curve samples continuously
	float texX = smooth_mode > 0.5 ? sx : (floor(bandPos) + 0.5) / bands;
	vec2 level = texture2D(mytexture, vec2(texX, 0.5)).ra;
	float h = max(level.x * maxH, 2.0);

	vec3 barCol = palette((p.y - baseY) / maxH) * (0.75 + 0.5 * level.x);

	// Soft halo around each bar, then the bar itself with an anti-aliased edge
	float d = barDistance(p, centerX, halfW, baseY, h);
	col += barCol * 0.35 * level.x * exp(-max(d, 0.0) / 7.0) * step(baseY, p.y);
	col = mix(col, barCol, clamp(0.5 - d, 0.0, 1.0));

	// Peak cap: a thin bright line that holds, then falls toward the bar
	if (show_peaks > 0.5 && level.y > 0.01) {
		float capY = baseY + level.y * maxH + 5.0;
		float capHalfW = smooth_mode > 0.5 ? cell * 0.5 : halfW;
		float cap = roundedBox(p, vec2(centerX, capY), vec2(capHalfW, 1.5), 1.5);
		col = mix(col, mix(palette(level.y), vec3(1.0), 0.6), clamp(0.5 - cap, 0.0, 1.0));
	}

	// Reflection: the bars mirrored below the baseline, squashed and fading out
	float reflDepth = baseY - p.y;
	float reflH = maxH * 0.35;
	if (reflDepth > 0.0 && reflDepth < reflH) {
		vec2 mirrored = vec2(p.x, baseY + reflDepth / 0.35);
		float dr = barDistance(mirrored, centerX, halfW, baseY, h);
		float fade = 0.22 * (1.0 - reflDepth / reflH);
		col = mix(col, palette(reflDepth / reflH * 0.6), clamp(0.5 - dr, 0.0, 1.0) * fade);
	}

	// Progress bar: rounded track, gradient fill up to the playhead, and a knob
	float x0 = TRACK_MARGIN;
	float x1 = resolution.x - TRACK_MARGIN;
	float headX = mix(x0, x1, progress);
	float track = roundedBox(p, vec2((x0 + x1) * 0.5, TRACK_Y), vec2((x1 - x0) * 0.5, 2.0), 2.0);
	vec3 fillCol = palette(0.1 + 0.8 * (p.x - x0) / (x1 - x0));
	bool played = p.x <= headX;
	col = mix(col, played ? fillCol : vec3(0.22, 0.20, 0.30), clamp(0.5 - track, 0.0, 1.0));
	if (played && p.x >= x0)
		col += fillCol * 0.25 * exp(-abs(p.y - TRACK_Y) / 4.0);

	float knob = length(p - vec2(headX, TRACK_Y)) - 6.0;
	col += palette(progress) * 0.5 * exp(-max(knob, 0.0) / 5.0);
	col = mix(col, vec3(1.0), clamp(0.5 - knob, 0.0, 1.0));

	gl_FragColor = vec4(col, 1.0);
}
