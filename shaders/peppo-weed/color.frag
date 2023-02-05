#version 330 core
out vec4 FragColor; 

in vec3 vertexColor;
in vec2 TexCoord;

uniform float time;
uniform sampler2D texture1;

void main() {
    FragColor = texture(texture1, TexCoord) * vec4(
        (sin(TexCoord.x + time) + 1.0) / 2.0,
        (cos(TexCoord.y + time) + 1.0) / 2.0,
        (cos(TexCoord.x + TexCoord.y + time) + 1.0) / 2.0,
        1.0);
    ;
};  
