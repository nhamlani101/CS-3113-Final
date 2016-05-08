
uniform sampler2D texture;
varying vec2 texCoordVar;
uniform int level;

void main() {
	if (level == 1) {
		vec4 finalColor = 1.0 - texture2D( texture, texCoordVar);
		finalColor.a = texture2D( texture, texCoordVar).a;
		gl_FragColor = finalColor;
	}
	else {
		gl_FragColor = texture2D( texture, texCoordVar);
	}
	
}