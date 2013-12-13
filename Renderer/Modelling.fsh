#version 130
uniform sampler2D tex;
uniform float alpha_th;
uniform float isLabelDepth;
varying in vec4 theColor;


void main(void)
{
    vec4 temp_c = texture2D(tex, gl_TexCoord[0].st);
    if(temp_c.a<alpha_th)
        discard;
    else
    {
        if(isLabelDepth>0.5)
        {
//            gl_FragColor = vec4(1.0,0.0,0.0,1.0);
            gl_FragColor = theColor;
        }
        else
            gl_FragColor = vec4(temp_c.rgb,1.0);

    }
}
