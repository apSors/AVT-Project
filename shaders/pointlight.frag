#version 430

const int MAX_BUOYS = 25;
uniform int buoyNumber_frag;

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
uniform sampler2D texmap4;
uniform samplerCube cubeMap;

uniform	sampler2D texUnitDiff;
uniform	sampler2D texUnitDiff1;
uniform	sampler2D texUnitSpec;
uniform	sampler2D texUnitNormalMap;

uniform bool normalMap;  //for normal mapping
uniform bool specularMap;
uniform uint diffMapCount;

uniform bool depthFog;
uniform bool isFogEnabled;

uniform int texMode;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 sunLightDir;
	vec3 buoyLightDir[MAX_BUOYS];
	vec3 headlightDir;
	vec3 headlightDir2;
	vec2 tex_coord;
	vec3 skyboxTexCoord;
} DataIn;

vec3 l;				// normalized light direction
vec3 h;				// half vector
float intensity;
float distance;		
float intSpec;		// specular component
float attenuation;
float spotEffect;
vec3 sd;

vec4 diff, auxSpec;

void main() {

	vec4 spec = vec4(0.0);
	vec3 n;

	float dist; // camera to point distance
	vec4 totalDiffuse = vec4(0.0);
	vec4 totalSpecular = vec4(0.0);
	vec4 texel, texel1; 

	if(normalMap)
		n = normalize(2.0 * texture(texUnitNormalMap, DataIn.tex_coord).rgb - 1.0);  //normal in tangent space
	else
		n = normalize(DataIn.normal);

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


	if(mat.texCount == 0){
		diff = mat.diffuse;
		auxSpec = mat.specular;
	}
	else {
		if(diffMapCount == 0)
			diff = mat.diffuse;
		else if(diffMapCount == 1)
			diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord);
		else
			diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord) * texture(texUnitDiff1, DataIn.tex_coord);

		if(specularMap) 
			auxSpec = mat.specular * texture(texUnitSpec, DataIn.tex_coord);
		else
			auxSpec = mat.specular;
	}
	// Light 1 - Sun light (Directional)
	if( isSunActive == true ){
		l = normalize(DataIn.sunLightDir);  
		intensity = max(dot(n, l), 0.0);	

		if (intensity > 0.0) {
			h = normalize(l + e);					
			intSpec = max(dot(h, n), 0.0);	
			totalSpecular += auxSpec * pow(intSpec, mat.shininess); 
		}
		
		totalDiffuse += intensity * diff;  
	}
	


	// Buoy lights (Point)
	if( isBuoyLightsActive == true ){
		for(int i = 0; i < 5; i++){
			l = normalize(DataIn.buoyLightDir[i]);				 
			distance = length(DataIn.buoyLightDir[i]);			
			attenuation = 1.0 / (buoy_const_att + buoy_linear_att * distance + buoy_quad_att * distance * distance);
			intensity = max(dot(n, l), 0.0) * attenuation;

			if (intensity > 0.0) {
				h = normalize(l + e);
				intSpec = max(dot(h, n), 0.0);
				totalSpecular += auxSpec * pow(intSpec, mat.shininess) * attenuation;
			}
			totalDiffuse += intensity * diff;
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
				totalSpecular += auxSpec * pow(intSpec, mat.shininess) * attenuation;
			}
			totalDiffuse += intensity * diff;
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
				totalSpecular += auxSpec * pow(intSpec, mat.shininess) * attenuation;
			}
			totalDiffuse += intensity * diff;
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
	else if (texMode == 3)
	{
		texel = texture(texmap3, DataIn.tex_coord);
		if (texel.a <= 0.3) discard;
		else finalColor = vec4(mat.ambient.rgb * 0.1, mat.ambient.a) + totalSpecular + texel * totalDiffuse;
	}
	else if (texMode == 4) // multitexturing
	{
		texel = texture(texmap, DataIn.tex_coord);  // texel from stone.tga
		texel1 = texture(texmap1, DataIn.tex_coord);  // texel from checker.tga
		finalColor = totalDiffuse * texel * texel1 + totalSpecular;
		//finalColor = vec4(max(intensity*mat.diffuse.rgb + totalSpecular.rgb, mat.ambient.rgb), mat.diffuse.a);
	}
	else if (texMode == 5)
	{
		finalColor = texture(cubeMap, DataIn.skyboxTexCoord);
		//finalColor = texture(texmap, DataIn.tex_coord);
	}
	else if (texMode == 6)
	{
		texel = texture(texmap4, DataIn.tex_coord); // Use texmap3 for particles
		finalColor = mat.diffuse * texel;
	}
	else
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from lightwood.tga
		finalColor = vec4(mat.ambient.rgb * 0.1, mat.ambient.a) + totalSpecular + texel * totalDiffuse;
        finalColor.a = mat.diffuse.a;
	}

	if(isFogEnabled){
		// Apply fog by blending the final color with the fog color based on fog amount
		colorOut = vec4(mix(fogColor, finalColor.rgb, 1.0 - fogAmount), finalColor.a);
	}
	else{
		colorOut = finalColor;
	}
}