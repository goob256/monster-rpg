#include <allegro.h>
#if defined __linux__ || defined ALLEGRO_MACOSX
#include <sys/time.h>
#else
#include <winalleg.h>
#endif

long currentTimeMillis()
{
#if defined __linux__ || defined ALLEGRO_MACOSX
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#else
	return timeGetTime();
#endif
}
