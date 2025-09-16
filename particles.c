#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <time.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600
#define PARTICLES_COUNT 1000
#define CAMERA_DIST 300.0
#define FOCAL_LENGTH 300.0
#define PARTICLE_SIZE_MIN 1
#define PARTICLE_SIZE_MAX 2

double camera_angle = 0.0;
double camera_yaw = 0.0;
double camera_pitch = 0.0;
double camera_distance = 600.0;
int mouse_last_x = 0, mouse_last_y = 0;
int mouse_dragging = 0;
double camera_pan_x = 0.0, camera_pan_y = 0.0, camera_pan_z = 0.0;

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
	double z;
	double vx;
	double vy;
	double vz;
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
    // Camera position in spherical coordinates
    double cx = WIDTH / 2.0 + camera_distance * cos(camera_pitch) * cos(camera_yaw);
    double cy = HEIGHT / 2.0 + camera_distance * cos(camera_pitch) * sin(camera_yaw);
    double cz = WIDTH / 2.0 + camera_distance * sin(camera_pitch);

    // Look-at target is the center
    double target_x = WIDTH / 2.0;
    double target_y = HEIGHT / 2.0;
    double target_z = WIDTH / 2.0;

    // Camera direction vector
    double dir_x = target_x - cx;
    double dir_y = target_y - cy;
    double dir_z = target_z - cz;
    double dir_len = sqrt(dir_x*dir_x + dir_y*dir_y + dir_z*dir_z);
    dir_x /= dir_len; dir_y /= dir_len; dir_z /= dir_len;

    // Up vector (approximate, always +z)
    double up_x = 0, up_y = 0, up_z = 1;

    // Right vector = dir x up
    double right_x = dir_y * up_z - dir_z * up_y;
    double right_y = dir_z * up_x - dir_x * up_z;
    double right_z = dir_x * up_y - dir_y * up_x;
    double right_len = sqrt(right_x*right_x + right_y*right_y + right_z*right_z);
    right_x /= right_len; right_y /= right_len; right_z /= right_len;

    // Recompute up to ensure orthogonality
    up_x = right_y * dir_z - right_z * dir_y;
    up_y = right_z * dir_x - right_x * dir_z;
    up_z = right_x * dir_y - right_y * dir_x;

    for (int colour = 0; colour < (COLOURS_COUNT); colour++)
    {
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
        for (int i = 0; i < (PARTICLES_COUNT); i++)
        {
            // Translate particle to camera space
            double px = particles[colour][i].x - cx;
            double py = particles[colour][i].y - cy;
            double pz = particles[colour][i].z - cz;

            // Rotate into camera coordinates
            double cxp = px * right_x + py * right_y + pz * right_z;
            double cyp = px * up_x + py * up_y + pz * up_z;
            double czp = px * dir_x + py * dir_y + pz * dir_z;

            // Only draw particles in front of the camera
            if (czp <= 1) continue;

            double sx = WIDTH / 2.0 + (cxp * FOCAL_LENGTH) / czp;
            double sy = HEIGHT / 2.0 - (cyp * FOCAL_LENGTH) / czp;

            // Size based on depth
            int size = PARTICLE_SIZE_MIN + (int)((PARTICLE_SIZE_MAX - PARTICLE_SIZE_MIN) * (1.0 - (czp / (WIDTH + camera_distance))));
            if (size < PARTICLE_SIZE_MIN) size = PARTICLE_SIZE_MIN;
            if (size > PARTICLE_SIZE_MAX) size = PARTICLE_SIZE_MAX;

            SDL_Rect rect = { (int)(sx - size/2), (int)(sy - size/2), size, size };
            SDL_RenderFillRect(s, &rect);
        }
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
            double fx = 0, fy = 0, fz = 0;
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
                        double dz = p1->z - p2->z;
                        if (dx != 0 || dy != 0 || dz != 0)
                        {
                            double d_squared = dx * dx + dy * dy + dz * dz;
                            if (d_squared < 6400)
                            {
                                double f = g / sqrt(d_squared);
                                fx += f * dx;
                                fy += f * dy;
                                fz += f * dz;
                            }
                        }
                    }
                }
            }
            p1->vx = (p1->vx + fx) * 0.15;
            p1->vy = (p1->vy + fy) * 0.15;
            p1->vz = (p1->vz + fz) * 0.15;
            p1->x += p1->vx;
            p1->y += p1->vy;
            p1->z += p1->vz;
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
            // For z, let's keep it in [0, WIDTH) for simplicity
            if (p1->z <= 0)
            {
                p1->vz *= -1;
                p1->z = 0;
            }
            if (p1->z >= WIDTH)
            {
                p1->vz *= -1;
                p1->z = WIDTH - 1;
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
            particles[colour][i].z = (rand() / ((double)RAND_MAX)) * (WIDTH - 100) + 50; // z in [50, WIDTH-50]
            particles[colour][i].vx = 0;
            particles[colour][i].vy = 0;
            particles[colour][i].vz = 0;
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
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window *win = SDL_CreateWindow("particles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer *s = SDL_CreateRenderer(win, -1, render_flags);
    create_particles(particles);

    int quit = 0;
    SDL_Event event;
    while (!quit)
    {
        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        double move_speed = 5.0 * (camera_distance / 600.0);
        double forward_x = cos(camera_pitch) * cos(camera_yaw);
        double forward_y = cos(camera_pitch) * sin(camera_yaw);
        double forward_z = sin(camera_pitch);
        double right_x = -sin(camera_yaw);
        double right_y = cos(camera_yaw);

        if (keystate[SDL_SCANCODE_S]) {
            camera_pan_x += forward_x * move_speed;
            camera_pan_y += forward_y * move_speed;
            camera_pan_z += forward_z * move_speed;
        }
        if (keystate[SDL_SCANCODE_W]) {
            camera_pan_x -= forward_x * move_speed;
            camera_pan_y -= forward_y * move_speed;
            camera_pan_z -= forward_z * move_speed;
        }
        if (keystate[SDL_SCANCODE_D]) {
            camera_pan_x += right_x * move_speed;
            camera_pan_y += right_y * move_speed;
        }
        if (keystate[SDL_SCANCODE_A]) {
            camera_pan_x -= right_x * move_speed;
            camera_pan_y -= right_y * move_speed;
        }

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
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouse_dragging = 1;
                    mouse_last_x = event.button.x;
                    mouse_last_y = event.button.y;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouse_dragging = 0;
                }
                break;
            case SDL_MOUSEMOTION:
                if (mouse_dragging) {
                    int dx = event.motion.x - mouse_last_x;
                    int dy = event.motion.y - mouse_last_y;
                    mouse_last_x = event.motion.x;
                    mouse_last_y = event.motion.y;
                    camera_yaw   -= dx * 0.005;
                    camera_pitch += dy * 0.005;
                    // Clamp pitch to avoid flipping
                    if (camera_pitch > M_PI/2 - 0.01) camera_pitch = M_PI/2 - 0.01;
                    if (camera_pitch < -M_PI/2 + 0.01) camera_pitch = -M_PI/2 + 0.01;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (event.wheel.y > 0) camera_distance *= 0.9;
                if (event.wheel.y < 0) camera_distance *= 1.1;
                if (camera_distance < 100.0) camera_distance = 100.0;
                if (camera_distance > 3000.0) camera_distance = 3000.0;
                break;
            }
        }

		update_particles(particles);
        clear(s, 0x2E, 0x34, 0x40);

        {
            // Camera position in spherical coordinates + pan
            double cx = WIDTH / 2.0 + camera_pan_x + camera_distance * cos(camera_pitch) * cos(camera_yaw);
            double cy = HEIGHT / 2.0 + camera_pan_y + camera_distance * cos(camera_pitch) * sin(camera_yaw);
            double cz = WIDTH / 2.0 + camera_pan_z + camera_distance * sin(camera_pitch);

            // Look-at target is the center + pan
            double target_x = WIDTH / 2.0 + camera_pan_x;
            double target_y = HEIGHT / 2.0 + camera_pan_y;
            double target_z = WIDTH / 2.0 + camera_pan_z;

            // Camera direction vector
            double dir_x = target_x - cx;
            double dir_y = target_y - cy;
            double dir_z = target_z - cz;
            double dir_len = sqrt(dir_x*dir_x + dir_y*dir_y + dir_z*dir_z);
            dir_x /= dir_len; dir_y /= dir_len; dir_z /= dir_len;

            // Up vector (approximate, always +z)
            double up_x = 0, up_y = 0, up_z = 1;

            // Right vector = dir x up
            double right_x = dir_y * up_z - dir_z * up_y;
            double right_y = dir_z * up_x - dir_x * up_z;
            double right_z = dir_x * up_y - dir_y * up_x;
            double right_len = sqrt(right_x*right_x + right_y*right_y + right_z*right_z);
            right_x /= right_len; right_y /= right_len; right_z /= right_len;

            // Recompute up to ensure orthogonality
            up_x = right_y * dir_z - right_z * dir_y;
            up_y = right_z * dir_x - right_x * dir_z;
            up_z = right_x * dir_y - right_y * dir_x;

            for (int colour = 0; colour < (COLOURS_COUNT); colour++)
            {
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
                for (int i = 0; i < (PARTICLES_COUNT); i++)
                {
                    // Translate particle to camera space
                    double px = particles[colour][i].x - cx;
                    double py = particles[colour][i].y - cy;
                    double pz = particles[colour][i].z - cz;

                    // Rotate into camera coordinates
                    double cxp = px * right_x + py * right_y + pz * right_z;
                    double cyp = px * up_x + py * up_y + pz * up_z;
                    double czp = px * dir_x + py * dir_y + pz * dir_z;

                    // Only draw particles in front of the camera
                    if (czp <= 1) continue;

                    double sx = WIDTH / 2.0 + (cxp * FOCAL_LENGTH) / czp;
                    double sy = HEIGHT / 2.0 - (cyp * FOCAL_LENGTH) / czp;

                    // Size based on depth
                    int size = PARTICLE_SIZE_MIN + (int)((PARTICLE_SIZE_MAX - PARTICLE_SIZE_MIN) * (1.0 - (czp / (WIDTH + camera_distance))));
                    if (size < PARTICLE_SIZE_MIN) size = PARTICLE_SIZE_MIN;
                    if (size > PARTICLE_SIZE_MAX) size = PARTICLE_SIZE_MAX;

                    SDL_Rect rect = { (int)(sx - size/2), (int)(sy - size/2), size, size };
                    SDL_RenderFillRect(s, &rect);
                }
            }
        }

        SDL_RenderPresent(s);
        SDL_Delay(1000 / 60);
    }
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(s);
    SDL_Quit();
}
