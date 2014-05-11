in vec2 uv;
out vec4 color;

uniform vec4 textColor;
uniform vec4 area;

uniform sampler2D charactersMap;

void main()
{
	vec2 pos = gl_FragCoord.xy;
	if (pos.x < area.x || pos.x > area.z || pos.y < area.y || pos.y > area.w) discard;

	float c = texture(charactersMap, uv).r;
	color = textColor * c;
}