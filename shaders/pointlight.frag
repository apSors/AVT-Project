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

uniform vec4 headlight_dir;			// Boat headlight diretion
uniform vec4 headlight_dir2;		// Boat headlight 2 diretion
uniform float headlight_angle;		// Boat headlight angle
uniform float headlight_exp;		// Boat headlight quality 

uniform float buoy_const_att;
uniform float buoy_linear_att;
uniform float buoy_quad_att;

uniform bool isSunActive;
uniform bool isBuoyLightsActive;
uniform bool isHeadlightsActive;

uniform vec4 sl_dir;
uniform vec4 sl_dir2;
uniform float sl_angle;     // Spotlight angle
uniform float sl_angle2;    // Spotlight 2 angle
uniform float sl_exp;       // Spotlight quality 
uniform float sl_exp2;      // Spotlight 2 quality

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform sampler2D texmap3;

uniform bool depthFog;
uniform int texMode;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 sunLightDir;
	vec3 buoyLightDir[6];
	vec3 headlightDir;
	vec3 headlightDir2;
	vec2 tex_coord;
} DataIn;

vec3 l;				// normalized light direction
vec3 h;				// half vector
float intensity;
float distance;		
float intSpec;		// specular component
float attenuation;
float spotEffect;
vec3 sd;

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

	// Light 1 - Sun light (Directional)
	if( isSunActive == true ){
		l = normalize(DataIn.sunLightDir);  
		intensity = max(dot(n, l), 0.0);	

		if (intensity > 0.0) {
			h = normalize(l + e);					
			intSpec = max(dot(h, n), 0.0);	
			totalSpecular += mat.specular * pow(intSpec, mat.shininess); 
		}
		totalDiffuse += intensity * mat.diffuse;  
	}
	


	// Buoy lights (Point)
	if( isBuoyLightsActive == true ){
		for(int i = 0; i <6; i++){
			l = normalize(DataIn.buoyLightDir[i]);				 
			distance = length(DataIn.buoyLightDir[i]);			
			attenuation = 1.0 / (buoy_const_att + buoy_linear_att * distance + buoy_quad_att * distance * distance);
			intensity = max(dot(n, l), 0.0) * attenuation;

			if (intensity > 0.0) {
				h = normalize(l + e);
				intSpec = max(dot(h, n), 0.0);
				totalSpecular += mat.specular * pow(intSpec, mat.shininess) * attenuation;
			}
			totalDiffuse += intensity * mat.diffuse;
		}
	}
	


	// Boat headlight (Spotlight)
	if( isHeadlightsActive == true ){
		l = normalize(DataIn.headlightDir);
		sd = normalize(vec3(-headlight_dir));
		spotEffect = dot(l, sd);	// Angle between spotlight direction and light direction

		if (spotEffect > headlight_angle) {
			attenuation = pow(spotEffect, headlight_exp); 
			intensity = max(dot(n, l), 0.0) * attenuation;

			if (intensity > 0.0) {
				h = normalize(l + e);
				intSpec = max(dot(h, n), 0.0);
				totalSpecular += mat.specular * pow(intSpec, mat.shininess) * attenuation;
			}
			totalDiffuse += intensity * mat.diffuse;
		}

		// Boat headlight 2 (Spotlight)
		l = normalize(DataIn.headlightDir2);
		sd = normalize(vec3(-headlight_dir2));
		spotEffect = dot(l, sd);  

		if (spotEffect > headlight_angle) {
			attenuation = pow(spotEffect, headlight_exp);  
			intensity = max(dot(n, l), 0.0) * attenuation;

			if (intensity > 0.0) {
				h = normalize(l + e);
				intSpec = max(dot(h, n), 0.0);
				totalSpecular += mat.specular * pow(intSpec, mat.shininess) * attenuation;
			}
			totalDiffuse += intensity * mat.diffuse;
		}
	}

	// Combine lighting results without textures yet
	vec4 finalColor = max(totalDiffuse + totalSpecular, mat.ambient);

	// Texture application depending on the mode
	if(texMode == 0) // modulate diffuse color with texel color
	{
		texel = texture(texmap, DataIn.tex_coord);  // texel from stone.tga
		finalColor = finalColor * texel; // Apply texture modulation
	}
	else if (texMode == 1) // diffuse color is replaced by texel color, with specular and ambient
	{
		texel = texture(texmap1, DataIn.tex_coord);  // texel from checker.png
		finalColor = vec4(mat.ambient.rgb * 0.1, mat.ambient.a) + totalSpecular + texel * totalDiffuse;
	}
	else if (texMode == 2) // diffuse color is replaced by texel color, with specular and ambient
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from lightwood.tga
		finalColor = vec4(mat.ambient.rgb * 0.1, mat.ambient.a) + totalSpecular + texel * totalDiffuse;
	}
	else if (texMode == 3) {
		texel = texture(texmap3, DataIn.tex_coord); // Use texmap3 for particles
		finalColor = mat.diffuse * texel;
    }
	else // multitexturing
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from lightwood.tga
		texel1 = texture(texmap1, DataIn.tex_coord);  // texel from checker.tga
		finalColor = totalDiffuse * texel * texel1 + totalSpecular;
		//finalColor = vec4(max(intensity*mat.diffuse.rgb + totalSpecular.rgb, mat.ambient.rgb), mat.diffuse.a);
	}

	// Apply fog by blending the final color with the fog color based on fog amount
	colorOut = vec4(mix(fogColor, finalColor.rgb, 1.0 - fogAmount), finalColor.a);
}