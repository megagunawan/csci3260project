#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec2 uv;
in layout(location=2) vec3 normal;

uniform mat4 modelTransformMatrix;
uniform mat4 projectionMatrix;
uniform vec3 ambientLight;
uniform vec3 ambientLight1;

out vec3 theColor;
out vec3 theColor1;
out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;

void main()
{
	vec4 v = vec4(position, 1.0);
	vec4 new_position = modelTransformMatrix * v;
	vec4 projectedPosition = projectionMatrix * new_position;
	gl_Position = projectedPosition;

	vec4 normal_temp = modelTransformMatrix * vec4(normal, 0);
	normalWorld = normal_temp.xyz;
	vertexPositionWorld = new_position.xyz;
	theColor = ambientLight;
	theColor1 = ambientLight1;
	UV = uv;
}