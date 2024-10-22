#version 430

const int MAX_BUOYS = 25;

uniform int buoyNumber_vert;

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform bool normalMap;

uniform vec4 sun_pos;

uniform vec4 buoy_pos[MAX_BUOYS];

uniform vec4 headlight_pos;
uniform vec4 headlight_pos2;

in vec4 position;
in vec4 normal, tangent, bitangent;    //por causa do gerador de geometria

in vec4 texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 sunLightDir;
	vec3 buoyLightDir[MAX_BUOYS];
	vec3 headlightDir;
	vec3 headlightDir2;
	vec2 tex_coord;
	vec3 skyboxTexCoord;
} DataOut;

void main () {

	//skybox
	DataOut.skyboxTexCoord = vec3(m_viewModel * position);
	DataOut.skyboxTexCoord.x = - DataOut.skyboxTexCoord.x;
	
	vec3 n, t, b;
	vec3 aux;
	vec4 pos = m_viewModel * position;

	 n = normalize(m_normal * normal.xyz);  // normal j� est� sendo calculado anteriormente

    DataOut.normal = n;
    DataOut.tex_coord = texCoord.st;

    // Light direction vectors for sun and buoys
	DataOut.sunLightDir = vec3(sun_pos - pos);

	for (int i = 0; i < buoyNumber_vert; i++)
	{
		DataOut.buoyLightDir[i] = vec3(buoy_pos[i] - pos);
	}
	
	DataOut.headlightDir = vec3(headlight_pos - pos);
    DataOut.headlightDir2 = vec3(headlight_pos2 - pos);

    DataOut.eye = vec3(-pos);

    if (normalMap) {  // Transform eye and light vectors by tangent basis
        // Normalize tangent, bitangent, and normal
        t = normalize(m_normal * tangent.xyz);
        b = normalize(m_normal * bitangent.xyz);
       
        // Sunlight transformation
        aux.x = dot(DataOut.sunLightDir, t);
        aux.y = dot(DataOut.sunLightDir, b);
        aux.z = dot(DataOut.sunLightDir, n);
        DataOut.sunLightDir = normalize(aux);

        // Transform all buoy light directions
        for (int i = 0; i < 6; i++) {
            aux.x = dot(DataOut.buoyLightDir[i], t);
            aux.y = dot(DataOut.buoyLightDir[i], b);
            aux.z = dot(DataOut.buoyLightDir[i], n);
            DataOut.buoyLightDir[i] = normalize(aux);
        }

        // Headlight 1 transformation
        aux.x = dot(DataOut.headlightDir, t);
        aux.y = dot(DataOut.headlightDir, b);
        aux.z = dot(DataOut.headlightDir, n);
        DataOut.headlightDir = normalize(aux);

        // Headlight 2 transformation
        aux.x = dot(DataOut.headlightDir2, t);
        aux.y = dot(DataOut.headlightDir2, b);
        aux.z = dot(DataOut.headlightDir2, n);
        DataOut.headlightDir2 = normalize(aux);

        // Eye direction transformation
        aux.x = dot(DataOut.eye, t);
        aux.y = dot(DataOut.eye, b);
        aux.z = dot(DataOut.eye, n);
        DataOut.eye = normalize(aux);
    }

	gl_Position = m_pvm * position;	
}