
/*
* This is an integer ascii Mandelbrot generator
*/
left_edge   = -420;
right_edge  =  300;
top_edge    =  300;
bottom_edge = -300;
x_step      =    7;
y_step      =   15;
/* how many iterations to do. higher values increase precision, but take longer, obviously. */
max_iter    =  500;
y0 = top_edge;
line = "";
while(y0 > bottom_edge)
{
    x0 = left_edge;
    while(x0 < right_edge)
    {
        y = 0;
        x = 0;
        /* 32 = ' ' */
        the_char = 32;
        i = 0;
        while (i < max_iter)
        {
            x_x = (x * x) / 200;
            y_y = (y * y) / 200;
            if (x_x + y_y > 800)
            {
                /* 48 = '0' */
                the_char = 48 + i;
                if(i > 9)
                {
                    /* 64 = '@' */
                    the_char = 64;
                }
                i = max_iter;
            }
            y = x * y / 100 + y0;
            x = x_x - y_y + x0;
            i = i + 1;
        }
        line += String.fromCharCode(the_char);
        x0 = x0 + x_step;
    }
    y0 = y0 - y_step;
    console.log(line);
    line = "";
}
