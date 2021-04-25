#version 410

in vec3 fragP;
in vec3 fragN;

uniform mat3 texMatrix;
uniform samplerCube cubeTex;

out vec4 outColor;

void main() {
  vec3 V = normalize(-fragP);
  vec3 N = normalize(fragN);
  vec3 T = refract(-V, N, 1.0 / 1.52);  // Air to glass

  outColor = texture(cubeTex, texMatrix * T);
}