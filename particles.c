#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

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
    { -0.8750413758098341, -0.9867392657262921, 0.1871414571940626, -0.3716655435839973, },
    { 0.8310654013562320, -0.9446349935348308, -0.3503011480673688, 0.2138385424454876, },
    { -0.0192168583251614, -0.6107133736883819, 0.7758341160490336, -0.3296555799104532, },
    { -0.1908757738726566, -0.0931470017382628, 0.5028578664655134, 0.0838339408318671, },
};

struct Particle
{
	double x;
	double y;
	double vx;
	double vy;
};

struct Particle particles[COLOURS_COUNT][PARTICLES_COUNT];

void randomise_laws(void)
{
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

void draw_particles(SDL_Renderer *s, struct Particle particles[COLOURS_COUNT][PARTICLES_COUNT])
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
			SDL_SetRenderDrawColor(s, 0xFF, 0x00, 0x00, 0xFF);
			break;
		case Green:
			SDL_SetRenderDrawColor(s, 0x00, 0xFF, 0x00, 0xFF);
			break;
		case Blue:
			SDL_SetRenderDrawColor(s, 0x00, 0x00, 0xFF, 0xFF);
			break;
		case Yellow:
			SDL_SetRenderDrawColor(s, 0xFF, 0xFF, 0x00, 0xFF);
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

void update_particles(struct Particle particles[COLOURS_COUNT][PARTICLES_COUNT])
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
							if (d_squared < 2500)
							{									// square of the max distance force can act over
								double f = g / sqrt(d_squared); // force falls off ∝ 1/d
								// double f = g / d;  			 // force falls off ∝ 1/d^2
								fx += f * dx;
								fy += f * dy;
							}
						}
					}
				}
			}
			p1->vx = (p1->vx + fx) * 0.10;
			p1->vy = (p1->vy + fy) * 0.10;
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
				p1->x = WIDTH;
			}
			if (p1->y <= 0)
			{
				p1->vy *= -1;
				p1->y = 0;
			}
			if (p1->y >= HEIGHT)
			{
				p1->vy *= -1;
				p1->y = HEIGHT;
			}
		}
	}
}

void create_particles(struct Particle particles[COLOURS_COUNT][PARTICLES_COUNT])
{
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

void reset(struct Particle particles[COLOURS_COUNT][PARTICLES_COUNT])
{
	create_particles(particles);
	randomise_laws();
	print_laws();
}

int main()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("error initializing SDL: %s\n", SDL_GetError());
	}
	SDL_Window *win = SDL_CreateWindow("particles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
	Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	SDL_Renderer *s = SDL_CreateRenderer(win, -1, render_flags);
	srand(4);
	create_particles(particles);

	int quit = 0;
	SDL_Event event;
	while (!quit)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode)
				{
				case SDL_SCANCODE_Q:
				case SDL_SCANCODE_ESCAPE:
					quit = 1;
					break;
				case SDL_SCANCODE_R:
					reset(particles);
					break;
				default:
					break;
				}
			}
		}

		update_particles(particles);
		clear(s, 0x00, 0x00, 0x00);
		draw_particles(s, particles);

		SDL_RenderPresent(s);
		SDL_Delay(1000 / 60);
	}
	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(s);
	SDL_Quit();
}
