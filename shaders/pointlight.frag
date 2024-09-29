#version 430

out vec4 colorOut;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

uniform vec4 sl_dir;
uniform vec4 sl_dir2;
uniform float sl_angle;		// Spotlight angle
uniform float sl_angle2;	// Spotlight 2 angle
uniform float sl_exp;		// Spotlight quality 
uniform float sl_exp2;		// Spotlight 2 quality

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec3 lightDir2;
	vec3 lightDir3;
} DataIn;

void main() {

	vec4 totalDiffuse = vec4(0.0);
	vec4 totalSpecular = vec4(0.0);

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);

	// Light 1 - Point light
	vec3 l1 = normalize(DataIn.lightDir);
	float intensity = max(dot(n, l1), 0.0);
	if (intensity > 0.0) {
		vec3 h1 = normalize(l1 + e);
		float intSpec = max(dot(h1, n), 0.0);
		totalSpecular += mat.specular * pow(intSpec, mat.shininess);
	}
	totalDiffuse += intensity * mat.diffuse;

	// Light 2 - Spotlight
	vec3 l2 = normalize(DataIn.lightDir2);
	vec3 sd = normalize(vec3(-sl_dir));
	float spotEffect = dot(l2, sd);  // Angle between spotlight direction and light direction
	if (spotEffect > sl_angle) {
		float attenuation = pow(spotEffect, sl_exp);  // Spot attenuation
		float intensity2 = max(dot(n, l2), 0.0) * attenuation;
		if (intensity2 > 0.0) {
			vec3 h2 = normalize(l2 + e);
			float intSpec2 = max(dot(h2, n), 0.0);
			totalSpecular += mat.specular * pow(intSpec2, mat.shininess) * attenuation;
		}
		totalDiffuse += intensity2 * mat.diffuse;
	}

	// Light 3 - Spotlight 2
	vec3 l3 = normalize(DataIn.lightDir3);
	vec3 sd2 = normalize(vec3(-sl_dir2));
	float spotEffect2 = dot(l3, sd2);  // Angle between spotlight direction and light direction
	if (spotEffect2 > sl_angle2) {
		float attenuation2 = pow(spotEffect2, sl_exp2);  // Spot attenuation
		float intensity3 = max(dot(n, l3), 0.0) * attenuation2;
		if (intensity3 > 0.0) {
			vec3 h3 = normalize(l3 + e);
			float intSpec3 = max(dot(h3, n), 0.0);
			totalSpecular += mat.specular * pow(intSpec3, mat.shininess) * attenuation2;
		}
		totalDiffuse += intensity3 * mat.diffuse;
	}

	// Combine results
	colorOut = max(totalDiffuse + totalSpecular, mat.ambient);
}
