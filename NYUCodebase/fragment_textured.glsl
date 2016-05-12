uniform sampler2D texture;
varying vec2 texCoordVar;
uniform int level;

vec3 saturation_func(vec3 rgb, float adjustment) {
	const vec3 W = vec3(0.2125, 0.7154, 0.0721);
	vec3 intensity = vec3(dot(rgb, W));
	return mix(intensity, rgb, adjustment);
}

void main() {
	if (level == 1) {
		vec4 finalColor;
		finalColor.rgb = saturation_func(texture2D( texture, texCoordVar).rgb, 4.0);
		finalColor.a = texture2D( texture, texCoordVar).a;	
		gl_FragColor = finalColor;
	}
	else if (level == 2) {
		vec4 finalColor = 1.0 - texture2D( texture, texCoordVar);
		finalColor.a = texture2D( texture, texCoordVar).a;
		gl_FragColor = finalColor;
	}
	else if (level == 3) {
		vec4 texColor = texture2D( texture, texCoordVar);
		vec4 finalColor = vec4((texColor.r + texColor.g + texColor.b)/3.0);
		finalColor.a = texture2D( texture, texCoordVar).a;
		gl_FragColor = finalColor;
	}
	else {
		gl_FragColor = texture2D( texture, texCoordVar);
	}
}