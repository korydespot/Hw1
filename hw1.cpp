//cs3350 Spring 2017 Lab-1
//Author: Kory Despot
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
#include "fonts.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 1000
#define GRAVITY 0.6
#define rnd() (float)rand() / (float)RAND_MAX
#define PI 3.14159265359


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
	Game() { n = 0; bubbler = 0; }
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

	for(int i = 0; i < 5; i++) {
                game.box[i].width = 100;
                game.box[i].height = 15;
                game.box[i].center.x = 150 + (i * 100);
                game.box[i].center.y = 500 - (i * 75);
        }
 	game.circle.radius = 100;
	game.circle.center.x = WINDOW_WIDTH;
	game.circle.center.y = 0;


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
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
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
	p->velocity.y = rnd() *0.5 - 0.25;
	p->velocity.x = rnd() *0.5 - 0.25;
	game->n++;
	
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			makeParticle(game, e->xbutton.x, y);
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
		if (++n < 10)
			return;
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
		//You may check other keys here.
	
		if(key == XK_b) {
			game->bubbler ^= 1;
		}

	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;	
	float d0, d1, dist;

	if (game->n <= 0)
		return;
	
	if(game->bubbler != 0){
		//bubbler is on	
		makeParticle(game, game->mouse[0], game->mouse[1]);
	}

	for(int i=0; i<game->n;i++){
		p = &game->particle[i];
		//check for collision with shapes...
		
		p->velocity.y -= GRAVITY;
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		
	
	if (game->bubbler != 0) {
            // the bubbler is on
            for (int i = 0; i < 100; i++) {
                        makeParticle(game, (game->mouse[0] + rand() % ((game->mouse[0]+20) - game->mouse[0])), game->mouse[1]);
                }
        }

        for (int i = 0; i < game->n; i++) {
                p = &game->particle[i];
                p->velocity.y -= GRAVITY;
                p->s.center.x += p->velocity.x;
                p->s.center.y += p->velocity.y;


        //check for collision with shapes...
                Shape *s;
                for (int j = 0; j < 5; j++) {
                        s = &game->box[j];
                        if ((p->s.center.y < s->center.y + s->height && p->s.center.y > s->center.y - s->height) &&
                                p->s.center.x >= s->center.x - s->width &&
                                p->s.center.x <= s->center.x + s->width) {
                                        p->s.center.y = s->center.y + s->height;
                                        p->velocity.y = -p->velocity.y * 0.6f;
                                        p->velocity.x += 0.01f;
                        }
                }
		for(int j=0; j<5;i++){	
			s = &game->box[j];
			
			if(p->s.center.y < s->center.y + s->height &&
	   	   	   p->s.center.y > s->center.y - s->height) {
				if(p->s.center.x > s->center.x - s->width
                   	   	   && p->s.center.x < s->center.x + s->width){
					p->s.center.y = s->center.y + s->height + 0.2;
					p->velocity.y = -p->velocity.y * 0.5;
					p->velocity.x += 0.0225;
				}
			} 
		}
	
		//check for off-screen
		if (p->s.center.y < 0.0 || p->s.center.y > 600) {
			std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[--game->n];
		}
		
		d0 = p->s.center.x - game->circle.center.x;
		d1 = p->s.center.y - game->circle.center.y;
		dist = sqrt(d0*d0 + d1*d1);

		if( dist <= game->circle.radius) {
			p->velocity.x = p->velocity.x/2.0 + (d0/dist);
			p->velocity.y = p->velocity.y/2.0 + (d1/dist);
		
		}
	}
}
}

void render(Game *game)
{
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	Rect r;
	//Draw shapes...

	//draw box
	char text [5][32] = {{"Requirements"}, {"Design"},{"Coding"},{"Testing"},{"Maintenance"}};
	Shape *s;
	for(int i=0; i<5; i++){
		glColor3ub(90,140,90);
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
		r.bot = s->center.y - 16;
		r.left = s->center.x;
		r.center = s->center.x;
		ggprint17(&r, 0, 0, text[i]);
	}

	// draw circle
	Shape *c;
	c = &game->circle;
	float x,y, radius;
	x = c->center.x;
	y = c->center.y;
	radius = c->radius;
	glPushMatrix();
	glColor3ub(0,50 + rand() + (120-50),255);
	int t = 25;
	float tpi = 2.0f * M_PI;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x,y);
	for(int i=0; i < t; i++){
		glVertex2f(x + (radius * cos(i + tpi/t)), y + (radius + sin(i * tpi/t)));
	}
	glEnd();
	glPopMatrix();

	//draw all particles here
	for(int i=0; i < game->n; i++) {
		glPushMatrix();
		glColor3ub(150,160,220);
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



