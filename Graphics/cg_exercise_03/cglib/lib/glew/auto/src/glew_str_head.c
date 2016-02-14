#ifdef GLEW_MX
GLboolean GLEWAPIENTRY glewContextIsSupported (const GLEWContext* ctx, const char* name)
#else
GLboolean GLEWAPIENTRY glewIsSupported (const char* name)
#endif
{
  const GLubyte* pos = (const GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if (_glewStrSame1(&pos, &len, (const GLubyte*)"GL_", 3))
    {
// CG_REVISION dfce8167a2ff99a11f4b6ce59777acd3a80ed38f
