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
uniform float sl_angle;     // Spotlight angle
uniform float sl_angle2;    // Spotlight 2 angle
uniform float sl_exp;       // Spotlight quality 
uniform float sl_exp2;      // Spotlight 2 quality
uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform int texMode;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec3 lightDir2;
	vec3 lightDir3;
	vec2 tex_coord;
} DataIn;

uniform bool depthFog = false;

void main() {

	float dist; // camera to point distance
	vec4 totalDiffuse = vec4(0.0);
	vec4 totalSpecular = vec4(0.0);
	vec4 texel, texel1; 

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);

	// Compute distance used in fog equations
	if(depthFog == false) {
		// Plane-based fog
		dist = abs(DataIn.eye.z); // Fragment depth in eye coordinates
	} else {
		// Range-based fog
		dist = length(DataIn.eye); // Distance from camera to fragment
	}

	// Fog parameters (increased density)
	float fogAmount = clamp(pow(dist * 0.03, 0.75), 0.0, 1.0);  // Increase density by modifying the exponent and multiplier
	vec3 fogColor = vec3(0.5, 0.6, 0.7);  // Color of the fog

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

	// Combine lighting results without textures yet
	vec4 finalColor = max(totalDiffuse + totalSpecular, mat.ambient);

	// Texture application depending on the mode
	if(texMode == 0) // modulate diffuse color with texel color
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from lightwood.tga
		finalColor = finalColor * texel; // Apply texture modulation
	}
	else if (texMode == 2) // diffuse color is replaced by texel color, with specular and ambient
	{
		texel = texture(texmap, DataIn.tex_coord);  // texel from stone.tga
		finalColor = vec4(mat.ambient.rgb * 0.1, mat.ambient.a) + totalSpecular + texel * totalDiffuse;
	}
	else // multitexturing
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from lightwood.tga
		texel1 = texture(texmap1, DataIn.tex_coord);  // texel from checker.tga
		finalColor = totalDiffuse * texel * texel1 + totalSpecular;
	}

	// Apply fog by blending the final color with the fog color based on fog amount
	colorOut = vec4(mix(fogColor, finalColor.rgb, 1.0 - fogAmount), finalColor.a);
}
