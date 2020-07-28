void main()
{
    // transforme la position du vertex
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

    // transforme les coordonn�es de texture
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    // transmet la couleur
    gl_FrontColor = gl_Color;
}