// Taken from https://en.wikipedia.org/wiki/Ordered_dithering 
// but adding one to both the numerator and denominator
const mat4 screen_door_pattern = mat4(
	1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
	13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
	4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
	16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
);

void screen_door_transparency(float transparency)
{
	if(transparency < 1.0 / 18.0)
	{
		return;
	}

	int x = int(mod(gl_FragCoord.x + drawable_id, 4.0));	
	int y = int(mod(gl_FragCoord.y + drawable_id, 4.0));

	if(screen_door_pattern[x][y] < transparency)
	{
		discard;
	}
}


