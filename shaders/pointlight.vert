#version 430

uniform int buoyNumber;

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 sun_pos;

uniform vec4 buoy_pos[buoyNumber];

uniform vec4 headlight_pos;
uniform vec4 headlight_pos2;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria

in vec4 texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 sunLightDir;
	vec3 buoyLightDir[buoyNumber];
	vec3 headlightDir;
	vec3 headlightDir2;
	vec2 tex_coord;
} DataOut;

void main () {

	vec4 pos = m_viewModel * position;

	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.sunLightDir = vec3(sun_pos - pos);

	for (int i = 0; i < buoyNumber; i++)
	{
		DataOut.buoyLightDir[i] = vec3(buoy_pos[i] - pos);
	}
	DataOut.buoyLightDir[0] = vec3(buoy_pos - pos);
	DataOut.buoyLightDir[1] = vec3(buoy_pos2 - pos);
	DataOut.buoyLightDir[2] = vec3(buoy_pos3 - pos);
	DataOut.buoyLightDir[3] = vec3(buoy_pos4 - pos);
	DataOut.buoyLightDir[4] = vec3(buoy_pos5 - pos);
	DataOut.buoyLightDir[5] = vec3(buoy_pos6 - pos);
	
	DataOut.headlightDir = vec3(headlight_pos - pos);
	DataOut.headlightDir2 = vec3(headlight_pos2 - pos);
	DataOut.eye = vec3(-pos);
	DataOut.tex_coord = texCoord.st;

	gl_Position = m_pvm * position;	
}