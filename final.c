#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "CSCIx229.h"
#include <string>
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


#define total_modes 4

int th=0;          //  Azimuth of view angle
int ph=25;          //  Elevation of view angle
int axes=1;        //  Display axes
int shader[] = {0, 0, 0, 0};
double cameraRadius = 3.;
double fov = 30;
int mode = 0;
double yaw = 3.1415;
double pitch = 0;
double mouseScale = 10000;
double cameraSpeed = 1.0;
bool leftMove = false;
bool rightMove = false;
bool forwardMove = false;
bool backMove = false;
bool sprint = false;

int oldTime = 0;

glm::vec3 cameraPos = glm::vec3(-3.0, 0.0, 0.0);
glm::vec3 targetPos = glm::vec3(-0.0);
glm::vec3 cameraDir = glm::vec3(-1.0, 0.0, 0.0);

const glm::vec3 upVec = glm::vec3(0.0, 1.0, 0.0);

const char * modeText[] = {"Infinite Spheres", "Mandelbox", "Mandelbulb", "Dynamic Mandelbulb"};

double radii [total_modes] = { 3., 20., 3., 3.};

glm::mat4 ViewMatrix = glm::mat4(1.0);
/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  //  Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}


/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   glUseProgram(shader[mode]);

   int width = glutGet(GLUT_WINDOW_WIDTH);
   int height = glutGet(GLUT_WINDOW_HEIGHT);

   glm::vec2 iResolution = glm::vec2(width, height);

   /*
   double cameraX = cameraRadius * Sin(th) * Cos(ph) * -1;
   double cameraY = cameraRadius * Sin(ph);
   double cameraZ = cameraRadius * Cos(th) * Cos(ph);
   glm::vec3 cameraPosition = glm::vec3(cameraX, cameraY, cameraZ);
   glm::vec3 targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
   */
   targetPos = cameraPos + cameraDir;
   ViewMatrix = glm::inverse(glm::lookAt(cameraPos, targetPos, upVec));
   

   int mvpID = glGetUniformLocation(shader[mode], "MVP");
   if (mvpID != -1) {
		glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(ViewMatrix));
   }
   int posID = glGetUniformLocation(shader[mode], "cameraPos");
   if (posID != -1) {
		glUniform3fv(posID, 1, glm::value_ptr(cameraPos));
   }
   int resID = glGetUniformLocation(shader[mode], "iResolution");
   if (resID != -1) {
		glUniform2fv(resID, 1, glm::value_ptr(iResolution));
   }
   int fovID = glGetUniformLocation(shader[mode], "fov");
   if (fovID != -1) {
		glUniform1f(fovID, fov);
   }

   float tim = (float)glutGet(GLUT_ELAPSED_TIME);
   tim = sin(tim/1500);

   int timeID = glGetUniformLocation(shader[mode], "time");
   if (timeID != -1) {
		glUniform1f(timeID, tim);
   }

   int modeID = glGetUniformLocation(shader[mode], "mode");
   if (modeID != -1) {
		glUniform1i(modeID, mode);
   }



   glBegin(GL_POLYGON);
   glVertex3d(-1, -1, 0);
   glVertex3d(-1, 1, 0);
   glVertex3d(1, 1, 0);
   glVertex3d(1, -1, 0);
   glEnd();

   glUseProgram(0);
   //  Five pixels from the lower left corner of the window
   glWindowPos2i(5,5);
   //  Print the text string
   Print("Angle=%d,%d FOV=%f, model=%s, pitch=%f, yaw=%f",th,ph, fov, modeText[mode], pitch, yaw);
   //  Render the scene
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   else if (key == GLUT_KEY_PAGE_UP)
	   cameraRadius -= 0.5;
   else if (key == GLUT_KEY_PAGE_DOWN)
	   cameraRadius += 0.5;
   if (cameraRadius > 20)
	   cameraRadius = 20;
   if (cameraRadius < 1)
	   cameraRadius = 1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   else if (ch == 'f')
	   fov -= 10;
   else if (ch == 'F')
	   fov += 10;
   else if (ch == 'm') {
	   mode = (mode+1)%total_modes;
	   cameraRadius = radii[mode];
	   cameraPos = glm::vec3(-cameraRadius, 0.0, 0.0);
	   targetPos = glm::vec3(0.0);
	   cameraDir = glm::vec3(1.0, 0.0, 0.0);
	   yaw = 0;
	   pitch = 0;
   }
   else if (ch == 'M') {
	   mode = (mode+total_modes-1)%total_modes;
	   cameraRadius = radii[mode];
	   cameraPos = glm::vec3(-cameraRadius, 0.0, 0.0);
	   targetPos = glm::vec3(0.0);
	   cameraDir = glm::vec3(1.0, 0.0, 0.0);
	   yaw = 0;
	   pitch = 0;
   }
   else if (ch == 'w') {
		forwardMove = true;
   }
   else if (ch == 'a') {
		leftMove = true;
   }
   else if (ch == 's') {
		backMove = true;
   }
   else if (ch == 'd') {
		rightMove = true;
   } else if (ch == ' ') {
		sprint = !sprint;
   }

   if (fov < 10)
	   fov = 10;
   if (fov > 50)
	   fov = 50;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

void keyUp (unsigned char ch, int x, int y) {
	if (ch == 'w') {
		forwardMove = false;
	} else if (ch == 'a') {
		leftMove = false;
	} else if (ch == 's') {
		backMove = false;
	} else if (ch == 'd') {
		rightMove = false;
	}
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   const double dim=1;
   //  Ratio of the width to the height of the window
   double w2h = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Orthogonal projection
   glOrtho(-w2h*dim,+w2h*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}


/*
 *  Read text file
 */
char* ReadText(char *file)
{
   int   n;
   char* buffer;
   //  Open file
   FILE* f = fopen(file,"rt");
   if (!f) Fatal("Cannot open text file %s\n",file);
   //  Seek to end to determine size, then rewind
   fseek(f,0,SEEK_END);
   n = ftell(f);
   rewind(f);
   //  Allocate memory for the whole file
   buffer = (char*)malloc(n+1);
   if (!buffer) Fatal("Cannot allocate %d bytes for text file %s\n",n+1,file);
   //  Snarf the file
   if (fread(buffer,n,1,f)!=1) Fatal("Cannot read %d bytes for text file %s\n",n,file);
   buffer[n] = 0;
   //  Close and return
   fclose(f);
   return buffer;
}

/*
 *  Print Shader Log
 */
void PrintShaderLog(int obj,char* file)
{
   int len=0;
   glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&len);
   if (len>1)
   {
      int n=0;
      char* buffer = (char *)malloc(len);
      if (!buffer) Fatal("Cannot allocate %d bytes of text for shader log\n",len);
      glGetShaderInfoLog(obj,len,&n,buffer);
      fprintf(stderr,"%s:\n%s\n",file,buffer);
      free(buffer);
   }
   glGetShaderiv(obj,GL_COMPILE_STATUS,&len);
   if (!len) Fatal("Error compiling %s\n",file);
}

/*
 *  Print Program Log
 */
void PrintProgramLog(int obj)
{
   int len=0;
   glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&len);
   if (len>1)
   {
      int n=0;
      char* buffer = (char *)malloc(len);
      if (!buffer) Fatal("Cannot allocate %d bytes of text for program log\n",len);
      glGetProgramInfoLog(obj,len,&n,buffer);
      fprintf(stderr,"%s\n",buffer);
   }
   glGetProgramiv(obj,GL_LINK_STATUS,&len);
   if (!len) Fatal("Error linking program\n");
}

/*
 *  Create Shader
 */
int CreateShader(GLenum type,char* file,char* sdf)
{
   //  Create the shader
   int shader = glCreateShader(type);
   //  Load source code from file
   char* source = ReadText(file);

   std::string string_source = std::string(source);

   std::string inject = "[SDF]";
   size_t i = string_source.find(inject)+5;
   string_source.insert(i, std::string(sdf));
   
   const char * new_source = string_source.c_str();
   

   glShaderSource(shader,1,(const char**)&new_source,NULL);
   free(source);
   //  Compile the shader
   fprintf(stderr,"Compile %s\n",file);
   glCompileShader(shader);
   //  Check for errors
   PrintShaderLog(shader,file);
   //  Return name
   return shader;
}

/*
 *  Create Shader Program
 */
int CreateShaderProg(char* VertFile,char* FragFile,char* sdf)
{
   //  Create program
   int prog = glCreateProgram();
   //  Create and compile vertex shader
   int vert = CreateShader(GL_VERTEX_SHADER  ,VertFile, "");
   //  Create and compile fragment shader
   int frag = CreateShader(GL_FRAGMENT_SHADER,FragFile, sdf);
   //  Attach vertex shader
   glAttachShader(prog,vert);
   //  Attach fragment shader
   glAttachShader(prog,frag);
   //  Link program
   glLinkProgram(prog);
   //  Check for errors
   PrintProgramLog(prog);
   //  Return name
   return prog;
}

void idle() {
	int newTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (newTime - oldTime)/1000.;
	if (sprint) {
		deltaTime *= 5.0;
	}
	oldTime = newTime;
	if (forwardMove) {
		cameraPos = cameraPos + cameraDir * deltaTime;
	}
	else if (backMove) {
		cameraPos = cameraPos - cameraDir * deltaTime;
	}

	glm::vec3 left = glm::normalize(glm::cross(upVec, cameraDir));

	if (leftMove) {
		cameraPos = cameraPos + left * deltaTime;
	} else if (rightMove) {
		cameraPos = cameraPos - left * deltaTime;
	}
	glutPostRedisplay();
}


//Solution adapted from https://stackoverflow.com/questions/728049/glutpassivemotionfunc-and-glutwarpmousepointer
void mouseMove(int x, int y) {
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);

	if (x == width/2 && y == height/2) return;

	int dx = x - width/2;
	int dy = y - height/2;

	yaw += ((double)dx)/mouseScale;
	pitch -= ((double)dy)/mouseScale;

	cameraDir.x = cos(yaw)*cos(pitch);
	cameraDir.y = sin(pitch);
	cameraDir.z = sin(yaw)*cos(pitch);

	cameraDir = glm::normalize(cameraDir);

	glutWarpPointer(width/2, height/2);
	glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600,600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
   //  Create the window
   glutCreateWindow("Em Eldar");
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardUpFunc(keyUp);
   glutKeyboardFunc(key);

   glutIdleFunc(idle);
   shader[0] = CreateShaderProg("quad.vert","march.frag", "\n\treturn sphereSDF(p, 0.3);");
   shader[1] = CreateShaderProg("quad.vert","march.frag", "\n\treturn mandelbox(p);");
   shader[2] = CreateShaderProg("quad.vert","march.frag", "\n\treturn mandelbulb(p);");
   shader[3] = CreateShaderProg("quad.vert","march.frag", "\n\treturn sdDinamMandelbulb(p, remap(time, -1., 1., 4., 9.));");
   int ww = glutGet(GLUT_WINDOW_WIDTH);
   int wh = glutGet(GLUT_WINDOW_HEIGHT);
   glutWarpPointer(ww/2, wh/2);
   glutSetCursor(GLUT_CURSOR_NONE);
   glutMotionFunc(mouseMove);
   glutPassiveMotionFunc(mouseMove);
   glutMainLoop();
   return 0;
}
