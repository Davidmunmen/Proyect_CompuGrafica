#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D papelTex;
uniform float opacidad;

void main()
{
    vec4 texColor = texture(papelTex, TexCoord);
    
    // Si el píxel es muy transparente, lo descartamos (útil para el papel picado)
    if(texColor.a < 0.1)
        discard;
        
    // Aplicamos la opacidad (para que el humo se desvanezca al subir)
    FragColor = vec4(texColor.rgb, texColor.a * opacidad);
}