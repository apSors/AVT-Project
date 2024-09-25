#version 430

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 l_pos;
uniform vec4 sl_pos;
uniform vec4 sl_pos2;
uniform vec4 sl_dir;
uniform vec4 sl_dir2;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria

out Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec3 lightDir2;
	vec3 lightDir3;
	vec3 spotLightDir;
	vec3 spotLightDir2;
} DataOut;

void main () {

	vec4 pos = m_viewModel * position;

	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.lightDir = vec3(l_pos - pos);
	DataOut.lightDir2 = vec3(sl_pos - pos);
	DataOut.lightDir3 = vec3(sl_pos2 - pos);
	DataOut.spotLightDir = vec3(sl_dir - pos);
	DataOut.spotLightDir2 = vec3(sl_dir2 - pos);
	DataOut.eye = vec3(-pos);

	gl_Position = m_pvm * position;	
}