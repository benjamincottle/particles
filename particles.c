#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_timer.h>
#include <GL/gl.h>

#define WIDTH 800
#define HEIGHT 600
#define PARTICLES_COUNT 500

enum Colours
{
	Red,
	Green,
	Blue,
	Yellow,
	COLOURS_COUNT,
};

// hand picked to show something interesting
// R {R,G,B,Y}
// G {R,G,B,Y}
// B {R,G,B,Y}
// Y {R,G,B,Y}
double LAWS[COLOURS_COUNT][COLOURS_COUNT] = {
    { -0.8434895723329343, -0.6863274619385262, 0.2477161014674771, 0.7682225135007046, },
    { -0.7806103861800443, 0.5859937917375908, -0.3331822023415855, 0.7840017666034409, },
    { -0.6603008646798789, 0.4897446723141450, 0.9346363153004256, -0.8722016726956711, },
    { 0.0048405868023822, -0.0538249151100521, -0.9224511049326747, -0.9847266003418372, },
};

typedef struct Particle
{
	double x;
	double y;
	double vx;
	double vy;
} Particle;

Particle particles[COLOURS_COUNT][PARTICLES_COUNT];

void randomise_laws(void)
{
	srand(time(NULL));
	for (int colour_p1 = Red; colour_p1 < COLOURS_COUNT; colour_p1++)
	{
		for (int colour_p2 = Red; colour_p2 < COLOURS_COUNT; colour_p2++)
		{
			LAWS[colour_p1][colour_p2] = (rand() / ((double)RAND_MAX)) * 2 - 1;
		}
	}
}

void print_laws(void)
{
	printf("double LAWS[COLOURS_COUNT][COLOURS_COUNT] = {\n");
	for (int colour_p1 = Red; colour_p1 < COLOURS_COUNT; colour_p1++)
	{
		printf("    { ");
		for (int colour_p2 = Red; colour_p2 < COLOURS_COUNT; colour_p2++)
		{
			printf("%0.16f, ", LAWS[colour_p1][colour_p2]);
		}
		printf("},\n");
	}
	printf("};\n");
}

void draw_particles(SDL_Renderer *s, Particle particles[COLOURS_COUNT][PARTICLES_COUNT])
{
	SDL_Point points[PARTICLES_COUNT];
	for (int colour = 0; colour < (COLOURS_COUNT); colour++)
	{
		for (int i = 0; i < (PARTICLES_COUNT); i++)
		{
			points[i].x = particles[colour][i].x;
			points[i].y = particles[colour][i].y;
		}
		switch (colour)
		{
		case Red:
			SDL_SetRenderDrawColor(s, 0xBF, 0x61, 0x6A, 0xFF);
			break;
		case Green:
			SDL_SetRenderDrawColor(s, 0xA3, 0xBE, 0x8C, 0xFF); 
			break;
		case Blue:
			SDL_SetRenderDrawColor(s, 0x5E, 0x81, 0xAC, 0xFF);
			break;
		case Yellow:
			SDL_SetRenderDrawColor(s, 0xEB, 0xCB, 0x8B, 0xFF);
			break;
		default:
			break;
		}
		SDL_RenderDrawPoints(s, points, PARTICLES_COUNT);
	}
}

void clear(SDL_Renderer *s, int r, int g, int b)
{
	SDL_SetRenderDrawColor(s, r, g, b, 0xFF);
	SDL_RenderClear(s);
}

void update_particles(Particle particles[COLOURS_COUNT][PARTICLES_COUNT])
{
	for (int colour_p1 = Red; colour_p1 < COLOURS_COUNT; colour_p1++)
	{
		for (int i = 0; i < PARTICLES_COUNT; i++)
		{
			double fx = 0;
			double fy = 0;
			struct Particle *p1 = &particles[colour_p1][i];
			for (int colour_p2 = Red; colour_p2 < COLOURS_COUNT; colour_p2++)
			{
				for (int j = 0; j < PARTICLES_COUNT; j++)
				{
					if (!(colour_p1 == colour_p2 && i == j))
					{
						struct Particle *p2 = &particles[colour_p2][j];
						double g = LAWS[colour_p1][colour_p2];
						double dx = p1->x - p2->x;
						double dy = p1->y - p2->y;
						if (dx != 0 || dy != 0)
						{
							double d_squared = dx * dx + dy * dy;
							if (d_squared < 6400)
							{								    // square of the max distance force can act over
								double f = g / sqrt(d_squared); // force falls off ∝ 1/d
								//double f = g / d_squared;     // force falls off ∝ 1/d^2
								fx += f * dx;
								fy += f * dy;
							}
						}
					}
				}
			}
			p1->vx = (p1->vx + fx) * 0.15;
			p1->vy = (p1->vy + fy) * 0.15;
			p1->x += p1->vx;
			p1->y += p1->vy;
			if (p1->x <= 0)
			{
				p1->vx *= -1;
				p1->x = 0;
			}
			if (p1->x >= WIDTH)
			{
				p1->vx *= -1;
				p1->x = WIDTH - 1;
			}
			if (p1->y <= 0)
			{
				p1->vy *= -1;
				p1->y = 0;
			}
			if (p1->y >= HEIGHT)
			{
				p1->vy *= -1;
				p1->y = HEIGHT - 1;
			}
		}
	}
}

void create_particles(Particle particles[COLOURS_COUNT][PARTICLES_COUNT])
{
	srand(4);
	for (int colour = Red; colour < COLOURS_COUNT; colour++)
	{
		for (int i = 0; i < PARTICLES_COUNT; i++)
		{
			particles[colour][i].x = (rand() / ((double)RAND_MAX)) * (WIDTH - 100) + 50;
			particles[colour][i].y = (rand() / ((double)RAND_MAX)) * (HEIGHT - 100) + 50;
			particles[colour][i].vx = 0;
			particles[colour][i].vy = 0;
		}
	}
}

void reset(Particle particles[COLOURS_COUNT][PARTICLES_COUNT])
{
	create_particles(particles);
	randomise_laws();
	print_laws();
}

int main()
{
	uint32_t WINDOWFLAGS = SDL_WINDOW_OPENGL;
	SDL_Window *Window = SDL_CreateWindow("particles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, WINDOWFLAGS);
	assert(Window);
	SDL_GLContext Context = SDL_GL_CreateContext(Window);
  	int32_t Running = 1;
  	int32_t FullScreen = 0;

	// create_particles(particles);

	while (Running)
	{	
		// Uint64 start = SDL_GetPerformanceCounter();
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_QUIT:
				Running = 0;
				break;
			case SDL_KEYDOWN:
				switch (Event.key.keysym.scancode)
				{
				case SDL_SCANCODE_Q:
				case SDL_SCANCODE_ESCAPE:
					Running = 0;
					break;

				case SDL_SCANCODE_F:
					FullScreen = !FullScreen;
					if (FullScreen)
					{
						SDL_SetWindowFullscreen(Window, WINDOWFLAGS | SDL_WINDOW_FULLSCREEN_DESKTOP);
					}
					else
					{
						SDL_SetWindowFullscreen(Window, WINDOWFLAGS);
					}
					break;
				case SDL_SCANCODE_R:
					reset(particles);
					break;
				default:
					break;
				}
			}
		}

		glViewport(0, 0, WIDTH, HEIGHT);
    	glClearColor(1.f, 0.f, 1.f, 0.f);
    	glClear(GL_COLOR_BUFFER_BIT);

    	SDL_GL_SwapWindow(Window);


		// update_particles(particles);
		// draw_particles(s, particles);
		
		// Uint64 end = SDL_GetPerformanceCounter();
		// float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency();
		// printf("Current FPS: %0.1f \n" , 1.0f / elapsed);
	}
	SDL_GL_DeleteContext(Context);
	SDL_DestroyWindow(Window);
	SDL_Quit();
	return 0;
}
