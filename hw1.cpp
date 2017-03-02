//modified by Kory Despot
//
//purpose: hw 1 assignment
//
//cs3350 Spring 2017 Lab-1
//author: Gordon Griesel
//date: 2014 to present
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
extern "C" {
#include "fonts.h"
}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 100000
#define GRAVITY 0.15
#define rnd() (float)rand() / (float)RAND_MAX

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box[5];
	Shape circle;
	Particle particle[MAX_PARTICLES];
	int n;
	int bubbler;
	int mouse[2];
	Game() {
		n = 0;
		bubbler = 0;
		for (int i = 0; i < 5; i++) {
			box[i].width = 90;
			box[i].height = 18;
			box[i].center.x = 115 + i*100;
			box[i].center.y = 500 - i*75;
		}
		circle.center.x = 700;
		circle.center.y = -100;
		circle.radius = 200;
	}

};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();

	//declare game object
	Game game;

	//start animation
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Hw1   LMB for particle");
}

void cleanupXWindows(void)
{
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void)
{
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask | PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);

	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeParticle(Game *game, int x, int y)
{
	if (game->n >= MAX_PARTICLES)
		return;
	std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = rnd() * 0.5 - 0.25;
	p->velocity.x = rnd() * 0.5 - 0.25;
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	// static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			for (int i = 1; i < 10; i++) {
				makeParticle(game, e->xbutton.x, y);
			}
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		int y = WINDOW_HEIGHT - e->xbutton.y;
		if (game->bubbler == 0) {
			game->mouse[0] = savex;
			game->mouse[1] = y;
		}
		for (int i =0; i < 5; i++) {
		    makeParticle(game, e->xbutton.x, y);
		}
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
		    return 1;
		}
		if (key == XK_b) {
		    game->bubbler ^= 1;
		}
	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;
	if (game->bubbler != 0) {
	    //the bubbler is on!
	    for (int i = 0; i < 50; i++)
	    {
	    	makeParticle(game, game->mouse[0], game->mouse[1]);
	    }
	}
	for (int i = 0; i < game->n; i++) {
		p = &game->particle[i];
		p->velocity.y -= GRAVITY;
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;

		//check for collision with boxes
		for (int j = 0; j < 5; j++) {
			if ((p->s.center.x >= game->box[j].center.x - game->box[j].width) &&
				(p->s.center.x <= game->box[j].center.x + game->box[j].width) &&
				(p->s.center.y < game->box[j].center.y + game->box[j].height) &&
				(p->s.center.y > game->box[j].center.y - game->box[j].height))
			{
	    		p->s.center.y = game->box[j].center.y + game->box[j].height + 0.1;
	    		p->velocity.x += 0.025;
			p->velocity.y *= -0.2;
			}
		}

		//check for collision with circle
		float d1, d2, dist;
		d1 = p->s.center.x - game->circle.center.x;
		d2 = p->s.center.y - game->circle.center.y;
		dist = sqrt((d1*d1) + (d2*d2));
		if (dist <= game->circle.radius) {
			d1 /= dist;;
			d2 /= dist;
			d1 *= game->circle.radius * 1.01;
			d2 *= game->circle.radius * 1.01;
			p->s.center.x = game->circle.center.x + d1;
			p->s.center.y = game->circle.center.y + d2;
			p->velocity.x += d1 * 0.003;
			p->velocity.y += d1 * 0.005;
		}

		//check for off-screen
		if (p->s.center.y < 0.0) {
			std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[--game->n];
		}
	}
}

void render(Game *game)
{
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);

	//draw boxes
	Shape *s;

	for (int i = 0; i < 5; i++) {
		glColor3ub(255-(20*i),22*(i+1),255-(20*i));
		s = &game->box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
		glEnd();
		glPopMatrix();
	}

	Rect r;
	for (int i = 0; i <5; i++) {
		s = &game->box[i];
		unsigned int cref = 0x00ffffffl;
		r.left = s->center.x;
		r.bot = s->center.y - 5;
		r.center = 1;
		if (i == 0) { ggprint16(&r, 16, cref, "Requirements"); }
		if (i == 1) { ggprint16(&r, 16, cref, "Design"); }
		if (i == 2) { ggprint16(&r, 16, cref, "Coding"); }
		if (i == 3) { ggprint16(&r, 16, cref, "Testing"); }
		if (i == 4) { ggprint16(&r, 16, cref, "Maintenance"); }
	}

	//draw circle
	static int firstTime = 1;
	static int v[60][2];
	static int n = 60;
	glColor3ub(034,056,153);
	if (firstTime) {
		float angle = 0.0;
		float inc = (3.14159 * 2.0) / (float) n;
		for (int i = 0; i < n; i++) {
			v[i][0] = cos(angle) * game->circle.radius + game->circle.center.x;
			v[i][1] = sin(angle) * game->circle.radius + game->circle.center.y;
			angle += inc;
		}
		firstTime = 0;
	}
	glPushMatrix();
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < n; i++) {
		glVertex2i(v[i][0], v[i][1]);
	}
	glEnd();
	glPopMatrix();

	//draw all particles here
	for (int i =0; i < game->n; i++) {
		glPushMatrix();
		glColor3ub(104,158,255);
		Vec *c = &game->particle[i].s.center;
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
}
